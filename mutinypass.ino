#include <ArduinoJson.h>
#include <AsyncFsWebServer.h>
#include <Base32-Decode.h>
#include <ezButton.h>
#include <FS.h>
#include <FFat.h>
#include "BleKeyboard.h"
#include "keymaps.h"
#include <map>
#include <mutex>
#include <TaskScheduler.h>
#include "TOTP.h"
#include <UUID.h>
#include <WiFi.h>

using namespace std;

std::mutex bleKeyboardMtx;
BleKeyboard bleKeyboard;
Scheduler runner;
UUID uuid;
JsonDocument cfg;

ezButton buttonArray[] = {
  ezButton(2),
  ezButton(3),
  ezButton(4),
  ezButton(5),
  ezButton(6),
  ezButton(7),
  ezButton(8),
  ezButton(9),
  ezButton(10),
  ezButton(11),
  ezButton(12) 
};

bool captiveRun = false;

// default configuration parameters
String layout = "IT";
bool lockEnabled = false;
uint16_t lockTime = 300;
String lockKey = "LEFT_SHIFT";
uint8_t keyDelay = 30;
String ntpServer = "pool.ntp.org";
String tzName = "Europe/Rome";
String tzConfig = "CET-1CEST,M3.5.0,M10.5.0/3";
bool factory = false;
String configFile = "/config/mutinypass.json";

// key map for anti desktop lock
std::map<String, uint8_t> keyMap = {
    { "ESC",  			KEY_ESC },
    { "LEFT_SHIFT", 	KEY_LEFT_SHIFT },
    { "LEFT_CTRL",  	KEY_LEFT_CTRL },
    { "LEFT_ALT",  		KEY_LEFT_ALT },
    { "RIGHT_SHIFT",  	KEY_RIGHT_SHIFT },
    { "RIGHT_CTRL",  	KEY_RIGHT_CTRL },
    { "RIGHT_ALT",  	KEY_RIGHT_ALT }
};

// keyboard layout map
std::map<String, const uint8_t * PROGMEM> keyboardLayout = {
	{ "DK", KeyboardLayout_da_DK },
	{ "DE", KeyboardLayout_de_DE },
	{ "US", KeyboardLayout_en_US },
	{ "ES", KeyboardLayout_es_ES },
	{ "FR", KeyboardLayout_fr_FR },
	{ "HU", KeyboardLayout_hu_HU },
	{ "IT", KeyboardLayout_it_IT },
	{ "BR", KeyboardLayout_pt_BR },
	{ "PT", KeyboardLayout_pt_PT },
	{ "SE", KeyboardLayout_sv_SE },
};

// TOTP algorithm map
std::map<String, mbedtls_md_type_t> shaMap = {
	{"SHA1", MBEDTLS_MD_SHA1},
	{"SHA256", MBEDTLS_MD_SHA256},
	{"SHA512", MBEDTLS_MD_SHA512}
};

// physical buttons map
std::map<uint8_t, uint8_t> buttonMap = {
	{0, 2},
	{1, 3},
	{2, 4},
	{3, 5},
	{4, 6},
	{5, 7},
	{6, 8},
	{7, 9},
	{8, 10},
	{9, 11},
	{10, 12}
};

// starts web server
AsyncFsWebServer server(80, FFat, "mutinypass");

// send configured key press to avoid desktop lock
void handleNolock() {
	lock_guard<mutex> lck(bleKeyboardMtx);
	uint8_t key = keyMap[lockKey];
  	if(bleKeyboard.isConnected() && lockEnabled == true) {
		bleKeyboard.press(key);
		bleKeyboard.release(key);
		// visual feedback
		digitalWrite(LED_RED, LOW);
		delay(250);
		digitalWrite(LED_RED, HIGH);
  	}
}

// starts noLock task
Task noLock(lockTime * 1000, TASK_FOREVER, &handleNolock);

// get TOTP
char *getOTP(const String &uuid) {
	JsonArray otps = cfg["otp"];
	static char code[12] = {0};
	time_t now;
	for (size_t i = 0; i < otps.size(); i++) {
  		JsonObject otp = otps[i];
 		if (uuid == otp["uuid"].as<String>()) {
 			mbedtls_md_type_t algorithm = shaMap[otp["algorithm"].as<String>()];
			uint8_t buff[64];
 			uint8_t len = base32decode((const char *)otp["secret"], buff, sizeof(buff));
		  	TOTP totp = TOTP(buff, len, otp["timestep"].as<int>(), otp["digits"].as<int>(), algorithm);
		  	time(&now);
		  	strcpy(code,totp.getCode(now));
			return code;
 		}
	}
	return code;
}

// parse macro sequences and send key press or strings
void sendSequence(const String &seq, const String &username, const String &password, char* otp) {
	lock_guard<mutex> lck(bleKeyboardMtx);
  for (int i = 0; i < seq.length(); i++) {
    if (seq[i] == '%') {
      if (i + 1 < seq.length()) {
        char code = seq[i + 1];
        i++; // skip after %
        switch (code) {
          case 'u':
            bleKeyboard.print(username); // type username
            break;
          case 'p':
            bleKeyboard.print(password); // type password
            break;
          case 'o':
            bleKeyboard.print(otp); // type otp
            break;
          case 't':
            // TAB
            bleKeyboard.press(KEY_TAB);
            bleKeyboard.releaseAll();
            break;
          case 'r':
            // RETURN
            bleKeyboard.press(KEY_RETURN);
            bleKeyboard.releaseAll();
            break;
          case 'c':
            // CTRL+ALT+DEL
            bleKeyboard.press(KEY_LEFT_CTRL);
            bleKeyboard.press(KEY_LEFT_ALT);
            bleKeyboard.press(KEY_DELETE);
            bleKeyboard.releaseAll();
            break;
          default:
            // unknown macro
            break;
        }
      }
    } else {
      // print "as is" every other characters
      bleKeyboard.write(seq[i]);
    }
  }
}

// save configuration
void saveConfig() {
    File file = FFat.open(configFile, FILE_WRITE);
	cfg["layout"] = layout;
	cfg["keydelay"] = keyDelay;
	cfg["lock"] = lockEnabled;
	cfg["locktime"] = lockTime;
	cfg["lockkey"] = lockKey;
	cfg["ntpserver"] = ntpServer;
	cfg["tzname"] = tzName;
	cfg["tzconfig"] = tzConfig;
	serializeJson(cfg, file);
	file.close();
}

// send key press sequence (identified by pin)
void handleButton(int pin) {
	JsonArray seqs = cfg["accounts"];
	for (size_t i = 0; i < seqs.size(); i++) {
		JsonObject seq = seqs[i];
 		if (pin == seq["pin"] && bleKeyboard.isConnected()) {
			// visual feedback
			digitalWrite(LED_RED, LOW);
			sendSequence(seq["sequence"], seq["username"], seq["password"], getOTP(seq["otp"]));
			digitalWrite(LED_RED, HIGH);
 		}
	}
}

// send key press sequence (identified by uuid)
void handleSend(AsyncWebServerRequest *request) {
	String uuid;
  	if(request->hasParam("uuid")) {
  		uuid = request->arg("uuid");
		JsonArray seqs = cfg["accounts"];
		for (size_t i = 0; i < seqs.size(); i++) {
  			JsonObject seq = seqs[i];
  			if (uuid == seq["uuid"]) {
			  	if(bleKeyboard.isConnected()) {
					// visual feedback
					digitalWrite(LED_RED, LOW);
					sendSequence(seq["sequence"], seq["username"], seq["password"], getOTP(seq["otp"]));
					digitalWrite(LED_RED, HIGH);
			  	}
  			};
		}
  	}
  	request->send(200, "text/plain", uuid);
}

// read accounts form and populate accounts array
void handleSave1(AsyncWebServerRequest *request) {
   	size_t count = request->params();
    for (size_t i = 0; i < count; i++) {
    	const AsyncWebParameter *p = request->getParam(i);
    	int index = i / 7;
    	if (p->name() == "uuid") {
    		uuid.generate();
  			cfg["accounts"][index]["uuid"] = (p->value() == "") ? String(uuid.toCharArray()) : p->value();
    	}
    	if (p->name() == "name") {
	 		cfg["accounts"][index]["name"] = p->value();
    	}
    	if (p->name() == "sequence") {
			cfg["accounts"][index]["sequence"] = p->value();
    	}
    	if (p->name() == "username") {
			cfg["accounts"][index]["username"] = p->value();
    	}
    	if (p->name() == "password") {
			cfg["accounts"][index]["password"] = p->value();
    	}
      	if (p->name() == "otp") {
        	cfg["accounts"][index]["otp"] = p->value();
      	}
      	if (p->name() == "pin") {
        	cfg["accounts"][index]["pin"] = (p->value() != "") ? p->value().toInt() : 0;
      	}
    }
	// delete empty accounts to avoid ghost records
	JsonArray accounts = cfg["accounts"];
	for (size_t i = 0; i < accounts.size(); i++) {
  		JsonObject account = accounts[i];
  		if (account["name"] == "" || account["sequence"] == "") {
  			accounts.remove(i);
  		}
	}
	// save configuration
	saveConfig();
	// redirect to main page
  	request->redirect("/");
}

// read OTPs form and populate otp array
void handleSave2(AsyncWebServerRequest *request) {
    size_t count = request->params();
    for (size_t i = 0; i < count; i++) {
      const AsyncWebParameter *p = request->getParam(i);
      int index = i / 6;
      if (p->name() == "uuid") {
      	uuid.generate();
        cfg["otp"][index]["uuid"] = (p->value() == "") ? String(uuid.toCharArray()) : p->value();
      }
      if (p->name() == "name") {
        cfg["otp"][index]["name"] = p->value();
      }
      if (p->name() == "secret") {
        cfg["otp"][index]["secret"] = p->value();
      }
      if (p->name() == "algorithm") {
        cfg["otp"][index]["algorithm"] = (p->value() != "") ? p->value() : "SHA1";
      }
      if (p->name() == "digits") {
        cfg["otp"][index]["digits"] = (p->value() != "") ? p->value().toInt() : 6;
      }
      if (p->name() == "timestep") {
        cfg["otp"][index]["timestep"] = (p->value() != "") ? p->value().toInt() : 30;
      }
    }
  // delete empty OTPs to avoid ghost records
  JsonArray otps = cfg["otp"];
  for (size_t i = 0; i < otps.size(); i++) {
      JsonObject otp = otps[i];
      if (otp["name"] == "" || otp["secret"] == "") {
        otps.remove(i);
      }
  }
  // save configuration
  saveConfig();
  // redirect to main page
    request->redirect("/");
}

// read configuration form inputs and set parameters
void handleSave3(AsyncWebServerRequest *request) {
   	size_t count = request->params();
    for (size_t i = 0; i < count; i++) {
    	const AsyncWebParameter *p = request->getParam(i);
    	if (p->name() == "layout") {
    		layout = p->value();
    	}
    	if (p->name() == "keydelay") {
    		keyDelay = p->value().toInt();
    	}
    	if (p->name() == "ntpserver") {
    		ntpServer = p->value();
    	}
    	if (p->name() == "tzname") {
    		tzName = p->value();
    	}
    	if (p->name() == "tzconfig") {
    		tzConfig = p->value();
    	}
    	if (p->name() == "lockkey") {
    		lockKey = p->value();
    	}
    	if (p->name() == "locktime") {
    		lockTime = p->value().toInt();
    	}
    	if (p->name() == "lock") {
    		lockEnabled = (p->value() == "on") ? true : false;
    	}
    	if (p->name() == "factory") {
    		if (p->value() == "on") {
    			factory = true;
          		// redirect to reload page
          		request->redirect("/reload.html");
          		return;
    		}
    	}
    }
    // update noLock interval
	noLock.setInterval(lockTime * 1000);
	// update delay between keystrokes and keyboard layout
	bleKeyboard.setLayout(keyboardLayout[layout]);
	bleKeyboard.setDelay(keyDelay);
	// save configuration
	saveConfig();
	// redirect to main page
  	request->redirect("/");
}

void setup() {
	// put your setup code here, to run once:
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	pinMode(LED_BUILTIN, OUTPUT);

	// stats filesystem
	if (!FFat.begin()) {
      	FFat.format();
      	ESP.restart();
  	}

	// read configuration (or save defaults if file not exists)
	if (FFat.exists(configFile)) {
 		File file = FFat.open(configFile, FILE_READ);
		deserializeJson(cfg, file);
		file.close();
		layout = cfg["layout"].as<String>();
		keyDelay = cfg["keydelay"].as<int>();
		lockEnabled = cfg["lock"].as<bool>();
		lockTime = cfg["locktime"].as<int>();
		lockKey = cfg["lockkey"].as<String>();
		ntpServer = cfg["ntpserver"].as<String>();
		tzName = cfg["tzname"].as<String>();
		tzConfig = cfg["tzconfig"].as<String>();
	} else {
		saveConfig();
	}

	// Ready LED (BUILTIN)
	digitalWrite(LED_BUILTIN, HIGH);
	// setup physical buttons
  	for (byte i = 0; i < (sizeof(buttonArray) / sizeof(ezButton)); i++) {
    	buttonArray[i].setDebounceTime(100);
  	}
 	// starts bluetooth keyboard
	bleKeyboard.begin();
	bleKeyboard.setDelay(keyDelay);
	bleKeyboard.setLayout(keyboardLayout[layout]);
	// starts noLock task 
	runner.addTask(noLock);
	noLock.setInterval(lockTime * 1000);
  	noLock.enable();

    // try to connect WiFi or run in AP mode with captive portal
    if (!server.startWiFi(10000)) {
        server.startCaptivePortal("MutinyPass", "12345678", "/setup");
	    captiveRun = true;
    }

    // enable ACE FS file web editor (temporarily, for troubleshooting purposes only)
    server.enableFsCodeEditor();

	// web server configuration
    server.setSetupPageTitle("Mutiny password manager");
    server.on("/send", HTTP_GET, handleSend);
    server.on("/save1", HTTP_POST, handleSave1);
    server.on("/save2", HTTP_POST, handleSave2);
    server.on("/save3", HTTP_POST, handleSave3);
	server.rewrite("/", "/index.html");
	server.init();

	// ntp and time zone configuration
	configTime(0, 0, ntpServer.c_str());
	setenv("TZ", tzConfig.c_str(), 1);
	tzset();
}

void loop() {
	// put your main code here, to run repeatedly:
	// delete configuration and restart to reload factory defaults
	if (factory == true) {
		FFat.remove(configFile);
    	ESP.restart();
	}
	// task scheduler execute
 	runner.execute();
 	// dns for captive prortal for WiFi configuration
	if (captiveRun)
  		server.updateDNS();
    // visual feedback for bluetooth connection
  	if(bleKeyboard.isConnected()) {
		digitalWrite(LED_BLUE, LOW);
  	} else {
  		digitalWrite(LED_BLUE, HIGH);
	}
  	for (byte i = 0; i < (sizeof(buttonArray) / sizeof(ezButton)); i++) {
    	buttonArray[i].loop();
    	if (buttonArray[i].isPressed()) {
    		handleButton(buttonMap[i]);
    	}
  	}
  	delay(1);
}

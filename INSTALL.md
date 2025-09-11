## Hardware howto

First of all, you need an Arduino Nano ESP32, you can buy in electronics parts shops or order on Arduino website or local electronics e-shop (or Amazon, if not possibile otherwise).

You need an USB C cable too and a PC or a Mac, so connect your Nano ESP32 to the PC.

If you want to use physical buttons, you need momentary buttons of any kind. You can use up to 11 buttons. You need to be hands-on with soldering iron and electronics; buttons must be connected between D2-D12 pins and GND.

## Software howto

You need Arduino IDE (on what OS you want). With the [Board Manager](https://support.arduino.cc/hc/en-us/articles/360016119519-Add-boards-to-Arduino-IDE) you need to install support for [Aduino Nano ESP32 board](https://docs.arduino.cc/tutorials/nano-esp32/getting-started-nano-esp32/).

Then, you need to install these libraries with the [Library Manager](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries/):
 * [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
 * [AsyncFsWebServer](https://github.com/cotestatnt/async-esp-fs-webserver)
 * [TaskScheduler](https://github.com/arkhipenko/TaskScheduler)
 * [Arduino-UUID](https://github.com/RobTillaart/UUID)
 * [ezButton](https://github.com/ArduinoGetStarted/button)
 * [Base32-Decode](https://github.com/dirkx/Arduino-Base32-Decode)

TOTP-Arduino and ESP32-NimBLE-Keyboard are not needed because are already integrated in the project (for now, temporarily).

So you can open, compile and load the project *mutinypass*. Once loaded and if it's working correctly, the orange LED will light.

## Configuration

First of all, you need to configure the WiFi. MutinyPass when unconfigured will as an AP with SSID *MutinyPass* and password *12345678*. You can connect with a client, there is a captive portal that bring you on http://192.168.4.1/setup (or you can open manually this page on the browser).
BEFORE configuring WiFi, the first think you *must* do is to load the content of the FFat partition. Go on *Update & FS* tab, then *Select the folder containing your files*, choose *data* directory and load it.
AFTER this step, you can configure WiFi. Go on *WiFi Setup*, click on *Scan WiFi networks*, choose your SSID and password, then *Connect to SSID*. From this moment, you have to open MutinyPass with the IP address of your WiFi network. You can open the main page, eg. http://192.168.1.10/

On the main page, you have to set an NTP server (or leave as is if MutinyPass can reach Internet), the time zone, a keyboard layout (*mandatory*). If you want, you also can send periodically a key press of a choosen key to avoid desktop lock for inactivity timeout. 
You can also reset all configurations (NOT the WiFi configuration, for now), the Arduino ESP32 will be restarted with the defaults.

Lately, you need to configure accounts and key sequences. Name and sequence are mandatory (or the configuration will not be saved), you can specify an username, a password, the association with an OTP, an hardware pin for a button.

Sequences are macros like these:
 * %c CTRL+ALT+DEL
 * %u username
 * %p password
 * %o OTP
 * %t TAB
 * %r RETURN

Likewise, you can configure one or more OTP generator. Name and secret (BASE32 encoded) are mandatory (or the configuration will not be saved). You can specify an algorithm (default SHA1), the number of digits (default 6) and a time step (default 30 seconds).

## Using MutinyPass

The most importan think is to associate the emulated BLE keyboard with your PC. The "keyboard" is visibile as *ESP32 keyboard* and you che freely associate it. BEWARE: you can associate only one PC (or tablet or phone) a time.

You can send key stroke sequence using the integrated webUI (discouraged cause it's sooo slow) or hardware buttons.

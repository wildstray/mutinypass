# Too much of a good thing? Mutiny!

## History behind...
Just suppose somebody works for a big company with strict cybersecurity requirements. With BYOD ban, with Zero Trust, with least priviledge level, with mandatory password complexity, with 5 minutes lockscreen, with nothing but passwords (no PIN, no biometrics, etc.), with copy and past disabled for login screen. Now imagine the humiliation of a computer engineer who, despite s/he fully understand all of these obligations, s/he have to turns into a password typing trained monkey...

<img alt="password typing monkey" src="https://github.com/wildstray/mutinypass/blob/main/_f68cc45e-4784-4f77-9aa9-384a35653ba7.jpeg" width="250" />

## What is it?
This is WIP piece of code and a sort of "password manager" one-of-a-kind: it's based on ESP32 and emulates a physical keyboard (BLE HID) and can "type" a password or an username and password. So you will no longer be forced to type passwords all-day. 
Moreover, it will prevent lockscreen timeout in an unique way: with periodic keypress (in the least invasive way possible). Anyway this isn't and won't be a secure password manager like Keepass: this is complementary to Keepass.

<img alt="Arduino Nano ESP32" src="https://docs.arduino.cc/static/553215686e39f3c2ffb9cc71809e0eff/image.svg" width="250" />

## What about company policy and legal issues?
Enough is too much. Mutiny! Anyway, it's really important to stress that this is a trick and this is totally legal (and, formally, it doen't violate any corporate policy at all): actually this is a "keyboard" and passwords and OTPs are actually "typed" as the business require to do...

<img alt="Jolly Roger" src="https://upload.wikimedia.org/wikipedia/commons/thumb/4/47/Pirate_Flag_of_Jack_Rackham.svg/250px-Pirate_Flag_of_Jack_Rackham.svg.png" width="250" />

## Throw down the gauntlet
This project offers a greater comfort and ergonimics to computer engineers... hoping that the message will reach big company managers and security officers... but beware: don't tickle the dragon's tails! If also bluetooth and USB keyboards will be banned, the game will raise further...  and it will encompass smartcards, FIDO2, biometric authentication and so on...

<img alt="Homer Simpson bird keyboard" src="https://i.giphy.com/HQGzdiNhg52oM.webp" width="250" />

## Tech details, spin-offs, limitations and future improvements
Library used:
 * [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
 * [AsyncFsWebServer](https://github.com/cotestatnt/async-esp-fs-webserver)
 * [ESP32-NimBLE-Keyboard](https://github.com/wakwak-koba/ESP32-NimBLE-Keyboard)
 * [TaskScheduler](https://github.com/arkhipenko/TaskScheduler)
 * [TOTP-Arduino](https://github.com/lucadentella/TOTP-Arduino)
 * [Arduino-UUID](https://github.com/RobTillaart/UUID)
 * [ezButton](https://github.com/ArduinoGetStarted/button)
 * [Base32-Decode](https://github.com/dirkx/Arduino-Base32-Decode)

Actually this software *IS NOT SECURE* cause AsyncFsWebServer does not support HTTPS and configuration file is a plain json file on the FFat partition, including plain text passwords. I'm considering to migrate from AsyncFsWebServer to PsychicHttp to add support for SSL. 
OTPs are currently supported. I modified and tested TOTP-Arduino library to supports 6-8-10 digits code and SHA256, SHA512, using [Mbed-TLS](https://github.com/Mbed-TLS/mbedtls) library.

I wrote a really simple index.html page for key press actions and for key press sequences and accounts configuration and other parameters. This page is not integrated with AsyncFsWebServer setup (and probably will not be). So you have to use /setup to configure WiFi connection or to update firmware (or to upload data into FFat partition) and index.html for MutinyPass operations. It's supported (and preferred to use!) hardware buttons thru digital input pins (D2-D12).

About the BLE keyboard emulation, there would be a separate discussion to make and it could also have tech spin-offs. All available libraries supports only US keyboard layout and 7 bit ASCII. I had to modify the ESP32-NimBLE-Keyboard. For now, are supported BR,DE,DK,ES,FR,IT,HU,US,PT,SE layouts and only 7 bit ASCII. To made extended ASCII usable, String (UTF-8) must be converted in ISO 8859-1/Latin1 and new complete (256 bytes) layouts are mandatory.

Note: there is an *important hardware limitation* because there is [only one radio for WiFi and BLE on ESP32](https://docs.espressif.com/projects/esp-idf/en/v5.1.1/esp32/api-guides/coexist.html), so once bluetooth is connected, WiFi will became slow, very slow and laggy but this is normal and cannot be optimized further (this is mainly the reason to use hardware keys instead of the webUI).

About the webUI: I'm NOT an UI/UX developer :-) So settle like this or write something better and PR me :-)

There is space for further improvements and Arduino Nano ESP32 is used only for ~35% of program space and ~20% of RAM. Suggestions and PR are very welcome! About Arduino-TOTP and ESP32-NimBLE-Keyboard, I'll have to fork and submit PRs, but it's not the focus of this project.

## License and release notes
This is a PoC and its use will be everyone's responsibility; the author releses this code "as-is" and assumes no responsibility at all! This project is released under GPLv3.

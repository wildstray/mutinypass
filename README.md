# Too much of a good thing? Mutiny!

## History behind...
Just suppose somebody works for a big company with strict cybersecurity requirements. With BYOD ban, with Zero Trust, with least priviledge level, with mandatory password complexity, with 5 minutes lockscreen, with nothing but passwords (no PIN, no biometrics, etc.), with copy and past disabled for login screen. Now imagine the humiliation of a computer engineer who, despite s/he fully understand all of these obligations, s/he turns into a password typing trained monkey...

<img alt="password typing monkey" src="https://github.com/wildstray/mutinypass/blob/main/_f68cc45e-4784-4f77-9aa9-384a35653ba7.jpeg" width="250" />

## What is it?
This is WIP piece of code and a sort of "password manager" one-of-a-kind: it's based on ESP32 and emulates a physical keyboard (BLE HID) and can "type" a password or an username and password. So you will no longer be forced to type passwords all-day. 
Moreover, it will prevent lockscreen timeout in an unique way: with periodic keypress (in the least invasive way possible).

<img alt="Arduino Nano ESP32" src="https://docs.arduino.cc/static/553215686e39f3c2ffb9cc71809e0eff/image.svg" width="250" />

## What about security?
What about security, company policy and so on? Enough is too much. Mutiny!

<img alt="Jolly Roger" src="https://upload.wikimedia.org/wikipedia/commons/thumb/4/47/Pirate_Flag_of_Jack_Rackham.svg/250px-Pirate_Flag_of_Jack_Rackham.svg.png" width="250" />

## Is this a real and secure password manager?
This is a PoC and its use will be everyone's responsibility; the author releses this code "as-is" and assumes no responsibility at all! This isn't and won't be a secure password manager like Keepass, this is complementary to Keepass.

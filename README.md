# Too much of a good thing? Mutiny!

## History behind...
Just suppose you work for a big company, with strict cybersecurity requirements. With BYOD ban, with Zero Trust, with 5 minutes lockscreen, with nothing more then passwords (no PIN, no biometrics, etc.), with mandatory password strengh.
Now imagine the humiliation of a computer engineer who, despite s/he fully understand all of these obligations, s/he turns into a password typing trained monkey...

## What is it?
This is WIP "password manager" one-of-a-kind: it's based on ESP32 and emulates a physical keyboard (BLE HID) and can "type" a password or an username and password. So you will no longer be forced to type passwords all-day. 
Moreover, it will prevent lockscreen timeout in an unique way: with periodic keypress (in the least invasive way possible).

## What about security?
Enough is too much. Mutiny! This is a PoC and its use will be everyone's responsibility. The author releses this code as-is and assumes no responsibility.

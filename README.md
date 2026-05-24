# WireTrip-HomeAlarm
A Wire Trip home alarm System made with two ESP32s, one being the emitter and the other one being the receiver

The Emitter Module:
  - An ESP32
  - An RFID RC522 I2C sensor
  - A Buzzer/Horn
  - Laser

The Receiver Module:
  - An ESP32
  - An LDR sensor
  - A Buzzer/Horn

The communication between the ESPs are made through ESP Now with each ESP MAC Address.

Each ESP32 is connected to its own pinboard. And are supposed to be battery powered.

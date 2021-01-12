# esp-homekit-analog-occupancy-sensor
This is an ESP8266 native Homekit firmware for reading the ADC pin and exposing to Homekit. 
The ADC pin (A0 on an ESP8266) returns a 10-bit (1024) value which varies based on capacitance across A0 and VCC. This firmware exposes the ADC reading as a custom homekit characteristic, as well as a threshold slider, to let you set the occupancy threshold as required.

Intended to be used with pressure sensitive strips as described here https://medium.com/@qz_li/smart-bed-7de9ad55276e. This could be placed under a bed mattress, couch, welcome mat, office chair mat etc.


Based off the wonderful https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266 library.
Implements WifiManager and ArduinoOTA.

# Issues
There are 2 main issues with this currently.

The threshold value is not saved across a reboot. 

Homekit name is hard-coded, which conflicts if you have more than one on the same network. Workaround - change the name for each board. Fix - implement dynamic name based off ESP.getChipId(). If anyone can help with that, dope.


# dev branch
The dev branch has some of my ideas on how to fix these, including an attempt at implementing EEPROM to save threshold value, and attempts at getting Homekit to use the same dynamic name as WifiManager and ArduinoOTA. This currently doesn't compile.

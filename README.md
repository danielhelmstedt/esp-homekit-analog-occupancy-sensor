# esp-homekit-analog-occupancy-sensor
This is an ESP8266 native Homekit firmware for reading the ADC pin and exposing to Homekit. 
The ADC pin (A0 on an ESP8266) returns a 10-bit (1024) value which varies based on capacitance across A0 and VCC. This firmware exposes the ADC reading as a custom homekit characteristic, as well as a threshold slider, to let you set the occupancy threshold as required.

This firmware includes WifiManager and ArduinoOTA for easy setup and updating.

Intended to be used with pressure sensitive strips as described here https://medium.com/@qz_li/smart-bed-7de9ad55276e. This could be placed under a bed mattress, couch, welcome mat, office chair mat etc.


Based off the wonderful https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266 library.

# Issues
While WifiManager, ArduinoOTA and Wifi.Hostname all use a generated hostname via ESP.getChipId, I haven't yet been able to get Homekit to accept the string value. So, the homekit name and serial number are static. This will conflict if you have 2 boards/devices with the same name.



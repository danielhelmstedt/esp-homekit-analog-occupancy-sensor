//  esp-homekit-analog-occupancy-sensor.ino
//  ESP8266-based, native HomeKit pressure sensing device via ADC reading and capaticive/pressure sensitive pads
//  Daniel Helmstedt 2022
//  Based on the work by Mixiaoxiao (Wang Bin) - https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266 

#include <ArduinoOTA.h>
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <ESP_EEPROM.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_occupancy;
extern "C" homekit_characteristic_t cha_sensorValue;
extern "C" homekit_characteristic_t cha_threshold;
char serialNumber[14];
static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;
struct {int eepromThreshold;} data; uint addr = 0; //eeprom storage config
#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__); //logging config

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);    //Initialize built-in LED
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED off (Active Low)
  
  Serial.begin(115200);
  Serial.println();
  
  //Generate hostname from MAC
  sprintf(serialNumber, "ESP-ADC-%X\0", ESP.getChipId());
  
  WiFiManager wifiManager;
  wifiManager.autoConnect(serialNumber);
  WiFi.hostname(serialNumber);
  ArduinoOTA.setHostname(serialNumber);
  ArduinoOTA.setPassword("12041997");
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");
  
  arduino_homekit_setup(&config);

  //EEPROM Setup
  EEPROM.begin(512);
  EEPROM.get(addr, data);
  cha_threshold.value.int_value = data.eepromThreshold;
  homekit_characteristic_notify(&cha_threshold, cha_threshold.value);
  Serial.print("Got saved threshold = ");Serial.println(data.eepromThreshold);
}

void loop() {
  ArduinoOTA.handle();
  arduino_homekit_loop();
  const uint32_t t = millis();
  if (t > next_report_millis) {
    // report sensor values every 1 seconds
    next_report_millis = t + 1 * 1000;
    eeprom_update();  //update eeprom if threshold has changed
    homekit_report(); //call the report function
  }
  if (t > next_heap_millis) {
    // show heap info every 15 seconds
    next_heap_millis = t + 15 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
  }
  delay(10);
}

void homekit_report() {
  int numReadings = 3;
  int total = 0;
  int average = 0;
  int reading = 0;  
  for (int i = 0; i <= numReadings; i++) {
    reading = analogRead(A0); // read from the sensor:
    total = total + reading; // add the reading to the total:
  }
  average = total / numReadings;  // calculate the average:
  
  cha_sensorValue.value.int_value = average;
  cha_occupancy.value.bool_value = average <= data.eepromThreshold ? 0 : 1;
  homekit_characteristic_notify(&cha_sensorValue, cha_sensorValue.value);
  homekit_characteristic_notify(&cha_occupancy, cha_occupancy.value);

  Serial.print(" reading = ");
  Serial.print(average);
  Serial.print(" -- threshold = ");
  Serial.println(data.eepromThreshold);
  if (average >= data.eepromThreshold){Serial.println("ON");};
}


void eeprom_update() {
  if(data.eepromThreshold != cha_threshold.value.int_value){
    data.eepromThreshold = cha_threshold.value.int_value;
    EEPROM.put(addr, data.eepromThreshold);EEPROM.commit();
    Serial.print("Threshold updated - ");Serial.println(data.eepromThreshold); 
  }
}

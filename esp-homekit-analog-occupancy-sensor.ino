//esp-homekit-analog-occupancy-sensor.ino
//ESP8266-based, native HomeKit pressure sensing device via ADC reading and capaticive/pressure sensitive pads
//TODO: Dynamic Homekit Name based off ESP.getChipID()

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
extern "C" char serial[14];

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;

//Data to save to EEPROM
struct { 
  int eepromThreshold;
} data;
uint addr = 0;

void setup() {
  Serial.begin(115200);
  Serial.println( );
  
  //Create dynamic hostname
  sprintf(serial, "ESP-ADC-%X\0", ESP.getChipId());

  //Read EEPROM
  EEPROM.begin(512);  //Initialize EEPROM
  EEPROM.get(addr, data);
  Serial.print("Reading saved EEPROM Threshold - ");
  Serial.println(data.eepromThreshold);
  
  pinMode(LED_BUILTIN, OUTPUT);    //Initialize built-in LED
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED off (Active Low)
  
  WiFiManager wifiManager;
  wifiManager.autoConnect(serial);
  WiFi.hostname(serial);

  ArduinoOTA.setHostname(serial);
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
  
  homekit_setup(); 
}

void loop() {
  ArduinoOTA.handle();
  homekit_loop();
  delay(10);
}

void homekit_setup() {
  arduino_homekit_setup(&config);
}

void homekit_loop() {
  arduino_homekit_loop();
  const uint32_t t = millis();
  if (t > next_report_millis) {
    // report sensor values every 1 seconds
    next_report_millis = t + 1 * 1000;
    homekit_report();
  }
  if (t > next_heap_millis) {
    // show heap info every 15 seconds
    next_heap_millis = t + 15 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
  }
}


/////////////////////////////////////////////////////////////////////
////////Function for reading sensor and reporting to HomeKit.////////
/////////////////////////////////////////////////////////////////////

void homekit_report() {
  int numReadings = 10;
  int total = 0;
  int average = 0;
  int reading = 0;  

  // Take 10 readings and average them
  for (int i = 0; i <= numReadings; i++) {
    reading = analogRead(A0);     // read from the sensor:
    total = total + reading; }    // add the reading to the total:
  average = total / numReadings;  // calculate the average:
  
  //Initialize HomeKit value from EEPROM on boot
  if(cha_threshold.value.int_value == 0 && data.eepromThreshold != 0) { 
    cha_threshold.value.int_value = data.eepromThreshold;
    homekit_characteristic_notify(&cha_threshold, cha_threshold.value);
    Serial.println("Updated HomeKit with saved value");
  }
  
  //Update HomeKit
  cha_sensorValue.value.int_value = average;
  cha_occupancy.value.bool_value = average <= cha_threshold.value.int_value ? 0 : 1; //Determine occupancy status
  homekit_characteristic_notify(&cha_occupancy, cha_occupancy.value);
  homekit_characteristic_notify(&cha_sensorValue, cha_sensorValue.value);
  Serial.print("threshold = ");
  Serial.println(cha_threshold.value.int_value);
  Serial.print("sensor average = ");
  Serial.println(average);
  Serial.print("occupancy = ");
  Serial.println(cha_occupancy.value.bool_value);  

  //Update EEPROM
  if(cha_threshold.value.int_value != data.eepromThreshold){
    data.eepromThreshold = cha_threshold.value.int_value; //sync values
    EEPROM.put(addr, data.eepromThreshold);               //prepare changes, if any
    EEPROM.commit();                                      //Perform write to flash
    Serial.print("Threshold changed - updated EEPROM to ");
    Serial.println(data.eepromThreshold); 
  }
}

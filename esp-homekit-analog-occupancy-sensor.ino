///// CHANGELOG
//0.2 added WifiManager Library
//0.3 added ArduinoOTA
//0.4 added blink when identifying during Homekit setup
//0.5 added sensorValue RO char and threshold RW char.
//0.6 Average/Filter ADC readings
//0.7 Dynamic Serial/Hostname/AP based off ESP.getChipID()

//TODO: Dynamic Homekit Name based off ESP.getChipID()
//TODO: Save threshold to EEPROM.

#include <ArduinoOTA.h>
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_occupancy;
extern "C" homekit_characteristic_t cha_sensorValue;
extern "C" homekit_characteristic_t cha_threshold;

void setup() {
  Serial.begin(115200);
  char out[20];
  sprintf(out, "PressureSensor-%X",ESP.getChipId());
  const char * serial_str = out;
  Serial.println(serial_str);
  pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED off.
  
  WiFiManager wifiManager;
  wifiManager.autoConnect(serial_str);
  WiFi.hostname(serial_str);

  ArduinoOTA.setHostname(serial_str);
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


//==============================
// Homekit setup and loop
//==============================
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
    // show heap info every 5 seconds
    next_heap_millis = t + 5 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
  }
}

//Function for reading sensor and reporting to Homekit
void homekit_report() {
  int numReadings = 10;
  int total = 0;
  int average = 0;
  int reading = 0;  
  // Take 10 readings and average them
  for (int i = 0; i <= numReadings; i++) {
    reading = analogRead(A0); // read from the sensor:
    total = total + reading; // add the reading to the total:
  }
  average = total / numReadings;  // calculate the average:
  cha_sensorValue.value.int_value = average;
  Serial.println("sensor average = " + average);
  
  cha_occupancy.value.bool_value = average <= cha_threshold.value.int_value ? 0 : 1; //Logic using Threshold
  homekit_characteristic_notify(&cha_occupancy, cha_occupancy.value);
  homekit_characteristic_notify(&cha_sensorValue, cha_sensorValue.value);
  LOG_D("occupancy %u", cha_occupancy.value.bool_value);  
}

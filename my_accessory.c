/*
   my_accessory.c
   Define the accessory in C language using the Macro in characteristics.h

  Based on the work by Mixiaoxiao (Wang Bin)
*/

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <Arduino.h>

//This is what's run when you press identify during Homekit setup. For some reason, not when pressing identify once paired.
void my_accessory_identify(homekit_value_t _value) {
  printf("accessory identify\n");
  for (int i = 0; i <= 5; i++) { //start at 0, run loop and add 1, repeat until 5
    digitalWrite(LED_BUILTIN, LOW);// turn the LED on.(Note that LOW = LED on; this is because it is active low on the ESP8266.
    delay(100);            // wait for 0.1 second.
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED off.
    delay(100); // wait for 0.1 second.
  }
}


// format: uint8; 0 ”Occupancy is not detected”, 1 ”Occupancy is detected”
homekit_characteristic_t cha_occupancy = HOMEKIT_CHARACTERISTIC_(OCCUPANCY_DETECTED, 0);


//Sensor Value. format: int32; 1 to 1024
homekit_characteristic_t cha_sensorValue = HOMEKIT_CHARACTERISTIC_(CUSTOM,
    .description = "Sensor",
    .type = "ECC64460-AEA2-409B-BF2A-32270BB11954",
    .format = homekit_format_int,
    .permissions = homekit_permissions_paired_read
                 | homekit_permissions_notify,
    .min_value = (float[]) {0},
    .max_value = (float[]) {1024},
    .min_step =  (float[]) {1},
);

//Sensor Threshold. format: int32; 1 to 1024
homekit_characteristic_t cha_threshold = HOMEKIT_CHARACTERISTIC_(CUSTOM,
    .description = "Threshold",
    .type = "151B3360-16F8-4C57-8CBA-F68B526C527A",
    .format = homekit_format_int,
    .permissions = homekit_permissions_paired_read
                 | homekit_permissions_paired_write
                 | homekit_permissions_notify,
    .min_value = (float[]) {0},
    .max_value = (float[]) {1024},
    .min_step =  (float[]) {1},
);

homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_sensor, .services = (homekit_service_t*[]) {
    HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "ESP ADC Sensor"),
      HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Daniel Helmstedt"),
      HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266"),
      HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "1234567"),
      HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL
    }),
    HOMEKIT_SERVICE(OCCUPANCY_SENSOR, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "Pressure Sensor"),
      &cha_occupancy,
      &cha_sensorValue,
      &cha_threshold, 
      NULL
    }),
    NULL
  }),
  NULL
};

homekit_server_config_t config = {
  .accessories = accessories,
  .password = "120-41-997",
  .setupId = "1QJ8",
};

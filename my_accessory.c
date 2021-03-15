//   my_accessory.c
//   Define the accessory in C language using the Macro in characteristics.h
//   Based on the work by Mixiaoxiao (Wang Bin) - https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <Arduino.h>

char serial[14] = "XXXXXX\0";


//Identify accessory by flashing LED 5x in 1 second
void my_accessory_identify(homekit_value_t _value) {
  printf("Identify Accessory\n");
  for (int i = 0; i <= 5; i++) {      //start at 0, run loop and add 1, repeat until 5
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (In-built LED is active LOW)
    delay(100);                       // wait for 0.1 second.
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off.
    delay(100);                       // wait for 0.1 second.
  }
}

//Sensor Value. format: int32; 1 to 1024
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
    .min_step =  (float[]) {1}
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
//    .value = HOMEKIT_INT_(50),
);

homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_sensor, .services = (homekit_service_t*[]) {
    HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, serial),
      HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Dan Helmstedt"),
      HOMEKIT_CHARACTERISTIC(MODEL, "ESP ADC Sensor"),
      HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, serial),
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

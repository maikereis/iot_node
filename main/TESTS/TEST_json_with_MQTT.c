#include <sdkconfig.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "ntp.h"
#include "deep_sleep.h"
#include "mqtt.h"
#include "spiffs_persistence.h"
#include "dht_sensor.h"
#include "adc_sampler.h"
#include "string.h"
#include "cJSON.h"

static const char *TAG = "TEST_json_with_MQTT";

void test_json_with_mqtt();

void app_main(void)
{
    test_json_with_mqtt();
}
void test_json_with_mqtt()
{
    int array_num = 3;
    cJSON *objects[3];
    for (int i = 0; i < array_num; i++)
    {
        objects[i] = cJSON_CreateObject();
    }

    cJSON_AddStringToObject(objects[0], "ID", "MK00");
    cJSON_AddNumberToObject(objects[0], "T", 20.5);
    cJSON_AddNumberToObject(objects[0], "H", 60.0);

    cJSON_AddStringToObject(objects[1], "ID", "MK01");
    cJSON_AddNumberToObject(objects[1], "T", 30.0);
    cJSON_AddNumberToObject(objects[1], "H", 70.0);

    cJSON_AddStringToObject(objects[2], "ID", "MK02");
    cJSON_AddNumberToObject(objects[2], "T", 40.0);
    cJSON_AddNumberToObject(objects[2], "H", 80.0);

    cJSON *root;
    root = cJSON_CreateArray();

    ESP_LOGI(TAG, "Array size %i", cJSON_GetArraySize(root));

    cJSON_AddItemToArray(root, objects[0]);
    ESP_LOGI(TAG, "Array size %i", cJSON_GetArraySize(root));

    cJSON_AddItemToArray(root, objects[1]);
    ESP_LOGI(TAG, "Array size %i", cJSON_GetArraySize(root));

    cJSON_AddItemToArray(root, objects[2]);
    ESP_LOGI(TAG, "Array size %i", cJSON_GetArraySize(root));

    const char *my_json_string = cJSON_PrintUnformatted(root);
    ESP_LOGI(TAG, "my_json_string\n%s, len: %i", my_json_string, strlen(my_json_string));

    wifi_start();
    if (wifi_state() == WIFI_CONNECTED)
    {
        ESP_LOGI("MAIN", "CONNECTED!");

        mqtt_start();

        mqtt_publish("/test", my_json_string, 0, 1, 0);

        mqtt_stop();
    }
    wifi_stop();
}
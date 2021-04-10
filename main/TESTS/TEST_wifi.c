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
#include "adc_sampler.h"

void test_wifi();
void test_reconnect();

void app_main()
{
    test_reconnect();
}

void test_wifi()
{
    wifi_start();
    if (wifi_state() == WIFI_CONNECTED)
        ESP_LOGI("MAIN", "CONNECTED!");
    wifi_stop();
}

void test_reconnect(){
    wifi_start();
    if (wifi_state() == WIFI_CONNECTED)
        ESP_LOGI("MAIN", "CONNECTED!");
    wifi_stop();

    vTaskDelay(3000/portTICK_PERIOD_MS);

    wifi_start();
    if (wifi_state() == WIFI_CONNECTED)
        ESP_LOGI("MAIN", "CONNECTED!");
    wifi_stop();
}

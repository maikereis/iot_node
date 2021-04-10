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

void test_ntp();

void app_main()
{
    test_ntp();
}

void test_ntp()
{
    wifi_start();
    if (wifi_state() == WIFI_CONNECTED)
        time_sync();
    wifi_stop();
}
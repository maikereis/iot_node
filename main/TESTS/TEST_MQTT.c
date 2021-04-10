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
#include "string.h"
#include "cJSON.h"

//static const char *TAG = "MQTT";

void test_json_with_mqtt();

void app_main(void)
{

    const char my_string[300] = "[{\"ID\":\"DL_MR\",\"S\":\"R82CH0\",\"Vrms\":13.33055933726963,\"Irms\":29.888893463240667,\"Pwr\":235.00148814309031,\"PF\":0.5898103686821381,\"D\":1612639406}]";
    bool client_err = false, pedding_msg = false;

    wifi_start();
    if (wifi_state() == WIFI_CONNECTED)
    {
        ESP_LOGI("MAIN", "CONNECTED!");

        mqtt_start();

        for (int i = 0; (i < 5) && (client_err == false); i++)
        {
            mqtt_publish("/test", my_string, 0, 1, 0, &client_err);
        }

        mqtt_stop(&pedding_msg);
        ESP_LOGI("MAIN", "STOPED!");
        if(pedding_msg){
            ESP_LOGI("MAIN", "PEDDING MESSAGES");
        }
    }
    wifi_stop();
}
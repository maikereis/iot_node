/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdkconfig.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "dht11.h"
static const char *TAG = "TEST_dht11";

void app_main(void)
{
    set_dht_gpio(15);
    double temp = 1, hum = 1;
    while (1)
    {
        read_dht(&temp, &hum);
        ESP_LOGI(TAG, "T: %f, H: %f", temp, hum);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

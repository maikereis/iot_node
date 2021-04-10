#include <sdkconfig.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"

#include "adc_sampler.h"

static const char *TAG = "TEST_current";

void app_main()
{

    while (1)
    {

        double irms = 0, idc = 0;
        adc_sample(CONFIG_CURRENT_CHANNEL, 0, 1, 549.0, &irms, &idc);

        ESP_LOGI(TAG, "Irms: %f, Idc:%f", irms, idc);

#ifdef CONFIG_EX_CURRENT_SENSOR
        double irms2 = 0, idc2 = 0;
        adc_sample(CONFIG_EX_CURRENT_CHANNEL, 0, 1, 560.0, &irms2, &idc2);
#endif
        ESP_LOGI(TAG, "EX Irms: %f, Idc:%f", irms2, idc2);

        /*
        double irms = 0, vrms=0, idc = 0, vdc = 0;
        adc_sampleVI(5, 0, 1, 540.0, &vrms, &vdc, 4, 0, 1, 540.0, &irms, &idc);
        */

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
#include <sdkconfig.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"

#include "adc_sampler.h"

static const char *TAG = "TEST_pwr";

void app_main()
{

    while (1)
    {
        //ESP_LOGI(TAG, "%02x:%02x:%02x:%02x:%02x:%02x\n",chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);

        double vrms = 0, vdc = 0, irms = 0, idc = 0, real_pwr = 0, pf = 0;

        adc_sampleVI(CONFIG_VOLTAGE_CHANNEL, CONFIG_VOLTAGE_ATTEN, CONFIG_VOLTAGE_ADC_UNIT, CONFIG_VOLTAGE_OFFSET, &vrms, &vdc,
                     CONFIG_CURRENT_CHANNEL, CONFIG_CURRENT_ATTEN, CONFIG_CURRENT_ADC_UNIT, CONFIG_CURRENT_OFFSET, &irms, &idc,
                     &real_pwr, &pf);

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
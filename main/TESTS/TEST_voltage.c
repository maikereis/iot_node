#include <sdkconfig.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"

#include "adc_sampler.h"

static const char *TAG = "TEST_voltage";

void app_main()
{
    power_on_sensor(26);
    while (1)
    {
        double vrms = 0, vdc = 0;
        adc_sample(CONFIG_VOLTAGE_CHANNEL, CONFIG_VOLTAGE_ATTEN, CONFIG_VOLTAGE_ADC_UNIT, 553, &vrms, &vdc);
        ESP_LOGI(TAG, "Vrms: %f, Vdc: %f", vrms, vdc);
    
        /*
        double irms = 0, idc = 0;
        adc_sample(CONFIG_CURRENT_CHANNEL, CONFIG_CURRENT_ATTEN, CONFIG_CURRENT_ADC_UNIT, 535, &irms, &idc);
        ESP_LOGI(TAG, "Irms: %f, Idc: %f", irms, idc);
        */

        /*
        double irms2 = 0, idc2 = 0;
        adc_sample(CONFIG_EX_CURRENT_CHANNEL, CONFIG_EX_CURRENT_ATTEN, CONFIG_EX_CURRENT_ADC_UNIT, 538, &irms2, &idc2);
        ESP_LOGI(TAG, "EX Irms: %f, Idc: %f", irms2, idc2);
        */
        vTaskDelay(200/ portTICK_PERIOD_MS);
    
    }
}
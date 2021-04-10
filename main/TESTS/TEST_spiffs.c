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
#include "cJSON.h"



void test_spiffs_read();

void test_spiffs_write();

void app_main()
{
    //test_spiffs_write();
    test_spiffs_read();
}

void test_spiffs_write()
{
    spiffs_init();
    spiffs_write_file();

}
void test_spiffs_read(){
    spiffs_init();
    //BUFFER AQUI
    char BUFFER[1500];
    spiffs_read_file(BUFFER, 1500);
}

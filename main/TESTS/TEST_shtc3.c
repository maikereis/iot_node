#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "shtc3.h"

static const char *TAG = "TEST_shtc3";

void app_main(void)
{
    init_sensor(I2C_MODE_MASTER, 21, 22, I2C_MASTER_FREQ_100KHZ);
    float temperature = 0, humidity = 0;
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (1)
    {
        wakeup_sensor(SENSOR_ADDR);
        read_out(SENSOR_ADDR, T_FIRST_N, &temperature, &humidity);
        sleep_sensor(SENSOR_ADDR);
        ESP_LOGI(TAG, "Temperature: %f, Humidade: %f", temperature, humidity);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

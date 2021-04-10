#include "deep_sleep.h"

static RTC_DATA_ATTR struct timeval sleep_enter_time;
static const char *TAG = "SLEEP";

void wakeup_cause()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    switch (esp_sleep_get_wakeup_cause())
    {

    case ESP_SLEEP_WAKEUP_TIMER:
        ESP_LOGI(TAG, "Wake up from timer. Time spent in deep sleep: %dms", sleep_time_ms);
        break;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
        ESP_LOGI(TAG, "Not a deep sleep reset");
        break;
    default:
        break;
    }
}

void go_sleep(uint16_t seconds)
{
    const uint16_t wakeup_time_sec = seconds;
    ESP_LOGI(TAG, "Set wakeup timer to %i seconds", wakeup_time_sec);
    esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);

    // Isolate GPIO12 pin from external circuits. This is needed for modules
    // which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
    // to minimize current consumption.
    rtc_gpio_isolate(GPIO_NUM_12);

    gettimeofday(&sleep_enter_time, NULL);
    esp_deep_sleep_start();
}

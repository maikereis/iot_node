#include "ntp.h"

static const char *TAG = "NTP";

#define SNTP_SERVER CONFIG_SNTP_SERVER
#define SNTP_GMT CONFIG_SNTP_TIME_ZONE
#define SNTP_SYNC_RETRY CONFIG_SNTP_MAX_SYNC_RETRY

static void time_sync_init(void);
static void time_sync_obtain(void);
static void time_sync_notification_cb(struct timeval *tv);

static void time_sync_init(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, SNTP_SERVER);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void time_sync()
{

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    time_sync_obtain();
    time(&now);

    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_IMMED)
    {
        struct timeval outdelta;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS)
        {
            adjtime(NULL, &outdelta);
            ESP_LOGI(TAG, "Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us",
                     outdelta.tv_sec,
                     outdelta.tv_usec / 1000,
                     outdelta.tv_usec % 1000);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

static void time_sync_obtain(void)
{
    time_sync_init();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < SNTP_SYNC_RETRY)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, SNTP_SYNC_RETRY);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

time_t time_get_timestamp(uint8_t verbose)
{
    time_t now = time(NULL);
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    time(&now);

    char strftime_buf[64];
    // Set timezone to Brasil Time
    setenv("TZ", "GMT+3", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%X %d/%m/%Y", &timeinfo);
    if(verbose)
        ESP_LOGI(TAG, "Brasilia/BR, %s", strftime_buf);
    return now;
}
bool time_is_outdated(){
    time_t current_timestamp, past_timestamp = 1546347600;
    current_timestamp = time_get_timestamp(0);
    if(current_timestamp < past_timestamp)
        return true;
    return false;
}

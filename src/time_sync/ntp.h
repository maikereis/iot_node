#ifndef __NTP_H__
#define __NTP_H__

#include <sdkconfig.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_attr.h>
#include <nvs_flash.h>
#include <esp_sntp.h>
#include <wifi.h>

void time_sync();
time_t time_get_timestamp(uint8_t verbose);
bool time_is_outdated();
#endif
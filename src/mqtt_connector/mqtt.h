#ifndef __MQTT_H__
#define __MQTT_H__


#include "sdkconfig.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "spiffs_persistence.h"



#define FILE_PATH CONFIG_SPIFFS_FILE_PATH
#define FILE_SIZE CONFIG_SPIFFS_FILE_SIZE

void mqtt_start(void);
void mqtt_publish(const char *topic, const char *msg, const int len, const int qos, const int size, bool *client_err);

void mqtt_stop(bool *pedding_msg);


#endif
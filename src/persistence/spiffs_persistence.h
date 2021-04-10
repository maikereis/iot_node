#ifndef __SPIFFS_PERSISTENCE_H__
#define __SPIFFS_PERSISTENCE_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <esp_system.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "cJSON.h"


void spiffs_init();
void spiffs_write_file(const char *PATH, cJSON *object);
bool spiffs_read_file(const char *PATH, char **lines, long *position);
void spiffs_delete_file(const char *PATH);
void spiffs_deinit();
void spiffs_get_file_size(const char *PATH, long *size);



#endif
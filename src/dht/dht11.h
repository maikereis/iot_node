#ifndef __DHT11_H__
#define __DHT11_H__

#include "sdkconfig.h"

#include "inttypes.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <time.h>
#include "utility.h"


void set_dht_gpio(gpio_num_t pin);
void read_dht(double *temp, double *hum);



#endif
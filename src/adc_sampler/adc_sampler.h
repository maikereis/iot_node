#ifndef __ADC_SAMPLER_H__
#define __ADC_SAMPLER_H__

#include "sdkconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "math.h"
#include <driver/dac.h>
#include "esp_log.h"
#include <soc/rtc_wdt.h>
#include "esp_task_wdt.h"
#include "driver/gpio.h"

#define DEFAULT_VREF 1100

void adc_sample(uint8_t adc_channel, uint8_t adc_atten, uint8_t adc_unit, double offsetmv, double *irms, double *idc);
void adc_sampleVI(uint8_t adc_channelV, uint8_t adc_attenV, uint8_t adc_unitV, double offsetmv_V, double *vrms, double *vdc,
                  uint8_t adc_channelI, uint8_t adc_attenI, uint8_t adc_unitI, double offsetmv_I, double *irms, double *idc, double *real_pwr, double *pf);
void adc_measure(uint8_t adc_channel, uint8_t adc_atten, uint8_t adc_unit, double *value, int n_samples);
void power_on_sensor(gpio_num_t gpio);

#endif
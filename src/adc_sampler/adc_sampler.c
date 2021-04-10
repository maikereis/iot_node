#include "adc_sampler.h"

static const char *TAG = "ADC_SAMPLER";

// cos(φ) = Pmed / Vrms * Irms
void adc_sample(uint8_t adc_channel, uint8_t adc_atten, uint8_t adc_unit, double offsetmv, double *irms, double *idc)
{

    esp_adc_cal_characteristics_t *adc_chars;
    adc_channel_t channel = adc_channel; //GPIO34 if ADC1, GPIO14 if ADC2
    adc_atten_t atten = adc_atten;
    adc_unit_t unit = adc_unit;

    fflush(stdout);

    //Configure ADC
    if (unit == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    _Bool lastcross = false, checkcross = false;

    uint32_t start_sample = 0, sample = 0, sample_counter = 0;

    uint16_t crossings = 0;
    uint16_t timeout = 1000;

    uint32_t start_time = 0, total_time = 0;

    *irms = 0;
    *idc = 0;
    double offset = offsetmv;
    double filtered_sample = 0;

    total_time = esp_log_timestamp();

    start_time = esp_log_timestamp();

    while (1)
    {
        esp_adc_cal_get_voltage(channel, adc_chars, &start_sample);
        if ((start_sample > (DEFAULT_VREF * 0.45)) && (start_sample < (DEFAULT_VREF * 0.55)))
            break;
        if ((esp_log_timestamp() - start_time) > timeout)
            break;
    }

    start_time = esp_log_timestamp();

    while ((crossings < 50) && ((esp_log_timestamp() - start_time) < timeout))
    {

        sample_counter++;
        esp_adc_cal_get_voltage(channel, adc_chars, &sample); //Read in voltage signal in mV

        offset = offset + ((sample - offset) / 2048);
        filtered_sample = sample - offset;
        //Check if has a 0 crossing

        *irms += filtered_sample * filtered_sample;
        *idc += filtered_sample;

        //detect zero crossing
        lastcross = checkcross;

        if (sample > start_sample)
            checkcross = true;
        else
            checkcross = false;

        if (sample_counter == 1)
            lastcross = checkcross;

        if (lastcross != checkcross)
            crossings++;
    }

    *idc = *idc / sample_counter;
    *irms = sqrt(*irms / sample_counter);

    //ESP_LOGI(TAG, "Vref: %d, Crossings: %i, Time: %d", adc_chars->vref, crossings, esp_log_timestamp() - total_time);
}

void adc_sampleVI(uint8_t adc_channelV, uint8_t adc_attenV, uint8_t adc_unitV, double offsetmv_V, double *vrms, double *vdc,
                  uint8_t adc_channelI, uint8_t adc_attenI, uint8_t adc_unitI, double offsetmv_I, double *irms, double *idc, double *real_pwr, double *pf)
{

    esp_adc_cal_characteristics_t *adc_charsV;
    adc_channel_t channelV = adc_channelV; //GPIO34 if ADC1, GPIO14 if ADC2
    adc_atten_t attenV = adc_attenV;
    adc_unit_t unitV = adc_unitV;

    //Configure ADC
    if (unitV == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channelV, attenV);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)channelV, attenV);
    }
    //Characterize ADC
    adc_charsV = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unitV, attenV, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_charsV);

    esp_adc_cal_characteristics_t *adc_charsI;
    adc_channel_t channelI = adc_channelI; //GPIO34 if ADC1, GPIO14 if ADC2
    adc_atten_t attenI = adc_attenI;
    adc_unit_t unitI = adc_unitI;
    //Configure ADC
    //Configure ADC
    if (unitI == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channelI, attenI);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)channelI, attenI);
    }
    //Characterize ADC
    adc_charsI = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unitI, attenI, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_charsI);

    _Bool lastcross = false, checkcross = false;

    uint32_t start_sampleV = 0, sampleV = 0, sampleI = 0, sample_counter = 0;

    uint16_t crossings = 0;
    uint16_t timeout = 1000;

    uint32_t start_time = 0, total_time = 0;

    *vrms = 0;
    *vdc = 0;

    *irms = 0;
    *idc = 0;

    double offsetV = offsetmv_V;
    double offsetI = offsetmv_I;

    double filtered_sampleV = 0;
    double filtered_sampleI = 0;

    double apparent_pwr = 0;

    total_time = esp_log_timestamp();

    start_time = esp_log_timestamp();

    while (1)
    {
        esp_adc_cal_get_voltage(channelV, adc_charsV, &start_sampleV);
        if ((start_sampleV > (DEFAULT_VREF * 0.45)) && (start_sampleV < (DEFAULT_VREF * 0.55)))
            break;
        if ((esp_log_timestamp() - start_time) > timeout)
            break;
    }

    start_time = esp_log_timestamp();

    while ((crossings < 50) && ((esp_log_timestamp() - start_time) < timeout))
    {

        sample_counter++;

        esp_adc_cal_get_voltage(channelV, adc_charsV, &sampleV); //Read in voltage signal in mV
        esp_adc_cal_get_voltage(channelI, adc_charsI, &sampleI); //Read in voltage signal in mV

        offsetV = offsetV + ((sampleV - offsetV) / 2048);
        filtered_sampleV = sampleV - offsetV;

        offsetI = offsetI + ((sampleI - offsetI) / 2048);
        filtered_sampleI = sampleI - offsetI;

        *vrms += filtered_sampleV * filtered_sampleV;
        *vdc += filtered_sampleV;

        *irms += filtered_sampleI * filtered_sampleI;
        *idc += filtered_sampleI;

        *real_pwr += filtered_sampleV * filtered_sampleI;

        //detect zero crossing
        lastcross = checkcross;

        if (sampleV > start_sampleV)
            checkcross = true;
        else
            checkcross = false;

        if (sample_counter == 1)
            lastcross = checkcross;

        if (lastcross != checkcross)
            crossings++;
    }

    *vdc = *vdc / sample_counter;
    *vrms = sqrt(*vrms / sample_counter);

    *idc = *idc / sample_counter;
    *irms = sqrt(*irms / sample_counter); //(k / 1000)

    *real_pwr = (*real_pwr / sample_counter); //(k / 1000)

    apparent_pwr = (*vrms) * (*irms);

    *pf = *real_pwr / apparent_pwr;

    ESP_LOGI(TAG, "Crossings: %i, Total_Time: %d, Vrms(V): %f, Irms(mV): %f, real_pwr(W): %f, pf: %f", crossings, esp_log_timestamp() - total_time, *vrms, *irms, *real_pwr, *pf);
}
void adc_measure(uint8_t adc_channel, uint8_t adc_atten, uint8_t adc_unit, double *value, int n_samples)
{

    uint32_t sample = 0;
    esp_adc_cal_characteristics_t *adc_chars;
    adc_channel_t channel = adc_channel; //GPIO34 if ADC1, GPIO14 if ADC2
    adc_atten_t atten = adc_atten;
    adc_unit_t unit = adc_unit;

    if (unit == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    for (uint8_t i = 0; i < 5; i++)
    {
        esp_adc_cal_get_voltage(channel, adc_chars, &sample);
        *value += sample;
    }
    *value = *value / 5;

    ESP_LOGI(TAG, "Value: %f", *value);
}

void power_on_sensor(gpio_num_t gpio)
{
    gpio_pad_select_gpio(gpio);
    gpio_set_pull_mode(gpio, GPIO_PULLDOWN_ONLY);
    gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio, 1);
}

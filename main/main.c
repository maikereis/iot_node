#include <sdkconfig.h>
#include "esp_system.h"

#include "wifi.h"
#include "ntp.h"
#include "deep_sleep.h"
#include "mqtt.h"
#include "spiffs_persistence.h"
#include "dht11.h"
#include "adc_sampler.h"
#include "cJSON.h"
#include "shtc3.h"

QueueHandle_t queue_1;

#define CYCLES CONFIG_ESP_MEASUREMENT_CYCLE
#define SLEEP_TIME CONFIG_ESP_SLEEP_TIME
#define CHIP_ID CONFIG_CLIENT_ID

nvs_handle_t my_handle;
int32_t boot_counter;
bool pedding_msg = true;
static const char *TAG = "MAIN";

static void persistence_sequency();
void update_boot_counter();
void read_boot_counter();

void read_boot_counter()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        boot_counter = 0; // value will default to 0, if not set yet in NVS
        err = nvs_get_i32(my_handle, "boot_counter", &boot_counter);
        switch (err)
        {
        case ESP_OK:
            ESP_LOGI(TAG, "Boot counter is %d", boot_counter);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG, "The value is not initialized yet!");
            break;
        default:
            ESP_LOGI(TAG, "Error (%s) reading!", esp_err_to_name(err));
        }
    }
}
void update_boot_counter()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    else
    {
        ++boot_counter;
        boot_counter %= CYCLES;
        err = nvs_set_i32(my_handle, "boot_counter", boot_counter);

        if (err != ESP_OK)
            ESP_LOGI(TAG, "Failed!");

        err = nvs_commit(my_handle);

        if (err != ESP_OK)
            ESP_LOGI(TAG, "Failed!");

        nvs_close(my_handle);
    }
}

void app_main()
{
    uint32_t start_time = 0;

#ifdef CONFIG_VOLTAGE_SENSOR
    power_on_sensor(26);
#endif

    start_time = esp_log_timestamp();
    spiffs_init();


#if defined(CONFIG_VOLTAGE_SENSOR) & defined(CONFIG_CURRENT_SENSOR)
    double vrms = 0, vdc = 0, irms = 0, idc = 0, real_pwr = 0, pf = 0;

    adc_sampleVI(CONFIG_VOLTAGE_CHANNEL, CONFIG_VOLTAGE_ATTEN, CONFIG_VOLTAGE_ADC_UNIT, CONFIG_VOLTAGE_OFFSET, &vrms, &vdc,
                 CONFIG_CURRENT_CHANNEL, CONFIG_CURRENT_ATTEN, CONFIG_CURRENT_ADC_UNIT, CONFIG_CURRENT_OFFSET, &irms, &idc,
                 &real_pwr, &pf);

#elif CONFIG_CURRENT_SENSOR
    double irms = 0, idc = 0;

    adc_sample(CONFIG_CURRENT_CHANNEL, CONFIG_CURRENT_ATTEN, CONFIG_CURRENT_ADC_UNIT, CONFIG_CURRENT_OFFSET, &irms, &idc);

#elif CONFIG_VOLTAGE_SENSOR
    double vrms = 0, vdc = 0;

    adc_sample(CONFIG_VOLTAGE_PIN, CONFIG_VOLTAGE_ATTEN, CONFIG_VOLTAGE_ADC_UNIT, CONFIG_VOLTAGE_OFFSET, &vrms, &vdc);

#endif

vTaskDelay(10/portTICK_PERIOD_MS);

#if defined(CONFIG_VOLTAGE_SENSOR) & defined(CONFIG_EX_CURRENT_SENSOR)
    double vrms2 = 0, vdc2 = 0, irms2 = 0, idc2 = 0, real_pwr2 = 0, pf2 = 0;

    adc_sampleVI(CONFIG_VOLTAGE_CHANNEL, CONFIG_VOLTAGE_ATTEN, CONFIG_VOLTAGE_ADC_UNIT, CONFIG_VOLTAGE_OFFSET, &vrms2, &vdc2,
                 CONFIG_EX_CURRENT_CHANNEL, CONFIG_EX_CURRENT_ATTEN, CONFIG_EX_CURRENT_ADC_UNIT, CONFIG_EX_CURRENT_OFFSET,
                 &irms2, &idc2,
                 &real_pwr2, &pf2);

#elif CONFIG_EX_CURRENT_SENSOR
    double irms2 = 0, idc2 = 0;

    adc_sample(CONFIG_EX_CURRENT_CHANNEL, CONFIG_EX_CURRENT_ATTEN, CONFIG_EX_CURRENT_ADC_UNIT, CONFIG_EX_CURRENT_OFFSET, &irms2, &idc2);
#endif

#ifdef CONFIG_SHTC3_SENSOR

    float temp = 0, hum = 0;

    init_sensor(I2C_MODE_MASTER, 21, 22, I2C_MASTER_FREQ_100KHZ);
    wakeup_sensor(SENSOR_ADDR);
    read_out(SENSOR_ADDR, T_FIRST_N, &temp, &hum);
    sleep_sensor(SENSOR_ADDR);

#endif

#ifdef CONFIG_DHT_SENSOR

    double temp2 = 0, hum2 = 0;

    set_dht_gpio(CONFIG_DHT_PIN);
    read_dht(&temp2, &hum2);

#endif

#ifdef CONFIG_LDR_SENSOR

    double lighting = 0;
    adc_measure(CONFIG_LDR_CHANNEL, CONFIG_LDR_ATTEN, CONFIG_LDR_ADC_UNIT, &lighting, 5);

#endif

    read_boot_counter();

    

    if (time_is_outdated())
    {
        wifi_start();
        if (wifi_state() == WIFI_CONNECTED)
            time_sync();
        wifi_stop();
    }

    if (time_is_outdated())
        go_sleep(SLEEP_TIME);

    time_t timestamp = time_get_timestamp(1);

#if defined(CONFIG_VOLTAGE_SENSOR) & defined(CONFIG_CURRENT_SENSOR)
    cJSON *object_a;
    object_a = cJSON_CreateObject();

    cJSON_AddStringToObject(object_a, "ID", CHIP_ID);
    cJSON_AddStringToObject(object_a, "S", CONFIG_CURRENT_BURDEN);
    cJSON_AddNumberToObject(object_a, "Vrms", vrms);
    cJSON_AddNumberToObject(object_a, "Irms", irms);
    cJSON_AddNumberToObject(object_a, "Pwr", real_pwr);
    cJSON_AddNumberToObject(object_a, "PF", pf);
    cJSON_AddNumberToObject(object_a, "D", timestamp);

    spiffs_write_file("/spiffs/data.txt", object_a);
    cJSON_Delete(object_a);

#elif CONFIG_CURRENT_SENSOR
    cJSON *object_aa;
    object_aa = cJSON_CreateObject();

    cJSON_AddStringToObject(object_aa, "ID", CHIP_ID);
    cJSON_AddStringToObject(object_aa, "S", CONFIG_CURRENT_BURDEN);
    CONFIG_BURDEN_22
    cJSON_AddNumberToObject(object_aa, "Irms", irms);
    cJSON_AddNumberToObject(object_aa, "D", timestamp);

    spiffs_write_file("/spiffs/data.txt", object_aa);
    cJSON_Delete(object_aa);

#elif CONFIG_VOLTAGE_SENSOR
    cJSON *object_ab;
    object_ab = cJSON_CreateObject();

    cJSON_AddStringToObject(object_ab, "ID", CHIP_ID);
    cJSON_AddStringToObject(object_ab, "S", "VSENS");
    cJSON_AddNumberToObject(object_ab, "Vrms", vrms);
    cJSON_AddNumberToObject(object_ab, "D", timestamp);

    spiffs_write_file("/spiffs/data.txt", object_ab);
    cJSON_Delete(object_ab);

#endif

#if defined(CONFIG_VOLTAGE_SENSOR) & defined(CONFIG_EX_CURRENT_SENSOR)
    cJSON *object_b;
    object_b = cJSON_CreateObject();

    cJSON_AddStringToObject(object_b, "ID", CHIP_ID);
    cJSON_AddStringToObject(object_b, "S", CONFIG_EX_CURRENT_BURDEN);
    cJSON_AddNumberToObject(object_b, "Vrms", vrms2);
    cJSON_AddNumberToObject(object_b, "Irms", irms2);
    cJSON_AddNumberToObject(object_b, "Pwr", real_pwr2);
    cJSON_AddNumberToObject(object_b, "PF", pf2);
    cJSON_AddNumberToObject(object_b, "D", timestamp);

    spiffs_write_file("/spiffs/data.txt", object_b);
    cJSON_Delete(object_b);

#elif CONFIG_EX_CURRENT_SENSOR

    cJSON *object_bb;
    object_bb = cJSON_CreateObject();

    cJSON_AddStringToObject(object_bb, "ID", CHIP_ID);
    cJSON_AddStringToObject(object_bb, "S", CONFIG_EX_CURRENT_BURDEN);
    cJSON_AddNumberToObject(object_bb, "Irms", irms);
    cJSON_AddNumberToObject(object_bb, "D", timestamp);

    spiffs_write_file("/spiffs/data.txt", object_bb);
    cJSON_Delete(object_bb);

#endif

#ifdef CONFIG_SHTC3_SENSOR
    cJSON *object_c;
    object_c = cJSON_CreateObject();

    cJSON_AddStringToObject(object_c, "ID", CHIP_ID);
    cJSON_AddStringToObject(object_c, "S", "SHTC3");
    cJSON_AddNumberToObject(object_c, "T", temp);
    cJSON_AddNumberToObject(object_c, "H", hum);
    cJSON_AddNumberToObject(object_c, "D", timestamp);

    spiffs_write_file("/spiffs/data.txt", object_c);
    cJSON_Delete(object_c);

#endif

#ifdef CONFIG_DHT_SENSOR
    cJSON *object_d;
    object_d = cJSON_CreateObject();

    cJSON_AddStringToObject(object_d, "ID", CHIP_ID);
    cJSON_AddStringToObject(object_d, "S", "DHT11");
    cJSON_AddNumberToObject(object_d, "T", temp2);
    cJSON_AddNumberToObject(object_d, "H", hum2);
    cJSON_AddNumberToObject(object_d, "D", timestamp);

    spiffs_write_file("/spiffs/data.txt", object_d);
    cJSON_Delete(object_d);

#endif


#ifdef CONFIG_LDR_SENSOR

    cJSON *object_f;
    object_f = cJSON_CreateObject();
    cJSON_AddStringToObject(object_f, "ID", CHIP_ID);
    cJSON_AddStringToObject(object_f, "S", "LDR0");
    cJSON_AddNumberToObject(object_f, "LL", lighting);
    cJSON_AddNumberToObject(object_f, "D", timestamp);

    spiffs_write_file("/spiffs/data.txt", object_f);
    cJSON_Delete(object_f);

#endif


    if (boot_counter == (CYCLES - 1))
    {
        persistence_sequency();
    }


    if (boot_counter == 0)
    {
        wifi_start();
        if (wifi_state() == WIFI_CONNECTED)
            time_sync();
        wifi_stop();
    }

    update_boot_counter();


    ESP_LOGI(TAG, "OP TIME: %d uS", esp_log_timestamp() - start_time);
    go_sleep(SLEEP_TIME);
}

static void persistence_sequency()
{
    //Initialize WIFI
    wifi_start();
    if (wifi_state() == WIFI_CONNECTED)
    {
        mqtt_start();
        
        char *buffer = NULL;
        long pos = 0;
        int after = 0, before = 0;

        before = esp_get_free_heap_size();

        bool a = false, client_err = false;
        for (int i = 0; (a == false) && (client_err == false); i++)
        {
            a = spiffs_read_file("/spiffs/data.txt", &buffer, &pos);
            mqtt_publish("/home", buffer, 0, 1, 0, &client_err);
        }

        after = esp_get_free_heap_size();
        ESP_LOGI(TAG, "Used mem: %i", before - after);

        mqtt_stop(&pedding_msg);
        if (!pedding_msg)
            spiffs_delete_file("/spiffs/data.txt");
        spiffs_deinit();
    }
    wifi_stop();
}
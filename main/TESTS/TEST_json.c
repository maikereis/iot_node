/* The example of ESP-IDF
 *
 * This sample code is in the public domain.
 */
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "string.h"
#include "spiffs_persistence.h"
#include "cJSON.h"
#include <math.h>

const char *PATH = "/spiffs/sensor_data.txt";

static const char *TAG = "TEST_json";
void test_spiffs_storage();

void app_main()
{	
	//spiffs_init();
	//spiffs_delete_file(PATH);
	test_spiffs_storage();
}
void test_spiffs_storage()
{

	spiffs_init();

	//CREATE JSON STRING
	//Fist write data to spiffs
	double temp = 24.3;
	double hum = 60.8;
	double timestamp = 1607479085;

	for (int i = 0; i < 1440; i++)
	{
		cJSON *objects;
		objects = cJSON_CreateObject();

		cJSON_AddNumberToObject(objects, "ID", i);
		cJSON_AddNumberToObject(objects, "T", temp + i);
		cJSON_AddNumberToObject(objects, "H", hum + i);
		cJSON_AddNumberToObject(objects, "D", timestamp + i);

		spiffs_write_file(PATH, objects);
		cJSON_Delete(objects);
		vTaskDelay(5 / portTICK_PERIOD_MS);
	}
	//Theb read data from
	char *buffer = NULL;
	long pos = 0;
	int after = 0, before = 0;

	before = esp_get_free_heap_size();

	bool a = false;
	for (int i = 0; a == false; i++)
	{
		a = spiffs_read_file(PATH, &buffer, &pos);
		ESP_LOGI(TAG, "%s", buffer);
	}
	
	after = esp_get_free_heap_size();
	ESP_LOGI(TAG, "Used mem: %i", before - after);
	spiffs_deinit();
}

void test_json_write()
{

	ESP_LOGI(TAG, "Serialize.....");
	int array_num = 3;

	cJSON *objects[3];
	for (int i = 0; i < array_num; i++)
	{
		objects[i] = cJSON_CreateObject();
	}

	cJSON_AddStringToObject(objects[0], "ID", "MK00");
	cJSON_AddNumberToObject(objects[0], "T", 20.0);
	cJSON_AddNumberToObject(objects[0], "H", 60.0);

	cJSON_AddStringToObject(objects[1], "ID", "MK01");
	cJSON_AddNumberToObject(objects[1], "T", 30.0);
	cJSON_AddNumberToObject(objects[1], "H", 70.0);

	cJSON_AddStringToObject(objects[2], "ID", "MK02");
	cJSON_AddNumberToObject(objects[2], "T", 40.0);
	cJSON_AddNumberToObject(objects[2], "H", 80.0);

	cJSON *root;
	root = cJSON_CreateArray();

	ESP_LOGI(TAG, "Array size %i", cJSON_GetArraySize(root));

	cJSON_AddItemToArray(root, objects[0]);
	ESP_LOGI(TAG, "Array size %i", cJSON_GetArraySize(root));

	cJSON_AddItemToArray(root, objects[1]);
	ESP_LOGI(TAG, "Array size %i", cJSON_GetArraySize(root));

	cJSON_AddItemToArray(root, objects[2]);
	ESP_LOGI(TAG, "Array size %i", cJSON_GetArraySize(root));

	const char *my_json_string = cJSON_PrintUnformatted(root);
	ESP_LOGI(TAG, "my_json_string\n%s, len: %i", my_json_string, strlen(my_json_string));

	cJSON_DeleteItemFromArray(root, 1);
	my_json_string = cJSON_Print(root);
	ESP_LOGI(TAG, "my_json_string\n%s, len: %i", my_json_string, strlen(my_json_string));

	cJSON_Delete(root);

	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
}
#include "spiffs_persistence.h"

#define FILE_PATH CONFIG_SPIFFS_FILE_PATH
static const char *TAG = "SPIFFS";

long get_file_size(FILE *f);

void spiffs_init()
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem.");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s).", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s).", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d.", total, used);
    }
}

void spiffs_write_file(const char *PATH, cJSON *object)
{

    //ESP_LOGI(TAG, "Opening file to write...");
    FILE *f = fopen(PATH, "a");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing.");
        return;
    }

    char *my_json_string = cJSON_PrintUnformatted(object);

    ESP_LOGI(TAG, "Writting \"%s\" to file", my_json_string);

    fprintf(f, "%s\n", my_json_string);

    fclose(f);
    //ESP_LOGI(TAG, "File closed");
}
// Read N lines from file to buffer 'lines' and return false
// When all lines in the file have been read, returns true (What happens when the cursor is at the end of the file)
// * * * Every call starts from the last readed position * * *
bool spiffs_read_file(const char *PATH, char **lines, long *position)
{
    ESP_LOGI(TAG, "Opening file to read...");
    FILE *f = fopen(PATH, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading.");
        return true;
    }
    cJSON_free(*lines);

    char *line = NULL;
    size_t len = 0;
    size_t read;
    cJSON *arr = cJSON_CreateArray();
    int counter = 0;

    fseek(f, *position, SEEK_SET);

    while ((((read = __getdelim(&line, &len, 10, f)) != -1)) && (counter < 40))
    {
        cJSON_AddItemToArray(arr, cJSON_Parse(line));
        counter++;
        *position = ftell(f);
    }
    *lines = cJSON_PrintUnformatted(arr);

    cJSON_Delete(arr);

    long aux = get_file_size(f);

    fclose(f);
    ESP_LOGI(TAG, "File closed");

    ESP_LOGI(TAG, "Pos: %lu, Aux: %lu", *position, aux);

    if (*position == aux)
    {
        return true;
    }
    else
    {
        return false;
    }
}
void spiffs_delete_file(const char *PATH)
{
    // Check if destination file exists before renaming
    struct stat st;
    if (stat(PATH, &st) == 0)
    {
        // Delete it if it exists
        unlink(PATH);
        ESP_LOGI(TAG, "File deleted.");
        return;
    }
    ESP_LOGI(TAG, "File don't exist.");

    size_t total = 0, used = 0;
    esp_err_t ret = esp_spiffs_info(NULL, &total, &used);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s).", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d.", total, used);
    }
}

void spiffs_deinit()
{
    // All done, unmount partition and disable SPIFFS
    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "Unmounted.");
}
long get_file_size(FILE *f)
{
    long pos = 0;
    fseek(f, 0, SEEK_END);
    pos = ftell(f);
    fseek(f, 0, SEEK_SET);
    return pos;
}

void spiffs_get_file_size(const char *PATH, long *size)
{
    FILE *f = fopen(PATH, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file.");
    }
    else
    {
        *size = get_file_size(f);
    }
    fclose(f);
}
#include "mqtt.h"

static const char *TAG = "MQTT";

#define MQTT_BROKER_URL CONFIG_BROKER_URL
#define MQTT_BROKER_PORT CONFIG_BROKER_PORT
#define MQTT_CLIENT_ID CONFIG_CLIENT_ID
#define MQTT_CLIENT_USERNAME CONFIG_CLIENT_USERNAME
#define MQTT_CLIENT_PASSWORD CONFIG_CLIENT_PASSWORD

static EventGroupHandle_t mqtt_event_group = NULL;

#define MQTT_CONNECTED BIT0
#define MQTT_PENDDING_PUB BIT1
#define MQTT_ERROR BIT2

static esp_mqtt_client_handle_t mqtt_client = NULL;

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id)
    {
    case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        //esp_mqtt_client_destroy(event->client);

        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        xEventGroupSetBits(mqtt_event_group, MQTT_PENDDING_PUB);

        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        xEventGroupSetBits(mqtt_event_group, MQTT_ERROR);

        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

void mqtt_start(void)
{

    mqtt_event_group = xEventGroupCreate();
    xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED);

    esp_mqtt_client_config_t mqtt_cfg = {
        .username = MQTT_CLIENT_USERNAME,
        .password = MQTT_CLIENT_PASSWORD,
        .host = MQTT_BROKER_URL,
        .port = MQTT_BROKER_PORT,
        .client_id = MQTT_CLIENT_ID};

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, &mqtt_event_handler, mqtt_client);
    esp_mqtt_client_start(mqtt_client);
}
void mqtt_publish(const char *topic, const char *msg, const int len, const int qos, const int retain, bool *client_err)
{
    if ((mqtt_event_group == NULL) | (mqtt_client == NULL))
    {
        ESP_LOGI(TAG, "Publish fail - client not initialized.");
        *client_err = true;
        return;
    }

    xEventGroupClearBits(mqtt_event_group, MQTT_PENDDING_PUB | MQTT_ERROR);

    const TickType_t xTicksToWait = 4000 / portTICK_PERIOD_MS;
    EventBits_t bits = xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED | MQTT_ERROR, false, false, xTicksToWait);

    if (bits & MQTT_CONNECTED)
    {
        int msg_id = esp_mqtt_client_publish(mqtt_client, topic, msg, len, qos, retain);
        ESP_LOGI(TAG, "Sent publish successful, msg_id=%d", msg_id);

        bits = xEventGroupWaitBits(mqtt_event_group, MQTT_PENDDING_PUB | MQTT_ERROR, false, false, xTicksToWait);

        if (bits & MQTT_PENDDING_PUB)
        {
            ESP_LOGI(TAG, "Publish done - msg_id=%d", msg_id);
        }
        else if (bits & MQTT_ERROR)
        {
            ESP_LOGI(TAG, "Publish fail - msg_id=%d", msg_id);
        }
    }
    else if (bits & MQTT_ERROR)
    {
        ESP_LOGI(TAG, "Publish fail - MQTT_ERROR");
        *client_err = true;
    }
    else
    {
        ESP_LOGI(TAG, "Publish fail - client not connected.");
        *client_err = true;
    }
}

void mqtt_stop(bool *pedding_msg)
{
    if ((mqtt_event_group == NULL) | (mqtt_client == NULL))
    {
        ESP_LOGI(TAG, "Stop fail - client not initialized.");
        *pedding_msg = true;
        return;
    }

    const TickType_t xTicksToWait = 4000 / portTICK_PERIOD_MS;

    EventBits_t bits = xEventGroupWaitBits(mqtt_event_group, MQTT_PENDDING_PUB | MQTT_ERROR, false, false, xTicksToWait);
    if (bits & MQTT_PENDDING_PUB)
    {
        ESP_LOGI(TAG, "Stop done - pendding messages was sent");
        esp_mqtt_client_stop(mqtt_client);
        *pedding_msg = false;
    }
    else if (bits & MQTT_ERROR)
    {
        ESP_LOGI(TAG, "Stop done - pendding messages aren't sent");
        *pedding_msg = true;
    }
}

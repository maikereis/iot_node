#include "wifi.h"

#define ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define ESP_MAXIMUM_RETRY CONFIG_ESP_WIFI_RETRY

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

static ip4_addr_t s_ip_addr;

//#ifdef CONFIG_EXAMPLE_CONNECT_IPV6
static ip6_addr_t s_ipv6_addr;
//#endif

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define GOT_IPV4_BIT BIT2
#define GOT_IPV6_BIT BIT3
#define WIFI_CONNECTED_BITS (GOT_IPV4_BIT | GOT_IPV6_BIT)

static const char *TAG = "WIFI";

static bool wifi_stt = WIFI_DISCONNECTED;

static int s_retry_num = 0;

esp_err_t wifi_stop();
bool wifi_state();

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        //ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        //ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
        tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");

        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            ESP_LOGI(TAG, "Trying reconnect...");
            esp_wifi_connect();
            s_retry_num++;
        }
        else
        {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "Fail connect to the AP Station");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP)
    {
        //ESP_LOGI(TAG, "WIFI_EVENT_STA_STOP");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        //ESP_LOGI(TAG, "IP_EVENT_GOT_IP4");
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        memcpy(&s_ip_addr, &event->ip_info.ip, sizeof(s_ip_addr));
        xEventGroupSetBits(wifi_event_group, GOT_IPV4_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_GOT_IP6)
    {
        //ESP_LOGI(TAG, "IP_EVENT_GOT_IP6");
        ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
        memcpy(&s_ipv6_addr, &event->ip6_info.ip, sizeof(s_ipv6_addr));
        xEventGroupSetBits(wifi_event_group, GOT_IPV6_BIT);
    }
}

void wifi_start()
{
    esp_log_level_set("wifi_init", ESP_LOG_ERROR); // enable WARN logs from WiFi stack
    esp_log_level_set("wifi", ESP_LOG_ERROR);          // enable WARN logs from WiFi stack
    esp_log_level_set("wpa", ESP_LOG_ERROR);
    esp_log_level_set("system_api", ESP_LOG_ERROR);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    //#ifdef CONFIG_EXAMPLE_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &event_handler, NULL));
    //#endif

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    //ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    const TickType_t xTicksToWait = 4000 / portTICK_PERIOD_MS;
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BITS,
                                           pdTRUE,
                                           pdTRUE,
                                           xTicksToWait);

    if (bits & WIFI_CONNECTED_BITS)
    {
        ESP_LOGI(TAG, "Connected to %s", ESP_WIFI_SSID);
        ESP_LOGI(TAG, "IPv4 address: " IPSTR, IP2STR(&s_ip_addr));
        ESP_LOGI(TAG, "IPv6 address: " IPV6STR, IPV62STR(s_ipv6_addr));
        wifi_stt = WIFI_CONNECTED;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Fail connect to %s", ESP_WIFI_SSID);
        wifi_stt = WIFI_DISCONNECTED;
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        wifi_stt = WIFI_DISCONNECTED;
    }
    
}

esp_err_t wifi_stop()
{
    if (wifi_event_group == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &event_handler));

    vEventGroupDelete(wifi_event_group);

    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    ESP_ERROR_CHECK(esp_event_loop_delete_default());

    wifi_stt = WIFI_DISCONNECTED;

    return ESP_OK;
}

bool wifi_state()
{
    return wifi_stt;
}

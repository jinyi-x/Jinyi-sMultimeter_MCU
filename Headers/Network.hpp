#ifndef __MULTIMETER_NETWORK_HEADER__
#define __MULTIMETER_NETWORK_HEADER__

#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_system.h>
#include <freertos/task.h>
#include <esp_smartconfig.h>
#include <smartconfig_ack.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#define bitConnected    BIT0
#define bitESPTouchDone BIT1
static EventGroupHandle_t WiFiEventGroupPtr;

static void taskSmartconfig(void* pvParameters);
static void EventHandler(void *arg, esp_event_base_t eventBase, int32_t eventID, void* eventData){
    if(eventBase == WIFI_EVENT && eventID == WIFI_EVENT_STA_START){
        xTaskCreate(taskSmartconfig, "Smartconfig", 4096, NULL, 3, NULL);
    } else if (eventBase == WIFI_EVENT && eventID == WIFI_EVENT_STA_DISCONNECTED){
        esp_wifi_connect();
        xEventGroupClearBits(WiFiEventGroupPtr, bitConnected);
    } else if (eventBase == IP_EVENT && eventID == IP_EVENT_STA_GOT_IP){
        xEventGroupSetBits(WiFiEventGroupPtr, bitConnected);
    } else if (eventBase == SC_EVENT && eventID == SC_EVENT_SCAN_DONE){
        ESP_LOGI("Smartconfig", "Scan Done");
    } else if (eventBase == SC_EVENT && eventID == SC_EVENT_FOUND_CHANNEL){
        ESP_LOGI("Smartconfig", "Found Channel");
    } else if(eventBase == SC_EVENT && eventID == SC_EVENT_GOT_SSID_PSWD){
        ESP_LOGI("Smartconfig", "Got AP Info");
        wifi_config_t wifiConfig;
        smartconfig_event_got_ssid_pswd_t* dataPtr = (smartconfig_event_got_ssid_pswd_t*)eventData;
        bzero(&wifiConfig, sizeof(wifi_config_t));
        memcpy(wifiConfig.sta.ssid, dataPtr->ssid, sizeof(wifiConfig.sta.ssid));
        memcpy(wifiConfig.sta.password, dataPtr->password, sizeof(wifiConfig.sta.password));
        wifiConfig.sta.bssid_set = dataPtr->bssid_set;
        if(wifiConfig.sta.bssid_set == true){
            memcpy(wifiConfig.sta.bssid, dataPtr->bssid, sizeof(wifiConfig.sta.bssid));
        }
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig));
        ESP_ERROR_CHECK(esp_wifi_connect());
    }
}
void initializeWifi(){
    tcpip_adapter_init();
    WiFiEventGroupPtr = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &EventHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &EventHandler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &EventHandler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void taskSmartconfig(void* pvParameters){
    EventBits_t uxBits;
    ESP_ERROR_CHECK(esp_smartconfig_set_type(CONFIG_ESP_SMARTCONFIG_TYPE));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    while(1){
        uxBits = xEventGroupWaitBits(WiFiEventGroupPtr, bitConnected | bitESPTouchDone, true, false, portMAX_DELAY);
        if(uxBits & bitConnected)
            ESP_LOGI("Smartconfig", "WiFi Connected");
        if(uxBits & bitESPTouchDone){
            ESP_LOGI("Smartconfig", "Smartconfig finished");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

#endif
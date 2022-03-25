#ifndef __MULTIMETER_NETWORK_HEADER__
#define __MULTIMETER_NETWORK_HEADER__

#include <cJSON.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <lwip/dns.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_system.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <freertos/task.h>
#include <esp_smartconfig.h>
#include <smartconfig_ack.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <PeripheralController.hpp>


#define bitConnected            BIT0
#define bitESPTouchDone         BIT1
#define bitMeasureResistance    BIT2
#define bitMeasureVoltage       BIT3
#define bitTransferResistance   BIT4
#define bitTransferVoltage      BIT5

TaskHandle_t SniffTask;
TaskHandle_t TransferTask;
static EventGroupHandle_t WiFiEventGroupPtr;
extern EventGroupHandle_t WiFiEventGroupPtr;
static void taskSmartconfig(void* pvParameters);
static void EventHandler(void *arg, esp_event_base_t eventBase, int32_t eventID, void* eventData){
    if(eventBase == WIFI_EVENT && eventID == WIFI_EVENT_STA_START){
        vTaskDelete(TransferTask);
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
static void taskServerSniff(void* pvParameters);
static void taskSmartconfig(void* pvParameters){
    EventBits_t uxBits;
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    while(1){
        uxBits = xEventGroupWaitBits(WiFiEventGroupPtr, bitConnected | bitESPTouchDone, true, false, portMAX_DELAY);
        if(uxBits & bitConnected)
            ESP_LOGI("Smartconfig", "WiFi Connected");
        if(uxBits & bitESPTouchDone){
            ESP_LOGI("Smartconfig", "Smartconfig finished");
            esp_smartconfig_stop();
            xTaskCreate(taskServerSniff, "Sniffer", 1024, NULL, 3, &SniffTask);
            vTaskDelete(NULL);
        }
    }
}
static void taskTransfer(void* pvParameters);
struct sockaddr_in ServerAddress;
static void taskServerSniff(void* pvParameters){
    EventBits_t uxBits;
    uxBits = xEventGroupWaitBits(WiFiEventGroupPtr, bitConnected, false, false, portMAX_DELAY);
    if(uxBits & bitConnected){
        // Start Sniffing Server
        bool sniffFlag = true;
        int NetTimeOut = 5000;
        int fd = socket(PF_INET, SOCK_DGRAM, 0);
        if(fd == -1){
            ESP_LOGE("Server Sniff", "Get Socket Failed, Please Reset");
            vTaskDelete(NULL);
        }
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &NetTimeOut, sizeof(int));
        memset(&ServerAddress, 0, sizeof(ServerAddress));
        ServerAddress.sin_family = AF_INET;
        ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        ServerAddress.sin_port = htons(ServerSniffPort);
        ServerAddress.sin_len = sizeof(ServerAddress);
        int ret = -1;
        char udpMsg[48];
        while(sniffFlag){
            do {
                sendto(fd, "WhereIsServer", sizeof("WhereIsServer"), 0, (struct sockaddr *)&ServerAddress, (socklen_t)ServerAddress.sin_len);
                ret = recvfrom(fd, udpMsg, 48, 0, (struct sockaddr *)&ServerAddress, (socklen_t*)&ServerAddress.sin_len);
                vTaskDelay(100);
                if(ret > 0){
                    ServerAddress.sin_addr.s_addr = inet_addr(udpMsg);
                    ServerAddress.sin_port = ServerTransferPort;
                    sniffFlag = false;
                    break;
                }
            } while(ret == -1);
        }
        xTaskCreate(taskTransfer, "Transfer", 4096, NULL, 2, &TransferTask);
        vTaskDelete(NULL);
    }
}
static void taskTransfer(void* pvParameters){
    int RxLen;
    char RxBuffer[128];
    char TxBuffer[128];
    EventBits_t uxBits = xEventGroupGetBits(WiFiEventGroupPtr);
    while(true){
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0){
            ESP_LOGE("Transfer", "Get Socket Failed: errno %d", errno);
            break;
        }
        int err = connect(sock, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress));
        if(err < 0){
            ESP_LOGE("Transfer", "Connect Failed: errno %d", errno);
            close(sock);
            continue;
        }
        while(true){
            RxLen = recv(sock, RxBuffer, sizeof(RxBuffer) - 1, 0);
            if(RxLen > 0){
                RxBuffer[RxLen] = 0;        // Terminate String
                cJSON* RootNodePtr = cJSON_Parse(RxBuffer);
                if(RootNodePtr){
                    cJSON* Operation = cJSON_GetObjectItem(RootNodePtr, "Op");
                    if(strcmp("MeasureVoltage", Operation -> string) == 0){
                        xEventGroupSetBits(WiFiEventGroupPtr, bitMeasureVoltage);
                    }
                    if(strcmp("MeasureResistance", Operation -> string) == 0){
                        xEventGroupSetBits(WiFiEventGroupPtr, bitMeasureVoltage);
                    }
                HandShakeLabel:
                    if(strcmp("HandShake", Operation -> string) == 0){
                        cJSON* RetRootNodePtr = cJSON_CreateObject();
                        cJSON_AddStringToObject(RetRootNodePtr, "Op", "HandShake");
                        cJSON_AddStringToObject(RetRootNodePtr, "Payload", "");
                        strcpy(TxBuffer, cJSON_Print(RetRootNodePtr));
                        int sendRet = send(sock, TxBuffer, sizeof(TxBuffer), 0);
                        if(sendRet < 0){
                            ESP_LOGE("Transfer", "Send HandShake Failed: errno %d", errno);
                            goto HandShakeLabel;
                        }
                    }
                }
            }
            uxBits = xEventGroupGetBits(WiFiEventGroupPtr);
            if(uxBits & bitTransferVoltage){
                cJSON* RetRootNodePtr = cJSON_CreateObject();
                cJSON_AddStringToObject(RetRootNodePtr, "Op", "ReturnVoltage");
                cJSON* PayloadObjectPtr = cJSON_CreateObject();
                cJSON_AddNumberToObject(PayloadObjectPtr, "Voltage", voltage);
                cJSON_AddStringToObject(RetRootNodePtr, "Payload", cJSON_Print(PayloadObjectPtr));
                xEventGroupClearBits(WiFiEventGroupPtr, bitTransferVoltage);
            }
            if(uxBits & bitTransferResistance){
                cJSON* RetRootNodePtr = cJSON_CreateObject();
                cJSON_AddStringToObject(RetRootNodePtr, "Op", "ReturnResistance");
                cJSON* PayloadObjectPtr = cJSON_CreateObject();
                cJSON_AddNumberToObject(PayloadObjectPtr, "Resistance", resistance);
                cJSON_AddStringToObject(RetRootNodePtr, "Payload", cJSON_Print(PayloadObjectPtr));
                xEventGroupClearBits(WiFiEventGroupPtr, bitTransferResistance);
            }
        }
        if(sock != -1){
            ESP_LOGW("Transfer", "Shutting Down Socket And Restarting");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}
#endif
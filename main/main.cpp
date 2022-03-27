#include <nvs_flash.h>
#include "Network.hpp"
#include "PeripheralController.hpp"
#include <esp8266/pin_mux_register.h>

static void taskMain(void* pvParameters){
    initializeWifi();
    xEventGroupWaitBits(WiFiEventGroupPtr, bitConnected, false, false, portMAX_DELAY);
    periphearalInitialize();
    EventBits_t uxBits;
    while(true){
        uxBits = xEventGroupGetBits(WiFiEventGroupPtr);
        if(uxBits & bitMeasureVoltage){
            xEventGroupSetBits(periphearalEventGroupPtr, voltageMeasureBit);
            xEventGroupWaitBits(periphearalEventGroupPtr, ReadFinishBit, true, false, portMAX_DELAY);
            xEventGroupSetBits(WiFiEventGroupPtr, bitTransferVoltage);
        }
        if(uxBits & bitMeasureResistance){
            xEventGroupSetBits(periphearalEventGroupPtr, resistorMeasureBit);
            xEventGroupWaitBits(periphearalEventGroupPtr, ReadFinishBit, true, false, portMAX_DELAY);
            xEventGroupSetBits(WiFiEventGroupPtr, bitTransferResistance);
        }
    }
}

extern "C" void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
    xTaskCreate(taskMain, "main", 1024, NULL, 1, NULL);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
}

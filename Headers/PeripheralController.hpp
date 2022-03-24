#ifndef __MULTIMETER_PERIPHERAL_CONTROLLER_HEADER__
#define __MULTIMETER_PERIPHERAL_CONTROLLER_HEADER__

#include <esp_log.h>
#include <esp_event.h>
#include <esp_system.h>
#include <driver/gpio.h>
#include <freertos/task.h>
#include <freertos/FreeRTOS.h>
#include "MacroDefinition.hpp"
#include <freertos/event_groups.h>

bool clkPol = false;
EventGroupHandle_t periphearalEventGroupPtr;
extern EventGroupHandle_t periphearalEventGroupPtr;

uint8_t adcSpiRead(){
    uint8_t retVal = 0;
    for(int i = 0; i < 16; i++){
        if(clkPol == false){
            clkPol = true;
            gpio_set_level(ADC_SCLK_PIN, 1);
            retVal = retVal | (gpio_get_level(ADC_DOUT_PIN) << (uint8_t)(i / 2));
        } else {
            gpio_set_level(ADC_SCLK_PIN, 0);
        }
    }
    return retVal;
}

void adcSpiWrite(uint8_t data){
    for(int i = 0; i < 16; i++){
        if(clkPol == false){
            clkPol = true;
            gpio_set_level(ADC_SCLK_PIN, 1);
            gpio_set_level(ADC_DIN_PIN, (data & 1 << (uint8_t)(i/2)) ? 1 : 0);
        } else {
            clkPol = false;
            gpio_set_level(ADC_SCLK_PIN, 0);
        }
    }
}

void adcCommandReset(void){
    adcSpiWrite(0x06);
}

void adcCommandStartConversion(void){
    adcSpiWrite(0x08);
}

void adcCommandPowerDown(void){
    adcSpiWrite(0x02);
}

void adcCommandReadData(void){
    adcSpiWrite(0x10);
}

void adcWriteRegister(uint8_t registerAddress, uint8_t registerValue){
    if (registerAddress < 0x03){
        uint8_t writeValue = (registerAddress & 0x03) << 2;
        writeValue = 0x40 | writeValue;
        adcSpiWrite(writeValue);
        adcSpiWrite(registerValue);
    }
}

static void periphearalInitialize(){
    gpio_config_t ioConfig;
    ioConfig.intr_type = GPIO_INTR_DISABLE;
    ioConfig.mode = GPIO_MODE_OUTPUT;
    ioConfig.pin_bit_mask = 
    (1ULL << MUX_A_PIN)   |
    (1ULL << MUX_B_PIN)   |
    (1ULL << ADC_CS_PIN)  |
    (1ULL << ADC_DIN_PIN) |
    (1ULL << ADC_SCLK_PIN);
    ioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&ioConfig);
    // Initialize Output GPIO
    ioConfig.mode = GPIO_MODE_INPUT;
    ioConfig.pin_bit_mask = 
    (1ULL << ADC_DRDY_PIN) |
    (1ULL << ADC_DOUT_PIN);
    gpio_config(&ioConfig);
    gpio_set_level(ADC_CS_PIN, 0);
    gpio_set_level(ADC_DIN_PIN, 0);
    gpio_set_level(ADC_SCLK_PIN, 0);
    gpio_set_level(MUX_A_PIN, 0);
    gpio_set_level(MUX_B_PIN, 1);
    // Initialize All GPIO And Output GPIO Default Level
    adcCommandReset();
    os_delay_us(500);
}

static void taskAdcRead(void* pvParameters){

}

#endif
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

EventGroupHandle_t periphearalEventGroupPtr;
extern EventGroupHandle_t periphearalEventGroupPtr;

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
    ioConfig.pull_down_en = 0;
    ioConfig.pull_up_en = 0;
    gpio_config(&ioConfig);
    // Initialize Output GPIO
    ioConfig.mode = GPIO_MODE_INPUT;
    ioConfig.pin_bit_mask = 
    (1ULL << ADC_DRDY_PIN) |
    (1ULL << ADC_DOUT_PIN);
    gpio_config(&ioConfig);
}

#endif
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

#define resistorMeasureBit      BIT0
#define voltageMeasureBit       BIT1
#define ReadFinishBit           BIT2



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
double voltage;
double resistance;

double getVoltage(){return voltage;}
double getResistance(){return resistance;}

static void taskAdcRead(void* pvParameters){
    uint8_t rangeFlag;
    EventBits_t uxBits;
    uxBits = xEventGroupWaitBits(periphearalEventGroupPtr, voltageMeasureBit | resistorMeasureBit, true, false, portMAX_DELAY);
    if(uxBits & voltageMeasureBit){
        gpio_set_level(MUX_A_PIN, 1);
        gpio_set_level(MUX_B_PIN, 1);
        // Using MAX Range
        adcWriteRegister(0x00, ADC_MUX_0_1_MASK | ADC_PGA_GAIN_1_MASK | ADC_PGA_DISABLE_MASK);
        adcWriteRegister(0x01, ADC_NORMAL_MODE_SAMPLE_RATE_20_MASK | ADC_MODE_NORMAL_MASK | ADC_SINGLE_SHOT_MODE_MASK | ADC_TEMPERATURE_SENSOR_ON_MASK | ADC_BURN_OUT_CURRENT_SOURCE_OFF_MASK);
        adcWriteRegister(0x02, ADC_VREF_SEL_INTERNAL_MASK | ADC_FILTER_50_MASK | ADC_AIN3_SWITCH_ALWAYS_OPEN_MASK | ADC_IDAC_CURRENT_0_MASK);
        adcWriteRegister(0x03, ADC_IDAC1_MUX_NC_MASK | ADC_IDAC2_MUX_NC_MASK | ADC_DRDY_MODE_DRDY_ONLY);
        // Initialize ADC Register
        adcCommandStartConversion();
        while(gpio_get_level(ADC_DRDY_PIN) != 0);
        // Waiting For Conversation
        uint16_t adcVal = adcSpiRead() << 8;
        adcVal = adcVal | adcSpiRead();
        rangeFlag = 3;
        double adcVoltage = adcVal * 2048 / 65535;  // Voltage(mV) Of Adc Analog Port
        if(adcVoltage < 1500 && adcVoltage > 1200){
            //Range 2V - 5V
            rangeFlag = 2;
            gpio_set_level(MUX_A_PIN, 1);
            gpio_set_level(MUX_B_PIN, 0);
        } else if (adcVoltage < 1200 && adcVoltage > 1020){
            rangeFlag = 1;
            // Range 0.2V - 2V
            gpio_set_level(MUX_A_PIN, 0);
            gpio_set_level(MUX_B_PIN, 1);
        } else if (adcVoltage < 1020){
            // Rango 0V - 0.2V
            rangeFlag = 0;
            gpio_set_level(MUX_A_PIN, 0);
            gpio_set_level(MUX_B_PIN, 0);
        }
        adcCommandStartConversion();
        while(gpio_get_level(ADC_DRDY_PIN));
        adcVal = adcSpiRead();
        adcVal = adcVal | adcSpiRead();
        adcVoltage = adcVal * 2048 / 65535;
        switch(rangeFlag){
            case 3:
                voltage = adcVoltage * 10;
                break;
            case 2:
                voltage = adcVoltage * 5;
                break;
            case 1:
                voltage = adcVoltage * 2;
                break;
            case  0:
                voltage = adcVoltage / 5;
                break;
            default:
                break;
        }
        xEventGroupSetBits(periphearalEventGroupPtr, ReadFinishBit);
    }
    if(uxBits & resistorMeasureBit){
        uint8_t CurrentMask = ADC_IDAC_CURRENT_50_MASK;
        uint16_t adcVal;
        double resistanceVal;
        adcWriteRegister(0x00, ADC_MUX_2_3_MASK | ADC_PGA_GAIN_1_MASK | ADC_PGA_DISABLE_MASK);
        adcWriteRegister(0x01, ADC_NORMAL_MODE_SAMPLE_RATE_20_MASK | ADC_MODE_NORMAL_MASK | ADC_SINGLE_SHOT_MODE_MASK | ADC_TEMPERATURE_SENSOR_ON_MASK | ADC_BURN_OUT_CURRENT_SOURCE_OFF_MASK);
        adcWriteRegister(0x03, ADC_IDAC1_MUX_AIN2_MASK | ADC_IDAC2_MUX_NC_MASK | ADC_DRDY_MODE_DRDY_ONLY);
        // Initialize ADC
    resMeasure:
        adcWriteRegister(0x02, ADC_VREF_SEL_INTERNAL_MASK | ADC_FILTER_50_MASK | ADC_AIN3_SWITCH_ALWAYS_OPEN_MASK | CurrentMask);
        adcCommandStartConversion();
        while(gpio_get_level(ADC_DRDY_PIN));
        adcVal = adcSpiRead() << 8;
        adcVal = adcVal | adcSpiRead();
        switch(CurrentMask){
            case ADC_IDAC_CURRENT_50_MASK:
                resistanceVal = adcVal * 2.048 / 65535 / 5e-5;
                break;
            case ADC_IDAC_CURRENT_100_MASK:
                resistanceVal = adcVal * 2.048 / 65535 / 1e-4;
                break;
            case ADC_IDAC_CURRENT_250_MASK:
                resistanceVal = adcVal * 2.048 / 65535 / 2.5e-4;
                break;
            case ADC_IDAC_CURRENT_500_MASK:
                resistanceVal = adcVal * 2048 / 65535 / 5e-4;
                break;
            case ADC_IDAC_CURRENT_1000_MASK:
                resistanceVal = adcVal * 2048 / 65535 / 1e-3;
                break;
            case ADC_IDAC_CURRENT_1500_MASK:
                resistanceVal = adcVal * 2048 / 65535 / 1.5e-3;
                break;
            default:
                break;
        }
        if(resistanceVal > 4e4 && CurrentMask != ADC_IDAC_CURRENT_50_MASK){
            CurrentMask = ADC_IDAC_CURRENT_50_MASK;
            goto resMeasure;
        }
        if(resistanceVal < 4e4 && resistanceVal > 2e4 && CurrentMask != ADC_IDAC_CURRENT_100_MASK){
            CurrentMask = ADC_IDAC_CURRENT_100_MASK;
            goto resMeasure;
        }
        if(resistanceVal < 2e4 && resistanceVal > 8e3 && CurrentMask != ADC_IDAC_CURRENT_250_MASK){
            CurrentMask = ADC_IDAC_CURRENT_250_MASK;
            goto resMeasure;
        }
        if(resistanceVal < 8e3 && resistanceVal > 4e3 && CurrentMask != ADC_IDAC_CURRENT_500_MASK){
            CurrentMask = ADC_IDAC_CURRENT_500_MASK;
            goto resMeasure;
        }
        if(resistanceVal < 4e3 && resistanceVal > 2e3 && CurrentMask != ADC_IDAC_CURRENT_1000_MASK){
            CurrentMask = ADC_IDAC_CURRENT_1000_MASK;
            goto resMeasure;
        }
        if(resistanceVal < 2e3 && CurrentMask != ADC_IDAC_CURRENT_1500_MASK){
            CurrentMask = ADC_IDAC_CURRENT_1500_MASK;
            goto resMeasure;
        }
        resistance = resistanceVal;
        xEventGroupSetBits(periphearalEventGroupPtr, ReadFinishBit);
    }
}

static void periphearalInitialize(){
    gpio_config_t ioConfig;
    ioConfig.intr_type = GPIO_INTR_DISABLE;
    ioConfig.mode = GPIO_MODE_OUTPUT;
    ioConfig.pin_bit_mask = (1ULL << MUX_A_PIN) | (1ULL << MUX_B_PIN)   | (1ULL << ADC_CS_PIN) | (1ULL << ADC_DIN_PIN) | (1ULL << ADC_SCLK_PIN);
    ioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&ioConfig);
    // Initialize Output GPIO
    ioConfig.mode = GPIO_MODE_INPUT;
    ioConfig.pin_bit_mask = (1ULL << ADC_DRDY_PIN) | (1ULL << ADC_DOUT_PIN);
    gpio_config(&ioConfig);
    gpio_set_level(ADC_CS_PIN, 0);
    gpio_set_level(ADC_DIN_PIN, 0);
    gpio_set_level(ADC_SCLK_PIN, 0);
    gpio_set_level(MUX_A_PIN, 1);
    gpio_set_level(MUX_B_PIN, 1);
    // Initialize All GPIO And Output GPIO Default Level
    adcCommandReset();
    os_delay_us(500);
    // Initialize ADC
    periphearalEventGroupPtr = xEventGroupCreate();
    xTaskCreate(taskAdcRead, "ADC Service", 4096, NULL, 1, NULL);
}
#endif
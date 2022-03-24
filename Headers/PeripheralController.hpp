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
    gpio_set_level(MUX_A_PIN, 0);
    gpio_set_level(MUX_B_PIN, 1);
    // Initialize All GPIO And Output GPIO Default Level
    adcCommandReset();
    os_delay_us(500);
    // Initialize ADC
    periphearalEventGroupPtr = xEventGroupCreate();
}
double voltage;
double resistance;
static void taskAdcRead(void* pvParameters){
    EventBits_t uxBits;
    while(1){
        uxBits = xEventGroupWaitBits(periphearalEventGroupPtr, resistorMeasureBit | voltageMeasureBit, true, false, portMAX_DELAY);
        xEventGroupClearBits(periphearalEventGroupPtr, ReadFinishBit);
        adcCommandReset();
        os_delay_us(500);
        if(uxBits & resistorMeasureBit){
            uint8_t CurrentMask = ADC_IDAC_CURRENT_50_MASK;
        resMeasureLabel:
            adcWriteRegister(0x01, ADC_NORMAL_MODE_SAMPLE_RATE_20_MASK | ADC_MODE_NORMAL_MASK | ADC_SINGLE_SHOT_MODE_MASK | ADC_TEMPERATURE_SENSOR_OFF_MASK | ADC_BURN_OUT_CURRENT_SOURCE_OFF_MASK);
            adcWriteRegister(0x02, ADC_VREF_SEL_INTERNAL_MASK | ADC_FILTER_50_MASK | ADC_AIN3_SWITCH_ALWAYS_OPEN_MASK | CurrentMask);
            switch(RESISTOR_ADC_CHANNEL){
                case 0:{
                    adcWriteRegister(0x00, ADC_MUX_0_G_MASK | ADC_PGA_GAIN_1_MASK | ADC_PGA_DISABLE_MASK);
                    adcWriteRegister(0x03, ADC_IDAC1_MUX_NC_MASK | ADC_IDAC2_MUX_AIN0_MASK | ADC_DRDY_MODE_DRDY_ONLY);
                }
                case 1:{
                    adcWriteRegister(0x00, ADC_MUX_1_G_MASK | ADC_PGA_GAIN_1_MASK | ADC_PGA_DISABLE_MASK);
                    adcWriteRegister(0x03, ADC_IDAC1_MUX_NC_MASK | ADC_IDAC2_MUX_AIN1_MASK | ADC_DRDY_MODE_DRDY_ONLY);
                }
                case 2:{
                    adcWriteRegister(0x00, ADC_MUX_0_G_MASK | ADC_PGA_GAIN_2_MASK | ADC_PGA_DISABLE_MASK);
                    adcWriteRegister(0x03, ADC_IDAC1_MUX_NC_MASK | ADC_IDAC2_MUX_AIN2_MASK | ADC_DRDY_MODE_DRDY_ONLY);
                }
                case 3:{
                    adcWriteRegister(0x00, ADC_MUX_3_G_MASK | ADC_PGA_GAIN_1_MASK | ADC_PGA_DISABLE_MASK);
                    adcWriteRegister(0x03, ADC_IDAC1_MUX_NC_MASK | ADC_IDAC2_MUX_AIN3_MASK | ADC_DRDY_MODE_DRDY_ONLY);
                }
                default:
                    break;
            }
            while(gpio_get_level(ADC_DRDY_PIN) != 1);
            uint16_t adcVal = adcSpiRead() << 8;
            adcVal |= adcSpiRead();
            double voltageVal = adcVal * 2.48 / 65535;
            switch(CurrentMask){
                case ADC_IDAC_CURRENT_50_MASK:
                    resistance = voltageVal / 5 * 1e5;
                    if(resistance < 2e4 && resistance > 8e3){
                        CurrentMask = ADC_IDAC_CURRENT_100_MASK;
                    } else if(resistance < 8e3 && resistance > 4e3) {
                        CurrentMask = ADC_IDAC_CURRENT_250_MASK;
                    } else if (resistance < 4e3 && resistance > 2e3){
                        CurrentMask = ADC_IDAC_CURRENT_500_MASK;
                    } else if (resistance < 2e3 && resistance > 1.3e3){
                        CurrentMask = ADC_IDAC_CURRENT_1000_MASK;
                    } else if (resistance < 1.3e3) {
                        CurrentMask = ADC_IDAC_CURRENT_1500_MASK;
                    }
                    goto resMeasureLabel;
                    break;
                case ADC_IDAC_CURRENT_100_MASK:
                    resistance = voltageVal * 1e4;
                    if(resistance < 8e3 && resistance > 4e3) {
                        CurrentMask = ADC_IDAC_CURRENT_250_MASK;
                    } else if (resistance < 4e3 && resistance > 2e3){
                        CurrentMask = ADC_IDAC_CURRENT_500_MASK;
                    } else if (resistance < 2e3 && resistance > 1.3e3){
                        CurrentMask = ADC_IDAC_CURRENT_1000_MASK;
                    } else if (resistance < 1.3e3) {
                        CurrentMask = ADC_IDAC_CURRENT_1500_MASK;
                    }
                    goto resMeasureLabel;
                    break;
                case ADC_IDAC_CURRENT_250_MASK:
                    resistance = voltageVal * 4e3;
                    if (resistance < 4e3 && resistance > 2e3){
                        CurrentMask = ADC_IDAC_CURRENT_500_MASK;
                    } else if (resistance < 2e3 && resistance > 1.3e3){
                        CurrentMask = ADC_IDAC_CURRENT_1000_MASK;
                    } else if (resistance < 1.3e3) {
                        CurrentMask = ADC_IDAC_CURRENT_1500_MASK;
                    }
                    break;
                case ADC_IDAC_CURRENT_500_MASK:
                    resistance = voltageVal * 2e3;
                    if (resistance < 2e3 && resistance > 1.3e3){
                        CurrentMask = ADC_IDAC_CURRENT_1000_MASK;
                    } else if (resistance < 1.3e3) {
                        CurrentMask = ADC_IDAC_CURRENT_1500_MASK;
                    }
                    break;
                case ADC_IDAC_CURRENT_1000_MASK:
                    resistance = voltageVal * 1e3;
                    if (resistance < 1.3e3) {
                        CurrentMask = ADC_IDAC_CURRENT_1500_MASK;
                    }
                    break;
                case ADC_IDAC_CURRENT_1500_MASK:
                    resistance = voltage / 1.5 * 1e3;
                    break;
                default:
                    break;
            }
            xEventGroupSetBits(periphearalEventGroupPtr, ReadFinishBit);
        }
        if(uxBits & voltageMeasureBit){
            
        }
    }
}

#endif
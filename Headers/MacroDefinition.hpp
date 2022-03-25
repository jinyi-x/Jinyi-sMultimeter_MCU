#ifndef __MULTIMETER_MACRO_DEFINITION_HEADER__
#define __MULTIMETER_MACRO_DEFINITION_HEADER__

#include <gpio.h>

#define ADC_DRDY_PIN    GPIO_NUM_12
#define ADC_CS_PIN      GPIO_NUM_13
#define ADC_DOUT_PIN    GPIO_NUM_14
#define ADC_SCLK_PIN    GPIO_NUM_16
#define ADC_DIN_PIN     GPIO_NUM_15

#define MUX_A_PIN       GPIO_NUM_4
#define MUX_B_PIN       GPIO_NUM_5
// GPIO Configuration

#define VOLTAGE_ADC_CHANNEL     0
#define RESISTOR_ADC_CHANNEL    1

// ADC Channel Select
#define ADC_MUX_0_1_MASK    0x00
#define ADC_MUX_0_2_MASK    0x10
#define ADC_MUX_0_3_MASK    0x20
#define ADC_MUX_1_2_MASK    0x30
#define ADC_MUX_1_3_MASK    0x40
#define ADC_MUX_2_3_MASK    0x50
#define ADC_MUX_1_0_MASK    0x60
#define ADC_MUX_3_2_MASK    0x70
#define ADC_MUX_0_G_MASK    0x80
#define ADC_MUX_1_G_MASK    0x90
#define ADC_MUX_2_G_MASK    0xA0
#define ADC_MUX_3_G_MASK    0xB0

#define ADC_PGA_GAIN_1_MASK     0x00
#define ADC_PGA_GAIN_2_MASK     0x02
#define ADC_PGA_GAIN_4_MASK     0x04
#define ADC_PGA_GAIN_8_MASK     0x06
#define ADC_PGA_GAIN_16_MASK    0x08
#define ADC_PGA_GAIN_32_MASK    0x0A
#define ADC_PGA_GAIN_64_MASK    0x0C
#define ADC_PGA_GAIN_128_MASK   0x0E

#define ADC_PGA_ENABLE_MASK     0x00
#define ADC_PGA_DISABLE_MASK    0x01

#define ADC_NORMAL_MODE_SAMPLE_RATE_20_MASK     0x00
#define ADC_NORMAL_MODE_SAMPLE_RATE_45_MASK     0x20
#define ADC_NORMAL_MODE_SAMPLE_RATE_90_MASK     0x40
#define ADC_NORMAL_MODE_SAMPLE_RATE_175_MASK    0x60
#define ADC_NORMAL_MODE_SAMPLE_RATE_330_MASK    0x80
#define ADC_NORMAL_MODE_SAMPLE_RATE_600_MASK    0xA0
#define ADC_NORMAL_MODE_SAMPLE_RATE_1000_MASK   0xC0

#define ADC_DUTY_CYCLE_MODE_SAMPLE_RATE_5_00_MASK       0x00            // 5.00 Hertz
#define ADC_DUTY_CYCLE_MODE_SAMPLE_RATE_11_25_MASK      0x20            // 11.25 Hertz
#define ADC_DUTY_CYCLE_MODE_SAMPLE_RATE_22_50_MASK      0x40            // 22.50 Hertz
#define ADC_DUTY_CYCLE_MODE_SAMPLE_RATE_44_00_MASK      0x60            // 44.00 Hertz
#define ADC_DUTY_CYCLE_MODE_SAMPLE_RATE_82_50_MASK      0x80            // 82.50 Hertz
#define ADC_DUTY_CYCLE_MODE_SAMPLE_RATE_150_00_MASK     0xA0            // 150.00 Hertz
#define ADC_DUTY_CYCLE_MODE_SAMPLE_RATE_250_00_MASK     0xC0            // 250.00 Hertz

#define ADC_TURBO_MODE_SAMPLE_RATE_40_MASK      0x00
#define ADC_TURBO_MODE_SAMPLE_RATE_90_MASK      0x20
#define ADC_TURBO_MODE_SAMPLE_RATE_180_MASK     0x40
#define ADC_TURBO_MODE_SAMPLE_RATE_350_MASK     0x60
#define ADC_TURBO_MODE_SAMPLE_RATE_660_MASK     0x80
#define ADC_TURBO_MODE_SAMPLE_RATE_1200_MASK    0xA0
#define ADC_TURBO_MODE_SAMPLE_RATE_2000_MASK    0xC0

#define ADC_MODE_NORMAL_MASK        0x00
#define ADC_MODE_DUTY_CYCLE_MASK    0x08
#define ADC_MODE_TURBO_MODE_MASK    0x10

#define ADC_SINGLE_SHOT_MODE_MASK           0x00
#define ADC_CONTINOUS_CONVERSION_MODE_MASK  0x04

#define ADC_TEMPERATURE_SENSOR_OFF_MASK     0x00
#define ADC_TEMPERATURE_SENSOR_ON_MASK      0x02

#define ADC_BURN_OUT_CURRENT_SOURCE_OFF_MASK 0x00
#define ADC_BURN_OUT_CURRENT_SOURCE_ON_MASK  0x01

#define ADC_VREF_SEL_INTERNAL_MASK      0x00
#define ADC_VREF_SEL_REF_PIN_MASK       0x40
#define ADC_VREF_SEL_AIN0_AIN3_PIN_MASK 0x80
#define ADC_VREF_SEL_ANALOG_SUPPLY_MASK 0xC0

#define ADC_FILTER_DISABLE_MASK 0x00
#define ADC_FILTER_50_60_MASK   0x10
#define ADC_FILTER_50_MASK      0x20
#define ADC_FILTER_60_MASK      0x30

#define ADC_AIN3_SWITCH_ALWAYS_OPEN_MASK    0x00
#define ADC_AIN3_CLOSE_AT_CONVERSION_MASK   0x08

#define ADC_IDAC_CURRENT_0_MASK     0x00
#define ADC_IDAC_CURRENT_50_MASK    0x02
#define ADC_IDAC_CURRENT_100_MASK   0x03
#define ADC_IDAC_CURRENT_250_MASK   0x04
#define ADC_IDAC_CURRENT_500_MASK   0x05
#define ADC_IDAC_CURRENT_1000_MASK  0x06
#define ADC_IDAC_CURRENT_1500_MASK  0x07

#define ADC_IDAC1_MUX_NC_MASK       0x00
#define ADC_IDAC1_MUX_AIN0_MASK     0x20
#define ADC_IDAC1_MUX_AIN1_MASK     0x40
#define ADC_IDAC1_MUX_AIN2_MASK     0x60
#define ADC_IDAC1_MUX_AIN3_MASK     0x80
#define ADC_IDAC1_MUX_REFP0_MASK    0xA0
#define ADC_IDAC1_MUX_REFN0_MASK    0xC0

#define ADC_IDAC2_MUX_NC_MASK       0x00
#define ADC_IDAC2_MUX_AIN0_MASK     0x08
#define ADC_IDAC2_MUX_AIN1_MASK     0x10
#define ADC_IDAC2_MUX_AIN2_MASK     0x18
#define ADC_IDAC2_MUX_AIN3_MASK     0x20
#define ADC_IDAC2_MUX_REFP0_MASK    0x28
#define ADC_IDAC2_MUX_REFN0_MASK    0x30

#define ADC_DRDY_MODE_DRDY_ONLY     0x00
#define ADC_DRDY_MODE_DRDY_AND_DOUT 0x02

#endif
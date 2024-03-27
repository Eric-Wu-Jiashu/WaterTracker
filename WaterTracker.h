#ifndef _WATERTRACKER_H
#define _WATERTRACKER_H

#include "stm32f1xx_hal.h"
#include "ssd1306_fonts.h"
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
#include "dma.h"

// OLED driver by afiskon
//github.com/afiskon/stm32-ssd1306
#include "ssd1306.h"

void WTInit();
void WTMain();

#define Pull_Down_Resistor 40 // pull down resistor value in the divider circuit

//setup time for first day, so system will reset on 0am every day
#define Current_Time_Hour 23
#define Current_Time_Minute 58

#endif


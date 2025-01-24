#ifndef __DELAY__
#define __DELAY__

#include <stdint.h>
#include "main.h"                       
#include "stm32f4xx_hal.h"   

void delay_us(uint16_t us);
void delay_ms(uint16_t ms);

#endif

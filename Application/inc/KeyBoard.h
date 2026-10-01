#ifndef __KETBOARD_H
#define __KETBOARD_H

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"

#include "PCF8574.h"
#include "Menu.h"

void KeyBoard_Init(void);
void ScanKeyBoard(void);

#endif

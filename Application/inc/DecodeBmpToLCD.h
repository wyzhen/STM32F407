#ifndef __DECODEBMPTOLCD_H
#define __DECODEBMPTOLCD_H

#include "FreeRTOS.h"
#include "task.h"

#include "stm32f4xx.h"                  // Device header
#include "bsp_debug_usart.h"
#include "bsp_spi_sdcard.h"
#include "ff.h"
#include "string.h"
#include "bmp.h"
#include "FATFs.h"
#include "LCD_SPI.h"
#include "OLED.h"
#include "Timer.h"

#define PixelNum	320*240

FRESULT Encode_BmpToLCD(char *path, char *filename, uint32_t num);		//解码bmp
void OnMyRailGun(char *path, uint32_t num);								//播放OnMyRailGun动画
FRESULT AllBmpTo1Bin(char *path, char *filename, uint32_t num);			//合并目录下全部Bmp图像转换为一个Bin文件
void PlayBinFlie(char *path, char *filename, uint32_t num);				//播放OnMyRailGun的Bin文件
void LCD_PlayBinFlie(char *path, char *filename, uint32_t num);

#endif

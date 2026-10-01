#include "KeyBoard.h"

extern uint16_t Scroll_num;

void KeyBoard_Delay(uint32_t num)			//微秒us
{
	uint32_t i;
	for (i = 0; i < num*15; i++);
}

void KeyBoard_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	//上拉模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void ScanKeyBoard(void)
{
	if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == RESET)
	{
		vTaskDelay(2);
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == RESET)
		{
			uint8_t Portdata;
			Portdata = PCF8574_ReadPort(0X4F);
			switch (Portdata)
			{
				case 0xFE:
					
					break;
				case 0xFD:
					
					break;
				case 0xFB:
					Scroll_num -=10;
					break;
				case 0xF7:
					
					break;
				case 0xEF:
					
					break;
				case 0xDF:
					
					break;
				case 0xBF:
					Scroll_num +=10;
					break;
				case 0x7F:
					
					break;
			}
		}
	}
}


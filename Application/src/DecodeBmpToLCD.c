#include "DecodeBmpToLCD.h"

extern FATFS fs;													/* FatFs文件系统对象 */
extern FIL fnew;													/* 文件对象 */
extern DIR dir;														/* 目录对象 */
extern FRESULT res_flash;                							/* 文件操作结果 */
extern FRESULT res_SD;                								/* 文件操作结果 */
extern UINT fnum;            					 					/* 文件成功读写数量 */
extern char fpath[100];                 							/* 保存当前扫描路径 */
extern BYTE readbuffer[512]; 										/* 读取文件缓存区 */
extern FILINFO fno;													/* 文件信息结构变量 */
extern uint8_t Image_Cache[MaxImageCache];							/* 图像数据存储缓存区 */
extern uint16_t main_j;												/* 帧数计数变量 */

FRESULT Encode_BmpToLCD(char *path, char *filename, uint32_t num)	//逐张解码bmp图像输出到0.96LCD
{
	uint32_t i, j, k;				//j表示一幅bmp图像素数
	BitMapInfo *pbmp;   			//临时指针
	uint8_t  color_byte;			//彩色位 16/24/32
	uint8_t  biCompression = 0;		//记录压缩方式
	uint32_t BmpOffset ;  			//从文件开始到位图数据(bitmap data)开始之间的的偏移量
	char str_temp[10];
	uint32_t temp, lseeknum = 0;
	uint8_t red, green, blue;
	
	i = strlen(path);

	NumToZeroStr(str_temp, num, 4);									//字符串地址，要转换的数字，一共转换的位数
//	printf("str_temp = %s\r\n", str_temp);
	
	sprintf(&path[i], "/%s", filename);								//合成完整文件路径
	sprintf(&path[i + strlen(filename) + 1], "%s.bmp", str_temp);							//合成完整文件路径
//	printf("path = %s\r\n", path);
	
	taskENTER_CRITICAL();
	res_SD = f_open(&fnew, (const TCHAR *)path, FA_OPEN_ALWAYS|FA_READ);//打开文件
	taskEXIT_CRITICAL();
	
	taskENTER_CRITICAL();
//	printf("f_open_res_SD = %d\r\n", res_SD);
	f_lseek(&fnew, 0);
	taskEXIT_CRITICAL();
	
	taskENTER_CRITICAL();
	res_SD = Read_File(&fnew, Image_Cache, sizeof(BitMapInfo), &fnum);
	taskEXIT_CRITICAL();
	
	pbmp = (BitMapInfo *)Image_Cache;
	color_byte = pbmp->bmiHeader.biBitCount;						//彩色位 16/24/32 
	printf("color_byte = %d\r\n", color_byte);	
	biCompression = pbmp->bmiHeader.biCompression;					//压缩方式
	printf("biCompression = %d\r\n", biCompression);
	printf("文件头类型%#x\r\n", pbmp->bmfHeader.bfType);
	
	LCD_WriteCmd(0x36);
	LCD_WriteData(0x70);
	LCD_SetRegion(0, 0, 319, 239);									//坐标设置
	LCD_WriteCmd(0x2C);											// Memory Write
	
	if (pbmp->bmfHeader.bfType == 0x4D42)
	{
		BmpOffset = pbmp->bmfHeader.bfOffBits;
//		printf("BmpOffset = %d\r\n", BmpOffset);
		lseeknum = BmpOffset;
		
		taskENTER_CRITICAL();
		f_lseek(&fnew, lseeknum);
		taskEXIT_CRITICAL();
		
		temp = PixelNum*3;
		for (j = 0; j <= (PixelNum*3 - 1)/MaxImageCache; j++)
		{
			
			if (temp < MaxImageCache)
			{
				taskENTER_CRITICAL();
				res_SD = Read_FullFile(&fnew, Image_Cache, &fnum);				//读取位图数据
				taskEXIT_CRITICAL();
				
//				printf("Read_File2_res_SD = %d\r\n", res_SD);
				for (k = 0; k < temp/3; k++)
				 {
					red = Image_Cache[k*3 + 0] >> 3;
					green = Image_Cache[k*3 + 1] >> 2;
					blue = Image_Cache[k*3 + 2] >> 3;
					LCD_WriteData( (green>>3) + (red<<3) );
					LCD_WriteData(blue + (green<<5) );
				 }
			}
			else
			{
				
				taskENTER_CRITICAL();
				res_SD = Read_File(&fnew, Image_Cache, MaxImageCache, &fnum);	//读取位图数据
				taskEXIT_CRITICAL();
				
//				printf("Read_File1_res_SD = %d\r\n", res_SD);
				for (k = 0; k < MaxImageCache/3; k++)
				 {
					red = Image_Cache[k*3 + 0] >> 3;
					green = Image_Cache[k*3 + 1] >> 2;
					blue = Image_Cache[k*3 + 2] >> 3;
					LCD_WriteData( (green>>3) + (red<<3) );
					LCD_WriteData(blue + (green<<5) );
				 }
			}
			
			temp -= MaxImageCache;
		}
		LCD_WriteCmd(0x36);
		LCD_WriteData(0x30);
		if (res_SD == FR_OK)
		{
//			printf("成功写入图像%s\r\n", path);								//输出文件名/* 可以在这里提取特定格式的文件路径 */
		}
//		else printf("写入图像失败%s\r\n", path);							//printf调试
	}
	else printf("%s不是正确的bmp位图！\r\n",path);							//printf调试
	
	main_j ++;

	f_close(&fnew);
	path[i] = 0;

	return res_SD; 
}

void OnMyRailGun(char *path, uint32_t num)						//直接解码bmp图像输出到0.96LCD
{
	char *filename = "onmyrailgun";
	
	Encode_BmpToLCD(path, filename, num);
	
}

/***************************************合并BMP转换为BIN*************************************************************/
//合并目录下全部Bmp图像转换为一个Bin文件
FRESULT AllBmpTo1Bin(char *path, char *filename, uint32_t num)		
{
	uint32_t i, j, k, l;								//j表示一幅bmp图像素数, l表示bmp图片总数
	BitMapInfo *pbmp;   								//bmp结构体变量指针
	FIL fp_Bin;											//Bin文件指针
	char path_Bin[256];
	uint8_t  color_byte;								//彩色位 16/24/32
	uint8_t  biCompression = 0;							//记录压缩方式
	uint32_t BmpOffset ;  								//从文件开始到位图数据(bitmap data)开始之间的的偏移量
	char str_temp[10];
	uint32_t temp, lseeknum = 0;
	uint8_t red, green, blue;
	
	i = strlen(path);
	strcpy(path_Bin, path);													//将path复制给panth_Bin
	sprintf(&path_Bin[i], "/%s.bin", filename);								//合成完整panth_Bin路径
	printf("path_Bin路径 %s\r\n", path_Bin);	
	res_SD = f_open(&fp_Bin, path_Bin, FA_CREATE_ALWAYS|FA_READ|FA_WRITE);			//新建Bin文件
	printf("f_openBin_res_SD = %d\r\n", res_SD);							//printf调试
	f_lseek(&fp_Bin, lseeknum);
	
	for (l = 0; l < num; l++)
	{
		
		NumToZeroStr(str_temp, l, 4);										//字符串地址，要转换的数字，一共转换的位数
		printf("\nstr_temp = %s\r\n", str_temp);
		
		sprintf(&path[i], "/%s %s.bmp", filename, str_temp);				//合成完整文件路径
		printf("path = %s\r\n", path);
		res_SD = f_open(&fnew, path, FA_OPEN_ALWAYS|FA_READ);				//打开文件
		printf("f_openfnew_res_SD = %d\r\n", res_SD);
		f_lseek(&fnew, 0);
		
		res_SD = Read_File(&fnew, Image_Cache, sizeof(BitMapInfo), &fnum);
		pbmp = (BitMapInfo *)Image_Cache;
		color_byte = pbmp->bmiHeader.biBitCount;							//彩色位 16/24/32 
		printf("color_byte = %d\r\n", color_byte);	
		biCompression = pbmp->bmiHeader.biCompression;						//压缩方式
		printf("biCompression = %d\r\n", biCompression);
		printf("文件头类型%#x\r\n", pbmp->bmfHeader.bfType);
		
		if (pbmp->bmfHeader.bfType == 0x4D42)
		{
			BmpOffset = pbmp->bmfHeader.bfOffBits;
	//		printf("BmpOffset = %d\r\n", BmpOffset);
			f_lseek(&fnew, BmpOffset);
	//		printf("f_lseek_res_SD = %d\r\n", res_SD);
			
			temp = PixelNum*4;
			for (j = 0; j <= (PixelNum*4 - 1)/MaxImageCache; j++)
			{
				if (temp < MaxImageCache)
				{
					res_SD = Read_FullFile(&fnew, Image_Cache, &fnum);				//读取位图数据
	//				printf("Read_File2_res_SD = %d\r\n", res_SD);
					for (k = 0; k < temp/4; k++)
					 {
						red = Image_Cache[k*4 + 0] >> 3;
						green = Image_Cache[k*4 + 1] >> 2;
						blue = Image_Cache[k*4 + 2] >> 3;
						Image_Cache[2*k] = (green>>3) + (red<<3);
						Image_Cache[2*k + 1] = blue + (green<<5);
						
					 }
					 f_lseek(&fp_Bin, lseeknum);
					 Write_File(&fp_Bin, Image_Cache, temp/2, &fnum);
					 lseeknum += temp/2;
				}
				else
				{
					res_SD = Read_File(&fnew, Image_Cache, MaxImageCache, &fnum);	//读取位图数据
	//				printf("Read_File1_res_SD = %d\r\n", res_SD);
					for (k = 0; k < MaxImageCache/4; k++)
					 {
						red = Image_Cache[k*4 + 0] >> 3;
						green = Image_Cache[k*4 + 1] >> 2;
						blue = Image_Cache[k*4 + 2] >> 3;
						Image_Cache[2*k] = (green>>3) + (red<<3);
						Image_Cache[2*k + 1] = blue + (green<<5);
						 
					 }
					 f_lseek(&fp_Bin, lseeknum);
					 Write_File(&fp_Bin, Image_Cache, MaxImageCache/2, &fnum);
					 lseeknum += MaxImageCache/2;
				}
				
				temp -= MaxImageCache;
			}
			
			if (res_SD == FR_OK)
			{
				printf("成功写入图像%s\r\n", path);									//输出文件名/* 可以在这里提取特定格式的文件路径 */
			}
			else printf("写入图像失败%s\r\n", path);								//printf调试
		}
		else printf("%s不是正确的bmp位图！\r\n",path);								//printf调试

		f_close(&fnew);
		path[i] = 0;
	}

	res_SD = f_close(&fp_Bin);
	printf("Write_fpBin_res_SD = %d\r\n", res_SD);
	return res_SD;
}

void PlayBinFlie(char *path, char *filename, uint32_t num)						//播放OnMyRailGun的Bin文件
{
	uint32_t i, j, k, l;
	uint32_t temp, lseeknum = 0;
	uint32_t BmpPixelNum = 142*80;
	i = strlen(path);
	
	taskENTER_CRITICAL();
	f_opendir(&dir, path);									//打开目录/文件夹
	taskEXIT_CRITICAL();
	
	sprintf(&path[i], "/%s", filename);									//合成完整文件路径
	
	taskENTER_CRITICAL();
	res_SD = f_open(&fnew, path, FA_OPEN_ALWAYS|FA_READ);				//打开文件
	printf("f_openfnew_res_SD = %d\r\n", res_SD);
	taskEXIT_CRITICAL();
	
	for (j = 0; j < num; j++)
	{
		
		TFT_SetRegion(9,0,150,79);									//坐标设置
		TFT_WriteCmd(0x2C);											// Memory Write
		
		temp = BmpPixelNum*2;
		for (k = 0; k <= (BmpPixelNum*2 - 1)/MaxImageCache; k++)
		{
			taskENTER_CRITICAL();
			f_lseek(&fnew, lseeknum);
			printf("f_lseek_res_SD = %d\r\n", res_SD);
			taskEXIT_CRITICAL();
			
			if (temp < MaxImageCache)
			{
				taskENTER_CRITICAL();
				res_SD = Read_File(&fnew, Image_Cache, temp, &fnum);				//读取位图数据
				taskEXIT_CRITICAL();
				
				taskENTER_CRITICAL();
				TFT_DC_HIGH;  //发送数据
				TFT_CS_LOW;  //使能LCD
				for (i = 0; i < temp; i++)
				{
					SPI_I2S_SendData(SPI1, Image_Cache[i]); //通过外设SPIx发送一个数据
					while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {}
				}
				TFT_CS_HIGH;  //取消片选 
				taskEXIT_CRITICAL();
				
//				for (l = 0; l < temp; l++)
//				 {
//					TFT_WriteData(Image_Cache[l]);
//				 }
				 lseeknum += temp;
			}
			else
			{
				taskENTER_CRITICAL();
				res_SD = Read_File(&fnew, Image_Cache, MaxImageCache, &fnum);	//读取位图数据
				taskEXIT_CRITICAL();
				
				taskENTER_CRITICAL();
				TFT_DC_HIGH;  //发送数据
				TFT_CS_LOW;  //使能LCD
				for (i = 0; i < MaxImageCache; i++)
				{
					SPI_I2S_SendData(SPI1, Image_Cache[i]); //通过外设SPIx发送一个数据
					while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {}
				}
				TFT_CS_HIGH;  //取消片选 
				taskEXIT_CRITICAL();
				
//				for (l = 0; l < MaxImageCache; l++)
//				 {
//					 TFT_WriteData(Image_Cache[l]);
//				 }
				 lseeknum += MaxImageCache;
			}
			
			temp -= MaxImageCache;
		}
//		printf("第 [%d] 副图\r\n", j);
		vTaskDelay(2);
		
		main_j ++;
	}
	path[i] = 0;
	
	taskENTER_CRITICAL();
	f_close(&fnew);
	taskEXIT_CRITICAL();
}

void LCD_PlayBinFlie(char *path, char *filename, uint32_t num)						//播放OnMyRailGun的Bin文件
{
	uint32_t i, j, k, l;
	uint32_t temp, lseeknum = 0, retry = 0;
	uint32_t BmpPixelNum = 142*80;
	i = strlen(path);
	
	taskENTER_CRITICAL();
	f_opendir(&dir, path);									//打开目录/文件夹
	taskEXIT_CRITICAL();
	
	sprintf(&path[i], "/%s", filename);									//合成完整文件路径
	
	taskENTER_CRITICAL();
	res_SD = f_open(&fnew, path, FA_OPEN_ALWAYS|FA_READ);				//打开文件
//	printf("f_openfnew_res_SD = %d\r\n", res_SD);
	taskEXIT_CRITICAL();
	
	LCD_WriteCmd(0x36);
	LCD_WriteData(0x70);
	for (j = 0; j < num; j++)
	{
		OLED_ShowChineseString(3, 0, "帧数", 's', 16);
		LCD_SetRegion(89,80,230,159);									//坐标设置
		LCD_WriteCmd(0x2C);											// Memory Write
		
		temp = BmpPixelNum*2;
		for (k = 0; k <= (BmpPixelNum*2 - 1)/MaxImageCache; k++)
		{
			taskENTER_CRITICAL();
			f_lseek(&fnew, lseeknum);
//			printf("f_lseek_res_SD = %d\r\n", res_SD);
			taskEXIT_CRITICAL();
			
			if (temp < MaxImageCache)
			{
				taskENTER_CRITICAL();
				res_SD = Read_File(&fnew, Image_Cache, temp, &fnum);				//读取位图数据
				taskEXIT_CRITICAL();
				
				taskENTER_CRITICAL();
				LCD_DC_HIGH;  //发送数据
				LCD_CS_LOW;  //使能LCD
				for (l = 0; l < temp; l++)
				{				 	
					while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位,1代表空
						{
						}			  
					SPI_I2S_SendData(SPI1, Image_Cache[l]); //通过外设SPIx发送一个数据
					while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//检查指定的SPI标志位设置与否:接受缓存非空标志位,1代表非空
						{
						}
					SPI_I2S_ReceiveData(SPI1);
				}
				LCD_CS_HIGH;  //取消片选 
				taskEXIT_CRITICAL();
				
//				for (l = 0; l < temp; l++)
//				{
//					LCD_WriteData(Image_Cache[l]);
//				}
				lseeknum += temp;
			}
			else
			{
				taskENTER_CRITICAL();
				res_SD = Read_File(&fnew, Image_Cache, MaxImageCache, &fnum);	//读取位图数据
				taskEXIT_CRITICAL();
				
				taskENTER_CRITICAL();
				LCD_DC_HIGH;  //发送数据
				LCD_CS_LOW;  //使能LCD
				for (l = 0; l < MaxImageCache; l++)
				{
					SPI_I2S_SendData(SPI1, Image_Cache[l]); //通过外设SPIx发送一个数据
					while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {}
				}
				LCD_CS_HIGH;  //取消片选 
				taskEXIT_CRITICAL();
				
//				for (l = 0; l < MaxImageCache; l++)
//				 {
//					 LCD_WriteData(Image_Cache[l]);
//				 }
				 lseeknum += MaxImageCache;
			}
			
			temp -= MaxImageCache;
		}
//		printf("第 [%d] 副图\r\n", j);
		vTaskDelay(2);
		
		main_j ++;
	}
	path[i] = 0;
	
	taskENTER_CRITICAL();
	f_close(&fnew);
	taskEXIT_CRITICAL();
}

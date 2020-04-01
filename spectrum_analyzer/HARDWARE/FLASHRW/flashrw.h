
/* Includes ------------------------------------------------------------------*/
#ifndef __FLASHRW_H
#define __FLASHRW_H	 

#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#define STARTADDR 0x800d000
//#define STARTADDR 0x800e000

#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
  #define FLASH_PAGE_SIZE    ((uint16_t)0x800)
  
#else
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#endif


int Flash_Read(uint32_t iAddress, uint8_t *buf, int32_t iNbrToRead) ;
int Flash_Write(uint32_t iAddress, uint8_t *buf, uint32_t iNbrToWrite);
u8 Flash_Erase_Paraset(uint32_t iAddress,u8 pagenum);
uint16_t Flash_Write_Without_check(uint32_t iAddress, uint8_t *buf, uint16_t iNumByteToWrite);


#endif

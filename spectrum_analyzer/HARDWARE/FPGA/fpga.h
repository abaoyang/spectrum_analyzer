#ifndef __FPGA_H
#define __FPGA_H		
#include "sys.h"	 
#include "stdlib.h"
#include "stm32f10x.h"
#include "stm32f10x_exti.h"

//////////////////////////////////////////////////////////////////////////////////

#define FPGA_LOW_DATA_PORT GPIOA
#define FPGA_HIGH_DATA_PORT GPIOB
#define FPGA_CS_PIN	GPIO_Pin_11

#define FPGA_RD_PIN	GPIO_Pin_8		//portA��8��
#define FPGA_WR_PIN	GPIO_Pin_8		//portB��8��
#define FPGA_ALE_PIN GPIO_Pin_9		//portB��9��
#define FPGA_IRQ_PIN	GPIO_Pin_12	//portA��12��
#define FPGA_MCU_GPIO_PIN	GPIO_Pin_13	//portC��13��

extern u8 fpga_data_ready;

void FPGA_Init(void);
void FPGA_Reset(void);
void FPGA_EXTI_Init(void);
void KEY_Init(void);
void FPGA_Write(u16 addr,u16 data);
u16 FPGA_Read(u16 addr);
					   																			 
//FPGA�Ĵ���  
#define IDREG             0x000
#define RESETREG          0x001
#define TRIGOPREG         0x002
#define TRIGMODEREG       0x003
#define MAXINTGREG        0x004
#define DATASTATE1REG     0x008
#define DATASTATE2REG     0x009
#define INTGSETBASEREG		0x400
#define DATASAMPLEREG     0x800
				  		 
#endif

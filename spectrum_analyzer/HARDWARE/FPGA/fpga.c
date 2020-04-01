/* Includes ------------------------------------------------------------------*/
#include "fpga.h"
#include "stdlib.h"
#include "usart.h"	 
#include "delay.h"	
#include "led.h"

u8 fpga_data_ready = 0;

void KEY_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//ʹ��PA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	  
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;                   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;            
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                    
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;                   
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;               
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}

void EXTI0_IRQHandler(void)
{    
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {	
		LED1=~LED1; 		
		USART1_Send_One(0x55);
    EXTI_ClearITPendingBit(EXTI_Line0);    
  }
}
void FPGA_Init(void)
{
	GPIO_InitTypeDef GPIOA_InitStructure,GPIOB_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);//ʹ��PA,PB
	GPIOA_InitStructure.GPIO_Pin = FPGA_CS_PIN|FPGA_RD_PIN;								//�˿�����, �������
	GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 			//�������
	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 			//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOA, &GPIOA_InitStructure);	  				     		//�����趨������ʼ��CS,READ
	GPIO_SetBits(GPIOA,FPGA_RD_PIN);												//Ĭ���Ǹߵ�ƽ
	GPIO_SetBits(GPIOA,FPGA_CS_PIN);												//Ĭ���Ǹߵ�ƽ
	
	GPIOB_InitStructure.GPIO_Pin = FPGA_WR_PIN|FPGA_ALE_PIN;								//�˿�����, �������
	GPIOB_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 			//�������
	GPIOB_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 			//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIOB_InitStructure);	  				     		//�����趨������ʼ��ALE,WR
	GPIO_SetBits(GPIOB,FPGA_WR_PIN);											//Ĭ���Ǹߵ�ƽ
	GPIO_ResetBits(GPIOB,FPGA_ALE_PIN);										//Ĭ���ǵ͵�ƽ
}
void FPGA_Reset(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//ʹ��PC
	GPIO_InitStructure.GPIO_Pin = FPGA_MCU_GPIO_PIN;								//�˿�����, �������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 			//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 			//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				     		//�����趨������ʼ��PC.13
	GPIO_SetBits(GPIOC,FPGA_MCU_GPIO_PIN);														//PC.13����� 
	GPIO_ResetBits(GPIOC,FPGA_MCU_GPIO_PIN);									//PC.13����ͣ���λFPGA
	delay_us(50);
	GPIO_SetBits(GPIOC,FPGA_MCU_GPIO_PIN);														//PC.13����� 	
}

void FPGA_EXTI_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);//ʹ��PC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);	//ʹ��portA
	GPIO_InitStructure.GPIO_Pin = FPGA_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	//GPIO_Init(GPIOC, &GPIO_InitStructure);
	//RCC_LSEConfig(RCC_LSE_OFF);  //PC13 PC14 PC15��Ϊ��ͨIO��ʹ��
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	  
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;                   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;            
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                    
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource15);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource12);
	//EXTI_InitStructure.EXTI_Line = EXTI_Line15;
	EXTI_InitStructure.EXTI_Line = EXTI_Line12;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;                   
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;               
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}

void EXTI15_10_IRQHandler(void)
{    
  if(EXTI_GetITStatus(EXTI_Line12) != RESET)
  {	
		//LED1=~LED1;		
		//USART1_Send_One(0x55);
		fpga_data_ready = 1;
    EXTI_ClearITPendingBit(EXTI_Line12);    
  }
}

static void FPGA_Chip_Select_Pin(uint8_t High_Low)
{
	if (High_Low)
		GPIO_SetBits(GPIOA, FPGA_CS_PIN);
	else		
		GPIO_ResetBits(GPIOA, FPGA_CS_PIN);
}

static void FPGA_Address_Lock_Enable_Pin(uint8_t High_Low)
{
	if (High_Low)
		GPIO_SetBits(GPIOB, FPGA_ALE_PIN);		
	else
		GPIO_ResetBits(GPIOB, FPGA_ALE_PIN);
		
}

static void FPGA_Write_Pin(uint8_t High_Low)
{
	if (High_Low)
		GPIO_SetBits(GPIOB, FPGA_WR_PIN);		
	else
		GPIO_ResetBits(GPIOB, FPGA_WR_PIN);
		
}
static void FPGA_Read_Pin(uint8_t High_Low)
{
	if (High_Low)
		GPIO_SetBits(GPIOA, FPGA_RD_PIN);		
	else
		GPIO_ResetBits(GPIOA, FPGA_RD_PIN);
		
}

static void FPGA_Data_Direction(uint8_t input)
{
	GPIO_InitTypeDef GPIOA_InitStructure,GPIOB_InitStructure;
		
	/* Configure FPGA IO: Data0-7 */
	GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 ; 
 	GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* Configure FPGA IO: Data8-15 */
	GPIOB_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 ; 
 	GPIOB_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	if (input){
		GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	  GPIOB_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	}
	else {
		GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	  GPIOB_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	}
	GPIO_Init(FPGA_LOW_DATA_PORT, &GPIOA_InitStructure);
	GPIO_Init(FPGA_HIGH_DATA_PORT, &GPIOB_InitStructure);
	
}

void FPGA_Write(u16 addr,u16 data)
{
	uint16_t data_low,data_high;
	uint16_t val;
	data_low = addr&0x00ff;
	data_high = (addr&0xfc00)|((addr&0x0300)>>8);	
	FPGA_Address_Lock_Enable_Pin(0);
	FPGA_Chip_Select_Pin(0);
	FPGA_Write_Pin(1);
	FPGA_Data_Direction(0);	
	val = GPIO_ReadOutputData(FPGA_LOW_DATA_PORT) & 0xFFFFFF00;
	GPIO_Write(FPGA_LOW_DATA_PORT, val | data_low);
	val = GPIO_ReadOutputData(FPGA_HIGH_DATA_PORT) & 0xFFFF03FC;
	GPIO_Write(FPGA_HIGH_DATA_PORT, val | data_high);
	FPGA_Address_Lock_Enable_Pin(1);
	delay_us(1);
	FPGA_Address_Lock_Enable_Pin(0);
	delay_us(1);
	
	data_low = data&0x00ff;
	data_high = (data&0xfc00)|((data&0x0300)>>8);		
	val = GPIO_ReadOutputData(FPGA_LOW_DATA_PORT) & 0xFFFFFF00;
	GPIO_Write(FPGA_LOW_DATA_PORT, val | data_low);
	val = GPIO_ReadOutputData(FPGA_HIGH_DATA_PORT) & 0xFFFF03FC;
	GPIO_Write(FPGA_HIGH_DATA_PORT, val | data_high);
	FPGA_Write_Pin(0);
	delay_us(1);
	FPGA_Write_Pin(1);
	FPGA_Chip_Select_Pin(1);	
}

u16 FPGA_Read(u16 addr)
{
	uint16_t data_low,data_high,data_received;
	uint16_t val;
	data_low = addr&0x00ff;
	data_high = (addr&0xfc00)|((addr&0x0300)>>8);	
	FPGA_Address_Lock_Enable_Pin(0);
	FPGA_Chip_Select_Pin(0);
	FPGA_Read_Pin(1);
	FPGA_Data_Direction(0);	
	val = GPIO_ReadOutputData(FPGA_LOW_DATA_PORT) & 0xFFFFFF00;
	GPIO_Write(FPGA_LOW_DATA_PORT, val | data_low);
	val = GPIO_ReadOutputData(FPGA_HIGH_DATA_PORT) & 0xFFFF03FC;
	GPIO_Write(FPGA_HIGH_DATA_PORT, val | data_high);
	FPGA_Address_Lock_Enable_Pin(1);
	delay_us(1);
	FPGA_Address_Lock_Enable_Pin(0);
	delay_us(1);
	
	FPGA_Data_Direction(1);
	FPGA_Read_Pin(0);
	delay_us(1);
	val = GPIO_ReadInputData(FPGA_LOW_DATA_PORT);
	data_received=0x00ff&val;
	val = GPIO_ReadInputData(FPGA_HIGH_DATA_PORT);
	data_received  = (val&0xfc00)|((val&0x0003)<<8)|data_received;
  FPGA_Read_Pin(1);
	FPGA_Chip_Select_Pin(1);
	return data_received;
}

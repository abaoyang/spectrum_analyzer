/******************** (C) COPYRIGHT 2013 VeryAI Team **************************
 * ����    ��VeryAI Team
 * ��汾  ��ST3.5.0
 * ʵ��ƽ̨��VeryAI STM32������
 * ����    ��LED��������  
 *
 * ��̳    ��http://www.veryai.com
 * �Ա�    ��http://veryaishop.taobao.com
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "led.h"   

/*
 * ��������LED_Init
 * ����  ����ʼ��PB8��PB9Ϊ�����.��ʹ���������ڵ�ʱ��,LED IO��ʼ��
 * ����  ����
 * ���  ����
 */	    
void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
// 	GPIO_InitTypeDef  GPIOC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//ʹ��PB

	GPIO_InitStructure.GPIO_Pin = LED1_PIN | LED2_PIN;								//�˿�����, �������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 			//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 			//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				     		//�����趨������ʼ��PB.8,PB9
	GPIO_SetBits(GPIOB,LED1_PIN);														//PB.8����� 
	GPIO_SetBits(GPIOB,LED2_PIN);													//PB.8����� 	
	
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//ʹ��PC
// 	GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_13;								//�˿�����, �������
// 	GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 			//�������
// 	GPIOC_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 			//IO���ٶ�Ϊ50MHz
// 	GPIO_Init(GPIOC, &GPIOC_InitStructure);	  				     		//�����趨������ʼ��PC.13
// 	GPIO_SetBits(GPIOC,GPIO_Pin_13);														//PC.13����� 
		
	
}

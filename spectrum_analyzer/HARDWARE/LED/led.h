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
#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
/*������ƶ���*/
// #define LED1_PIN	GPIO_Pin_8
// #define LED2_PIN	GPIO_Pin_9
//#define LED1 PBout(8)
//#define LED2 PBout(9)

/*�����ǰ�ƶ���*/
#define LED1_PIN	GPIO_Pin_6
#define LED2_PIN	GPIO_Pin_7
#define LED1 PBout(6)			
#define LED2 PBout(7)	

//#define LEDTEST PCout(13)

void LED_Init(void);//��ʼ��
	 				    
#endif

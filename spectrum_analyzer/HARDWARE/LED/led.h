/******************** (C) COPYRIGHT 2013 VeryAI Team **************************
 * 作者    ：VeryAI Team
 * 库版本  ：ST3.5.0
 * 实验平台：VeryAI STM32开发板
 * 描述    ：LED驱动代码  
 *
 * 论坛    ：http://www.veryai.com
 * 淘宝    ：http://veryaishop.taobao.com
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
/*开发板灯定义*/
// #define LED1_PIN	GPIO_Pin_8
// #define LED2_PIN	GPIO_Pin_9
//#define LED1 PBout(8)
//#define LED2 PBout(9)

/*光谱仪板灯定义*/
#define LED1_PIN	GPIO_Pin_6
#define LED2_PIN	GPIO_Pin_7
#define LED1 PBout(6)			
#define LED2 PBout(7)	

//#define LEDTEST PCout(13)

void LED_Init(void);//初始化
	 				    
#endif

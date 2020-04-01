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
#include "led.h"   

/*
 * 函数名：LED_Init
 * 描述  ：初始化PB8和PB9为输出口.并使能这两个口的时钟,LED IO初始化
 * 输入  ：无
 * 输出  ：无
 */	    
void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
// 	GPIO_InitTypeDef  GPIOC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//使能PB

	GPIO_InitStructure.GPIO_Pin = LED1_PIN | LED2_PIN;								//端口配置, 推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 			//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 			//IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				     		//根据设定参数初始化PB.8,PB9
	GPIO_SetBits(GPIOB,LED1_PIN);														//PB.8输出高 
	GPIO_SetBits(GPIOB,LED2_PIN);													//PB.8输出高 	
	
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//使能PC
// 	GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_13;								//端口配置, 推挽输出
// 	GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 			//推挽输出
// 	GPIOC_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 			//IO口速度为50MHz
// 	GPIO_Init(GPIOC, &GPIOC_InitStructure);	  				     		//根据设定参数初始化PC.13
// 	GPIO_SetBits(GPIOC,GPIO_Pin_13);														//PC.13输出高 
		
	
}

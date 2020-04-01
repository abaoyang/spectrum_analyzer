/******************** (C) COPYRIGHT 2013 VeryAI Team **************************
 * 作者    ：VeryAI Team
 * 库版本  ：ST3.5.0
 * 实验平台：VeryAI STM32开发板
 * 描述    : 串口驱动代码  
 *
 * 论坛    ：http://www.veryai.com
 * 淘宝    ：http://veryaishop.taobao.com
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "usart.h"	  
 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos 使用	  
#endif  
//u8 rcvfsm = 0,rcvcheckresult = 0,rcvlen = 0,rcvonepayload = 0,payloadlen = 0;
//u8 rcv[USART_REC_LEN];

u8 rx_buff[RX_BUFF_MAX];
u8 ptr_start=0,ptr_end=0;
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
    USART_SendData(USART1,(uint8_t)ch);   
	return ch;
}
#endif 
 
#if EN_USART1_RX   					//如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
//u8 USART_RX_BUF[USART_REC_LEN];//接收缓冲,最大USART_REC_LEN个字节.

//接收状态
//bit15，	接收完成标志,bit14，	接收到0x0d,bit13~0，	接收到的有效字节数目
//u16 USART_RX_STA=0;       	//接收状态标记	  

/*
 * 函数名：uart_init
 * 描述  ：初始化IO 串口1 
 * 输入  ：bound:波特率
 * 输出  ：无
 */	
void uart_init(u32 bound){
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
 	USART_DeInit(USART1);  		//复位串口1
	
	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化PA10

  //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口
#if EN_USART1_RX		  //如果使能了接收  
  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
   
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
#endif
  USART_Cmd(USART1, ENABLE);                    //使能串口 
}

void USART1_Send_One(u8 Data)
{
  USART_SendData(USART1,Data);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC) == RESET);	
}

void USART1_Send_Bytes(u8 *buf, u8 count)
{
  int i;
	for(i=0;i<count;i++)
	{
			USART1_Send_One(buf[i]);
	}
		
}

/**************************实现函数**********************************************
*功    能:		usart1接收一个字节
*********************************************************************************/
u8 usart1_receive(void)
{
	while((USART1->SR&0x20)==0);
	return USART1->DR;
}

/*
 * 函数名：USART1_IRQHandler
 * 描述  ：串口1中断服务程序
 * 输入  ：无
 * 输出  ：无
 */	
void USART1_IRQHandler(void)
{
	u8 tmp_reg=0;
	tmp_reg = USART1->SR;
	if(((tmp_reg&0x20)==0) && (tmp_reg&0x08) != 0)		// fix bug: rxne =0 but overrun bit =1 
	{
		tmp_reg = USART1->SR;
		tmp_reg = USART1->DR;
	}
	tmp_reg = USART1->CR1;
	tmp_reg = USART1->CR2;
	tmp_reg = USART1->CR3;

//	rx_buf_tmp = USART1->DR;

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		rx_buff[ptr_end] = usart1_receive();
		ptr_end++;
		if(ptr_end >= RX_BUFF_MAX)
		{
			ptr_end = 0;
		}
	}
//	u8 Res;
//	u8 i;
//#ifdef OS_TICKS_PER_SEC	 		//如果时钟节拍数定义了,说明要使用ucosII了.
//	OSIntEnter();    
//#endif
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//接收中断(接收到的数据必须是0x55,0xAA开头，0x77结尾，且需要异或校验)
//	{
//		Res =USART_ReceiveData(USART1);//(USART1->DR);读取接收到的数据
//		
//		switch(rcvfsm)
//		{
//			case 0:
//				if(0x55 == Res) 
//				{
//					rcvfsm = 1;
//					rcvlen ++;
//				}
//			  break;
//			case 1:
//				if(0xAA == Res)
//				{
//					rcvfsm = 2;
//					rcvlen ++;
//				  rcv[0] = 0x55;
//				  rcv[1] = 0xAA;
//				}
//			  else 
//				{
//					rcvfsm = 0;
//					rcvlen = 0;
//				}
//				break;
//			case 2:
//				if((0x77 == Res)&& (rcvlen>4))
//				{
//					if(
//						((rcvlen==5)&&((rcv[3]==0x11)||(rcv[3]==0x12)||(rcv[3]==0x13)||(rcv[3]==0x14)||(rcv[3]==0x15)||(rcv[3]==0x16)||
//						(rcv[3]==0x17)||(rcv[3]==0x18)||(rcv[3]==0x19)||(rcv[3]==0x1a)||(rcv[3]==0x22)||(rcv[3]==0x31)||(rcv[3]==0x32)||
//					 (rcv[3]==0x33)||(rcv[3]==0x34)))
//					||((rcvlen==6)&&(rcv[3]==0x43))
//					||((rcvlen==7)&&((rcv[3]==0x03)||(rcv[3]==0x05)||(rcv[3]==0x06)||(rcv[3]==0x07)||(rcv[3]==0x08)||(rcv[3]==0x09)||
//					(rcv[3]==0x0a)||(rcv[3]==0x24)||(rcv[3]==0x42)||(rcv[3]==0x47)||(rcv[3]==0x61)))
//					||((rcvlen==9)&&((rcv[3]==0x04)||(rcv[3]==0x44)||(rcv[3]==0x45)||(rcv[3]==0x60)))
//					||((rcvlen==11)&&(rcv[3]==0x46))					
//					||((rcvlen==15)&&(rcv[3]==0x23))
//					||((rcvlen==21)&&((rcv[3]==0x01)||(rcv[3]==0x02)||(rcv[3]==0x21)))					
//					)
//					{
//						rcv[rcvlen]=Res;
//						rcvlen ++;
//						for(i = 0;i<(rcvlen-2);i++)
//						{
//							rcvcheckresult = rcvcheckresult^rcv[i];
//						}
//						if (rcvcheckresult == rcv[rcvlen-2])
//						{
//								rcvonepayload = 1;
//								payloadlen=rcvlen;
//								rcvcheckresult = 0;
//								rcvfsm = 0;
//								rcvlen = 0;
//						}
//						else
//						{
//							rcvcheckresult = 0;
//							rcvfsm = 0;
//							rcvlen = 0;		
//						}		
//					}
//					else
//					{
//						rcv[rcvlen]=Res;
//						rcvlen ++;
//						if(rcvlen>22)
//						{
//							rcvcheckresult = 0;
//							rcvfsm = 0;
//							rcvlen = 0;	
//						}
//					}				
//				}				
//				
//				else
//				{
//					rcv[rcvlen]=Res;
//					rcvlen ++;
//				}
//				break;
//			
//			default:
//				break;				
//				
//		}   
//  } 
//#ifdef OS_TICKS_PER_SEC	 		//如果时钟节拍数定义了,说明要使用ucosII了.
//	OSIntExit();  											 
//#endif
} 
#endif	


/******************** (C) COPYRIGHT 2013 VeryAI Team **************************
 * ����    ��VeryAI Team
 * ��汾  ��ST3.5.0
 * ʵ��ƽ̨��VeryAI STM32������
 * ����    : ������������  
 *
 * ��̳    ��http://www.veryai.com
 * �Ա�    ��http://veryaishop.taobao.com
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sys.h"
#include "usart.h"	  
 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��	  
#endif  
//u8 rcvfsm = 0,rcvcheckresult = 0,rcvlen = 0,rcvonepayload = 0,payloadlen = 0;
//u8 rcv[USART_REC_LEN];

u8 rx_buff[RX_BUFF_MAX];
u8 ptr_start=0,ptr_end=0;
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
    USART_SendData(USART1,(uint8_t)ch);   
	return ch;
}
#endif 
 
#if EN_USART1_RX   					//���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
//u8 USART_RX_BUF[USART_REC_LEN];//���ջ���,���USART_REC_LEN���ֽ�.

//����״̬
//bit15��	������ɱ�־,bit14��	���յ�0x0d,bit13~0��	���յ�����Ч�ֽ���Ŀ
//u16 USART_RX_STA=0;       	//����״̬���	  

/*
 * ��������uart_init
 * ����  ����ʼ��IO ����1 
 * ����  ��bound:������
 * ���  ����
 */	
void uart_init(u32 bound){
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
 	USART_DeInit(USART1);  		//��λ����1
	
	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��PA10

  //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������
#if EN_USART1_RX		  //���ʹ���˽���  
  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
   
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
#endif
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
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

/**************************ʵ�ֺ���**********************************************
*��    ��:		usart1����һ���ֽ�
*********************************************************************************/
u8 usart1_receive(void)
{
	while((USART1->SR&0x20)==0);
	return USART1->DR;
}

/*
 * ��������USART1_IRQHandler
 * ����  ������1�жϷ������
 * ����  ����
 * ���  ����
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
//#ifdef OS_TICKS_PER_SEC	 		//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
//	OSIntEnter();    
//#endif
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//�����ж�(���յ������ݱ�����0x55,0xAA��ͷ��0x77��β������Ҫ���У��)
//	{
//		Res =USART_ReceiveData(USART1);//(USART1->DR);��ȡ���յ�������
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
//#ifdef OS_TICKS_PER_SEC	 		//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
//	OSIntExit();  											 
//#endif
} 
#endif	


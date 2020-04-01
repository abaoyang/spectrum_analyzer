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
#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
	
//#define USART_REC_LEN	200  	//�����������ֽ��� 200
//#define USART_REC_LEN	30
#define EN_USART1_RX 	1			//ʹ�ܣ�1��/��ֹ��0������1����

#define RX_BUFF_MAX					128

	  	
//extern u8  USART_RX_BUF[USART_REC_LEN];//���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
//extern u16 USART_RX_STA;    //����״̬���	

//extern u8 rcv[USART_REC_LEN];
//extern u8 payloadlen;
//extern u8 rcvonepayload;
//extern u8 rcvfsm,rcvcheckresult,rcvlen,rcvonepayload ,payloadlen;

extern u8 rx_buff[RX_BUFF_MAX];
extern u8 ptr_start;
extern u8 ptr_end;


//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);
void USART1_Send_One(u8 Data);
void USART1_Send_Bytes(u8 *buf, u8 count);
#endif



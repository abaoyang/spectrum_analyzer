/******************** (C) COPYRIGHT 2013 VeryAI Team **************************
 * 作者    ：VeryAI Team
 * 库版本  ：ST3.5.0
 * 实验平台：VeryAI STM32开发板
 * 描述    ：ADC实验
 *
 * 论坛    ：http://www.veryai.com
 * 淘宝    ：http://veryaishop.taobao.com
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "flashrw.h"
#include "fpga.h"
#define SN_ADRR	0X0			//16byte
#define FW_ADRR 0X10		//16byte
#define PIXNUM_ADRR	0X20	//2byte
#define WAVELEN_ADRR 0X22		//4byte
#define GRAT_ADRR 0X26		//2byte
#define FILTER_ADRR 0X28	//2byte
#define SLIT_ADRR 0X2A   //2byte
#define ARRAYCM_ADRR 0X2C		//2byte
#define ARRAYWL_ADRR  0X2E  //2byte
#define DETECTOR_ADRR  0X30 //1byte
#define ABCD_ADRR	0X32	//16byte
#define CP_ADRR 0X42		//1024*8byte

#define STATE_IDLE							0
#define STATE_RX_HDR1						1
#define STATE_RX_HDR2						2
#define STATE_RX_SERNO					3
#define STATE_RX_CMD						4
#define STATE_RX_SEQ						5
#define STATE_RX_PAYLOAD				6
#define STATE_RX_PARITY					7

#define USART_REC_LEN	30
#define FALSE										0
#define TRUE										1

u8 rcv[USART_REC_LEN];

u8 rx_buf_tmp=0;
u8 rcvfsm = STATE_IDLE,syn_state=FALSE,rcvcheckresult = 0,rcvlen = 0,rcvonepayload = 0,payloadlen = 0,payload_num=0;

u8 flashrd[16];
u8 flashwr[16];

u16 fpgaadd;
u16 fpgadata;
u16 fpgadatahigh;

u16 pix_position;
u16 pix_number = 1024;  //总体像素个数
u16 rawdatabuf[1024];
u32 maxintgvalue;


u8 cmdreturn[30];
u8 workmode = 2;			//初始设计时考虑通过FPGA设置和读取工作模式，后直接在MCU中设置，默认单次模式=2
u8 nostop = 0;
u16 trigdelay;






enum PCCMD
{
	SETSN = 0X01,
	SETFW = 0X02,
	SETPIXNUM = 0X03,
	SETWAVELEN	= 0X04,
	SETGRAT	= 0X05,
	SETFILTER = 0X06,
	SETSLIT = 0X07,
	SETARRAYCM=0X08,
	SETARRAYWL = 0X09,
	SETDETECTOR =0X0A,
	ERASESET = 0X0B,
	READSN = 0X11,
	READFW = 0X12,
	READPIXNUM = 0X13,
	READWAVELEN = 0X14,
	READGRAT = 0X15,
	READFILTER = 0X16,
	READSLIT = 0X17,
	READARRAYCM=0X18,
	READARRAYWL =0X19,
	READDETECTOR =0X1A,
	WRITECALPO = 0X21,
	READCALPO = 0X22,
	WRITECP = 0X23,
	READCP = 0X24,
	READTEMPINFO = 0X31,
	READTEMPSLOT = 0X32,
	READWORKMODE = 0X33,
	READTRIGDELAY	= 0X34,
	SETTEMPSLOT =0X42,
	SETWORKMODE = 0X43,
	SETTRIGDELAY = 0X44,
	SETALLINTGTIME = 0X45,
	SETSINGLEINTGTIME = 0X46,
	CALLDATA = 0X47,
	STARTCOLLECT = 0X48,
	STOPCOLLECT = 0X49,
	
	
	FPGAWRITECMD =0X60,
	FPGAREADCMD =0X61
};


 
 /*
 * 函数名：main
 * 描述  ：
 * 输入  ：无
 * 输出  ：无
 */	
int main(void)
{	 
  u16 i,j;	
	u8 sendcheckresult;
	u16 tempdata;
	enum PCCMD cmdno;
	delay_init();	    	 			//延时函数初始化	  
	NVIC_Configuration(); 	 	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级	
	uart_init(115200);
 	LED_Init();			     			//LED端口初始化
	FPGA_EXTI_Init();
	FPGA_Init();
	FPGA_Reset();  
	cmdreturn[0] = 0xaa;   //光谱仪发往上位机的数据头0xaa,0x55
	cmdreturn[1] = 0x55;	

	
	while(1)
	{
		if(ptr_end != ptr_start)
			{
				rx_buf_tmp = rx_buff[ptr_start];
				ptr_start++;
				if(ptr_start>=RX_BUFF_MAX)
				{
					ptr_start = 0;
				}
			
				switch(rcvfsm)
				{
					case STATE_IDLE:
						if(0x55 == rx_buf_tmp)
						{
							rcvfsm = STATE_RX_HDR1;
							rcvcheckresult = rcvcheckresult ^ rx_buf_tmp;
							rcv[0] = 0x55;
						}
						else
						{
							syn_state = FALSE;
							rcvfsm = STATE_IDLE;
							rcvcheckresult = 0;
						}
					break;
						
					case STATE_RX_HDR1:
						if(0xAA == rx_buf_tmp)
						{
							rcvfsm = STATE_RX_HDR2;
							rcvcheckresult = rcvcheckresult ^ rx_buf_tmp;
							rcv[1] = 0xAA;
						}
						else
						{
							syn_state = FALSE;
							rcvfsm = STATE_IDLE;
							rcvcheckresult = 0;
						}
					break;
					case STATE_RX_HDR2:
						rcv[2] = rx_buf_tmp;
					  rcvfsm = STATE_RX_SERNO;
					  rcvcheckresult = rcvcheckresult ^ rx_buf_tmp;
					break;
					case STATE_RX_SERNO:
						rcv[3] = rx_buf_tmp;
					  rcvcheckresult = rcvcheckresult ^ rx_buf_tmp;
					  if((rcv[3]==0x0B)||(rcv[3]==0x11)||(rcv[3]==0x12)||(rcv[3]==0x13)||(rcv[3]==0x14)||(rcv[3]==0x15)||(rcv[3]==0x16)||
								(rcv[3]==0x17)||(rcv[3]==0x18)||(rcv[3]==0x19)||(rcv[3]==0x1a)||(rcv[3]==0x22)||(rcv[3]==0x31)||(rcv[3]==0x32)||
							 (rcv[3]==0x33)||(rcv[3]==0x34)||(rcv[3]==0x48)||(rcv[3]==0x49))
						{
               rcvfsm = STATE_RX_PAYLOAD;		
							 payloadlen = 0;
						}
					  else if((rcv[3]==0x43)||(rcv[3]==0x0a))
						{
               rcvfsm = STATE_RX_SEQ;		
							 payloadlen = 1;
						}	
						else if((rcv[3]==0x03)||(rcv[3]==0x05)||(rcv[3]==0x06)||(rcv[3]==0x07)||(rcv[3]==0x08)||(rcv[3]==0x09)||
							(rcv[3]==0x0a)||(rcv[3]==0x24)||(rcv[3]==0x42)||(rcv[3]==0x47)||(rcv[3]==0x61))
						{
               rcvfsm = STATE_RX_SEQ;		
							 payloadlen = 2;
						}	
						else if((rcv[3]==0x04)||(rcv[3]==0x44)||(rcv[3]==0x45)||(rcv[3]==0x60))
						{
               rcvfsm = STATE_RX_SEQ;		
							 payloadlen = 4;
						}
						else if(rcv[3]==0x46)
						{
               rcvfsm = STATE_RX_SEQ;		
							 payloadlen = 6;
						}	
						else if(rcv[3]==0x23)
						{
               rcvfsm = STATE_RX_SEQ;		
							 payloadlen = 10;
						}		
						else if((rcv[3]==0x01)||(rcv[3]==0x02)||(rcv[3]==0x21))
						{
               rcvfsm = STATE_RX_SEQ;		
							 payloadlen = 16;
						}
						else
						{
							syn_state = FALSE;
							rcvfsm = STATE_IDLE;
							rcvcheckresult = 0;						
						} 						
					break;
					case 	STATE_RX_SEQ:
						if(payload_num<payloadlen)
						{
							rcvfsm = STATE_RX_SEQ;	
							rcv[payload_num+4] = rx_buf_tmp;
							rcvcheckresult = rcvcheckresult ^ rx_buf_tmp;
						}
						payload_num++;
						if(payload_num == payloadlen)
						{
							payload_num = 0;
							rcvfsm = STATE_RX_PAYLOAD;
						}
					break;
					case STATE_RX_PAYLOAD:
						if(rcvcheckresult == rx_buf_tmp)
						{
							rcv[payloadlen+4] = rx_buf_tmp;
							rcvfsm = STATE_RX_PARITY;
//							syn_state = TRUE;
							rcvcheckresult = 0;
						}
						else
						{
							syn_state = FALSE;
							rcvfsm = STATE_IDLE;
							rcvcheckresult = 0;
						}
					break;
				case STATE_RX_PARITY:
						if(0x77 == rx_buf_tmp)
						{
							rcv[payloadlen+4+1] = rx_buf_tmp;
							rcvfsm = STATE_IDLE;
							syn_state = TRUE;
							rcvcheckresult = 0;
						}
						else
						{
							syn_state = FALSE;
							rcvfsm = STATE_IDLE;
							rcvcheckresult = 0;
						}
					break;
						default:
							rcvfsm = STATE_IDLE;
							rcvcheckresult = 0;
							syn_state = FALSE;
						break;	

						

//此后的串口数据解析方法证明亦可用
//					case 0:
//						if(0x55 == rx_buf_tmp) 
//						{
//							rcvfsm = 1;
//							rcvlen ++;
//						}
//						break;
//					case 1:
//						if(0xAA == rx_buf_tmp)
//						{
//							rcvfsm = 2;
//							rcvlen ++;
//							rcv[0] = 0x55;
//							rcv[1] = 0xAA;
//						}
//						else 
//						{
//							rcvfsm = 0;
//							rcvlen = 0;
//						}
//						break;



//					case 2:
//						if((0x77 == rx_buf_tmp)&& (rcvlen>4))
//						{
//							if(
//								((rcvlen==5)&&((rcv[3]==0x11)||(rcv[3]==0x12)||(rcv[3]==0x13)||(rcv[3]==0x14)||(rcv[3]==0x15)||(rcv[3]==0x16)||
//								(rcv[3]==0x17)||(rcv[3]==0x18)||(rcv[3]==0x19)||(rcv[3]==0x1a)||(rcv[3]==0x22)||(rcv[3]==0x31)||(rcv[3]==0x32)||
//							 (rcv[3]==0x33)||(rcv[3]==0x34)))
//							||((rcvlen==6)&&(rcv[3]==0x43))
//							||((rcvlen==7)&&((rcv[3]==0x03)||(rcv[3]==0x05)||(rcv[3]==0x06)||(rcv[3]==0x07)||(rcv[3]==0x08)||(rcv[3]==0x09)||
//							(rcv[3]==0x0a)||(rcv[3]==0x24)||(rcv[3]==0x42)||(rcv[3]==0x47)||(rcv[3]==0x61)))
//							||((rcvlen==9)&&((rcv[3]==0x04)||(rcv[3]==0x44)||(rcv[3]==0x45)||(rcv[3]==0x60)))
//							||((rcvlen==11)&&(rcv[3]==0x46))					
//							||((rcvlen==15)&&(rcv[3]==0x23))
//							||((rcvlen==21)&&((rcv[3]==0x01)||(rcv[3]==0x02)||(rcv[3]==0x21)))					
//							)
//							{
//								rcv[rcvlen]=rx_buf_tmp;
//								rcvlen ++;
//								for(i = 0;i<(rcvlen-2);i++)
//								{
//									rcvcheckresult = rcvcheckresult^rcv[i];
//								}
//								if (rcvcheckresult == rcv[rcvlen-2])
//								{
//										rcvonepayload = 1;
//										payloadlen=rcvlen;
//										rcvcheckresult = 0;
//										rcvfsm = 0;
//										rcvlen = 0;
//								}
//								else
//								{
//									rcvcheckresult = 0;
//									rcvfsm = 0;
//									rcvlen = 0;		
//								}		
//							}
//							else
//							{
//								rcv[rcvlen]=rx_buf_tmp;
//								rcvlen ++;
//								if(rcvlen>22)
//								{
//									rcvcheckresult = 0;
//									rcvfsm = 0;
//									rcvlen = 0;	
//								}
//							}				
//						}				
//						
//						else
//						{
//							rcv[rcvlen]=rx_buf_tmp;
//							rcvlen ++;
//							if(rcvlen>22)
//								{
//									rcvcheckresult = 0;
//									rcvfsm = 0;
//									rcvlen = 0;	
//								}
//						}
//						break;
					
//					default:
//						rcvfsm = 0;
//						rcvcheckresult = 0;
//						rcvonepayload = 0;
//						break;				
				} 
			}				
		
		if(syn_state == TRUE)				//对成功解析出的一个数据包进行处理。
//		if(rcvonepayload == 1)
		{
			syn_state=FALSE;
//			rcvonepayload = 0;
//			payloadlen = 0;		
			sendcheckresult = 0;
			cmdreturn[2] = rcv[2];   //光谱仪返回上位机的包序列号，将串口收到的包序号原封不动发回上位机
			cmdno = rcv[3];			
			cmdreturn[3] = cmdno;    //光谱仪返回上位机命令号
			switch(cmdno)
			{
				case SETSN:
					for(i=0;i<16;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(SN_ADRR, flashwr, 16);
					
					Flash_Read(SN_ADRR, flashrd, 16);
					for(i=0;i<16;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<16)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
						break;
					
				case SETFW:
					
					for(i=0;i<16;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(FW_ADRR, flashwr, 16);
					
					Flash_Read(FW_ADRR, flashrd, 16);
					for(i=0;i<16;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<16)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
						break;
					
				case 	SETPIXNUM:
					for(i=0;i<2;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(PIXNUM_ADRR, flashwr, 2);
					
					Flash_Read(PIXNUM_ADRR, flashrd, 2);
					for(i=0;i<2;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<2)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	SETWAVELEN:
					for(i=0;i<4;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(WAVELEN_ADRR, flashwr, 4);
					
					Flash_Read(WAVELEN_ADRR, flashrd, 4);
					for(i=0;i<4;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<4)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	SETGRAT:
					for(i=0;i<2;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(GRAT_ADRR, flashwr, 2);
					
					Flash_Read(GRAT_ADRR, flashrd, 2);
					for(i=0;i<2;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<2)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	SETFILTER:
					for(i=0;i<2;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(FILTER_ADRR, flashwr, 2);
					
					Flash_Read(FILTER_ADRR, flashrd, 2);
					for(i=0;i<2;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<2)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	SETSLIT:
					for(i=0;i<2;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(SLIT_ADRR, flashwr, 2);
					
					Flash_Read(SLIT_ADRR, flashrd, 2);
					for(i=0;i<2;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<2)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	SETARRAYCM:
					for(i=0;i<2;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(ARRAYCM_ADRR, flashwr, 2);
					
					Flash_Read(ARRAYCM_ADRR, flashrd, 2);
					for(i=0;i<2;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<2)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	SETARRAYWL:
					for(i=0;i<2;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(ARRAYWL_ADRR, flashwr, 2);
					
					Flash_Read(ARRAYWL_ADRR, flashrd, 2);
					for(i=0;i<2;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<2)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	SETDETECTOR:
					for(i=0;i<1;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(DETECTOR_ADRR, flashwr, 1);
					
					Flash_Read(DETECTOR_ADRR, flashrd, 1);
					for(i=0;i<1;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<1)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case ERASESET:					
					cmdreturn[4] = Flash_Erase_Paraset(SN_ADRR,6);
				  for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					break;
				
				case 	READSN:
					Flash_Read(SN_ADRR, flashrd, 16);
				  for(i=0;i<16;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(16+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[16+4]= sendcheckresult;
					cmdreturn[16+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(16+4+2));
					
					break;
				case 	READFW:
					Flash_Read(FW_ADRR, flashrd, 16);
				  for(i=0;i<16;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(16+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[16+4]= sendcheckresult;
					cmdreturn[16+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(16+4+2));
					
					break;
					
				case 	READPIXNUM:
					Flash_Read(PIXNUM_ADRR, flashrd, 2);
				  for(i=0;i<2;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(2+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[2+4]= sendcheckresult;
					cmdreturn[2+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(2+4+2));
					
					break;
					
				case 	READWAVELEN:
					Flash_Read(WAVELEN_ADRR, flashrd, 4);
				  for(i=0;i<4;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(4+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[4+4]= sendcheckresult;
					cmdreturn[4+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(4+4+2));
					
					break;
				case 	READGRAT:
					Flash_Read(GRAT_ADRR, flashrd, 2);
				  for(i=0;i<2;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(2+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[2+4]= sendcheckresult;
					cmdreturn[2+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(2+4+2));
					
					break;
				case 	READFILTER:
					Flash_Read(FILTER_ADRR, flashrd, 2);
				  for(i=0;i<2;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(2+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[2+4]= sendcheckresult;
					cmdreturn[2+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(2+4+2));
					
					break;
				case 	READSLIT:
					Flash_Read(SLIT_ADRR, flashrd, 2);
				  for(i=0;i<2;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(2+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[2+4]= sendcheckresult;
					cmdreturn[2+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(2+4+2));
					
					break;
				case 	READARRAYCM:
					Flash_Read(ARRAYCM_ADRR, flashrd, 2);
				  for(i=0;i<2;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(2+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[2+4]= sendcheckresult;
					cmdreturn[2+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(2+4+2));
					
					break;
				case 	READARRAYWL:
					Flash_Read(ARRAYWL_ADRR, flashrd, 2);
				  for(i=0;i<2;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(2+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[2+4]= sendcheckresult;
					cmdreturn[2+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(2+4+2));
					
					break;
				case READDETECTOR:
					Flash_Read(DETECTOR_ADRR, flashrd, 1);
				  for(i=0;i<1;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(1+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[1+4]= sendcheckresult;
					cmdreturn[1+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(1+4+2));
					break;					
					
				case 	WRITECALPO:
					for(i=0;i<16;i++)
					{
						flashwr[i]=rcv[4+i];
					}					
					Flash_Write_Without_check(ABCD_ADRR, flashwr, 16);	//写入ABCD四个float数，按字节收到按字节写
					
					Flash_Read(ABCD_ADRR, flashrd, 16);
					for(i=0;i<16;i++)
					{
						if(flashrd[i] != rcv[4+i]) break;
					}
					if(i<16)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
									
					break;
				case 	READCALPO:
					Flash_Read(ABCD_ADRR, flashrd, 16);
				  for(i=0;i<16;i++)
					{
						cmdreturn[4+i]=flashrd[i];
					}
					for(i=0;i<(16+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[16+4]= sendcheckresult;
					cmdreturn[16+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(16+4+2));
					
					break;
				case 	WRITECP:
					pix_position = (((u16)rcv[4])<<8)|((u16)rcv[5]&0x00ff);
					for(i=0;i<8;i++)
					{
						flashwr[i]=rcv[6+i];
					}					
					Flash_Write_Without_check((CP_ADRR+pix_position*8), flashwr, 8);	//写入ABCD四个float数，按字节收到按字节写
					
					Flash_Read((CP_ADRR+pix_position*8), flashrd, 8);
					for(i=0;i<8;i++)
					{
						if(flashrd[i] != rcv[6+i]) break;
					}
					if(i<8)
						cmdreturn[4] = 0;
					else
						cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	READCP:
					cmdreturn[4] = rcv[4];
				  cmdreturn[5] = rcv[5];
					pix_position = (((u16)rcv[4])<<8)|((u16)rcv[5]&0x00ff);
					Flash_Read((CP_ADRR+pix_position*8), flashrd, 8);
				  for(i=0;i<8;i++)
					{
						cmdreturn[6+i]=flashrd[i];
					}
					for(i=0;i<(10+4);i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];					
					}
					cmdreturn[10+4]= sendcheckresult;
					cmdreturn[10+4+1]= 0xee;
					USART1_Send_Bytes(cmdreturn,(10+4+2));
					
					break;
				case 	READTEMPINFO:
					
					break;
				case 	READTEMPSLOT:
					
					break;
				case 	READWORKMODE:
//					fpgaadd = TRIGMODEREG;
//					fpgadata = FPGA_Read(fpgaadd); 
//				  cmdreturn[4] = (u8)((fpgadata&0xf000)>>12);
				  cmdreturn[4] = workmode;
				  for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	READTRIGDELAY:
					fpgaadd = TRIGMODEREG;
				  fpgadata = FPGA_Read(fpgaadd);
				  cmdreturn[4]= (u8)((fpgadata&0x0f00)>>8);
				  cmdreturn[5]= (u8)(fpgadata&0x00ff);
				  for(i=0;i<6;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[6]= sendcheckresult;
					cmdreturn[7]= 0xee;
					USART1_Send_Bytes(cmdreturn,8);
					
					break;
				case 	SETTEMPSLOT:
					
					break;
				case 	SETWORKMODE:
					if(rcv[4] == 1 || rcv[4] ==2)		//只有单次模式2和连续模式1两种，出现其他数字返回设置错误
					{
						workmode = rcv[4];
						if(workmode == 2)
							nostop = 0;
//				  fpgaadd = TRIGMODEREG;
//				  fpgadata = FPGA_Read(fpgaadd); 
//				  fpgadata=((u16)workmode<<12)|(fpgadata&0x0fff);
//				  FPGA_Write(fpgaadd,fpgadata);
//				  fpgadata=FPGA_Read(fpgaadd);
 				  
						cmdreturn[4] = 1;
					}
					else
						cmdreturn[4] = 0;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	SETTRIGDELAY:
					fpgaadd = TRIGMODEREG;				  
					trigdelay = (((u16)rcv[4]<<8)&0xff00)|((u16)rcv[5]&0x00ff);
				  fpgadata = FPGA_Read(fpgaadd);
				  fpgadata=(fpgadata&0xf000)|(trigdelay&0x0fff);
				  FPGA_Write(fpgaadd,fpgadata);
				  fpgadata=FPGA_Read(fpgaadd);
				  if(((u8)((fpgadata&0x0f00)>>8)==(rcv[4]&0x0f))&&((u8)(fpgadata&0x00ff)==rcv[5]))
						cmdreturn[4] = 1;
					else
						cmdreturn[4] = 0;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
				  
					break;
				case 	SETALLINTGTIME:
					fpgaadd = 0x0800;					
				  fpgadatahigh = (((u16)rcv[4]<<8)&0xff00)|((u16)rcv[5]&0x00ff);
				  fpgadata = (((u16)rcv[6]<<8)&0xff00)|((u16)rcv[7]&0x00ff);
				  for (i=0;i<pix_number;i++)
					{
						FPGA_Write(fpgaadd+2*i+1,fpgadatahigh);
						tempdata = FPGA_Read(fpgaadd+2*i+1);						
						if(tempdata == fpgadatahigh)						
							cmdreturn[4] = 1;
						else 
							cmdreturn[4] = 0;
						FPGA_Write(fpgaadd+2*i,fpgadata);
						tempdata = FPGA_Read(fpgaadd+2*i);	
						if(tempdata == fpgadata)						
							cmdreturn[4] = 1;
						else 
							cmdreturn[4] = 0;
					}
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					
					break;
				case 	SETSINGLEINTGTIME:
					fpgaadd = (((u16)rcv[4]<<8)&0xff00)|((u16)rcv[5]&0x00ff);
				  fpgadatahigh = (((u16)rcv[6]<<8)&0xff00)|((u16)rcv[7]&0x00ff);
				  fpgadata = (((u16)rcv[8]<<8)&0xff00)|((u16)rcv[9]&0x00ff);
				  if(maxintgvalue<(((((u32)fpgadatahigh)<<16)&0xffff0000)|(((u32)fpgadata)&0x0000ffff)))
						maxintgvalue=(((((u32)fpgadatahigh)<<16)&0xffff0000)|(((u32)fpgadata)&0x0000ffff));    		//判断当前写入的积分时间是否比最大积分时间大，如果大，将最大积分时间用当前值代替。
				
				  FPGA_Write(fpgaadd+1,fpgadatahigh);				  
				  FPGA_Write(fpgaadd,fpgadata);				
				  tempdata=FPGA_Read(fpgaadd+1);
				  if(tempdata == fpgadatahigh)
					{
						tempdata=FPGA_Read(fpgaadd);
						if(tempdata == fpgadata)						
							cmdreturn[4] = 1;
						else 
							cmdreturn[4] = 0;
					}
					else 
						cmdreturn[4] = 0;				
				  
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);
					
					break;
				case 	CALLDATA:
					i = (((u16)rcv[4]<<8)&0xff00)|((u16)rcv[5]&0x00ff);	
				  if(!(i == 0xffff))
					{
						nostop = 0;
						cmdreturn[4] =rcv[4];
						cmdreturn[5] =rcv[5];
						cmdreturn[6] =(u8)((rawdatabuf[i]&0xff00)>>8);
						cmdreturn[7] =(u8)(rawdatabuf[i]&0x00ff);
						for(i=0;i<8;i++)
							{
								sendcheckresult=sendcheckresult^cmdreturn[i];
							}
						cmdreturn[8] = sendcheckresult;
						cmdreturn[9] =	0xee;
						USART1_Send_Bytes(cmdreturn,10);
					}
					else if(workmode == 2)
					{
						nostop = 0;
						for(i=0;i<pix_number;i++)
						{
							cmdreturn[4] =(u8)((i&0xff00)>>8);
							cmdreturn[5] =(u8)(i&0x00ff);
							cmdreturn[6] =(u8)((rawdatabuf[i]&0xff00)>>8);
							cmdreturn[7] =(u8)(rawdatabuf[i]&0x00ff);
							for(j=0;j<8;j++)
							{
								sendcheckresult=sendcheckresult^cmdreturn[j];
							}
							cmdreturn[8] = sendcheckresult;
							cmdreturn[9] =	0xee;
							USART1_Send_Bytes(cmdreturn,10);
						}
					}
					else if(workmode == 1)
					{
						nostop = 1;
					}
					
					break;
						
				case STARTCOLLECT:
				  i = 36000;
					fpgadatahigh = (u16)((maxintgvalue&0xffff0000)>>16);
					fpgadata = (u16)(maxintgvalue&0x0000ffff);
				  FPGA_Write(0x0004,fpgadata);
				  FPGA_Write(0x0005,fpgadatahigh);								//写入最大积分倍（逻辑文档第三步，默认第一步和第二步前面已经完成
					FPGA_Write(0x0003,1);													//触发积分采集过程（逻辑文档第五步）			  
				  
				  while((!((FPGA_Read(0x0009)&0x1000)>>12))&&(--i));					//循环等待当前数据采样完成，等待两秒如无反馈，则退出。				
					if(i==0)						
							cmdreturn[4] = 0;																				//延时2秒左右结束无数据返回，反馈采集失败
					else
					{
						fpgadata = FPGA_Read(0x0008);								//fifo中需要读取的数据个数
						for (i=0;i<fpgadata;i++)
						{
							rawdatabuf[i] = FPGA_Read(0x1000);				//将fifo数据读入单片机数组中
						}
						cmdreturn[4] = 1;
					}
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);	
					
					break;
				
				case STOPCOLLECT:
					
					FPGA_Write(0x0003,0);								//停止积分采集过程（逻辑文档第五步）	
					cmdreturn[4] = 1;
					for(i=0;i<5;i++)
					{
						sendcheckresult=sendcheckresult^cmdreturn[i];
					}
					cmdreturn[5]= sendcheckresult;
					cmdreturn[6]= 0xee;
					USART1_Send_Bytes(cmdreturn,7);					
					
					break;
						
				case	FPGAWRITECMD:
					fpgaadd = (((u16)rcv[4]<<8)&0xff00)|((u16)rcv[5]&0x00ff);
				  fpgadata = (((u16)rcv[6]<<8)&0xff00)|((u16)rcv[7]&0x00ff);
				  FPGA_Write(fpgaadd,fpgadata);
					for(i=0;i<4;i++)
						{
							sendcheckresult=sendcheckresult^cmdreturn[i];
						}
					cmdreturn[4] = sendcheckresult;
					cmdreturn[5] =	0xee;
					USART1_Send_Bytes(cmdreturn,(4+2));
					
					break;
				case	FPGAREADCMD:
					fpgaadd = (((u16)rcv[4]<<8)&0xff00)|((u16)rcv[5]&0x00ff);
				  fpgadata = FPGA_Read(fpgaadd);
				  cmdreturn[4] = (u8)((fpgadata&0xff00)>>8);
				  cmdreturn[5] = (u8)(fpgadata&0x00ff);
					for(i=0;i<6;i++)
							{
								sendcheckresult=sendcheckresult^cmdreturn[i];
							}
					cmdreturn[6] = sendcheckresult;
					cmdreturn[7] =	0xee;	
					USART1_Send_Bytes(cmdreturn,(4+2+2));		
					break;
					
				
				default:
					break;					
			}
		}
		if((workmode == 1)&&(nostop == 1))			//连续模式处理（发出对0xffff采样后）
		{
					for(i=0;i<pix_number;i++)
					{
						cmdreturn[4] =(u8)((i&0xff00)>>8);
						cmdreturn[5] =(u8)(i&0x00ff);
						cmdreturn[6] =(u8)((rawdatabuf[i]&0xff00)>>8);
						cmdreturn[7] =(u8)(rawdatabuf[i]&0x00ff);
						for(j=0;j<8;j++)
						{
							sendcheckresult=sendcheckresult^cmdreturn[j];
						}
						cmdreturn[8] = sendcheckresult;
						cmdreturn[9] =	0xee;
						USART1_Send_Bytes(cmdreturn,10);
					}
					i = 36000;
					FPGA_Write(0x0003,1);													//触发积分采集过程（逻辑文档第五步）			  
				  
				  while((!((FPGA_Read(0x0009)&0x1000)>>12))&&(--i));					//循环等待当前数据采样完成，等待两秒如无反馈，则退出。				
					if(i==0)						
							cmdreturn[4] = 0;																				//延时2秒左右结束无数据返回，反馈采集失败
					else
					{
						fpgadata = FPGA_Read(0x0008);								//fifo中需要读取的数据个数
						for (i=0;i<fpgadata;i++)
						{
							rawdatabuf[i] = FPGA_Read(0x1000);				//将fifo数据读入单片机数组中
						}
					}
		}		
	}
}

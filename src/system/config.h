/*---------------------------------------------------------------------*/
/* --- STC MCU Limited ------------------------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.STCMCU.com --------------------------------------------*/
/* --- Web: www.STCMCUDATA.com  ---------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 如果要在程序中使用此代码,请在程序中注明使用了STC的资料及程序            */
/*---------------------------------------------------------------------*/

#ifndef		__CONFIG_H
#define		__CONFIG_H

//========================================================================
//                               主时钟定义
//========================================================================

// #define MAIN_Fosc		22118400L	//定义主时钟
#define MAIN_Fosc		12000000L	//定义主时钟
//#define MAIN_Fosc		11059200L	//定义主时钟
//#define MAIN_Fosc		 5529600L	//定义主时钟
//#define MAIN_Fosc		24000000L	//定义主时钟


// #define STC8Hxx		//STC8H系列
#define STC8Gxx		//STC8G系列


// #define HAVE_TIMER_2

//========================================================================
//                                头文件
//========================================================================

#include	"type_def.h"
#include	"STC8G.H"

//========================================================================
//                             外部函数和变量声明
//========================================================================


#define	TRUE	1
#define	FALSE	0

//=============================================================

//========================================

//                                           7      6      5      4      3      2      1      0      Reset Value
//sfr IPH   = 0xB7; //中断优先级高位       PPCAH  PLVDH   PADCH   PSH   PT1H    PX1H   PT0H   PX0H    0000,0000
#define		PPCAH	0x80
#define		PLVDH	0x40
#define		PADCH	0x20
#define		PSH		0x10
#define		PT1H	0x08
#define		PX1H	0x04
#define		PT0H	0x02
#define		PX0H	0x01

//外部中断0中断优先级控制
#define 	INT0_Priority(n)			do{if(n == 0) IPH &= ~PX0H, PX0 = 0; \
																if(n == 1) IPH &= ~PX0H, PX0 = 1; \
																if(n == 2) IPH |= PX0H, PX0 = 0; \
																if(n == 3) IPH |= PX0H, PX0 = 1; \
															}while(0)
//外部中断1中断优先级控制
#define 	INT1_Priority(n)			do{if(n == 0) IPH &= ~PX1H, PX1 = 0; \
																if(n == 1) IPH &= ~PX1H, PX1 = 1; \
																if(n == 2) IPH |= PX1H, PX1 = 0; \
																if(n == 3) IPH |= PX1H, PX1 = 1; \
															}while(0)
//定时器0中断优先级控制
#define 	Timer0_Priority(n)		do{if(n == 0) IPH &= ~PT0H, PT0 = 0; \
																if(n == 1) IPH &= ~PT0H, PT0 = 1; \
																if(n == 2) IPH |= PT0H, PT0 = 0; \
																if(n == 3) IPH |= PT0H, PT0 = 1; \
															}while(0)
//定时器1中断优先级控制
#define 	Timer1_Priority(n)		do{if(n == 0) IPH &= ~PT1H, PT1 = 0; \
																if(n == 1) IPH &= ~PT1H, PT1 = 1; \
																if(n == 2) IPH |= PT1H, PT1 = 0; \
																if(n == 3) IPH |= PT1H, PT1 = 1; \
															}while(0)
//串口1中断优先级控制
#define 	UART1_Priority(n)			do{if(n == 0) IPH &= ~PSH, PS = 0; \
																if(n == 1) IPH &= ~PSH, PS = 1; \
																if(n == 2) IPH |= PSH, PS = 0; \
																if(n == 3) IPH |= PSH, PS = 1; \
															}while(0)
//ADC中断优先级控制
#define 	ADC_Priority(n)				do{if(n == 0) IPH &= ~PADCH, PADC = 0; \
																if(n == 1) IPH &= ~PADCH, PADC = 1; \
																if(n == 2) IPH |= PADCH, PADC = 0; \
																if(n == 3) IPH |= PADCH, PADC = 1; \
															}while(0)
//低压检测中断优先级控制
#define 	LVD_Priority(n)				do{if(n == 0) IPH &= ~PLVDH, PADC = 0; \
																if(n == 1) IPH &= ~PLVDH, PADC = 1; \
																if(n == 2) IPH |= PLVDH, PADC = 0; \
																if(n == 3) IPH |= PLVDH, PADC = 1; \
															}while(0)
//CCP/PCA/PWM中断优先级控制
#define 	PCA_Priority(n)				do{if(n == 0) IPH &= ~PPCAH, PPCA = 0; \
																if(n == 1) IPH &= ~PPCAH, PPCA = 1; \
																if(n == 2) IPH |= PPCAH, PPCA = 0; \
																if(n == 3) IPH |= PPCAH, PPCA = 1; \
															}while(0)

#define	Priority_0			0	//中断优先级为 0 级（最低级）
#define	Priority_1			1	//中断优先级为 1 级（较低级）
#define	Priority_2			2	//中断优先级为 2 级（较高级）
#define	Priority_3			3	//中断优先级为 3 级（最高级）


//========================================================================
//                            端口模式设置
//========================================================================

#define	GPIO_PullUp		0	//上拉准双向口
#define	GPIO_HighZ		1	//浮空输入
#define	GPIO_OUT_OD		2	//开漏输出
#define	GPIO_OUT_PP		3	//推挽输出

#define	GPIO_Pin_0		0x01	//IO引脚 Px.0
#define	GPIO_Pin_1		0x02	//IO引脚 Px.1
#define	GPIO_Pin_2		0x04	//IO引脚 Px.2
#define	GPIO_Pin_3		0x08	//IO引脚 Px.3
#define	GPIO_Pin_4		0x10	//IO引脚 Px.4
#define	GPIO_Pin_5		0x20	//IO引脚 Px.5
#define	GPIO_Pin_6		0x40	//IO引脚 Px.6
#define	GPIO_Pin_7		0x80	//IO引脚 Px.7
#define	GPIO_Pin_LOW	0x0F	//IO低4位引脚
#define	GPIO_Pin_HIGH	0xF0	//IO高4位引脚
#define	GPIO_Pin_All	0xFF	//IO所有引脚

#define	GPIO_P0			0		//
#define	GPIO_P1			1
#define	GPIO_P2			2
#define	GPIO_P3			3
#define	GPIO_P4			4
#define	GPIO_P5			5
#define	GPIO_P6			6
#define	GPIO_P7			7

//准双向口
#define  P0_MODE_IO_PU(Pin)				P0M1 &= ~(Pin), P0M0 &= ~(Pin);
#define  P1_MODE_IO_PU(Pin)				P1M1 &= ~(Pin), P1M0 &= ~(Pin);
#define  P2_MODE_IO_PU(Pin)				P2M1 &= ~(Pin), P2M0 &= ~(Pin);
#define  P3_MODE_IO_PU(Pin)				P3M1 &= ~(Pin), P3M0 &= ~(Pin);
#define  P4_MODE_IO_PU(Pin)				P4M1 &= ~(Pin), P4M0 &= ~(Pin);
#define  P5_MODE_IO_PU(Pin)				P5M1 &= ~(Pin), P5M0 &= ~(Pin);
#define  P6_MODE_IO_PU(Pin)				P6M1 &= ~(Pin), P6M0 &= ~(Pin);
#define  P7_MODE_IO_PU(Pin)				P7M1 &= ~(Pin), P7M0 &= ~(Pin);
//高阻输入
#define  P0_MODE_IN_HIZ(Pin)			P0M1 |= (Pin), P0M0 &= ~(Pin);
#define  P1_MODE_IN_HIZ(Pin)			P1M1 |= (Pin), P1M0 &= ~(Pin);
#define  P2_MODE_IN_HIZ(Pin)			P2M1 |= (Pin), P2M0 &= ~(Pin);
#define  P3_MODE_IN_HIZ(Pin)			P3M1 |= (Pin), P3M0 &= ~(Pin);
#define  P4_MODE_IN_HIZ(Pin)			P4M1 |= (Pin), P4M0 &= ~(Pin);
#define  P5_MODE_IN_HIZ(Pin)			P5M1 |= (Pin), P5M0 &= ~(Pin);
#define  P6_MODE_IN_HIZ(Pin)			P6M1 |= (Pin), P6M0 &= ~(Pin);
#define  P7_MODE_IN_HIZ(Pin)			P7M1 |= (Pin), P7M0 &= ~(Pin);
//漏极开路
#define  P0_MODE_OUT_OD(Pin)			P0M1 |= (Pin), P0M0 |= (Pin);
#define  P1_MODE_OUT_OD(Pin)			P1M1 |= (Pin), P1M0 |= (Pin);
#define  P2_MODE_OUT_OD(Pin)			P2M1 |= (Pin), P2M0 |= (Pin);
#define  P3_MODE_OUT_OD(Pin)			P3M1 |= (Pin), P3M0 |= (Pin);
#define  P4_MODE_OUT_OD(Pin)			P4M1 |= (Pin), P4M0 |= (Pin);
#define  P5_MODE_OUT_OD(Pin)			P5M1 |= (Pin), P5M0 |= (Pin);
#define  P6_MODE_OUT_OD(Pin)			P6M1 |= (Pin), P6M0 |= (Pin);
#define  P7_MODE_OUT_OD(Pin)			P7M1 |= (Pin), P7M0 |= (Pin);
//推挽输出
#define  P0_MODE_OUT_PP(Pin)			P0M1 &= ~(Pin), P0M0 |= (Pin);
#define  P1_MODE_OUT_PP(Pin)			P1M1 &= ~(Pin), P1M0 |= (Pin);
#define  P2_MODE_OUT_PP(Pin)			P2M1 &= ~(Pin), P2M0 |= (Pin);
#define  P3_MODE_OUT_PP(Pin)			P3M1 &= ~(Pin), P3M0 |= (Pin);
#define  P4_MODE_OUT_PP(Pin)			P4M1 &= ~(Pin), P4M0 |= (Pin);
#define  P5_MODE_OUT_PP(Pin)			P5M1 &= ~(Pin), P5M0 |= (Pin);
#define  P6_MODE_OUT_PP(Pin)			P6M1 &= ~(Pin), P6M0 |= (Pin);
#define  P7_MODE_OUT_PP(Pin)			P7M1 &= ~(Pin), P7M0 |= (Pin);


//========================================

#define		ENABLE		1
#define		DISABLE		0





#endif

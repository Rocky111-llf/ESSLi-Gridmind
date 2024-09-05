/**
  ******************************************************************************
  * @file    Global.h
  * @author  RZ
  * @brief   Header for System Common
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 GridMind Co. Ltd.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define MAINFPGA_CLK		(100000000L)
#define HB_CLK				(100000000L)

/*System Card and Resource Variables*/
extern S_OPTCARD  		sOpt0;
extern S_DIDOCARD 		sDio0;
extern S_AOCARD 		sAo0;
extern S_ROPCARD		sRop0;
extern S_ROPCARD		sRop1;

extern tVSC_CTL Ctl_VSC1;
extern tLI_CTL Ctl_LI1;

/*系统DIDO控制*/
#define SysinstDIDO 		sDio0
//i急停总开关
#define IsSysEmSw()			(!(SysinstDIDO.Dat->bit.DI0))
//i变压器断路器反馈
#define IsCB()				(SysinstDIDO.Dat->bit.DI1)
//i变压器接触器反馈
#define IsKM()				(SysinstDIDO.Dat->bit.DI2)
//o变压器断路器分励
#define CBCtl(A)			(SysinstDIDO.Cmd->bit.DO0 = (A))
//o变压器接触器控制
#define KMCtl(A)			(SysinstDIDO.Cmd->bit.DO1 = (A))
//o风扇控制
#define SysFanCtl(A)		(SysinstDIDO.Cmd->bit.DO2 = (A))
//o故障指示灯
#define SysFaultLedCtl(A)	(SysinstDIDO.Cmd->bit.DO3 = (A))
//o运行指示灯
#define SysRunLedCtl(A)		(SysinstDIDO.Cmd->bit.DO4 = (A))

/*常量定义*/
#define sqrt3 (1.7320508f)
#define sqrt2 (1.4142136f)
#define OneDivThree (0.33333333f)
#define PI   (3.1415926f)

#define CARD_TYPE_MAIN	0x0
#define CARD_TYPE_OPT	0x1
#define CARD_TYPE_ANA	0x2
#define CARD_TYPE_DIDO	0x3
#define CARD_TYPE_DI	0x4
#define CARD_TYPE_DO	0x5
#define CARD_TYPE_AI	0x6
#define CARD_TYPE_AO	0x7
#define CARD_TYPE_ROP	0x8

extern u32 u32TempGlobal;

//自有定时器结构体定义
typedef struct {
	u16 Cnt;
	u16 Reload;
	u8 Alarm;
} TIMER;


//桥臂电流结构体定义
typedef struct {
	const PID* PIDInit[7];
} PIDTbl_t;

typedef enum{HB_MODE_BLK = 0, HB_MODE_NEG = 1, HB_MODE_POS = 2, HB_MODE_ZERO = 3, HB_MODE_PWM = 4, HB_MODE_RST = 5, HB_MODE_FBLK = 6, HB_MODE_BYP = 7} HB_MODE;

//自有定时器结构体定义
typedef struct {
	u16 Cnt;
	u16 Reload;
	u8 Alarm;
} TIMER_SYS;

//自有BOOL枚举定义
typedef enum {
	BOOL_FALSE = 0, BOOL_TRUE = 1
} BOOL_SYS;

//全字中设置半字，仅用于非FPGA变量
#define SetMSW(A,B)			(((u16*)(&(A)))[1] = (B))
#define SetLSW(A,B)			(((u16*)(&(A)))[0] = (B))
//全字/半字中设置字节
#define SetWordB0(A,B)		(((u8*)(&(A)))[0] = (B))
#define SetWordB1(A,B)		(((u8*)(&(A)))[1] = (B))
#define SetWordB2(A,B)		(((u8*)(&(A)))[2] = (B))
#define SetWordB3(A,B)		(((u8*)(&(A)))[3] = (B))

#define VarDef_(A,B)	A##_##B
#define VarDef(A,B)		VarDef_(A,B)

#define STRLINK_(A,B)	A##B
#define STRLINK(A,B)	STRLINK_(A,B)

//获取结构体偏移量
#define OFFSET_OF(type, member) (unsigned long)(&(((type *)0)->member))

//部分硬件定义
#define GPIO_DEVICE_ID  	XPAR_XGPIOPS_0_DEVICE_ID
#define MIO7_PS_LED			7							//底板PS LED
///SLCR寄存器，绝对地址，
#define SLCR_UNLOCK_ADDR    0xF8000008
#define SLCR_LOCK_ADDR      0xF8000004
#define UNLOCK_KEY          0xDF0D  //解锁码
#define LOCK_KEY            0x767B  //加锁码
//FPGA_RST_CTRL寄存器，绝对地址
#define FPGA_RST_CTRL          0xF8000240
#define PL_RST_MASK            0x01//低四位对应FCLK_RESETN[3:0]
#define PL_CLR_MASK            0x00

extern void BSPInit(u32);
extern void PL_IntrHandler(void);

extern void PEMInit(void);
extern void GetOmegaTheta(tVSC_CTL* tVSCHandler);
/*定时器处理相关*/
extern void TimerProc(TIMER_SYS* timer1);
extern void TimerReload(TIMER_SYS* timer1);

extern void CmdDown(void);
extern void CfgCard(void);
extern void FaultProc(void);

extern BOOL_SYS FANCtl;

//Following System Variables
extern U_RUNLED CabineRunLed;
extern TIMER_SYS MainTimer1;
extern TIMER_SYS MainTimer2;
extern TIMER_SYS MainTimer3;
//正、余弦表
extern const float SINTBL[];
extern const float COSTBL[];

extern S_FPGA_DOWNLINK *hS_FpgaDownLink;
extern S_FPGA_UPLINK *hS_FpgaUpLink;

static INLINE void f32Lim(float * x, float min, float max)
{
	if(*x > max)
		*x = max;
	else if(*x < min)
		*x = min;
}

#define HardLimit(A,min,max)	(((A)>(max))?((A)=(max)):(((A)<(min))?((A)=(min)):((A)=(A))))			//为避免宏函数异常，A/min/max均为变量或常数
#define SetDAC(Value,Max,Handle,Ch)	((Handle).Cmd->bit.AOdat[(Ch)] = (1.0f/(Max)*32768)*(Value))

// 串口屏参数辅助下发定义
extern float ComDatF[];
#define VSC_CTLMODE ComDatF[0]
#define VSC_PREF ComDatF[1]
#define VSC_QREF ComDatF[2]
#define VSC_UACREF ComDatF[3]
#define VSC_UDCREF ComDatF[4]

// 是否启用无功外环定义
#define QVLOOP 

// 是否孤网启动
//#define ISLANDED_START

extern float debug1;
extern float debug2;

#endif



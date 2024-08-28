/**
  ******************************************************************************
  * @file    SuperCapLib.h
  * @author  RZ
  * @brief   Header for SuperCap Control
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
#ifndef _ESSLILIB_H_
#define _ESSLILIB_H_

#define CELL_V_INDEX_NUM	(100)
#define CELL_T_INDEX_NUM	(100)

//系统运行状态枚举定义，空闲、启动、运行、停机、旁路、故障
#ifndef SYSSTATUS_DEFINED
#define SYSSTATUS_DEFINED
typedef enum {
	IDLE = 0, START = 1, RUN = 2, STOP = 3, MAINTANCE = 5,_FAULT = 0xFF
} SYSSTATUS;
#endif

//铅炭电池工作模式：充放电模式、功率模式
typedef enum{IDLE_LI = 0, CH_DISCH_LI = 1, PQ_LI = 2} CTLMODE_LI;

//铅炭电池充放电模式：空闲、功率模式、涓流、恒流、恒压、浮充
typedef enum{CHG_IDLE_LI = 0, CHG_PQ_LI = 1, CHG_TC_LI = 2, CHG_CC_LI = 3, CHG_CV_LI = 4, CHG_OK_LI = 5} CHGSTS_LI;

//禁充禁放状态,禁充、禁放、禁止充放
typedef enum{CHDISALLOW_LI = 0,CHnALLOW_LI = 1, DISCHnALLOW_LI = 2, CHDISnALLOW_LI = 3} CHDISHEN_LI;
/**********故障定义区************/
//故障定义
#define ERRLI_OV_SW				(1<<0)		   //bit0: 软件输出过压
#define ERRLI_OC_SW				(1<<1)         //bit1: 软件输出过流
#define ERRLI_BMS_COM			(1<<2)         //bit2: BMS通信故障
#define ERRLI_BMS_ALARM			(1<<3)         //bit3: BMS告警
#define ERRLI_VSC_SW			(1<<4)         //bit4: VSC故障
#define ERRLI_BMS_HW			(1<<5)         //bit5: BMS干接点故障
#define ERRLI_EMER_HW			(1<<6)         //bit6: 急停
#define ERRLI_STARTFAIL			(1<<7)         //bit7: 启动失败
#define ERRLI_RSV4_SW			(1<<8)         //bit8: 保留
#define ERRLI_RSV5_SW			(1<<9)         //bit9: 保留
#define ERRLI_RSV6_SW			(1<<10)        //bit10:保留
#define ERRLI_RSV7_SW			(1<<11)        //bit11:保留
#define ERRLI_RSV8_SW			(1<<12)        //bit12:保留

#define ERRLI0_EN				1
#define ERRLI1_EN				1
#define ERRLI2_EN				1
#define ERRLI3_EN				1
#define ERRLI4_EN				1
#define ERRLI5_EN				1
#define ERRLI6_EN				1
#define ERRLI7_EN				1
#define ERRLI8_EN				0
#define ERRLI9_EN				0
#define ERRLI10_EN				0
#define ERRLI11_EN				0
#define ERRLI12_EN				0
#define ERRLI13_EN				0
#define ERRLI14_EN				0
#define ERRLI15_EN				0

#define ERRLIx_EN_MASK		(ERRLI0_EN+(ERRLI1_EN<<1)+(ERRLI2_EN<<2)+(ERRLI3_EN<<3)+(ERRLI4_EN<<4)+(ERRLI5_EN<<5)+(ERRLI6_EN<<6)+(ERRLI7_EN<<7)+(ERRLI8_EN<<8)+(ERRLI9_EN<<9)+(ERRLI10_EN<<10)+(ERRLI11_EN<<11)+(ERRLI12_EN<<12)+(ERRLI13_EN<<13)+(ERRLI14_EN<<14)+(ERRLI15_EN<<15))
/**********故障定义区结束************/
typedef struct
{
	float SOC;
	float SOH;
	float StackV;
	float StackI;
	float CellVMax;
	float CellVMin;
	float CellTMax;
	float CellTMin;
	float RatedV;
	float RatedAh;
	float MaxChgI;
	float MaxDischI;
	float MaxChgP;
	float MaxDischP;
	u16 CellVMaxNO;
	u16 CellVMinNO;
	u16 CellTMaxNO;
	u16 CellTMinNO;
	u16 StringNum;
	u16 CycleNum;
	u8 ChgAllow;
	u8 DischAllow;
	u8 ChgErrStage;
	u8 ChgErrCode;
	u8 DischErrStage;
	u8 DischErrCode;
	u8 ChgStatus;
	u8 OtherErrCode;

	float CELLV[CELL_V_INDEX_NUM];		//电芯电压
	s8 CELLT[CELL_T_INDEX_NUM];			//电芯温度

	u8 HeartBeat;
	u8 HeartBeatPre;

}tLIBMS_STACKINFO;

typedef struct
{
	SYSSTATUS MainStatus_Ref;			//RUN IDLE ...etc
	SYSSTATUS MainStatus;				//RUN IDLE ...etc
	CTLMODE_LI CtlMode_Ref;
	CTLMODE_LI CtlMode,CtlModePre;		//PQ/VQ/Id&Iq Control Mode
	u16 StartUpStatus;
	u16 ErrStatus;						//Is Error Occurred
	u16 DI;
	u16 DO;
	u32 Protect[4];
	u32 FrameNum;

	tVSC_CTL* pVSC;						//关联的VSC句柄
	u16 gErrMask,gSysErrReg,gSysErrFlag;
	u16 gErrClr,gEnHBOut,gErrTriped;
	u16 BMSAlarmLatched[2];

	u16 ChgNotAllowedFlag;
	u16 DischNotAllowedFlag;
	u16 ChDischAllowedSts;				//禁充禁放标志位，Bit0禁充，Bit1禁放

	float Vo,Io;						//端口电压电流
	float Vopu,Iopu;					//端口电压电流
	float SOC;
	float V_Max;						//允许的整组最高工作电压
	float V_Min;						//允许的整组最低工作电压
	float I_Max;						//最大电流
	float P_Norm;						//额定功率
	float P_Cmd,P_Ref,Q_Cmd,Q_Ref,SOC_Ref;
	float PQSlopStep;
	float IoLimH,IoLimL;				//功率动态电流限幅

	float Vb_Reci;						//基准电压倒数
	float Ib_Reci;						//基准电流倒数
	float Pb_Reci;						//基准功率倒数

	float ChVCellMaxAllowed;			//单体禁充允许电压
	float DischVCellMinAllowed;			//单体禁放允许电压
	float DisChVCellRecoverdThr;
	float DischSOCMin;					//禁放SOC

	float ChgI_TCpu,ChgV_TC2CCpu,ChgI_CCpu,ChgV_CVpu,ChgI_CV2OKpu;
	u8 ChgSts;


	PID P_PID,Io_PID,Vo_PID,VLimH_PID,VLimL_PID;
	const PID P_PID_INIT,VO_PID_INIT,IO_PID_INIT;

	LIMIT Vo_Limit;
	LIMIT Io_Limit;

	const LIMIT VO_LIMITREF;
	const LIMIT IO_LIMITREF;

	//启动运行逻辑相关辅助变量
	SYSSTATUS MainStatusPre;				//用于状态逻辑处理
	u16 StartUpStatusPre;					//用于状态逻辑处理
	u32 StartFailReg;						//高16位具体失败信息，低16位用于指示失败环节
	u16 StartUpDelayCnt;					//用于启动过程中的延迟控制

	S_CANCTL tCANCtl;
	u16 CANCtlCnt;
	u16 CANOTCnt;
	u8 BMS_ComOK;
	tLIBMS_STACKINFO tStackInfo;

	u16 (*BMSDrySts)(void);				//BMS干接点反馈

	void (*Init)(void);						//初始化控制函数
	void (*ClrErr)(void);					//故障清除
	u16 (*EmSwSts)(void);					//ROP急停开关
	u16 (*RstSwSts)(void);					//ROP故障复位开关
	void (*CFanCtl)(u16);					//风扇控制
	void (*CRunLedCtl)(u16);				//运行指示灯控制
	void (*CFaultLedCtl)(u16);				//故障指示灯控制
}tLI_CTL;

extern void LIMainFSM(tLI_CTL* tLIHandler);
extern void LIInit(tLI_CTL* tLIHandler);

#endif


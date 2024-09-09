/**
  ******************************************************************************
  * @file    VSCLib.h
  * @author  RZ
  * @brief   Header for VSC Control
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
#ifndef _VSCLIB_H_
#define _VSCLIB_H_

//系统运行状态枚举定义，空闲、启动、运行、停机、旁路、故障
#ifndef SYSSTATUS_DEFINED
#define SYSSTATUS_DEFINED
typedef enum {
	IDLE = 0, START = 1, RUN = 2, STOP = 3, MAINTANCE = 5,_FAULT = 0xFF
} SYSSTATUS;
#endif

//枚举定义
typedef enum{PQCTL = 0,VQCTL = 1,IDQCTL = 2,VACCTL = 3} CTLMODE;
typedef enum{DC_PRECHG = 0,AC_PRECHG = 1} STARTMODE;
typedef enum{VFCTL = 0,DROOPCTL = 1,VSGCTL = 2,UniversalGFM=3} GFMMODE;

//桥臂电流结构体定义
typedef struct {
	float AH;
	float BH;
	float CH;
	float AL;
	float BL;
	float CL;
} S_IARM;

//桥臂电流结构体定义
typedef struct {
	float RMS_Va;
	float RMS_Vb;
	float RMS_Vc;
	float RMS_Ia;
	float RMS_Ib;
	float RMS_Ic;
} RMSInfo_t;

typedef struct
{
//运行回读值及交互变量
	float Va,Vb,Vc;						//实际值
	float Ia,Ib,Ic;						//实际值
	float P,Q,PF,f;						//实际值
	float Vdc,Idc,Pdc;					//实际值
	float Id,Iq;						//标幺值
	float PSet,QSet,VdcSet;				//实际值
	SYSSTATUS MainStatus_Ref;			//RUN IDLE ...etc
	SYSSTATUS MainStatus;				//RUN IDLE ...etc
	CTLMODE CtlMode_Ref;
	CTLMODE CtlMode;					//PQ/VQ/Id&Iq Control Mode
	// 添加孤网和并网状态标志位
	u16 ISLANDED; // 标志位 1 -> 孤网 --- 0 -> 并网
	// 添加构网控制模式
	GFMMODE GFMCtlMode;
	GFMMODE GFMCtlMode_Pre;
	STARTMODE StartMode_Ref;
	STARTMODE StartMode;
	u16 StartUpStatus;
	u16 ErrStatus;						//Is Error Occurred
	u16 DI;
	u16 DO;
	u32 Protect[4];
	u32 FrameNum;

//系统控制内部运行变量
	PARA3S2S2R UGrid,UConv,IGrid;					//电网电压
	PARA3S2S2R Iconv,Ucap;
	float Omega,deltaOmega,Vmag,deltaVmag,Theta,PLLFre;
	SINCOS_t Theta_GridSincos;
	float ThetaPhase;
	SINCOS_t ThetaPhase_GridSincos;
	float DCV_Bus,DCI_Bus;							//直流测量值
	float P_Ref,Q_Ref,Vdc_Ref,Vac_Ref;
	float Id_Ref;									//实际使用的内部参考
	float Iq_Ref;
	float Idc_Ref;
	PID ThetaPID,IdPID,IqPID,VBusPID,P_PID,Q_PID,Vc_PID,IBusPID,Id2PID,Iq2PID;
	// 增加交流电压DQ轴PID
	PID Vd_PID,Vq_PID;
	float mdc_RefOutLoop;
	float mdc;
	s16 PWM_A,PWM_B,PWM_C;
	u16 gErrMask,gSysErrReg,gSysErrFlag;
	u16 gErrClr,gEnHBOut,gErrTriped;
	float PQSlopStep,VdcSlopStep,VacSlopStep;
//运算中间变量
	LPF AC_VARMS_G,AC_VBRMS_G,AC_VCRMS_G;           //电网电压有效值
	LPF AC_IARMS_G,AC_IBRMS_G,AC_ICRMS_G;           //电网电流有效值
	LPF DC_PWM_A,DC_PWM_B,DC_PWM_C;
	float P_AC_AVG,Q_AC_AVG,P_DC_AVG,S_AC_AVG;
	float P_AC,Q_AC,P_DC;
	RMSInfo_t RMSInfo;
	float P_Cmd,Q_Cmd,Vdc_Cmd,Id_Cmd,Iq_Cmd,Vac_Cmd,Fre_Cmd;		//接收到的指令，到内部参考还需要使用Slope
	PARA3S2S2R I2arm,Uccsc;
	S_IARM sIarm;
	s16 *ADC_Va,*ADC_Vb,*ADC_Vc,*ADC_Ia,*ADC_Ib,*ADC_Ic,*ADC_Vdc,*ADC_Idc;

	LIMIT ACVINA_Limit;								//限值判断变量
	LIMIT ACVINB_Limit;
	LIMIT ACVINC_Limit;
	LIMIT DCVPreCharge_Limit;
	LIMIT FrePLL_Limit;

	const PID THETA_PID_INIT;
	const PID PQ_PID_INIT;
	const PID IDQ_PID_INIT;
	const PID VBUS_PID_INIT;
	const PID VC_PID_INIT;
	const PID IBUS_PID_INIT;
	const PID IDQ2_PID_INIT;

	const LPF AC_RMSREF;
	const LPF DC_PWMREF;

	const LIMIT ACVIN_LIMITREF;
	const LIMIT DCVPRECHG_LIMITREF;
	const LIMIT FREPLL_LIMITREF;
//模块控制寄存器
	S_OPTCARD_CMD *pHBCmd_A,*pHBCmd_B,*pHBCmd_C,*pHBCmd_N;
	S_OPTCARD_DAT *pHBDat_A,*pHBDat_B,*pHBDat_C,*pHBDat_N;
	S_ROPCARD *pRop;

//启动运行逻辑相关辅助变量
	SYSSTATUS MainStatusPre;					//用于状态逻辑处理
	u16 StartUpStatusPre;						//用于状态逻辑处理
	u32 StartFailReg;							//高16位具体失败信息，低16位用于指示失败环节
	u16 StartUpDelayCnt;						//用于启动过程中的延迟控制
	u16 StartUpCheckEn;
	u16 StartCheckFlag;
	u16 RelayDelayCnt;
	u16 OutLoopCnt;
//开关操作与返回
	void (*AcBrkCtl)(u16);						//交流断路器控制
	u16 (*AcBrkSts)(void);						//交流断路器状态反馈
	void (*AcContCtl)(u16);						//交流接触器控制
	u16 (*AcContSts)(void);						//交流接触器状态反馈
	void (*AcPreChCtl)(u16);					//交流预充控制
	u16 (*AcPreChSts)(void);					//交流预充状态反馈
	void (*DcContCtl)(u16);						//直流接触器控制
	u16 (*DcContSts)(void);						//直流接触器状态反馈
	void (*DcPreChCtl)(u16);					//直流预充控制
	u16 (*DcPreChSts)(void);					//直流预充状态反馈
	u16 (*HBOK)(void);							//功率模块状态正常
	u16 (*EmSwSts)(void);						//ROP急停开关
	u16 (*RstSwSts)(void);						//ROP故障复位开关
	void (*CFanCtl)(u16);						//风扇控制
	void (*CFaultLedCtl)(u16);					//直流接触器控制
	void (*ClrErr)(void);						//故障清除
//变流器类型
	u16 Flag3P4W;
//初始化函数
	void (*Init)(void);							//初始化控制函数

}tVSC_CTL;

extern void VSCParasInit(tVSC_CTL*);
extern void VSCFastTask(tVSC_CTL*);
extern void VSCMidTask(tVSC_CTL*);
extern void VSCInit(tVSC_CTL*);
extern void svgen(tVSC_CTL*);
extern void VSCFaultDet(tVSC_CTL*);
extern void VSCSysCtl(tVSC_CTL*);
extern void VSCControlLoop(tVSC_CTL*);
extern void VSCAnalogNormaliz(tVSC_CTL*);
extern void VSCRMS_PQCalc(tVSC_CTL*);

#endif



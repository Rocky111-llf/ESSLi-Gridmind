/**
  ******************************************************************************
  * @file    VSCxvars.c
  * @author  RZ
  * @brief   VSCxvars.c module
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

#include "Includings.h"

#define DevID	VSC1

#define instROP		sRop0				//ROP板实例
#define instOpt		sOpt0				//光纤板实例
#define Is3P4W		0
#define OPT_A		0					//光纤板A相通道
#define OPT_B		1					//光纤板B相通道
#define OPT_C		2					//光纤板C相通道
#if Is3P4W
#define OPT_N		0					//光纤板N相通道
#endif

#define PQ_SLOP_STEP 		(0.0001f)	//功率爬坡
#define VDC_SLOP_STEP		(0.001f)	//直流电压爬坡 pu/ms
#define VAC_SLOP_STEP		(0.001f)	//交流电压爬坡 pu/ms

/***********开入开出定义***********/
//o交流断路器跳闸控制
#define AcBrkCtl_(A)	{instROP.Cmd->bit.DO.bit.DO3 = (A);}
//i交流断路器反馈
#define IsAcBrk()		(instROP.Dat->bit.DI.bit.DI0)
//o直流断路器跳闸控制
#define DcBrkCtl_(A)	{instROP.Cmd->bit.DO.bit.DO2 = (A);}
//i直流断路器反馈
#define IsDcBrk()		(instROP.Dat->bit.DI.bit.DI1)
//o交流接触器控制
#define AcContCtl_(A)	{instROP.Cmd->bit.DO.bit.DO4 = (A);instROP.Cmd->bit.DO.bit.DO0 = (A);}
//i交流接触器反馈
#define IsAcCont()		(instROP.Cmd->bit.DO.bit.DO4|instROP.Cmd->bit.DO.bit.DO0)
//o直流接触器控制
#define DcContCtl_(A)	{instROP.Cmd->bit.DO.bit.DO8 = (A);}
//i直流接触器反馈
#define IsDcCont()		(instROP.Cmd->bit.DO.bit.DO8)
//o交流预充控制
#define AcPreChCtl_(A)	{instROP.Cmd->bit.DO.bit.DO5 = (A);instROP.Cmd->bit.DO.bit.DO1 = (A);}
//i交流预充反馈
#define IsAcPreCh()		(instROP.Cmd->bit.DO.bit.DO5|instROP.Cmd->bit.DO.bit.DO1)
//o直流预充控制
#define DcPreChCtl_(A)	{instROP.Cmd->bit.DO.bit.DO7 = (A);}
//i直流预充反馈
#define IsDcPreCh()		(instROP.Cmd->bit.DO.bit.DO7)
//o风扇控制
#define CFanCtl_(A)		{instROP.Cmd->bit.DO.bit.DO6 = (A);}
//i急停开关
#define IsEmSw()		(!(instROP.Dat->bit.DI.bit.DI2))
//i复位开关
#define IsRstSw()		(instROP.Dat->bit.DI.bit.DI3)
//o故障指示灯
#define CFaultLedCtl_(A)	{instROP.Cmd->bit.DO.bit.DO9 = (A);}
/***********开入开出定义结束***********/

/***********半桥控制及反馈信号定义***********/
#define Macro_pHBCmd_A	(&(instOpt.Cmd->bit[OPT_A]))
#define Macro_pHBCmd_B	(&(instOpt.Cmd->bit[OPT_B]))
#define Macro_pHBCmd_C	(&(instOpt.Cmd->bit[OPT_C]))
#define Macro_pHBCmd_N	(&(instOpt.Cmd->bit[OPT_N]))

#define Macro_pHBDat_A	(&(instOpt.Dat->bit[OPT_A]))
#define Macro_pHBDat_B	(&(instOpt.Dat->bit[OPT_B]))
#define Macro_pHBDat_C	(&(instOpt.Dat->bit[OPT_C]))
#define Macro_pHBDat_N	(&(instOpt.Dat->bit[OPT_N]))

#if Is3P4W
	#define Macro_HBOK		(((instOpt.Dat->DAT[20])&((1<<OPT_A)|(1<<OPT_B)|(1<<OPT_C)|(1<<OPT_N)))==((1<<OPT_A)|(1<<OPT_B)|(1<<OPT_C)|(1<<OPT_N)))
#else
	#define Macro_HBOK		(((instOpt.Dat->DAT[20])&((1<<OPT_A)|(1<<OPT_B)|(1<<OPT_C)))==((1<<OPT_A)|(1<<OPT_B)|(1<<OPT_C)))
#endif
/***********半桥控制及反馈信号定义结束***********/


/***********ADC通道映射***********/
#define ACV_A_G				(instROP.Dat->bit.ADdat[10])		//交流电压A
#define ACV_B_G				(instROP.Dat->bit.ADdat[11])		//交流电压B
#define ACV_C_G				(instROP.Dat->bit.ADdat[9])			//交流电压C
#define ACI_A_G				(instROP.Dat->bit.ADdat[0])			//交流电流A
#define ACI_B_G				(instROP.Dat->bit.ADdat[2])			//交流电流B
#define ACI_C_G				(ACI_A_G)							//交流电流C
#define ACV_A_CAP			(instROP.Dat->bit.ADdat[8])			//电容电压A
#define ACV_B_CAP			(instROP.Dat->bit.ADdat[6])			//电容电压B
#define ACV_C_CAP			(instROP.Dat->bit.ADdat[7])			//电容电压C
#define ACI_A_BR			(instROP.Dat->bit.ADdat[3])			//逆变电流A
#define ACI_B_BR			(instROP.Dat->bit.ADdat[1])			//逆变电流B
#define ACI_C_BR			(ACI_A_BR)							//逆变电流C
#define DCV_BUS				(instROP.Dat->bit.ADdat[4])			//直流母线电压
#define DCI_BUS				(instROP.Dat->bit.ADdat[5])			//直流母线电流
#define ADC_RSV1			(instROP.Dat->bit.ADdat[12])		//霍尔备用通道
#define ADC_RSV2			(instROP.Dat->bit.ADdat[13])		//霍尔备用通道
#define ADC_RSV3			(instROP.Dat->bit.ADdat[14])		//霍尔备用通道
#define ADC_RSV4			(instROP.Dat->bit.ADdat[15])		//霍尔备用通道
/***********ADC通道映射结束***********/

/***********PID初始值设置***********/
const PID VarDef(THETA_PID_INIT,DevID) = {0,0,0,0, //Ref,Feedback,Err,ErrPre
		0.2, //Kp 11.78
		1,//Ki
		0,//Kd
		0,//I
		0,//Out
		OMEGAMAX,//OutMax
		OMEGAMIN,//OutMin
		0.38,//Tf
		0};//UF;j
const PID VarDef(PQ_PID_INIT,DevID) = {0,0,0,0, //Ref,Feedback,Err,ErrPre
		0.2, //Kp
		1,//Ki
		0,//Kd
		0,//I
		0,//Out
		1.1,//OutMax
		-1.1,//OutMin
		10,//Tf
		0};//UF;
const PID VarDef(IDQ_PID_INIT,DevID) = {0,0,0,0, //Ref,Feedback,Err,ErrPre
		CURLOOP_Kp, //Kp
		1,//Ki
		0,//Kd
		0,//I
		0,//Out
		1.0,//OutMax
		-1.0,//OutMin
		CURLOOP_Tf,//Tf
		0};//UF;
const PID VarDef(VBUS_PID_INIT,DevID) = {0,0,0,0, //Ref,Feedback,Err,ErrPre
		0.5, //Kp  0.5
		1,//Ki
		0,//Kd
		0,//I
		0,//Out
		1.2,//OutMax
		-1.2,//OutMin
		10,//Tf		5
		0};//UF;

const PID VarDef(VC_PID_INIT,DevID) = {0,0,0,0, //Ref,Feedback,Err,ErrPre
		0.8, //Kp  0.8
		0.1,//Ki
		0,//Kd
		0,//I
		0,//Out
		1.3,//OutMax
		-1.3,//OutMin
		5,//Tf
		0};//UF;
const PID VarDef(IBUS_PID_INIT,DevID) = {0,0,0,0, //Ref,Feedback,Err,ErrPre
		0.1, //Kp  0.8
		2,//Ki
		0,//Kd
		0,//I
		0,//Out
		0.2,//OutMax
		-0.2,//OutMin
		5,//Tf
		0};//UF;
//环流控制PID
const PID VarDef(IDQ2_PID_INIT,DevID) = {0,0,0,0, //Ref,Feedback,Err,ErrPre
		CURLOOP_Kp, //Kp
		1,//Ki
		0,//Kd
		0,//I
		0,//Out
		0.2,//OutMax
		-0.2,//OutMin
		CURLOOP_Tf,//Tf
		0};//UF;

const LPF VarDef(AC_RMSREF,DevID) = {0.0,0.0,0.0,0.0,RMSLPFCOFF1,RMSLPFCOFF2};
const LPF VarDef(DC_PWMREF,DevID) = {0.0,0.0,0.0,0.0,DCPWMLPFCOFF1,DCPWMLPFCOFF2};
const LIMIT VarDef(ACVIN_LIMITREF,DevID) = {1.2f,0.9f,1.3f,0.5f,200,0,0,0,0};
const LIMIT VarDef(DCVPRECHG_LIMITREF,DevID) = {1.35f,0.9f,1.35f,0.9f,100,0,0,0,0};
const LIMIT VarDef(FREPLL_LIMITREF,DevID) = {60,40,60,40,200,0,0,0,0};
/***********PID初始值设置结束***********/


//开关操作及反馈函数
void VarDef(AcBrkCtl__,DevID)(u16 Cmd)					//交流断路器控制
{
#ifdef AcBrkCtl_
	AcBrkCtl_(Cmd);
#endif
}
u16 VarDef(AcBrkSts__,DevID)(void)						//交流断路器状态反馈
{
#ifdef IsAcBrk
	u16 Rtn = IsAcBrk();
	return Rtn;
#else
	return 0;
#endif
}
void VarDef(AcContCtl__,DevID)(u16 Cmd)					//交流接触器控制
{
#ifdef AcContCtl_
	AcContCtl_(Cmd);
#endif
}
u16 VarDef(AcContSts__,DevID)(void)						//交流接触器状态反馈
{
#ifdef IsAcCont
	u16 Rtn = IsAcCont();
	return Rtn;
#else
	return 0;
#endif
}
void VarDef(AcPreChCtl__,DevID)(u16 Cmd)				//交流预充控制
{
#ifdef AcPreChCtl_
	AcPreChCtl_(Cmd);
#endif
}
u16 VarDef(AcPreChSts__,DevID)(void)					//交流预充状态反馈
{
#ifdef IsAcPreCh
	u16 Rtn = IsAcPreCh();
	return Rtn;
#else
	return 0;
#endif
}
void VarDef(DcContCtl__,DevID)(u16 Cmd)					//直流接触器控制
{
#ifdef DcContCtl_
	DcContCtl_(Cmd);
#endif
}
u16 VarDef(DcContSts__,DevID)(void)						//直流接触器状态反馈
{
#ifdef IsDcCont
	u16 Rtn = IsDcCont();
	return Rtn;
#else
	return 0;
#endif
}
void VarDef(DcPreChCtl__,DevID)(u16 Cmd)				//直流预充控制
{
#ifdef DcPreChCtl_
	DcPreChCtl_(Cmd);
#endif
}
u16 VarDef(DcPreChSts__,DevID)(void)					//直流预充状态反馈
{
#ifdef IsDcPreCh
	u16 Rtn = IsDcPreCh();
	return Rtn;
#else
	return 0;
#endif
}
u16 VarDef(HBOK__,DevID)(void)							//H桥正常状态反馈
{
#ifdef Macro_HBOK
	u16 Rtn = Macro_HBOK;
	return Rtn;
#else
	return 0;
#endif
}
u16 VarDef(EmSw__,DevID)(void)							//急停开关状态反馈（按下为低）
{
#ifdef IsEmSw
	static u16 EmSwStsPre = 0;
	u16 Rtn = IsEmSw();
	if((EmSwStsPre)&&(Rtn))
		return Rtn;
	else
	{
		EmSwStsPre = Rtn;
		return 0;
	}
#else
	return 0;
#endif
}
u16 VarDef(RstSw__,DevID)(void)							//复位状态反馈（高有效）
{
#ifdef IsRstSw
	u16 Rtn = IsRstSw();
	return Rtn;
#else
	return 0;
#endif
}
void VarDef(CFanCtl__,DevID)(u16 Cmd)					//风扇控制
{
#ifdef CFanCtl_
	CFanCtl_(Cmd);
#endif
}
void VarDef(CFaultLedCtl__,DevID)(u16 Cmd)				//故障指示灯控制
{
#ifdef CFaultLedCtl_
	CFaultLedCtl_(Cmd);
#endif
}
void VarDef(ClrErr__,DevID)(void)						//清除故障
{
	VarDef(Ctl,DevID).gErrClr = 3;
}
/*
#define FanCtl_(A)		{instROP.Cmd->bit.DO.bit.DO6 = (A);}
//i急停开关
#define IsEmSw()		(instROP.Dat->bit.DI.bit.DI2)
//i复位开关
#define IsRstSw()		(instROP.Dat->bit.DI.bit.DI3)
//o故障指示灯
#define FaultLEDCtl_(A)	{instROP.Cmd->bit.DO.bit.DO9 = (A);}
 */

void VarDef(Init,DevID)(void)
{
	VarDef(Ctl,DevID).pHBCmd_A = Macro_pHBCmd_A;
	VarDef(Ctl,DevID).pHBCmd_B = Macro_pHBCmd_B;
	VarDef(Ctl,DevID).pHBCmd_C = Macro_pHBCmd_C;
#if Is3P4W
	VarDef(Ctl,DevID).pHBCmd_N = Macro_pHBCmd_N;
#endif
	VarDef(Ctl,DevID).pHBDat_A = Macro_pHBDat_A;
	VarDef(Ctl,DevID).pHBDat_B = Macro_pHBDat_B;
	VarDef(Ctl,DevID).pHBDat_C = Macro_pHBDat_C;
#if Is3P4W
	VarDef(Ctl,DevID).pHBDat_N = Macro_pHBDat_N;
#endif

	VarDef(Ctl,DevID).ADC_Va = &ACV_A_G;
	VarDef(Ctl,DevID).ADC_Vb = &ACV_B_G;
	VarDef(Ctl,DevID).ADC_Vc = &ACV_C_G;
	VarDef(Ctl,DevID).ADC_Ia = &ACI_A_G;
	VarDef(Ctl,DevID).ADC_Ib = &ACI_B_G;
	VarDef(Ctl,DevID).ADC_Ic = &ACI_C_G;
	VarDef(Ctl,DevID).ADC_Vdc = &DCV_BUS;
	VarDef(Ctl,DevID).ADC_Idc = &DCI_BUS;

	VarDef(Ctl,DevID).gErrClr = 100;

	VarDef(Ctl,DevID).pRop = &instROP;

	instROP.Cmd->bit.DO.all = 0x00;
	instROP.Cfg->bit.AdcCnvPoint = 300;
}

//结构体初值
tVSC_CTL VarDef(Ctl,DevID) = {
		.THETA_PID_INIT = VarDef(THETA_PID_INIT,DevID),
		.PQ_PID_INIT = VarDef(PQ_PID_INIT,DevID),
		.IDQ_PID_INIT = VarDef(IDQ_PID_INIT,DevID),
		.VBUS_PID_INIT = VarDef(VBUS_PID_INIT,DevID),
		.VC_PID_INIT = VarDef(VC_PID_INIT,DevID),
		.IBUS_PID_INIT = VarDef(IBUS_PID_INIT,DevID),
		.IDQ2_PID_INIT = VarDef(IDQ2_PID_INIT,DevID),
		.AC_RMSREF = VarDef(AC_RMSREF,DevID),
		.DC_PWMREF = VarDef(DC_PWMREF,DevID),
		.ACVIN_LIMITREF = VarDef(ACVIN_LIMITREF,DevID),
		.DCVPRECHG_LIMITREF = VarDef(DCVPRECHG_LIMITREF,DevID),
		.FREPLL_LIMITREF = VarDef(FREPLL_LIMITREF,DevID),
		.OutLoopCnt = 0,
		.AcBrkCtl = VarDef(AcBrkCtl__,DevID),
		.AcBrkSts = VarDef(AcBrkSts__,DevID),
		.AcContCtl = VarDef(AcContCtl__,DevID),
		.AcContSts = VarDef(AcContSts__,DevID),
		.AcPreChCtl = VarDef(AcPreChCtl__,DevID),
		.AcPreChSts = VarDef(AcPreChSts__,DevID),
		.DcContCtl = VarDef(DcContCtl__,DevID),
		.DcContSts = VarDef(DcContSts__,DevID),
		.DcPreChCtl = VarDef(DcPreChCtl__,DevID),
		.DcPreChSts = VarDef(DcPreChSts__,DevID),
		.HBOK = VarDef(HBOK__,DevID),
		.CFanCtl = VarDef(CFanCtl__,DevID),
		.CFaultLedCtl = VarDef(CFaultLedCtl__,DevID),
		.EmSwSts = VarDef(EmSw__,DevID),
		.RstSwSts = VarDef(RstSw__,DevID),
		.Flag3P4W = Is3P4W,
		.PQSlopStep = PQ_SLOP_STEP,
		.VdcSlopStep = VDC_SLOP_STEP,
		.Init = VarDef(Init,DevID),
		.ClrErr = VarDef(ClrErr__,DevID),
};
#undef DevID



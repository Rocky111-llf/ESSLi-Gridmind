/**
  ******************************************************************************
  * @file    EssPbxvars.c
  * @author  RZ
  * @brief   EssPbxvars.c module
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

#define DevID	LI1

#define instVSC				Ctl_VSC1		//VSC实例
#define instDIDO			sDio0

#define PORT_CAN	CAN0					//所用通信CAN接口
#define PORT_BAUD	CANBAUD_250

#define LI_SNUM				(68)		//单体电池串联数
#define PB_VCELL			(3.2)		//单体额定电压


#define P_NORM				(3000)
#define VDC_MAX				(VDC_MAX_CELL*LI_SNUM)			//最大允许电压
#define VDC_MIN				(VDC_MIN_CELL*LI_SNUM)			//最低允许电压
#define IDC_MAX				(18)							//最大允许电流

//三段式充电，恒流-恒压-浮充,如电压低需采用涓流
#define CHGI_TC			(0.5f)			//涓流充电电流,A
#define CHGV_TC2CC		(2.9f)			//单体恒流阈值电压，高于该电压时可采用恒流充电,V
#define CHGI_CC			(10.0f)			//单体恒流充电电流，A
#define CHGV_CV			(3.6f)			//单体恒压电压，V
#define CHGI_CV2OK		(0.5f)			//浮充转换电流，A

#define VDC_MAX_CELL	(3.6f)			//单体最高工作电压
#define VDC_MIN_CELL	(2.8f)			//单体最低工作电压

#define VDC_CHMAX_CELL		(3.58f)			//禁充单体电压（高于BMS设置值，为3.6V）
#define VDC_DISCHMIN_CELL	(2.85f)			//禁放单体电压（低于BMS设置值，为2.8V）
#define VDC_CHDISCH_THR		(0.2f)			//禁充禁放回差恢复电压
#define SOC_DISCHMIN		(30.0f)			//禁放SOC


/***********开入开出定义***********/
//iBMS告警干接点
#define IsBMSDry()			0//(instDIDO.Dat->bit.DI3)
//i急停总开关
#define IsEmSw()			(IsSysEmSw())
//o风扇控制
#define CFanCtl_(A)			(SysFanCtl(A))
//o运行指示灯
#define CRunLedCtl_(A)		(SysRunLedCtl(A))
//o故障指示灯
#define CFaultLedCtl_(A)	(SysFaultLedCtl(A))
/***********开入开出定义结束***********/

#define PQ_SLOP_STEP 		(0.01f)		//功率爬坡，每ms增量

const PID VarDef(P_PID_INIT,DevID) = {0,0,0,0, //Ref,Feedback,Err,ErrPre
		0.01, //Kp
		1,//Ki
		0,//Kd
		0,//I
		0,//Out
		1.1,//OutMax
		-1.1,//OutMin
		0.3,//Tf
		0};//UF;

const PID VarDef(VO_PID_INIT,DevID) = {0,0,0,0, //Ref,Feedback,Err,ErrPre
		0.5, //Kp  0.5
		1,//Ki
		0,//Kd
		0,//I
		0,//Out
		1.2,//OutMax
		-1.2,//OutMin
		10,//Tf		5
		0};//UF;

const PID VarDef(IO_PID_INIT,DevID) = {0,0,0,0, //Ref,Feedback,Err,ErrPre
		0.2, //Kp
		1,//Ki
		0,//Kd
		0,//I
		0,//Out
		1.2,//OutMax
		-1.2,//OutMin
		1,//Tf
		0};//UF;

const LIMIT VarDef(VO_LIMITREF,DevID) = {1.02f,0.7f,1.05f,-0.1f,50,0,0,0,0};
const LIMIT VarDef(IO_LIMITREF,DevID) = {1.15f,-1.15f,1.15f,-1.15f,50,0,0,0,0};
/***********PID初始值设置结束***********/


//开关操作及反馈函数
u16 VarDef(BMSDrySts__,DevID)(void)						//交流断路器状态反馈
{
#ifdef IsBMSDry
	u16 Rtn = IsBMSDry();
	return Rtn;
#else
	return 1;
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
void VarDef(CFanCtl__,DevID)(u16 Cmd)					//风扇控制
{
#ifdef CFanCtl_
	CFanCtl_(Cmd);
#endif
}
void VarDef(CRunLedCtl__,DevID)(u16 Cmd)				//运行指示灯控制
{
#ifdef CRunLedCtl_
	CRunLedCtl_(Cmd);
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
	VarDef(Ctl,DevID).gErrClr = 1;
}

void VarDef(Init,DevID)(void)
{
	VarDef(Ctl,DevID).gErrClr = 1;
	VarDef(Ctl,DevID).pVSC = &instVSC;

	VarDef(Ctl,DevID).tCANCtl.CanBaud = rate_500kbps;
	VarDef(Ctl,DevID).tCANCtl.hParent = (void*)&(VarDef(Ctl,DevID));

	VarDef(Ctl,DevID).BMSAlarmLatched[0]  = 0;
	VarDef(Ctl,DevID).BMSAlarmLatched[1]  = 0;
}

//结构体初值
tLI_CTL VarDef(Ctl,DevID) = {
		.V_Max = VDC_MAX,
		.V_Min = VDC_MIN,
		.I_Max = IDC_MAX,
		.P_Norm = P_NORM,
		.Vb_Reci = (1.0f/VDC_MAX),
		.Ib_Reci = (1.0f/IDC_MAX),
		.Pb_Reci = (1.0f/P_NORM),
		.P_PID_INIT = VarDef(P_PID_INIT,DevID),
		.VO_PID_INIT = VarDef(VO_PID_INIT,DevID),
		.IO_PID_INIT = VarDef(IO_PID_INIT,DevID),
		.VO_LIMITREF = VarDef(VO_LIMITREF,DevID),
		.IO_LIMITREF = VarDef(IO_LIMITREF,DevID),
		.BMSDrySts = VarDef(BMSDrySts__,DevID),

		.ChVCellMaxAllowed = (VDC_CHMAX_CELL*1000),
		.DischVCellMinAllowed = (VDC_DISCHMIN_CELL*1000),
		.DisChVCellRecoverdThr = (VDC_CHDISCH_THR*1000),

		.ChgI_TCpu = CHGI_TC/IDC_MAX,
		.ChgV_TC2CCpu = (CHGV_TC2CC*LI_SNUM/VDC_MAX),
		.ChgI_CCpu = CHGI_CC/IDC_MAX,
		.ChgV_CVpu = (CHGV_CV*LI_SNUM/VDC_MAX),
		.ChgI_CV2OKpu = CHGI_CV2OK/IDC_MAX,

		.CFanCtl = VarDef(CFanCtl__,DevID),
		.CRunLedCtl = VarDef(CRunLedCtl__,DevID),
		.CFaultLedCtl = VarDef(CFaultLedCtl__,DevID),
		.EmSwSts = VarDef(EmSw__,DevID),
		.PQSlopStep = PQ_SLOP_STEP,
		.Init = VarDef(Init,DevID),
		.ClrErr = VarDef(ClrErr__,DevID),
};
#undef DevID


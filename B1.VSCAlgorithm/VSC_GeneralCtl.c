/*****************************************************************************
Copyright: 2019-2021, GridMind. Co., Ltd.
File name: VSC_GeneralCtl.c
Description: VSC基本控制
Author: RZ
Version: 版本
Date: 完成日期
History: 修改历史记录列表， 每条修改记录应包括修改日期、修改者及修改内容简述。
*****************************************************************************/


#include "Includings.h"

void VSCCmd_Slope(tVSC_CTL*);			//指令斜坡处理
void VSCLocalParasUpdate(tVSC_CTL* tVSCHandler);
void VSCMainFSM(tVSC_CTL* tVSCHandler);
void VSCMainFSM_Init(tVSC_CTL* tVSCHandler);

/*************************************************
 Function: VSCParasInit
 Description: VSC运行前参数初始化
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void VSCParasInit(tVSC_CTL* tVSCHandler)
{
//Reference Cmd Clear
	tVSCHandler->Id_Ref = 0;
	tVSCHandler->Iq_Ref = 0;
	tVSCHandler->Vdc_Ref = 1.0f;
	tVSCHandler->P_Ref = 0;					//内部参考
	tVSCHandler->Q_Ref = 0;
	tVSCHandler->P_Cmd = 0.0f;				//外部指令
	tVSCHandler->Q_Cmd = 0.0f;
	tVSCHandler->Vdc_Cmd = 1.0f;
	tVSCHandler->Id_Cmd = 0.0f;
	tVSCHandler->Iq_Cmd = 0.0f;
	tVSCHandler->Vac_Cmd = 0.0f;
	tVSCHandler->Fre_Cmd = 0.0f;
	// 增加Vac_Ref初始化
	tVSCHandler->Vac_Ref = 0.0f;
	// 增加孤网并网标志位初始化
#ifdef ISLANDED_START
	tVSCHandler-> ISLANDED = 1;
#else 
	tVSCHandler-> ISLANDED = 0;
#endif
//PID Init
	tVSCHandler->ThetaPID = tVSCHandler->THETA_PID_INIT;
	tVSCHandler->IdPID = tVSCHandler->IDQ_PID_INIT;
	tVSCHandler->IqPID = tVSCHandler->IDQ_PID_INIT;
	tVSCHandler->VBusPID = tVSCHandler->VBUS_PID_INIT;
	tVSCHandler->Q_PID = tVSCHandler->PQ_PID_INIT;
	tVSCHandler->P_PID = tVSCHandler->PQ_PID_INIT;
	tVSCHandler->Vc_PID = tVSCHandler->VC_PID_INIT;
	// 增加交流电压DQ轴PID
	tVSCHandler->Vd_PID = tVSCHandler->VC_PID_INIT;
	tVSCHandler->Vq_PID = tVSCHandler->VC_PID_INIT;
	tVSCHandler->IBusPID = tVSCHandler->IBUS_PID_INIT;
	tVSCHandler->Id2PID = tVSCHandler->IDQ2_PID_INIT;
	tVSCHandler->Iq2PID = tVSCHandler->IDQ2_PID_INIT;
//PID Init End

/*RMS检测LPF初始化*/
	//电网侧电压电流有效值
	tVSCHandler->AC_VARMS_G = tVSCHandler->AC_RMSREF;
	tVSCHandler->AC_VBRMS_G = tVSCHandler->AC_RMSREF;
	tVSCHandler->AC_VCRMS_G = tVSCHandler->AC_RMSREF;
	tVSCHandler->AC_IARMS_G = tVSCHandler->AC_RMSREF;
	tVSCHandler->AC_IBRMS_G = tVSCHandler->AC_RMSREF;
	tVSCHandler->AC_ICRMS_G = tVSCHandler->AC_RMSREF;
	tVSCHandler->RMSInfo.RMS_Va = 0.0f;
	tVSCHandler->RMSInfo.RMS_Vb = 0.0f;
	tVSCHandler->RMSInfo.RMS_Vc = 0.0f;
	tVSCHandler->RMSInfo.RMS_Ia = 0.0f;
	tVSCHandler->RMSInfo.RMS_Ib = 0.0f;
	tVSCHandler->RMSInfo.RMS_Ic = 0.0f;
/*RMS检测LPF初始化结束*/

	tVSCHandler->ACVINA_Limit = tVSCHandler->ACVIN_LIMITREF;
	tVSCHandler->ACVINB_Limit = tVSCHandler->ACVIN_LIMITREF;
	tVSCHandler->ACVINC_Limit = tVSCHandler->ACVIN_LIMITREF;
	tVSCHandler->DCVPreCharge_Limit = tVSCHandler->DCVPRECHG_LIMITREF;
	tVSCHandler->FrePLL_Limit = tVSCHandler->FREPLL_LIMITREF;

	tVSCHandler->gErrMask = 0;
	tVSCHandler->gSysErrReg = 0;
	tVSCHandler->gSysErrFlag = BOOL_FALSE;

	tVSCHandler->mdc_RefOutLoop = 1.0f;

	tVSCHandler->DC_PWM_A = tVSCHandler->DC_PWMREF;
	tVSCHandler->DC_PWM_B = tVSCHandler->DC_PWMREF;
	tVSCHandler->DC_PWM_C = tVSCHandler->DC_PWMREF;

	tVSCHandler->OutLoopCnt = 0;
}

/****************************************************************************
*
*本地参数更新
*
****************************************************************************/
void VSCLocalParasUpdate(tVSC_CTL* tVSCHandler)						//本地数据更新
{
	tVSCHandler->ErrStatus = tVSCHandler->gSysErrReg;
	tVSCHandler->Va = tVSCHandler->RMSInfo.RMS_Va*RATED_ACV;
	tVSCHandler->Vb = tVSCHandler->RMSInfo.RMS_Vb*RATED_ACV;
	tVSCHandler->Vc = tVSCHandler->RMSInfo.RMS_Vc*RATED_ACV;
	tVSCHandler->Ia = tVSCHandler->RMSInfo.RMS_Ia*RATED_ACI;
	tVSCHandler->Ib = tVSCHandler->RMSInfo.RMS_Ib*RATED_ACI;
	tVSCHandler->Ic = tVSCHandler->RMSInfo.RMS_Ic*RATED_ACI;
	tVSCHandler->P = tVSCHandler->P_AC_AVG*RATED_S;
	tVSCHandler->Q = tVSCHandler->Q_AC_AVG*RATED_S;
	tVSCHandler->Id = tVSCHandler->IGrid.P2R.d;
	tVSCHandler->Iq = tVSCHandler->IGrid.P2R.q;
	tVSCHandler->Vdc = RATED_DCV*tVSCHandler->DCV_Bus;
	tVSCHandler->Idc = RATED_DCI*tVSCHandler->DCI_Bus;
	tVSCHandler->Pdc = tVSCHandler->P_DC_AVG*RATED_S;
	tVSCHandler->PSet = tVSCHandler->P_Cmd*RATED_S;
	tVSCHandler->QSet = tVSCHandler->Q_Cmd*RATED_S;
	tVSCHandler->VdcSet = tVSCHandler->Vdc_Cmd * RATED_DCV;

	tVSCHandler->f = tVSCHandler->PLLFre;
	tVSCHandler->DI = (u16)(tVSCHandler->pRop->Dat->bit.DI.all);
	tVSCHandler->DO = (u16)(tVSCHandler->pRop->Cmd->bit.DO.all);

	tVSCHandler->PF = tVSCHandler->P_AC_AVG/tVSCHandler->S_AC_AVG;					//221004增加PF显示计算
	if(tVSCHandler->PF < 0.0f)
		tVSCHandler->PF = -tVSCHandler->PF;

	if(tVSCHandler->MainStatus_Ref == START)
	{
		if(tVSCHandler->MainStatus == IDLE && (tVSCHandler->gErrClr == 0))			//只有在IDLE状态才能切换进入启动
		{
			tVSCHandler->MainStatus = START;
			tVSCHandler->StartMode = tVSCHandler->StartMode_Ref;
			tVSCHandler->CtlMode = tVSCHandler->CtlMode_Ref;
		}
	}
	else if(tVSCHandler->MainStatus_Ref == STOP)
	{
		if(tVSCHandler->MainStatus == START || tVSCHandler->MainStatus == RUN)				//非错误状态下可以进入停机状态
			tVSCHandler->MainStatus = STOP;
	}
}

/*************************************************
 Function: VSCSysCtl
 Description: 外环运算
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void VSCSysCtl(tVSC_CTL* tVSCHandler)
{
	if(tVSCHandler->CtlMode == PQCTL)
	{
		// 外环无PI的跟网定功率控制
		//有功分量
		tVSCHandler->P_PID.Ref = tVSCHandler->P_Ref;
		tVSCHandler->P_PID.FeedBack = tVSCHandler->P_AC_AVG;
		PIDProc_Int_Sepa(&tVSCHandler->P_PID);
		tVSCHandler->Id_Ref = tVSCHandler->P_PID.Out;
		HardLimit(tVSCHandler->Id_Ref, -1.1f, 1.1f);
		//无功分量,控制信号需要反相
		tVSCHandler->Q_PID.Ref = -tVSCHandler->Q_Ref;
		tVSCHandler->Q_PID.FeedBack = -tVSCHandler->Q_AC_AVG;
		PIDProc_Int_Sepa(&tVSCHandler->Q_PID);
		tVSCHandler->Iq_Ref = tVSCHandler->Q_PID.Out;
		HardLimit(tVSCHandler->Iq_Ref, -1.1f, 1.1f);
	}
	else if(tVSCHandler->CtlMode == VQCTL)
	{
	//母线电压控制PID
		tVSCHandler->VBusPID.Ref = tVSCHandler->Vdc_Ref;
		tVSCHandler->VBusPID.FeedBack = tVSCHandler->DCV_Bus;
		PIDProc(&tVSCHandler->VBusPID);

		tVSCHandler->Id_Ref = tVSCHandler->VBusPID.Out;
		HardLimit(tVSCHandler->Id_Ref, -1.1f, 1.1f);

		//无功分量，也未采用PI控制
		tVSCHandler->Iq_Ref = -tVSCHandler->Q_Ref/tVSCHandler->UGrid.P2R.d;									//221004 修改，无功功率指令变号
		HardLimit(tVSCHandler->Iq_Ref, -1.1f, 1.1f);
	}
	else if(tVSCHandler->CtlMode == IDQCTL)
	{
		tVSCHandler->Idc_Ref = I_DC_LIM;													//直流限流值

#if IQTEST
		if(IqTestEn)
		{
			if(IqTestCnt < IqTestPrd)
				IqTestCnt++;
			else
			{
				IqTestCnt = 0;
				if(Iq_Ref == IqTestRef)
					Iq_Ref = -IqTestRef;
				else
					Iq_Ref = IqTestRef;
			}
		}
		else
			Iq_Ref = 0.0f;
#endif

		HardLimit(tVSCHandler->Id_Ref, -1.1f, 1.1f);
		//无功分量
		HardLimit(tVSCHandler->Iq_Ref, -1.1f, 1.1f);
	}
	else if(tVSCHandler->CtlMode == VACCTL)
	{
		//构网控制电压外环
		if(tVSCHandler->GFMCtlMode == VFCTL){
			// 空载时不含电流内环
			// d轴电压跟踪参考值
			// TODO:如果报错的化,是交流电压参考值爬坡太快,需要调交流参考电压爬坡步长
			tVSCHandler->Vd_PID.Ref = tVSCHandler->Vac_Ref;
			tVSCHandler->Vd_PID.FeedBack = tVSCHandler->UGrid.P2R.d;
			PIDProc(&tVSCHandler->Vd_PID);
			tVSCHandler->Id_Ref = tVSCHandler->Vd_PID.Out;

			// q轴电压跟踪0
			tVSCHandler->Vq_PID.Ref = 0;
			tVSCHandler->Vq_PID.FeedBack = tVSCHandler->UGrid.P2R.q;
			PIDProc(&tVSCHandler->Vq_PID);
			tVSCHandler->Iq_Ref = tVSCHandler->Vq_PID.Out;

			HardLimit(tVSCHandler->Id_Ref, -1.1f, 1.1f);
			HardLimit(tVSCHandler->Iq_Ref, -1.1f, 1.1f);
		}else{
			// TODO
		}
	}
}

/*************************************************
 Function: VSCAnalogNormaliz
 Description: 模拟参数标幺化
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void VSCControlLoop(tVSC_CTL* tVSCHandler)
{
	float MagP2R,MagP2R_Reci;

	//电压功率外环
	if(tVSCHandler->OutLoopCnt < OUTLOOPDIV)
		tVSCHandler->OutLoopCnt++;
	else
		tVSCHandler->OutLoopCnt = 0;

	if(tVSCHandler->MainStatus == RUN)
	{
		//外环控制
		if(tVSCHandler->OutLoopCnt == 0)
		// 构网电压外环，跟网外环分频10倍
			VSCSysCtl(tVSCHandler);
		if (tVSCHandler->CtlMode != VACCTL)
		{
			//交流电流环
			//Id
			tVSCHandler->IdPID.Ref = tVSCHandler->Id_Ref;
			tVSCHandler->IdPID.FeedBack = tVSCHandler->IGrid.P2R.d;
			PIDProc(&tVSCHandler->IdPID);
			//Iq
			tVSCHandler->IqPID.Ref = tVSCHandler->Iq_Ref;
			tVSCHandler->IqPID.FeedBack = tVSCHandler->IGrid.P2R.q;
			PIDProc(&tVSCHandler->IqPID);
			//OutV Calc
			float VRatio = (NORM_V/(tVSCHandler->DCV_Bus*(RATED_DCV*0.5f*1.154f)));
			tVSCHandler->UConv.P2R.d = (tVSCHandler->UGrid.P2R.d - tVSCHandler->IdPID.Out + (tVSCHandler->IGrid.P2R.q*tVSCHandler->PLLFre*(2.0f*PI*(Larm/NORM_Z))))*VRatio;
			tVSCHandler->UConv.P2R.q = (tVSCHandler->UGrid.P2R.q - tVSCHandler->IqPID.Out - (tVSCHandler->IGrid.P2R.d*tVSCHandler->PLLFre*(2.0f*PI*(Larm/NORM_Z))))*VRatio;

			//单位圆限幅
			MagP2R = FastSqrt2((tVSCHandler->UConv.P2R.d*tVSCHandler->UConv.P2R.d)+(tVSCHandler->UConv.P2R.q*tVSCHandler->UConv.P2R.q),&MagP2R_Reci);
			if(MagP2R>0.9999999f)
			{
				tVSCHandler->UConv.P2R.d = tVSCHandler->UConv.P2R.d*MagP2R_Reci;
				tVSCHandler->UConv.P2R.q = tVSCHandler->UConv.P2R.q*MagP2R_Reci;
			}
			ipark(&tVSCHandler->UConv.P2R,&tVSCHandler->UConv.P2S,&tVSCHandler->ThetaPhase_GridSincos);				//2r to 2s
		}
		else
		{
			// CtlMode == VACCTL
			if(tVSCHandler->GFMCtlMode == VFCTL){
				// 空载时不含电流内环，直接用Id_ref和Iq_ref的参考值等效调制比信号
				float VRatio = (NORM_V/(tVSCHandler->DCV_Bus*(RATED_DCV*0.5f*1.154f)));
				tVSCHandler->UConv.P2R.d = (tVSCHandler->Id_Ref)*VRatio;
				tVSCHandler->UConv.P2R.q = (tVSCHandler->Iq_Ref)*VRatio;
			}
			//单位圆限幅
			MagP2R = FastSqrt2((tVSCHandler->UConv.P2R.d*tVSCHandler->UConv.P2R.d)+(tVSCHandler->UConv.P2R.q*tVSCHandler->UConv.P2R.q),&MagP2R_Reci);
			if(MagP2R>0.9999999f)
			{
				tVSCHandler->UConv.P2R.d = tVSCHandler->UConv.P2R.d*MagP2R_Reci;
				tVSCHandler->UConv.P2R.q = tVSCHandler->UConv.P2R.q*MagP2R_Reci;
			}
			ipark(&tVSCHandler->UConv.P2R,&tVSCHandler->UConv.P2S,&tVSCHandler->ThetaPhase_GridSincos);
		}
		//调制波设置
		svgen(tVSCHandler);

		tVSCHandler->pHBCmd_A->ModWave = tVSCHandler->PWM_A;
		tVSCHandler->pHBCmd_B->ModWave = tVSCHandler->PWM_B;
		tVSCHandler->pHBCmd_C->ModWave = tVSCHandler->PWM_C;
		if(tVSCHandler->Flag3P4W)
			tVSCHandler->pHBCmd_N->ModWave = (1.0f/3)*(tVSCHandler->PWM_A+tVSCHandler->PWM_B+tVSCHandler->PWM_C);
	}
}


/*************************************************
 Function: VSCAnalogNormaliz
 Description: 模拟参数标幺化
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void VSCAnalogNormaliz(tVSC_CTL* tVSCHandler)
{
//以线电压作为PLL输入
	tVSCHandler->UGrid.P3S.a = (1.0f*ACV_TRANS_G*ACVRATIO/NORM_V/sqrt3)*((s32)*tVSCHandler->ADC_Va-*tVSCHandler->ADC_Vb);
	tVSCHandler->UGrid.P3S.b = (1.0f*ACV_TRANS_G*ACVRATIO/NORM_V/sqrt3)*((s32)*tVSCHandler->ADC_Vb-*tVSCHandler->ADC_Vc);
	tVSCHandler->UGrid.P3S.c = (1.0f*ACV_TRANS_G*ACVRATIO/NORM_V/sqrt3)*((s32)*tVSCHandler->ADC_Vc-*tVSCHandler->ADC_Va);
	tVSCHandler->IGrid.P3S.a = (1.0f*ACI_TRANS_G*ACIRATIO/NORM_I)*(*tVSCHandler->ADC_Ia);
	tVSCHandler->IGrid.P3S.b = (1.0f*ACI_TRANS_G*ACIRATIO/NORM_I)*(*tVSCHandler->ADC_Ib);
	tVSCHandler->IGrid.P3S.c = -tVSCHandler->IGrid.P3S.a - tVSCHandler->IGrid.P3S.b;

	tVSCHandler->DCV_Bus = (DCVRATIO/RATED_DCV)*(*tVSCHandler->ADC_Vdc);
	tVSCHandler->DCI_Bus = (DCIRATIO/RATED_DCI)*(*tVSCHandler->ADC_Idc);
}

void VSCRMS_PQCalc(tVSC_CTL* tVSCHandler)
{
//RMS计算
	//电网电压电流有效值
	tVSCHandler->AC_VARMS_G.In = 2.0*tVSCHandler->UGrid.P3S.a*tVSCHandler->UGrid.P3S.a;
	tVSCHandler->AC_VBRMS_G.In = 2.0*tVSCHandler->UGrid.P3S.b*tVSCHandler->UGrid.P3S.b;
	tVSCHandler->AC_VCRMS_G.In = 2.0*tVSCHandler->UGrid.P3S.c*tVSCHandler->UGrid.P3S.c;
	tVSCHandler->AC_IARMS_G.In = 2.0*tVSCHandler->IGrid.P3S.a*tVSCHandler->IGrid.P3S.a;
	tVSCHandler->AC_IBRMS_G.In = 2.0*tVSCHandler->IGrid.P3S.b*tVSCHandler->IGrid.P3S.b;
	tVSCHandler->AC_ICRMS_G.In = 2.0*tVSCHandler->IGrid.P3S.c*tVSCHandler->IGrid.P3S.c;

	LPFProc(&tVSCHandler->AC_VARMS_G);
	LPFProc(&tVSCHandler->AC_VBRMS_G);
	LPFProc(&tVSCHandler->AC_VCRMS_G);
	LPFProc(&tVSCHandler->AC_IARMS_G);
	LPFProc(&tVSCHandler->AC_IBRMS_G);
	LPFProc(&tVSCHandler->AC_ICRMS_G);

	tVSCHandler->RMSInfo.RMS_Va	= FastSqrt(tVSCHandler->AC_VARMS_G.Out);
	tVSCHandler->RMSInfo.RMS_Vb	= FastSqrt(tVSCHandler->AC_VBRMS_G.Out);
	tVSCHandler->RMSInfo.RMS_Vc	= FastSqrt(tVSCHandler->AC_VCRMS_G.Out);
	tVSCHandler->RMSInfo.RMS_Ia	= FastSqrt(tVSCHandler->AC_IARMS_G.Out);
	tVSCHandler->RMSInfo.RMS_Ib	= FastSqrt(tVSCHandler->AC_IBRMS_G.Out);
	tVSCHandler->RMSInfo.RMS_Ic	= FastSqrt(tVSCHandler->AC_ICRMS_G.Out);
//功率计算
	tVSCHandler->P_AC = (tVSCHandler->P_AC*(OUTLOOPDIV-1)+(tVSCHandler->UGrid.P2R.d*tVSCHandler->IGrid.P2R.d + tVSCHandler->UGrid.P2R.q*tVSCHandler->IGrid.P2R.q))*(1.0f/OUTLOOPDIV);
	tVSCHandler->Q_AC = (tVSCHandler->Q_AC*(OUTLOOPDIV-1)+(tVSCHandler->UGrid.P2R.q*tVSCHandler->IGrid.P2R.d - tVSCHandler->UGrid.P2R.d*tVSCHandler->IGrid.P2R.q))*(1.0f/OUTLOOPDIV);
	tVSCHandler->P_DC = tVSCHandler->DCV_Bus*tVSCHandler->DCI_Bus;

	tVSCHandler->P_AC_AVG = (tVSCHandler->P_AC_AVG*(PQAVGCOFF-1)+tVSCHandler->P_AC)*(1.0f/PQAVGCOFF);
	tVSCHandler->Q_AC_AVG = (tVSCHandler->Q_AC_AVG*(PQAVGCOFF-1)+tVSCHandler->Q_AC)*(1.0f/PQAVGCOFF);
	tVSCHandler->P_DC_AVG = (tVSCHandler->P_DC_AVG*(PQAVGCOFF-1)+tVSCHandler->P_DC)*(1.0f/PQAVGCOFF);

	tVSCHandler->S_AC_AVG = FastSqrt(tVSCHandler->P_AC_AVG*tVSCHandler->P_AC_AVG + tVSCHandler->Q_AC_AVG*tVSCHandler->Q_AC_AVG);
}


/*************************************************
 Function: VSCFaultDet
 Description: 关键故障检测
 Input: 无
 Output: 无
 Return: 无
 Others: //用于中断中的故障检测，受整体故障掩码控制
 *************************************************/
void VSCFaultDet(tVSC_CTL* tVSCHandler)
{
	u32 HardFaultReg;
//功率模块状态判断
	if(!(tVSCHandler->HBOK()))
	{
		if(tVSCHandler->gErrMask & ERR_PEM_SW){
			tVSCHandler->gSysErrReg |= ERR_PEM_SW;
		}
	}

	HardFaultReg = 0;//hSen0_Dat->bit.Fault;
//硬件交流过压
	if(HardFaultReg & (u32)0x7){
		tVSCHandler->gSysErrReg |= ERR_AC_OV_HW;
	}
//硬件交流过流
	if(HardFaultReg & ((u32)0x7<<3)){
		tVSCHandler->gSysErrReg |= ERR_AC_OC_HW;
	}
//硬件直流过压
	if(HardFaultReg & ((u32)0x1<<8)){
		tVSCHandler->gSysErrReg |= ERR_DC_OV_HW;
	}
//硬件直流过流
	if(HardFaultReg & ((u32)0x1<<6)){
		tVSCHandler->gSysErrReg |= ERR_DC_OC_HW;
	}
//急停开关
	if(tVSCHandler->EmSwSts()){
		tVSCHandler->gSysErrReg |= ERR_EMER_HW;
	}

	if((tVSCHandler->gSysErrReg) & ((tVSCHandler->gErrMask)|ERR_EMER_HW|ERR_BOARDLINK_HW))		//故障信号在掩码范围内(急停、板连接总是有效)
	{
		if(!(tVSCHandler->gSysErrFlag))
		{
//			HBBlk(1);						//闭锁所有模块
			tVSCHandler->pHBCmd_A->Mode = HB_MODE_FBLK;
			tVSCHandler->pHBCmd_B->Mode = HB_MODE_FBLK;
			tVSCHandler->pHBCmd_C->Mode = HB_MODE_FBLK;
			if(tVSCHandler->Flag3P4W)
				tVSCHandler->pHBCmd_N->Mode = HB_MODE_FBLK;
			tVSCHandler->gSysErrFlag = 1;
			tVSCHandler->gErrTriped = tVSCHandler->gSysErrReg;
		}
		tVSCHandler->gEnHBOut = 0;
		tVSCHandler->MainStatus = _FAULT;		//进入故障状态
	}

	if(tVSCHandler->gErrClr)
	{
//		gErrMask = 0;
		tVSCHandler->pHBCmd_A->Mode = HB_MODE_RST;
		tVSCHandler->pHBCmd_B->Mode = HB_MODE_RST;
		tVSCHandler->pHBCmd_C->Mode = HB_MODE_RST;
		if(tVSCHandler->Flag3P4W)
			tVSCHandler->pHBCmd_N->Mode = HB_MODE_RST;
		tVSCHandler->gErrClr--;
		tVSCHandler->gSysErrReg = 0;
		tVSCHandler->gSysErrFlag = 0;
		tVSCHandler->gErrTriped = 0;
		HBBlk(0);
		CfgCard();									//清除硬件故障
	}
}

/****************************************************************************
*
*指令爬坡
*
****************************************************************************/
void VSCCmd_Slope(tVSC_CTL* tVSCHandler)
{
	float PQStep,VdcStep,VacStep;
	PQStep = tVSCHandler->PQSlopStep;
	VdcStep = tVSCHandler->VdcSlopStep;
	VacStep = tVSCHandler->VacSlopStep;
	if(tVSCHandler->CtlMode == PQCTL)
	{
//有功功率爬坡
		if((tVSCHandler->P_Ref+PQStep) < tVSCHandler->P_Cmd)
			tVSCHandler->P_Ref += PQStep;
		else if((tVSCHandler->P_Ref-PQStep) > tVSCHandler->P_Cmd)
			tVSCHandler->P_Ref -= PQStep;
		else
			tVSCHandler->P_Ref = tVSCHandler->P_Cmd;
//无功功率爬坡
		if((tVSCHandler->Q_Ref+PQStep) < tVSCHandler->Q_Cmd)
			tVSCHandler->Q_Ref += PQStep;
		else if((tVSCHandler->Q_Ref-PQStep) > tVSCHandler->Q_Cmd)
			tVSCHandler->Q_Ref -= PQStep;
		else
			tVSCHandler->Q_Ref = tVSCHandler->Q_Cmd;
	}
	else if(tVSCHandler->CtlMode == VQCTL)
	{
//直流电压
		if((tVSCHandler->Vdc_Ref+VdcStep) < tVSCHandler->Vdc_Cmd)
			tVSCHandler->Vdc_Ref += VdcStep;
		else if((tVSCHandler->Vdc_Ref-VdcStep) > tVSCHandler->Vdc_Cmd)
			tVSCHandler->Vdc_Ref -= VdcStep;
		else
			tVSCHandler->Vdc_Ref = tVSCHandler->Vdc_Cmd;
//无功功率爬坡
		if((tVSCHandler->Q_Ref+PQStep) < tVSCHandler->Q_Cmd)
			tVSCHandler->Q_Ref += PQStep;
		else if((tVSCHandler->Q_Ref-PQStep) > tVSCHandler->Q_Cmd)
			tVSCHandler->Q_Ref -= PQStep;
		else
			tVSCHandler->Q_Ref = tVSCHandler->Q_Cmd;
	}
	else if(tVSCHandler->CtlMode == IDQCTL)
	{
		tVSCHandler->Id_Ref = tVSCHandler->Id_Cmd;
		tVSCHandler->Iq_Ref = tVSCHandler->Iq_Cmd;
	}
	else if(tVSCHandler->CtlMode == VACCTL)
	{
//有功功率爬坡
		if((tVSCHandler->P_Ref+PQStep) < tVSCHandler->P_Cmd)
			tVSCHandler->P_Ref += PQStep;
		else if((tVSCHandler->P_Ref-PQStep) > tVSCHandler->P_Cmd)
			tVSCHandler->P_Ref -= PQStep;
		else
			tVSCHandler->P_Ref = tVSCHandler->P_Cmd;
//无功功率爬坡
		if((tVSCHandler->Q_Ref+PQStep) < tVSCHandler->Q_Cmd)
			tVSCHandler->Q_Ref += PQStep;
		else if((tVSCHandler->Q_Ref-PQStep) > tVSCHandler->Q_Cmd)
			tVSCHandler->Q_Ref -= PQStep;
		else
			tVSCHandler->Q_Ref = tVSCHandler->Q_Cmd;
//交流电压爬坡
		if((tVSCHandler->Vac_Ref+VacStep) < tVSCHandler->Vac_Cmd)
			tVSCHandler->Vac_Ref += VacStep;
		else if((tVSCHandler->Vac_Ref-VacStep) > tVSCHandler->Vac_Cmd)
			tVSCHandler->Vac_Ref -= VacStep;
		else
			tVSCHandler->Vac_Ref = tVSCHandler->Vac_Cmd;
	}
}


/*************************************************
 Function: VSCInit
 Description: VSC初始化
 Input: VSC Handle
 Output: 无
 Return:
 Others:数据更新、主状态更新、指令爬坡
 *************************************************/
void VSCInit(tVSC_CTL* tVSCHandler)
{
	tVSCHandler->Init();
	VSCParasInit(tVSCHandler);
	VSCMainFSM_Init(tVSCHandler);
}
/*************************************************
 Function: VSCFastTask
 Description: VSC快速任务
 Input: VSC Handle
 Output: 无
 Return:
 Others:数据更新、主状态更新、指令爬坡
 *************************************************/
void VSCFastTask(tVSC_CTL* tVSCHandler)
{
	VSCLocalParasUpdate(tVSCHandler);
	VSCMainFSM(tVSCHandler);
	VSCCmd_Slope(tVSCHandler);
}
/*************************************************
 Function: VSCMidTask
 Description: VSC中速任务
 Input: VSC Handle
 Output: 无
 Return:
 Others:故障指示灯、故障清除按键
 *************************************************/
void VSCMidTask(tVSC_CTL* tVSCHandler)
{
	if((tVSCHandler->MainStatus == _FAULT))
		tVSCHandler->CFaultLedCtl(1);
	else
		tVSCHandler->CFaultLedCtl(0);
	//复位按键
	if(tVSCHandler->RstSwSts())
	{
		if(tVSCHandler->MainStatus != RUN)			//防止误触发
			tVSCHandler->ClrErr();
	}
}

void VSCSlowTask(tVSC_CTL* tVSCHandler)
{
}


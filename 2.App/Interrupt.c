/**
  ******************************************************************************
  * @file    SysInit.c
  * @author  RZ
  * @brief   Interrupt.c module
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
#include <math.h>

SINCOS_t ThetaCCSC_GridSincos;

volatile u16 DCI_Lim_Triged = 0;

void PL_IntrHandler(void)
{
/****************************VSC1 Calc Start****************************/
//模拟信号输入计算
	VSCAnalogNormaliz(&Ctl_VSC1);
// 电压电流有效值计算与滤波，功率计算与滤波
	VSCRMS_PQCalc(&Ctl_VSC1);
	if(Ctl_VSC1.CtlMode != VACCTL){
		// 跟网控制
		//检测电压电流坐标变换及PLL
		clarke(&Ctl_VSC1.UGrid.P3S,&Ctl_VSC1.UGrid.P2S);									//3S-2S
		GetOmegaTheta(&Ctl_VSC1);															//PLL
		park(&Ctl_VSC1.UGrid.P2S,&Ctl_VSC1.UGrid.P2R,&Ctl_VSC1.Theta_GridSincos);			//2S-2R
		//电流坐标变换
		Ctl_VSC1.ThetaPhase = Ctl_VSC1.Theta-THETA30DEG;
		clarke(&Ctl_VSC1.IGrid.P3S,&Ctl_VSC1.IGrid.P2S);									//3S-2S
		arm_sin_cos_f32_1(Ctl_VSC1.ThetaPhase,&Ctl_VSC1.ThetaPhase_GridSincos);
		park(&Ctl_VSC1.IGrid.P2S,&Ctl_VSC1.IGrid.P2R,&Ctl_VSC1.ThetaPhase_GridSincos);		//2S-2R
		if(Ctl_VSC1.CtlMode == PQCTL){
			// 跟网控制
			// 功率外环,外环含PI的跟网定功率控制
			// 有功分量
			Ctl_VSC1.P_PID.Ref = Ctl_VSC1.P_Ref;
			Ctl_VSC1.P_PID.FeedBack = Ctl_VSC1.P_AC_AVG;
			PIDProc_Int_Sepa(&Ctl_VSC1.P_PID);
			// 无功分量,控制信号需要反相
			Ctl_VSC1.Q_PID.Ref = -Ctl_VSC1.Q_Ref;
			Ctl_VSC1.Q_PID.FeedBack = -Ctl_VSC1.Q_AC_AVG;
			PIDProc_Int_Sepa(&Ctl_VSC1.Q_PID);
		}
	}else{
		// 构网控制
		// 功率外环
		// VF控制
		if(Ctl_VSC1.GFMCtlMode == VFCTL){
			if(Ctl_VSC1.GFMCtlMode != Ctl_VSC1.GFMCtlMode_Pre){
				// 切换构网控制模式后初次进入，清零Theta值
				Ctl_VSC1.Theta = 0;
				Ctl_VSC1.GFMCtlMode_Pre = Ctl_VSC1.GFMCtlMode;
			}
			Ctl_VSC1.Omega = 50.0f*2*PI;
			Ctl_VSC1.Theta += Ctl_VSC1.Omega/INTFRE; // VCO
			//交流电压通过辅助下发的第四个附参设定，通过tVSCHandler->Vac_Cmd传递给tVSCHandler->Vac_ref
		}else if(Ctl_VSC1.GFMCtlMode == DROOPCTL){
			// 下垂控制
			if(Ctl_VSC1.GFMCtlMode != Ctl_VSC1.GFMCtlMode_Pre){
				// 切换到这个模式时，清PQ_PID和交流电压PID的I,清零Theta值
				Ctl_VSC1.Vd_PID.I = 0;
				Ctl_VSC1.Vq_PID.I = 0;
				Ctl_VSC1.Theta = 0;
				Ctl_VSC1.GFMCtlMode_Pre = Ctl_VSC1.GFMCtlMode;
			}
						// 20240912修改下垂控制
		    Ctl_VSC1.Omega =((Ctl_VSC1.P_Ref - Ctl_VSC1.P_AC_AVG)*Dp_Droop+1.0f)*Omega0;
			Ctl_VSC1.Theta += Ctl_VSC1.Omega/INTFRE; // 累加得到相角,INTFRE为中断频率
			// Ctl_VSC1.Vmag  =((Ctl_VSC1.Q_Ref - Ctl_VSC1.Q_AC_AVG)*Dq_Droop+1.0f);//添加无功积分环节

			Ctl_VSC1.deltaVmag +=(Ctl_VSC1.Q_Ref - Ctl_VSC1.Q_AC_AVG)*Dq_Droop/INTFRE;//添加无功积分环节
			Ctl_VSC1.Vmag = Ctl_VSC1.deltaVmag+1.0f;

			// 无功PI控制
			// Ctl_VSC1.Q_PID.Ref = -Ctl_VSC1.Q_Ref;
			// Ctl_VSC1.Q_PID.FeedBack = -Ctl_VSC1.Q_AC_AVG;
			// PIDProc_Int_Sepa(&Ctl_VSC1.Q_PID);
			// Ctl_VSC1.Vmag = Ctl_VSC1.Q_PID.Out+1.0f;
			Ctl_VSC1.Vac_Ref = Ctl_VSC1.Vmag;
		    debug1=10;
		}else if(Ctl_VSC1.GFMCtlMode == VSGCTL){
			// 20240912修改VSG控制
			debug2=20;
			Ctl_VSC1.deltaOmega += ((Ctl_VSC1.P_Ref - Ctl_VSC1.P_AC_AVG)*NORM_S/Omega0-Dpvsg*Ctl_VSC1.deltaOmega)/Jvsg/INTFRE;
			Ctl_VSC1.Omega = (Ctl_VSC1.deltaOmega + 1.0)*2.0*PI*50.0;
			Ctl_VSC1.Theta += Ctl_VSC1.Omega/INTFRE; // 累加得到相角,INTFRE为中断频率
			Ctl_VSC1.deltaVmag += ((Ctl_VSC1.Q_Ref - Ctl_VSC1.Q_AC_AVG)*NORM_S-Dqvsg*Ctl_VSC1.deltaOmega)/Kqvsg/INTFRE;
			Ctl_VSC1.Vmag = (Ctl_VSC1.deltaVmag+E0)/NORM_V;
			Ctl_VSC1.Vac_Ref = Ctl_VSC1.Vmag;
		}
		else{
			//TODO 统一构网控制
		    //有功P-w环控制
			Ctl_VSC1.deltaOmega += (UniA*(Ctl_VSC1.P_Ref - Ctl_VSC1.P_AC_AVG)-UniB*Ctl_VSC1.deltaOmega)/INTFRE;
			Ctl_VSC1.Omega = (Ctl_VSC1.deltaOmega + Omega0)*50.0f*2.0f*PI;
			//无功Q-V环控制
			Ctl_VSC1.deltaVmag += (UniC*(Ctl_VSC1.Q_Ref - Ctl_VSC1.Q_AC_AVG)-UniD*Ctl_VSC1.deltaVmag)/INTFRE;
			Ctl_VSC1.Vmag = Ctl_VSC1.deltaVmag+E0;
			Ctl_VSC1.Vac_Ref = Ctl_VSC1.Vmag;
		}
		//构网功率外环得到Theta和Vmag,利用Theta进行网侧电压和电流的派克变换
		if(Ctl_VSC1.Theta > THETAMAX)
		{
			Ctl_VSC1.Theta -= THETAMAX;
		}
		else if(Ctl_VSC1.Theta < THETAMIN)
			Ctl_VSC1.Theta += THETAMAX;
		arm_sin_cos_f32_1(Ctl_VSC1.Theta, &Ctl_VSC1.Theta_GridSincos);
		//PCS网侧电压坐标变换
		clarke(&Ctl_VSC1.UGrid.P3S,&Ctl_VSC1.UGrid.P2S);
		park(&Ctl_VSC1.UGrid.P2S,&Ctl_VSC1.UGrid.P2R,&Ctl_VSC1.Theta_GridSincos);
		//PCS网侧电流坐标变换
		Ctl_VSC1.ThetaPhase = Ctl_VSC1.Theta-THETA30DEG;
		//VF控制空载时不需要对电流进行坐标变换
		clarke(&Ctl_VSC1.IGrid.P3S,&Ctl_VSC1.IGrid.P2S);
		arm_sin_cos_f32_1(Ctl_VSC1.ThetaPhase,&Ctl_VSC1.ThetaPhase_GridSincos);
		park(&Ctl_VSC1.IGrid.P2S,&Ctl_VSC1.IGrid.P2R,&Ctl_VSC1.ThetaPhase_GridSincos);
	}
	//控制环路计算
	VSCControlLoop(&Ctl_VSC1);
/****************************VSC1 Calc End ****************************/

/****************************DCDC1 Calc Start****************************/

/****************************DCDC1 Calc End****************************/

//故障处理及指令下发
	FaultProc();
	CmdDown();

//软定时器控制
	TimerProc(&MainTimer1);
	TimerProc(&MainTimer2);
	TimerProc(&MainTimer3);
//DAC输出控制
	// SetDAC((Ctl_VSC1.P_Ref),10.0f,sAo0,0);
	// SetDAC((Ctl_VSC1.P_AC_AVG),10.0f,sAo0,1);
	// SetDAC((fabs(Ctl_VSC1.P_PID.Err)),10.0f,sAo0,2);
    SetDAC((Ctl_VSC1.Theta),10.0f,sAo0,3);
//	SetDAC((Ctl_VSC1.Vac_Ref),10.0f,sAo0,0);
	// SetDAC((Ctl_VSC1.IGrid.P3S.a),10.0f,sAo0,0);
	SetDAC((Ctl_VSC1.IGrid.P3S.a),10.0f,sAo0,0);
	SetDAC((Ctl_VSC1.IGrid.P3S.b),10.0f,sAo0,1);
	SetDAC((Ctl_VSC1.Vac_Cmd),10.0f,sAo0,2);
	// SetDAC((Ctl_VSC1.IGrid.P3S.a),1.2f,sAo0,0);//并网电流
	// SetDAC((Ctl_VSC1.UGrid.P3S.a),1.2f,sAo0,1);//并网电压
}


/*************************************************
 Function: FaultProc
 Description: 故障处理单元
 Input: 无
 Output: 无
 Return: 无
 Others: //需要根据每一个硬件单独增加
 *************************************************/
void FaultProc()
{
	if(SysChkPlatform())			//系统通信故障
	{
		Ctl_VSC1.gSysErrReg |= ERR_BOARDLINK_HW;
	}
	VSCFaultDet(&Ctl_VSC1);
}


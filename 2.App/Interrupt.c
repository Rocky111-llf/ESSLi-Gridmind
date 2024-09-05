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
			Ctl_VSC1.Theta += 50*2*PI/INTFRE; // VCO
			if(Ctl_VSC1.Theta > THETAMAX)
			{
				Ctl_VSC1.Theta -= THETAMAX;
			}
			else if(Ctl_VSC1.Theta < THETAMIN)
				Ctl_VSC1.Theta += THETAMAX;
			arm_sin_cos_f32_1(Ctl_VSC1.Theta, &Ctl_VSC1.Theta_GridSincos);
			//电压坐标变换
			clarke(&Ctl_VSC1.UGrid.P3S,&Ctl_VSC1.UGrid.P2S);
			park(&Ctl_VSC1.UGrid.P2S,&Ctl_VSC1.UGrid.P2R,&Ctl_VSC1.Theta_GridSincos);
			//电流坐标变换
			Ctl_VSC1.ThetaPhase = Ctl_VSC1.Theta-THETA30DEG;
			// VF控制空载时不需要对电流进行坐标变换
			// clarke(&Ctl_VSC1.IGrid.P3S,&Ctl_VSC1.IGrid.P2S);
			arm_sin_cos_f32_1(Ctl_VSC1.ThetaPhase,&Ctl_VSC1.ThetaPhase_GridSincos);
			// park(&Ctl_VSC1.IGrid.P2S,&Ctl_VSC1.IGrid.P2R,&Ctl_VSC1.ThetaPhase_GridSincos);
		}else{
			// TODO：其他构网控制
		}
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
	SetDAC((Ctl_VSC1.P_Ref),10.0f,sAo0,0);
	SetDAC((Ctl_VSC1.P_AC_AVG),10.0f,sAo0,1);
	// SetDAC((Ctl_VSC1.IGrid.P3S.c),10.0f,sAo0,2);
	// SetDAC((Ctl_VSC1.Theta),1.0,sAo0,3);
//	SetDAC((Ctl_VSC1.IGrid.P3S.a),1.2f,sAo0,4);
//	SetDAC((Ctl_VSC1.IGrid.P3S.b),1.2f,sAo0,5);
//	SetDAC((Ctl_VSC1.IGrid.P3S.c),1.2f,sAo0,6);
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


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
//ģ���ź��������
	VSCAnalogNormaliz(&Ctl_VSC1);
//����ѹ��������任��PLL
	clarke(&Ctl_VSC1.UGrid.P3S,&Ctl_VSC1.UGrid.P2S);									//3S-2S
	GetOmegaTheta(&Ctl_VSC1);															//PLL
	park(&Ctl_VSC1.UGrid.P2S,&Ctl_VSC1.UGrid.P2R,&Ctl_VSC1.Theta_GridSincos);			//2S-2R
//��������任
	Ctl_VSC1.ThetaPhase = Ctl_VSC1.Theta-THETA30DEG;
	clarke(&Ctl_VSC1.IGrid.P3S,&Ctl_VSC1.IGrid.P2S);									//3S-2S
	arm_sin_cos_f32_1(Ctl_VSC1.ThetaPhase,&Ctl_VSC1.ThetaPhase_GridSincos);
	park(&Ctl_VSC1.IGrid.P2S,&Ctl_VSC1.IGrid.P2R,&Ctl_VSC1.ThetaPhase_GridSincos);		//2S-2R
//���ƻ�·����
	VSCControlLoop(&Ctl_VSC1);
/****************************VSC1 Calc End ****************************/

/****************************DCDC1 Calc Start****************************/

/****************************DCDC1 Calc End****************************/

//���ϴ���ָ���·�
	FaultProc();
	CmdDown();

	VSCRMS_PQCalc(&Ctl_VSC1);

//��ʱ������
	TimerProc(&MainTimer1);
	TimerProc(&MainTimer2);
	TimerProc(&MainTimer3);
//DAC�������
//	SetDAC((Ctl_VSC1.UGrid.P3S.a),1.2f,sAo0,0);
//	SetDAC((Ctl_VSC1.UGrid.P3S.b),1.2f,sAo0,1);
//	SetDAC((Ctl_VSC1.UGrid.P3S.c),1.2f,sAo0,2);
//	SetDAC((Ctl_VSC1.Vdc),400,sAo0,3);
//	SetDAC((Ctl_VSC1.IGrid.P3S.a),1.2f,sAo0,4);
//	SetDAC((Ctl_VSC1.IGrid.P3S.b),1.2f,sAo0,5);
//	SetDAC((Ctl_VSC1.IGrid.P3S.c),1.2f,sAo0,6);
	SetDAC((Ctl_VSC1.IGrid.P3S.a),1.2f,sAo0,0);//��������
	SetDAC((Ctl_VSC1.UGrid.P3S.a),1.2f,sAo0,1);//������ѹ
}


/*************************************************
 Function: FaultProc
 Description: ���ϴ���Ԫ
 Input: ��
 Output: ��
 Return: ��
 Others: //��Ҫ����ÿһ��Ӳ����������
 *************************************************/
void FaultProc()
{
	if(SysChkPlatform())			//ϵͳͨ�Ź���
	{
		Ctl_VSC1.gSysErrReg |= ERR_BOARDLINK_HW;
	}
	VSCFaultDet(&Ctl_VSC1);
}


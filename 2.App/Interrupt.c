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
//ģ���ź��������
	VSCAnalogNormaliz(&Ctl_VSC1);
// ��ѹ������Чֵ�������˲������ʼ������˲�
	VSCRMS_PQCalc(&Ctl_VSC1);
	if(Ctl_VSC1.CtlMode != VACCTL){
		// ��������
		//����ѹ��������任��PLL
		clarke(&Ctl_VSC1.UGrid.P3S,&Ctl_VSC1.UGrid.P2S);									//3S-2S
		GetOmegaTheta(&Ctl_VSC1);															//PLL
		park(&Ctl_VSC1.UGrid.P2S,&Ctl_VSC1.UGrid.P2R,&Ctl_VSC1.Theta_GridSincos);			//2S-2R
		//��������任
		Ctl_VSC1.ThetaPhase = Ctl_VSC1.Theta-THETA30DEG;
		clarke(&Ctl_VSC1.IGrid.P3S,&Ctl_VSC1.IGrid.P2S);									//3S-2S
		arm_sin_cos_f32_1(Ctl_VSC1.ThetaPhase,&Ctl_VSC1.ThetaPhase_GridSincos);
		park(&Ctl_VSC1.IGrid.P2S,&Ctl_VSC1.IGrid.P2R,&Ctl_VSC1.ThetaPhase_GridSincos);		//2S-2R
		if(Ctl_VSC1.CtlMode == PQCTL){
			// ��������
			// �����⻷,�⻷��PI�ĸ��������ʿ���
			// �й�����
			Ctl_VSC1.P_PID.Ref = Ctl_VSC1.P_Ref;
			Ctl_VSC1.P_PID.FeedBack = Ctl_VSC1.P_AC_AVG;
			PIDProc_Int_Sepa(&Ctl_VSC1.P_PID);
			// �޹�����,�����ź���Ҫ����
			Ctl_VSC1.Q_PID.Ref = -Ctl_VSC1.Q_Ref;
			Ctl_VSC1.Q_PID.FeedBack = -Ctl_VSC1.Q_AC_AVG;
			PIDProc_Int_Sepa(&Ctl_VSC1.Q_PID);
		}
	}else{
		// ��������
		// �����⻷
		// VF����
		if(Ctl_VSC1.GFMCtlMode == VFCTL){
			if(Ctl_VSC1.GFMCtlMode != Ctl_VSC1.GFMCtlMode_Pre){
				// �л���������ģʽ����ν��룬����Thetaֵ
				Ctl_VSC1.Theta = 0;
				Ctl_VSC1.GFMCtlMode_Pre = Ctl_VSC1.GFMCtlMode;
			}
			Ctl_VSC1.Omega = 50.0f*2*PI;
			Ctl_VSC1.Theta += Ctl_VSC1.Omega/INTFRE; // VCO
			//������ѹͨ�������·��ĵ��ĸ������趨��ͨ��tVSCHandler->Vac_Cmd���ݸ�tVSCHandler->Vac_ref
		}else if(Ctl_VSC1.GFMCtlMode == DROOPCTL){
			// �´�����
			if(Ctl_VSC1.GFMCtlMode != Ctl_VSC1.GFMCtlMode_Pre){
				// �л������ģʽʱ����PQ_PID�ͽ�����ѹPID��I,����Thetaֵ
				Ctl_VSC1.Vd_PID.I = 0;
				Ctl_VSC1.Vq_PID.I = 0;
				Ctl_VSC1.Theta = 0;
				Ctl_VSC1.GFMCtlMode_Pre = Ctl_VSC1.GFMCtlMode;
			}
						// 20240912�޸��´�����
		    Ctl_VSC1.Omega =((Ctl_VSC1.P_Ref - Ctl_VSC1.P_AC_AVG)*Dp_Droop+1.0f)*Omega0;
			Ctl_VSC1.Theta += Ctl_VSC1.Omega/INTFRE; // �ۼӵõ����,INTFREΪ�ж�Ƶ��
			// Ctl_VSC1.Vmag  =((Ctl_VSC1.Q_Ref - Ctl_VSC1.Q_AC_AVG)*Dq_Droop+1.0f);//����޹����ֻ���

			Ctl_VSC1.deltaVmag +=(Ctl_VSC1.Q_Ref - Ctl_VSC1.Q_AC_AVG)*Dq_Droop/INTFRE;//����޹����ֻ���
			Ctl_VSC1.Vmag = Ctl_VSC1.deltaVmag+1.0f;

			// �޹�PI����
			// Ctl_VSC1.Q_PID.Ref = -Ctl_VSC1.Q_Ref;
			// Ctl_VSC1.Q_PID.FeedBack = -Ctl_VSC1.Q_AC_AVG;
			// PIDProc_Int_Sepa(&Ctl_VSC1.Q_PID);
			// Ctl_VSC1.Vmag = Ctl_VSC1.Q_PID.Out+1.0f;
			Ctl_VSC1.Vac_Ref = Ctl_VSC1.Vmag;
		    debug1=10;
		}else if(Ctl_VSC1.GFMCtlMode == VSGCTL){
			// 20240912�޸�VSG����
			debug2=20;
			Ctl_VSC1.deltaOmega += ((Ctl_VSC1.P_Ref - Ctl_VSC1.P_AC_AVG)*NORM_S/Omega0-Dpvsg*Ctl_VSC1.deltaOmega)/Jvsg/INTFRE;
			Ctl_VSC1.Omega = (Ctl_VSC1.deltaOmega + 1.0)*2.0*PI*50.0;
			Ctl_VSC1.Theta += Ctl_VSC1.Omega/INTFRE; // �ۼӵõ����,INTFREΪ�ж�Ƶ��
			Ctl_VSC1.deltaVmag += ((Ctl_VSC1.Q_Ref - Ctl_VSC1.Q_AC_AVG)*NORM_S-Dqvsg*Ctl_VSC1.deltaOmega)/Kqvsg/INTFRE;
			Ctl_VSC1.Vmag = (Ctl_VSC1.deltaVmag+E0)/NORM_V;
			Ctl_VSC1.Vac_Ref = Ctl_VSC1.Vmag;
		}
		else{
			//TODO ͳһ��������
		    //�й�P-w������
			Ctl_VSC1.deltaOmega += (UniA*(Ctl_VSC1.P_Ref - Ctl_VSC1.P_AC_AVG)-UniB*Ctl_VSC1.deltaOmega)/INTFRE;
			Ctl_VSC1.Omega = (Ctl_VSC1.deltaOmega + Omega0)*50.0f*2.0f*PI;
			//�޹�Q-V������
			Ctl_VSC1.deltaVmag += (UniC*(Ctl_VSC1.Q_Ref - Ctl_VSC1.Q_AC_AVG)-UniD*Ctl_VSC1.deltaVmag)/INTFRE;
			Ctl_VSC1.Vmag = Ctl_VSC1.deltaVmag+E0;
			Ctl_VSC1.Vac_Ref = Ctl_VSC1.Vmag;
		}
		//���������⻷�õ�Theta��Vmag,����Theta���������ѹ�͵������ɿ˱任
		if(Ctl_VSC1.Theta > THETAMAX)
		{
			Ctl_VSC1.Theta -= THETAMAX;
		}
		else if(Ctl_VSC1.Theta < THETAMIN)
			Ctl_VSC1.Theta += THETAMAX;
		arm_sin_cos_f32_1(Ctl_VSC1.Theta, &Ctl_VSC1.Theta_GridSincos);
		//PCS�����ѹ����任
		clarke(&Ctl_VSC1.UGrid.P3S,&Ctl_VSC1.UGrid.P2S);
		park(&Ctl_VSC1.UGrid.P2S,&Ctl_VSC1.UGrid.P2R,&Ctl_VSC1.Theta_GridSincos);
		//PCS�����������任
		Ctl_VSC1.ThetaPhase = Ctl_VSC1.Theta-THETA30DEG;
		//VF���ƿ���ʱ����Ҫ�Ե�����������任
		clarke(&Ctl_VSC1.IGrid.P3S,&Ctl_VSC1.IGrid.P2S);
		arm_sin_cos_f32_1(Ctl_VSC1.ThetaPhase,&Ctl_VSC1.ThetaPhase_GridSincos);
		park(&Ctl_VSC1.IGrid.P2S,&Ctl_VSC1.IGrid.P2R,&Ctl_VSC1.ThetaPhase_GridSincos);
	}
	//���ƻ�·����
	VSCControlLoop(&Ctl_VSC1);
/****************************VSC1 Calc End ****************************/

/****************************DCDC1 Calc Start****************************/

/****************************DCDC1 Calc End****************************/

//���ϴ���ָ���·�
	FaultProc();
	CmdDown();

//��ʱ������
	TimerProc(&MainTimer1);
	TimerProc(&MainTimer2);
	TimerProc(&MainTimer3);
//DAC�������
	// SetDAC((Ctl_VSC1.P_Ref),10.0f,sAo0,0);
	// SetDAC((Ctl_VSC1.P_AC_AVG),10.0f,sAo0,1);
	// SetDAC((fabs(Ctl_VSC1.P_PID.Err)),10.0f,sAo0,2);
    SetDAC((Ctl_VSC1.Theta),10.0f,sAo0,3);
//	SetDAC((Ctl_VSC1.Vac_Ref),10.0f,sAo0,0);
	// SetDAC((Ctl_VSC1.IGrid.P3S.a),10.0f,sAo0,0);
	SetDAC((Ctl_VSC1.IGrid.P3S.a),10.0f,sAo0,0);
	SetDAC((Ctl_VSC1.IGrid.P3S.b),10.0f,sAo0,1);
	SetDAC((Ctl_VSC1.Vac_Cmd),10.0f,sAo0,2);
	// SetDAC((Ctl_VSC1.IGrid.P3S.a),1.2f,sAo0,0);//��������
	// SetDAC((Ctl_VSC1.UGrid.P3S.a),1.2f,sAo0,1);//������ѹ
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


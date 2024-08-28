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

#define instVSC				Ctl_VSC1		//VSCʵ��
#define instDIDO			sDio0

#define PORT_CAN	CAN0					//����ͨ��CAN�ӿ�
#define PORT_BAUD	CANBAUD_250

#define LI_SNUM				(68)		//�����ش�����
#define PB_VCELL			(3.2)		//������ѹ


#define P_NORM				(3000)
#define VDC_MAX				(VDC_MAX_CELL*LI_SNUM)			//��������ѹ
#define VDC_MIN				(VDC_MIN_CELL*LI_SNUM)			//��������ѹ
#define IDC_MAX				(18)							//����������

//����ʽ��磬����-��ѹ-����,���ѹ����������
#define CHGI_TC			(0.5f)			//���������,A
#define CHGV_TC2CC		(2.9f)			//���������ֵ��ѹ�����ڸõ�ѹʱ�ɲ��ú������,V
#define CHGI_CC			(10.0f)			//���������������A
#define CHGV_CV			(3.6f)			//�����ѹ��ѹ��V
#define CHGI_CV2OK		(0.5f)			//����ת��������A

#define VDC_MAX_CELL	(3.6f)			//������߹�����ѹ
#define VDC_MIN_CELL	(2.8f)			//������͹�����ѹ

#define VDC_CHMAX_CELL		(3.58f)			//���䵥���ѹ������BMS����ֵ��Ϊ3.6V��
#define VDC_DISCHMIN_CELL	(2.85f)			//���ŵ����ѹ������BMS����ֵ��Ϊ2.8V��
#define VDC_CHDISCH_THR		(0.2f)			//������Żز�ָ���ѹ
#define SOC_DISCHMIN		(30.0f)			//����SOC


/***********���뿪������***********/
//iBMS�澯�ɽӵ�
#define IsBMSDry()			0//(instDIDO.Dat->bit.DI3)
//i��ͣ�ܿ���
#define IsEmSw()			(IsSysEmSw())
//o���ȿ���
#define CFanCtl_(A)			(SysFanCtl(A))
//o����ָʾ��
#define CRunLedCtl_(A)		(SysRunLedCtl(A))
//o����ָʾ��
#define CFaultLedCtl_(A)	(SysFaultLedCtl(A))
/***********���뿪���������***********/

#define PQ_SLOP_STEP 		(0.01f)		//�������£�ÿms����

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
/***********PID��ʼֵ���ý���***********/


//���ز�������������
u16 VarDef(BMSDrySts__,DevID)(void)						//������·��״̬����
{
#ifdef IsBMSDry
	u16 Rtn = IsBMSDry();
	return Rtn;
#else
	return 1;
#endif
}
u16 VarDef(EmSw__,DevID)(void)							//��ͣ����״̬����������Ϊ�ͣ�
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
void VarDef(CFanCtl__,DevID)(u16 Cmd)					//���ȿ���
{
#ifdef CFanCtl_
	CFanCtl_(Cmd);
#endif
}
void VarDef(CRunLedCtl__,DevID)(u16 Cmd)				//����ָʾ�ƿ���
{
#ifdef CRunLedCtl_
	CRunLedCtl_(Cmd);
#endif
}
void VarDef(CFaultLedCtl__,DevID)(u16 Cmd)				//����ָʾ�ƿ���
{
#ifdef CFaultLedCtl_
	CFaultLedCtl_(Cmd);
#endif
}
void VarDef(ClrErr__,DevID)(void)						//�������
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

//�ṹ���ֵ
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


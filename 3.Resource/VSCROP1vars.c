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

#define instROP		sRop0				//ROP��ʵ��
#define instOpt		sOpt0				//���˰�ʵ��
#define Is3P4W		0
#define OPT_A		0					//���˰�A��ͨ��
#define OPT_B		1					//���˰�B��ͨ��
#define OPT_C		2					//���˰�C��ͨ��
#if Is3P4W
#define OPT_N		0					//���˰�N��ͨ��
#endif

#define PQ_SLOP_STEP 		(0.0001f)	//��������
#define VDC_SLOP_STEP		(0.001f)	//ֱ����ѹ���� pu/ms
#define VAC_SLOP_STEP		(0.001f)	//������ѹ���� pu/ms

/***********���뿪������***********/
//o������·����բ����
#define AcBrkCtl_(A)	{instROP.Cmd->bit.DO.bit.DO3 = (A);}
//i������·������
#define IsAcBrk()		(instROP.Dat->bit.DI.bit.DI0)
//oֱ����·����բ����
#define DcBrkCtl_(A)	{instROP.Cmd->bit.DO.bit.DO2 = (A);}
//iֱ����·������
#define IsDcBrk()		(instROP.Dat->bit.DI.bit.DI1)
//o�����Ӵ�������
#define AcContCtl_(A)	{instROP.Cmd->bit.DO.bit.DO4 = (A);instROP.Cmd->bit.DO.bit.DO0 = (A);}
//i�����Ӵ�������
#define IsAcCont()		(instROP.Cmd->bit.DO.bit.DO4|instROP.Cmd->bit.DO.bit.DO0)
//oֱ���Ӵ�������
#define DcContCtl_(A)	{instROP.Cmd->bit.DO.bit.DO8 = (A);}
//iֱ���Ӵ�������
#define IsDcCont()		(instROP.Cmd->bit.DO.bit.DO8)
//o����Ԥ�����
#define AcPreChCtl_(A)	{instROP.Cmd->bit.DO.bit.DO5 = (A);instROP.Cmd->bit.DO.bit.DO1 = (A);}
//i����Ԥ�䷴��
#define IsAcPreCh()		(instROP.Cmd->bit.DO.bit.DO5|instROP.Cmd->bit.DO.bit.DO1)
//oֱ��Ԥ�����
#define DcPreChCtl_(A)	{instROP.Cmd->bit.DO.bit.DO7 = (A);}
//iֱ��Ԥ�䷴��
#define IsDcPreCh()		(instROP.Cmd->bit.DO.bit.DO7)
//o���ȿ���
#define CFanCtl_(A)		{instROP.Cmd->bit.DO.bit.DO6 = (A);}
//i��ͣ����
#define IsEmSw()		(!(instROP.Dat->bit.DI.bit.DI2))
//i��λ����
#define IsRstSw()		(instROP.Dat->bit.DI.bit.DI3)
//o����ָʾ��
#define CFaultLedCtl_(A)	{instROP.Cmd->bit.DO.bit.DO9 = (A);}
/***********���뿪���������***********/

/***********���ſ��Ƽ������źŶ���***********/
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
/***********���ſ��Ƽ������źŶ������***********/


/***********ADCͨ��ӳ��***********/
#define ACV_A_G				(instROP.Dat->bit.ADdat[10])		//������ѹA
#define ACV_B_G				(instROP.Dat->bit.ADdat[11])		//������ѹB
#define ACV_C_G				(instROP.Dat->bit.ADdat[9])			//������ѹC
#define ACI_A_G				(instROP.Dat->bit.ADdat[0])			//��������A
#define ACI_B_G				(instROP.Dat->bit.ADdat[2])			//��������B
#define ACI_C_G				(ACI_A_G)							//��������C
#define ACV_A_CAP			(instROP.Dat->bit.ADdat[8])			//���ݵ�ѹA
#define ACV_B_CAP			(instROP.Dat->bit.ADdat[6])			//���ݵ�ѹB
#define ACV_C_CAP			(instROP.Dat->bit.ADdat[7])			//���ݵ�ѹC
#define ACI_A_BR			(instROP.Dat->bit.ADdat[3])			//������A
#define ACI_B_BR			(instROP.Dat->bit.ADdat[1])			//������B
#define ACI_C_BR			(ACI_A_BR)							//������C
#define DCV_BUS				(instROP.Dat->bit.ADdat[4])			//ֱ��ĸ�ߵ�ѹ
#define DCI_BUS				(instROP.Dat->bit.ADdat[5])			//ֱ��ĸ�ߵ���
#define ADC_RSV1			(instROP.Dat->bit.ADdat[12])		//��������ͨ��
#define ADC_RSV2			(instROP.Dat->bit.ADdat[13])		//��������ͨ��
#define ADC_RSV3			(instROP.Dat->bit.ADdat[14])		//��������ͨ��
#define ADC_RSV4			(instROP.Dat->bit.ADdat[15])		//��������ͨ��
/***********ADCͨ��ӳ�����***********/

/***********PID��ʼֵ����***********/
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
//��������PID
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
/***********PID��ʼֵ���ý���***********/


//���ز�������������
void VarDef(AcBrkCtl__,DevID)(u16 Cmd)					//������·������
{
#ifdef AcBrkCtl_
	AcBrkCtl_(Cmd);
#endif
}
u16 VarDef(AcBrkSts__,DevID)(void)						//������·��״̬����
{
#ifdef IsAcBrk
	u16 Rtn = IsAcBrk();
	return Rtn;
#else
	return 0;
#endif
}
void VarDef(AcContCtl__,DevID)(u16 Cmd)					//�����Ӵ�������
{
#ifdef AcContCtl_
	AcContCtl_(Cmd);
#endif
}
u16 VarDef(AcContSts__,DevID)(void)						//�����Ӵ���״̬����
{
#ifdef IsAcCont
	u16 Rtn = IsAcCont();
	return Rtn;
#else
	return 0;
#endif
}
void VarDef(AcPreChCtl__,DevID)(u16 Cmd)				//����Ԥ�����
{
#ifdef AcPreChCtl_
	AcPreChCtl_(Cmd);
#endif
}
u16 VarDef(AcPreChSts__,DevID)(void)					//����Ԥ��״̬����
{
#ifdef IsAcPreCh
	u16 Rtn = IsAcPreCh();
	return Rtn;
#else
	return 0;
#endif
}
void VarDef(DcContCtl__,DevID)(u16 Cmd)					//ֱ���Ӵ�������
{
#ifdef DcContCtl_
	DcContCtl_(Cmd);
#endif
}
u16 VarDef(DcContSts__,DevID)(void)						//ֱ���Ӵ���״̬����
{
#ifdef IsDcCont
	u16 Rtn = IsDcCont();
	return Rtn;
#else
	return 0;
#endif
}
void VarDef(DcPreChCtl__,DevID)(u16 Cmd)				//ֱ��Ԥ�����
{
#ifdef DcPreChCtl_
	DcPreChCtl_(Cmd);
#endif
}
u16 VarDef(DcPreChSts__,DevID)(void)					//ֱ��Ԥ��״̬����
{
#ifdef IsDcPreCh
	u16 Rtn = IsDcPreCh();
	return Rtn;
#else
	return 0;
#endif
}
u16 VarDef(HBOK__,DevID)(void)							//H������״̬����
{
#ifdef Macro_HBOK
	u16 Rtn = Macro_HBOK;
	return Rtn;
#else
	return 0;
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
u16 VarDef(RstSw__,DevID)(void)							//��λ״̬����������Ч��
{
#ifdef IsRstSw
	u16 Rtn = IsRstSw();
	return Rtn;
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
void VarDef(CFaultLedCtl__,DevID)(u16 Cmd)				//����ָʾ�ƿ���
{
#ifdef CFaultLedCtl_
	CFaultLedCtl_(Cmd);
#endif
}
void VarDef(ClrErr__,DevID)(void)						//�������
{
	VarDef(Ctl,DevID).gErrClr = 3;
}
/*
#define FanCtl_(A)		{instROP.Cmd->bit.DO.bit.DO6 = (A);}
//i��ͣ����
#define IsEmSw()		(instROP.Dat->bit.DI.bit.DI2)
//i��λ����
#define IsRstSw()		(instROP.Dat->bit.DI.bit.DI3)
//o����ָʾ��
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

//�ṹ���ֵ
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



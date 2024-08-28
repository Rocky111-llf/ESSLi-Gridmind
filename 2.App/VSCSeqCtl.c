/*****************************************************************************
Copyright: 2019-2023, GridMind. Co., Ltd.
File name: VSCSeqCtl.c
Description: ˳������㷨
Author: RZ
Version: �汾
Date: �������
History: �޸���ʷ��¼�б� ÿ���޸ļ�¼Ӧ�����޸����ڡ��޸��߼��޸����ݼ�����
*****************************************************************************/
#include "Includings.h"


#define SetSSDelay(A)	(tVSCHandler->StartUpDelayCnt = (A))
#define SSDelayOK()		(!tVSCHandler->StartUpDelayCnt)
#define SetSS(A)		(tVSCHandler->StartUpStatus = (A))

void VSCStopMachine(tVSC_CTL* tVSCHandler);
void VSCRunCheck(tVSC_CTL* tVSCHandler);

#define ACPRE_TIME		(3000)

//��������״̬��״̬ö��
typedef enum {
	SSAC_Prepare = 0,				/*׼��״̬*/
	SSAC_FPGACheck,					/*FPGA״̬���*/
	SSAC_StParaInit,				/*����������ʼ��*/
	SSAC_SWChk,						/*��·��״̬���*/
	SSAC_GridChk,					/*����״̬��Ⲣ�������򿪱���*/
	SSAC_HBCfg,						/*H�����ü���λ*/
	SSAC_HBChk,						/*H��״̬���*/
	SSAC_PreChgDio,					/*��Ԥ��Ӵ�������������Ԥ���*/
	SSAC_PreChgCtl,					/*�ɿ�����*/
	SSAC_SWACDCOn,					/*�Ͻ���ֱ������*/
	SSAC_FANChk,					/*�����������������ģʽ*/
	SSAC_StartFail					/*����ʧ�ܣ�����������п��ز�����*/
} ACSTART_STEPS;

//ֱ������״̬��״̬ö��
typedef enum {
	SSDC_Prepare = 0,				/*׼��״̬*/
	SSDC_FPGACheck,					/*FPGA״̬���*/
	SSDC_StParaInit,				/*����������ʼ��*/
	SSDC_SWChk,						/*��·��״̬���*/
	SSDC_GridChk,					/*����״̬��Ⲣ�������򿪱������������Դ����������˲�*/
	SSDC_DcChk,						/*����ⲿֱ��ĸ�ߵ�ѹ*/
	SSDC_HBCfg,						/*H�����ü���λ*/
	SSDC_HBChk,						/*H��״̬���*/
	SSDC_PreChgDio,					/*��Ԥ��Ӵ�������������Ԥ���*/
	SSDC_PreChgCtl,					/*�ɿ�����*/
	SSDC_SWACDCOn,					/*�Ͻ���ֱ������*/
	SSDC_FANChk,					/*�������*/
	SSDC_StartFail					/*����ʧ�ܣ�����������п��ز�����*/
} DCSTART_STEPS;

/****************************************************************************
*
*��������״̬��
*
****************************************************************************/
/*�������̣�
Ԥ����ɺ󡢱���ģ�飬��ֱ���࿪��
*/
void VSCStartUp_ACPreChg(tVSC_CTL* tVSCHandler)
{
	u16 u16Temp;
	u16 StateChanged = 0;

	if(tVSCHandler->StartUpStatusPre != tVSCHandler->StartUpStatus)
	{
		StateChanged = 1;
		tVSCHandler->StartUpStatusPre = tVSCHandler->StartUpStatus;
	}
	switch(tVSCHandler->StartUpStatus){
	case SSAC_Prepare:								//׼��״̬
		tVSCHandler->StartUpStatusPre = SSAC_Prepare;
		tVSCHandler->StartFailReg = 0;
		tVSCHandler->StartUpCheckEn = 0;
		tVSCHandler->StartCheckFlag = 0;

		SetSS(SSAC_FPGACheck);
		break;
	case SSAC_FPGACheck:							//�忨��Ϣ���
		if(StateChanged)
		{//���ν���
			SetSSDelay(1000);
		}
		if(SSDelayOK())
		{
			u16Temp = SysChkPlatform();
			if(u16Temp != 0)
			{
				SetMSW(tVSCHandler->StartFailReg,u16Temp);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSAC_StartFail);
			}
			else
				SetSS(SSAC_StParaInit);
		}
		break;
	case SSAC_StParaInit:
		if(StateChanged)
		{//���ν���
			SetSSDelay(1000);
		}
		if(SSDelayOK())
		{
			VSCParasInit(tVSCHandler);
			tVSCHandler->gErrMask = (ERR_STARTFAIL);					//��������ʧ�ܱ���
			tVSCHandler->ClrErr();
			SetSS(SSAC_SWChk);
		}
		break;
	case SSAC_SWChk:									//
		if(StateChanged)
		{//���ν���
			SetSSDelay(1000);
			tVSCHandler->AcContCtl(0);
			tVSCHandler->DcContCtl(0);
			tVSCHandler->AcPreChCtl(0);
			tVSCHandler->DcPreChCtl(0);
		}
		if(SSDelayOK())
		{
			if(!((tVSCHandler->AcContSts())||(tVSCHandler->DcContSts())||(tVSCHandler->AcPreChSts())||(tVSCHandler->DcPreChSts())))		//���س�ʼλ������
				SetSS(SSAC_GridChk);
			else
			{
				u16Temp = tVSCHandler->AcContSts() + (tVSCHandler->DcContSts()<<1) +(tVSCHandler->AcPreChSts()<<2) + (tVSCHandler->DcPreChSts()<<3);
				SetMSW(tVSCHandler->StartFailReg,u16Temp);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSAC_StartFail);
			}
		}
		break;
	case SSAC_GridChk:
		if(StateChanged)
		{//���ν���
			SetSSDelay(500);				//���ü��ʱ��
			tVSCHandler->StartUpCheckEn |= (0x1|0x2);	//���ε�ѹ����ʱ��������
		}
		if(SSDelayOK())
		{
			if(tVSCHandler->StartCheckFlag&(0x1|0x2))
			{
				u16Temp = 0;
				SetMSW(tVSCHandler->StartFailReg,u16Temp);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSAC_StartFail);
			}
			else
				SetSS(SSAC_HBCfg);
		}
		break;
	case SSAC_HBCfg:
		if(StateChanged)
		{//���ν���
			SetSSDelay(500);							//���ü��ʱ��
			CfgCard();									//�·����а忨����
		}
		else
		{
			if(SSDelayOK())
				SetSS(SSAC_HBChk);
			else
			{
				u16Temp = (u16)tVSCHandler->pHBDat_A->CfgFlag + \
							   tVSCHandler->pHBDat_B->CfgFlag + \
							   tVSCHandler->pHBDat_C->CfgFlag;// +  tVSCHandler->pHBDat_N->CfgFlag;
				if(u16Temp != 3)
				{
					SetMSW(tVSCHandler->StartFailReg,u16Temp);
					SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
					SetSS(SSAC_StartFail);
				}
			}
		}
		break;
	case SSAC_HBChk:
		if(StateChanged)
		{//���ν���
			SetSSDelay(500);							//���ü��ʱ��
			tVSCHandler->gEnHBOut = 0;
			HBBlk(0);									//�ͷ�ȫ�ֱ���
			tVSCHandler->pHBCmd_A->Mode = HB_MODE_RST;
			tVSCHandler->pHBCmd_B->Mode = HB_MODE_RST;
			tVSCHandler->pHBCmd_C->Mode = HB_MODE_RST;
			//tVSCHandler->pHBCmd_N->Mode = HB_MODE_RST;
		}
		else
		{
			if(SSDelayOK())			//����ģ��Ӧ�ô��ڸ�λ״̬����ģ����Ч
				SetSS(SSAC_PreChgDio);
			else
			{
				if(tVSCHandler->HBOK())
				{
					u16Temp = (u16)(tVSCHandler->pHBDat_A->HBMode!=HB_MODE_RST) + \
							  (u16)(tVSCHandler->pHBDat_B->HBMode!=HB_MODE_RST) + \
							  (u16)(tVSCHandler->pHBDat_C->HBMode!=HB_MODE_RST);// + (u16)(tVSCHandler->pHBDat_A->HBMode!=HB_MODE_RST);

					if(u16Temp != 0)
					{
						SetMSW(tVSCHandler->StartFailReg,1);
						SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
						SetSS(SSAC_StartFail);
					}
				}
				else
				{
					SetMSW(tVSCHandler->StartFailReg,0);
					SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
					SetSS(SSAC_StartFail);
				}
			}
		}
		break;
	case SSAC_PreChgDio:									//Ԥ�俪��ǰ����Ҫ�������б���
		if(StateChanged)
		{//���ν���
			SetSSDelay(ACPRE_TIME);								//���ü��ʱ��
			tVSCHandler->gErrMask |= (ERR_PEM_SW);						//����ģ�鱣��
			tVSCHandler->StartUpCheckEn |= 0x4;
			tVSCHandler->AcPreChCtl(1);
		}
		if(SSDelayOK())
		{
			u16Temp = tVSCHandler->pHBDat_A->CapV;
			if((u16Temp > tVSCHandler->RMSInfo.RMS_Va*(RATED_ACV*sqrt2*ACPRECH_VOL_MIN)) && (u16Temp > tVSCHandler->RMSInfo.RMS_Vb*(RATED_ACV*sqrt2*ACPRECH_VOL_MIN)) && (u16Temp > tVSCHandler->RMSInfo.RMS_Vc*(RATED_ACV*sqrt2*ACPRECH_VOL_MIN)))
				SetSS(SSAC_SWACDCOn);
			else
			{
				SetMSW(tVSCHandler->StartFailReg,0);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSAC_StartFail);
			}
		}
		else if(tVSCHandler->StartUpDelayCnt == 1000)		//��鿪��
		{
//			if(!tVSCHandler->AcPreChSts())
//			{
//				u16Temp = 0x1000;				//Ԥ�俪�رպ�ʧ��
//				SetMSW(tVSCHandler->StartFailReg,u16Temp);
//				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
//				SetSS(SSAC_StartFail);
//			}
		}
		break;
	case SSAC_SWACDCOn:
		if(StateChanged)
		{//���ν���
			SetSSDelay(500);							//���ÿ��ؼ����Ϣ
			tVSCHandler->AcContCtl(1);					//�Ͻ����Ӵ���
			tVSCHandler->AcPreChCtl(0);					//�ֽ���Ԥ��
			tVSCHandler->DcContCtl(1);					//��ֱ���Ӵ���
		}
		if(SSDelayOK())
		{
//			if((tVSCHandler->AcContSts())&&(!tVSCHandler->AcPreChSts()))
				SetSS(SSAC_FANChk);
//			else
//			{
//				SetMSW(tVSCHandler->StartFailReg,0);
//				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
//				SetSS(SSAC_StartFail);
//			}
		}
		break;
	case SSAC_FANChk:
		tVSCHandler->gErrMask |= ERRx_EN_MASK;
		tVSCHandler->Vdc_Cmd = 1.0f;
		tVSCHandler->Vdc_Ref = tVSCHandler->DCV_Bus;				//������
		tVSCHandler->MainStatus = RUN;								//��������״̬
		tVSCHandler->pHBCmd_A->Mode = HB_MODE_PWM;					//ģ������ΪPWMģʽ
		tVSCHandler->pHBCmd_B->Mode = HB_MODE_PWM;
		tVSCHandler->pHBCmd_C->Mode = HB_MODE_PWM;
		//tVSCHandler->pHBCmd_N->Mode = HB_MODE_PWM;
		FANCtl = BOOL_TRUE;
		SysFanCtl(1);
		tVSCHandler->CFanCtl(1);
		break;
	case SSAC_StartFail:
		tVSCHandler->AcContCtl(0);
		tVSCHandler->AcPreChCtl(0);
		tVSCHandler->DcContCtl(0);
//		tVSCHandler->DcPreChCtl(0);
		tVSCHandler->gSysErrReg |= ERR_STARTFAIL;
		break;
	default:
		SetSS(SSAC_StartFail);
		break;
	}
	if(tVSCHandler->StartUpDelayCnt)
		tVSCHandler->StartUpDelayCnt--;

	if(tVSCHandler->StartCheckFlag)			//�������������б���
	{
		SetMSW(tVSCHandler->StartFailReg,tVSCHandler->StartCheckFlag);
		SetLSW(tVSCHandler->StartFailReg,SSAC_StartFail);
		SetSS(SSAC_StartFail);
	}
}


/*************************************************
 Function: StartUp_DCPreChg
 Description: ֱ��Ԥ��״̬��
 Input: ��
 Output: ��
 Return: 0:OK,Others:Fault
 Others: ����MMC�忨λ��Ϊ 5678
 *************************************************/
void VSCStartUp_DCPreChg(tVSC_CTL* tVSCHandler)
{
	u16 u16Temp;
	u16 StateChanged = 0;
	if(tVSCHandler->StartUpStatusPre != tVSCHandler->StartUpStatus)
	{
		StateChanged = 1;
		tVSCHandler->StartUpStatusPre = tVSCHandler->StartUpStatus;
	}
	switch(tVSCHandler->StartUpStatus){
	case SSDC_Prepare:								//׼��״̬
		tVSCHandler->StartUpStatusPre = SSDC_Prepare;
		tVSCHandler->StartFailReg = 0;
		tVSCHandler->StartUpCheckEn = 0;
		tVSCHandler->StartCheckFlag = 0;

		SetSS(SSDC_FPGACheck);
		break;
	case SSDC_FPGACheck:							//�忨��Ϣ���
		if(StateChanged)
		{//���ν���
			SetSSDelay(1000);
		}
		if(SSDelayOK())
		{
			u16Temp = SysChkPlatform();
			if(u16Temp != 0)
			{
				SetMSW(tVSCHandler->StartFailReg,u16Temp);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSDC_StartFail);
			}
			else
				SetSS(SSDC_StParaInit);
		}
		break;
	case SSDC_StParaInit:
		if(StateChanged)
		{//���ν���
			SetSSDelay(1000);
		}
		if(SSDelayOK())
		{
			VSCParasInit(tVSCHandler);
			tVSCHandler->gErrMask = (ERR_STARTFAIL);					//��������ʧ�ܱ���
			tVSCHandler->ClrErr();
			SetSS(SSDC_SWChk);
		}
		break;
	case SSDC_SWChk:						//
		if(StateChanged)
		{//���ν���
			SetSSDelay(1000);
			tVSCHandler->AcContCtl(0);
			tVSCHandler->DcContCtl(0);
			tVSCHandler->AcPreChCtl(0);
			tVSCHandler->DcPreChCtl(0);
		}
		if(SSDelayOK()) // ���ν�����·�����
		{
			// if(!((tVSCHandler->AcContSts())||(tVSCHandler->DcContSts())||(tVSCHandler->AcPreChSts())||(tVSCHandler->DcPreChSts())))		//���س�ʼλ������
			// {
			// 	if(tVSCHandler->CtlMode == VACCTL)
					SetSS(SSDC_DcChk);
			// 	else
			// 		SetSS(SSDC_GridChk);
			// }
			// else
			// {
			// 	u16Temp = tVSCHandler->AcContSts() + (tVSCHandler->DcContSts()<<1) +(tVSCHandler->AcPreChSts()<<2) + (tVSCHandler->DcPreChSts()<<3);
			// 	SetMSW(tVSCHandler->StartFailReg,u16Temp);
			// 	SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
			// 	SetSS(SSDC_StartFail);
			// }
		}
		break;
	case SSDC_GridChk:
		if(StateChanged)
		{//���ν���
			SetSSDelay(500);				//���ü��ʱ��
			tVSCHandler->StartUpCheckEn |= (0x1|0x2);	//���ε�ѹ����ʱ��������
		}
		if(SSDelayOK())
		{
			if(tVSCHandler->StartCheckFlag&(0x1|0x2))
			{
				u16Temp = 0;
				SetMSW(tVSCHandler->StartFailReg,u16Temp);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSDC_StartFail);
			}
			else
				SetSS(SSDC_HBCfg);			//��ǰ����ֱ����ѹ��⣬����Ϊֱ��Ԥ��������
		}
		break;
	case SSDC_DcChk:
		if(StateChanged)
		{//���ν���
			SetSSDelay(500);				//���ü��ʱ��
			// ����ֱ����ѹ���
			// tVSCHandler->StartUpCheckEn |= (0x8);		//����ֱ����ѹ���
		}
		if(SSDelayOK())
		{
			// if(tVSCHandler->StartCheckFlag&(0x8))
			// {
			// 	u16Temp = 0;
			// 	SetMSW(tVSCHandler->StartFailReg,u16Temp);
			// 	SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
			// 	SetSS(SSDC_StartFail);
			// }
			// else
				SetSS(SSDC_HBCfg);
		}
		break;
	case SSDC_HBCfg:
		if(StateChanged)
		{//���ν���
			SetSSDelay(500);							//���ü��ʱ��
			CfgCard();									//�·����а忨����
		}
		else
		{
			if(SSDelayOK())
				SetSS(SSDC_HBChk);
			else
			{
				u16Temp = (u16)tVSCHandler->pHBDat_A->CfgFlag + \
							   tVSCHandler->pHBDat_B->CfgFlag + \
							   tVSCHandler->pHBDat_C->CfgFlag;
							   //tVSCHandler->pHBDat_N->CfgFlag;
				if(u16Temp != 3)
				{
					SetMSW(tVSCHandler->StartFailReg,u16Temp);
					SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
					SetSS(SSAC_StartFail);
				}
			}
		}
		break;
	case SSDC_HBChk:
		if(StateChanged)
		{//���ν���
			SetSSDelay(500);							//���ü��ʱ��
			tVSCHandler->gEnHBOut = 0;
			HBBlk(0);									//�ͷ�ȫ�ֱ���
			tVSCHandler->pHBCmd_A->Mode = HB_MODE_RST;
			tVSCHandler->pHBCmd_B->Mode = HB_MODE_RST;
			tVSCHandler->pHBCmd_C->Mode = HB_MODE_RST;
			//tVSCHandler->pHBCmd_N->Mode = HB_MODE_RST;
		}
		else
		{
			if(SSDelayOK())			//����ģ��Ӧ�ô��ڸ�λ״̬����ģ����Ч
				SetSS(SSDC_PreChgDio);
			// else
			// {
			// 	if(tVSCHandler->HBOK())
			// 	{
			// 		u16Temp = (u16)(tVSCHandler->pHBDat_A->HBMode==HB_MODE_RST) + \
			// 				  (u16)(tVSCHandler->pHBDat_B->HBMode==HB_MODE_RST) + \
			// 				  (u16)(tVSCHandler->pHBDat_C->HBMode==HB_MODE_RST);
			// 				  //(u16)(tVSCHandler->pHBDat_N->HBMode==HB_MODE_RST);

			// 		if(u16Temp != 3)
			// 		{
			// 			SetMSW(tVSCHandler->StartFailReg,1);
			// 			SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
			// 			SetSS(SSAC_StartFail);
			// 		}
			// 	}
			// 	else
			// 	{
			// 		SetMSW(tVSCHandler->StartFailReg,0);
			// 		SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
			// 		SetSS(SSDC_StartFail);
			// 	}
			// }
		}
		break;
	case SSDC_PreChgDio:									//Ԥ�俪��ǰ����Ҫ�������б���
		if(StateChanged)
		{//���ν���
			SetSSDelay(3000);								//���ü��ʱ��
			tVSCHandler->gErrMask |= (ERR_PEM_SW);			//����ģ�鱣��
			tVSCHandler->StartUpCheckEn |= 0x4;
			tVSCHandler->DcPreChCtl(1);
		}
		if(SSDelayOK())
		{
			u16Temp = tVSCHandler->pHBDat_A->CapV;
			if((u16Temp > tVSCHandler->DCV_Bus*(RATED_DCV*DCPRECH_VOL_MIN)))
				SetSS(SSDC_SWACDCOn);
			else
			{
				SetMSW(tVSCHandler->StartFailReg,0);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSDC_StartFail);
			}
		}
		else if(tVSCHandler->StartUpDelayCnt == 2000)		//��鿪��
		{
//			if(!tVSCHandler->DcPreChSts())
//			{
//				u16Temp = 0x1000;				//Ԥ�俪�رպ�ʧ��
//				SetMSW(tVSCHandler->StartFailReg,u16Temp);
//				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
//				SetSS(SSDC_StartFail);
//			}
		}
		break;
	case SSDC_SWACDCOn:
		if(StateChanged)
		{//���ν���
			SetSSDelay(1000);							//���ÿ��ؼ����Ϣ
			tVSCHandler->AcContCtl(1);					//�Ͻ����Ӵ���
			tVSCHandler->AcPreChCtl(0);					//�ֽ���Ԥ��
			tVSCHandler->DcContCtl(1);					//��ֱ���Ӵ���
		}
		if(SSDelayOK())
		{
//			if((tVSCHandler->AcContSts())&&(tVSCHandler->DcContSts())&&(!tVSCHandler->AcPreChSts())&&(!tVSCHandler->DcPreChSts()))
				SetSS(SSDC_FANChk);
//			else
//			{
//				SetMSW(tVSCHandler->StartFailReg,0);
//				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
//				SetSS(SSDC_StartFail);
//			}
		}
		else if(tVSCHandler->StartUpDelayCnt == 500)
		{
			tVSCHandler->DcPreChCtl(0);					//�ӳٷ�ֱ��Ԥ��
		}
		break;
	case SSDC_FANChk:
		tVSCHandler->gErrMask |= ERRx_EN_MASK;
		tVSCHandler->MainStatus = RUN;					//��������״̬
		tVSCHandler->pHBCmd_A->Mode = HB_MODE_PWM;					//ģ������ΪPWMģʽ
		tVSCHandler->pHBCmd_B->Mode = HB_MODE_PWM;
		tVSCHandler->pHBCmd_C->Mode = HB_MODE_PWM;
		//tVSCHandler->pHBCmd_N->Mode = HB_MODE_PWM;
		FANCtl = BOOL_TRUE;
		SysFanCtl(1);
		tVSCHandler->CFanCtl(1);
		break;
	case SSDC_StartFail:
		tVSCHandler->AcContCtl(0);
		tVSCHandler->AcPreChCtl(0);
		tVSCHandler->DcContCtl(0);
		tVSCHandler->DcPreChCtl(0);
		tVSCHandler->gSysErrReg |= ERR_STARTFAIL;
		break;
	default:
		SetSS(SSDC_StartFail);
		break;
	}
	if(tVSCHandler->StartUpDelayCnt)
		tVSCHandler->StartUpDelayCnt--;

	if(tVSCHandler->StartCheckFlag)			//�������������б���
	{
		SetMSW(tVSCHandler->StartFailReg,tVSCHandler->StartCheckFlag);
		SetLSW(tVSCHandler->StartFailReg,SSDC_StartFail);
		SetSS(SSDC_StartFail);
	}
}

/*************************************************
 Function: AcStartUpCheck
 Description: ���������м�״̬���
 Input: ��
 Output: ��
 Return: 0:OK,Others:Fault
 Others: ���ڽ������������м��
 *************************************************/
void VSCAcStartUpCheck(tVSC_CTL* tVSCHandler)
{
	LimitProc(&tVSCHandler->FrePLL_Limit,tVSCHandler->PLLFre,START);
	LimitProc(&tVSCHandler->ACVINA_Limit,tVSCHandler->RMSInfo.RMS_Va,START);
	LimitProc(&tVSCHandler->ACVINA_Limit,tVSCHandler->RMSInfo.RMS_Vb,START);
	LimitProc(&tVSCHandler->ACVINA_Limit,tVSCHandler->RMSInfo.RMS_Vc,START);

	if(tVSCHandler->StartUpCheckEn & 0x1)		//���໷���
	{
		if(tVSCHandler->FrePLL_Limit.Result != LIMIT_OK)
			tVSCHandler->StartCheckFlag |= 0x1;
	}
	if(tVSCHandler->StartUpCheckEn & 0x2)		//�����ѹ���
	{
		if((tVSCHandler->ACVINA_Limit.Result)+(tVSCHandler->ACVINB_Limit.Result)+(tVSCHandler->ACVINC_Limit.Result) != LIMIT_OK)
			tVSCHandler->StartCheckFlag |= 0x2;
	}
	if(tVSCHandler->StartUpCheckEn & 0x4)		//ģ����
	{//����ģ����
		if(!tVSCHandler->HBOK())
			tVSCHandler->StartCheckFlag |= 0x4;
	}
}

/*************************************************
 Function: DcStartUpCheck
 Description: ֱ�������м�״̬���
 Input: ��
 Output: ��
 Return: 0:OK,Others:Fault
 Others: ����ֱ�����������м��
 *************************************************/
void VSCDcStartUpCheck(tVSC_CTL* tVSCHandler)
{
	LimitProc(&tVSCHandler->FrePLL_Limit,tVSCHandler->PLLFre,START);
	LimitProc(&tVSCHandler->ACVINA_Limit,tVSCHandler->RMSInfo.RMS_Va,START);
	LimitProc(&tVSCHandler->ACVINB_Limit,tVSCHandler->RMSInfo.RMS_Vb,START);
	LimitProc(&tVSCHandler->ACVINC_Limit,tVSCHandler->RMSInfo.RMS_Vc,START);
	LimitProc(&tVSCHandler->DCVPreCharge_Limit,tVSCHandler->DCV_Bus,START);

	if(tVSCHandler->StartUpCheckEn & 0x1)		//���໷���
	{
		if(tVSCHandler->FrePLL_Limit.Result != LIMIT_OK)
			tVSCHandler->StartCheckFlag |= 0x1;
	}
	if(tVSCHandler->StartUpCheckEn & 0x2)		//�����ѹ���
	{
		if((tVSCHandler->ACVINA_Limit.Result)+(tVSCHandler->ACVINB_Limit.Result)+(tVSCHandler->ACVINC_Limit.Result) != LIMIT_OK)
			tVSCHandler->StartCheckFlag |= 0x2;
	}
	if(tVSCHandler->StartUpCheckEn & 0x4)		//ģ����
	{//����ģ����
		if(!tVSCHandler->HBOK())
			tVSCHandler->StartCheckFlag |= 0x4;
	}
	if(tVSCHandler->StartUpCheckEn & 0x8)		//ֱ����ѹ���
	{//����ֱ����ѹ���
		if(tVSCHandler->DCVPreCharge_Limit.Result != LIMIT_OK)
			tVSCHandler->StartCheckFlag |= 0x8;
	}
}

/****************************************************************************
*
*�����м��
*
****************************************************************************/
void VSCRunCheck(tVSC_CTL* tVSCHandler)
{
	// �����Ƿѹ��⣬��������ʱ��Ҫ����
	if((LimitProc(&tVSCHandler->ACVINA_Limit,tVSCHandler->RMSInfo.RMS_Va,RUN)+LimitProc(&tVSCHandler->ACVINB_Limit,tVSCHandler->RMSInfo.RMS_Vb,RUN)+LimitProc(&tVSCHandler->ACVINC_Limit,tVSCHandler->RMSInfo.RMS_Vc,RUN))!=LIMIT_OK)
	{
		// if(tVSCHandler->CtlMode != VACCTL){
		// 	tVSCHandler->gSysErrReg |= ERR_AC_UVOV_SW;
		// }
	}
	if(LimitProc(&tVSCHandler->FrePLL_Limit,tVSCHandler->PLLFre,RUN) != LIMIT_OK)
	{
		// if(tVSCHandler->CtlMode != VACCTL)
		// 	tVSCHandler->gSysErrReg |= ERR_PLL_SW;
	}
}

/****************************************************************************
*
*��״̬��
*
****************************************************************************/
void VSCMainFSM(tVSC_CTL* tVSCHandler)
{
//��״̬�������ڿ���ϵͳ������״̬��ת
	switch(tVSCHandler->MainStatus){
	case IDLE:
		break;
	case START:
		if(tVSCHandler->StartMode == DC_PRECHG)
		{
			VSCStartUp_DCPreChg(tVSCHandler);
			VSCDcStartUpCheck(tVSCHandler);
		}
		else
		{
			VSCStartUp_ACPreChg(tVSCHandler);
			VSCAcStartUpCheck(tVSCHandler);
		}
		break;
	case RUN:
		VSCRunCheck(tVSCHandler);
		break;
	case STOP:
		VSCStopMachine(tVSCHandler);
		break;
	case _FAULT:
//		tVSCHandler->AcBrkCtl(0);								//����������·��
		VSCStopMachine(tVSCHandler);
		tVSCHandler->MainStatus_Ref = IDLE;						//��ֹ�����������������
		break;
	default:
		tVSCHandler->StartUpStatus = 0;
		tVSCHandler->MainStatus = IDLE;
		break;
	}
	if(tVSCHandler->MainStatusPre != tVSCHandler->MainStatus)
	{
		tVSCHandler->MainStatusPre = tVSCHandler->MainStatus;
		if(tVSCHandler->MainStatus == START)
			tVSCHandler->StartMode = tVSCHandler->StartMode_Ref;	//����ǰLatch����ģʽ
		tVSCHandler->StartUpStatus = 0;										//״̬�л�ʱ��tVSCHandler->StartUpStatus����
		tVSCHandler->RelayDelayCnt = 0;
		tVSCHandler->gEnHBOut = 0;
	}

}

/*************************************************
 Function: MainFSM_Init
 Description:
 Input: ��
 Output: ��
 Return:
 Others:
 *************************************************/
void VSCMainFSM_Init(tVSC_CTL* tVSCHandler)
{
	tVSCHandler->MainStatus_Ref = IDLE;
	tVSCHandler->MainStatus = IDLE;
	tVSCHandler->StartMode_Ref = AC_PRECHG;
	tVSCHandler->MainStatusPre = _FAULT-1;
}

void VSCStopMachine(tVSC_CTL* tVSCHandler)
{
//	HBBlk(1);										//����H��
	tVSCHandler->pHBCmd_A->Mode = HB_MODE_FBLK;
	tVSCHandler->pHBCmd_B->Mode = HB_MODE_FBLK;
	tVSCHandler->pHBCmd_C->Mode = HB_MODE_FBLK;
//	tVSCHandler->pHBCmd_N->Mode = HB_MODE_FBLK;
	if(tVSCHandler->RelayDelayCnt < 50000)
		tVSCHandler->RelayDelayCnt++;
	if(tVSCHandler->RelayDelayCnt > 100)							//�����ӳ�100ms
	{
		tVSCHandler->AcBrkCtl(0);									//������բ����
		tVSCHandler->AcContCtl(0);
		tVSCHandler->AcPreChCtl(0);
		if((RATED_DCI*tVSCHandler->DCI_Bus)<DCRELAY_OPENCURRENT_TH)
		{
			tVSCHandler->DcPreChCtl(0);
			tVSCHandler->DcContCtl(0);
			if(!(tVSCHandler->AcContSts() || tVSCHandler->AcPreChSts()))
			{
				if(tVSCHandler->gSysErrFlag)
					tVSCHandler->MainStatus = _FAULT;
				else
					tVSCHandler->MainStatus = IDLE;
			}
		}
	}
}



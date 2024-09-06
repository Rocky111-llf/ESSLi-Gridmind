/**
  ******************************************************************************
  * @file    ui_com.c
  * @author  RZ
  * @brief   Intelligent Screen Driver
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
#include "ui_com.h"

u8 CmdPending_UART1;					//�������������
u8 RcvStatus_UART1;						//����״̬��
u8 Cmd_UART1;							//��������
u8 pCmdDatBuf_UART1 = 0;				//���ݴ洢��ָ��
u8 CmdDatBuf_UART1[256] __attribute__((aligned (4)));				//�������ݴ洢��,Ϊ�˷��㸡�㣬����4�ֽڶ���
static u8 RcvIntTimeCnt_UART1;

u8 SetRxDat_UART1(u8 Dat);
void DataRcv_UART1(void);
void DataProc_UART1(void);
void OTProc_UART1(void);

u8 Link_Active_UART1;
u16 Link_ActiveCnt_UART1 = 0;

extern void AuxCom(u8 Group, float *Dat);


/****************************************************************************
*
*UART1ͨѶЭ����ر�����ʼ����UART1Ӳ����������main�г�ʼ����
*
****************************************************************************/
void ProtocolInit_UART1(void)
{
	Init_UART1();
	RcvIntTimeCnt_UART1 = 0;
	RcvStatus_UART1 = 0;
	CmdPending_UART1 = 0;
	Cmd_UART1 = 0;
	Link_Active_UART1 = 0;
	Link_ActiveCnt_UART1 = 500;
}

void ProtocolProc_UART1(void)
{
	DataRcv_UART1();
	DataProc_UART1();
	SetTxFIFOLoop_UART1();
	OTProc_UART1();
}

/****************************************************************************
*
*UART1�������ݴ���
*
****************************************************************************/
void DataRcv_UART1(void)
{
	u8 AntiDeadLockCnt;									//�������Ĵ���
	u8 ibuf;
	static u8 SumChk,XorChk,RcvDatLen_UART1;			//�������ݳ���;

	GetRxFIFOLoop_UART1();								//��Ӳ��FIFO�д���ȡ

	if(CmdPending_UART1 != 0)							//��һ������δ��������
		return;

	AntiDeadLockCnt = LeftToRx_UART1();					//��ദ������ʣ������
	while((!CmdPending_UART1) && (AntiDeadLockCnt--))
	{
		RcvIntTimeCnt_UART1 = 0;
		GetRxDat_UART1(&ibuf);
		switch (RcvStatus_UART1)							//֡״̬��
		{
			case 0:
				CmdPending_UART1 = 0;
				pCmdDatBuf_UART1 = 0;						//������Ч������ָ��
				SumChk = 0;
				XorChk = 0;
				if(ibuf == 0x7E)							//�ж�֡ͷ1
					RcvStatus_UART1 = 1;
				else
					RcvStatus_UART1 = 0;
				break;
			case 1:
				if(ibuf == 0xE7)							//�ж�֡ͷ2
				{
					RcvStatus_UART1 = 2;
				}
				else if(ibuf == 0x7E)
					RcvStatus_UART1 = 1;
				else
					RcvStatus_UART1 = 0;
				break;
			case 2:
				if(ibuf == 0x7E)							//�ж�֡ͷ3
				{
#if USE_ADDR_UART1
					RcvStatus_UART1 = 3;					//֡ͷ�յ�
#else
					RcvStatus_UART1 = 7;					//֡ͷ�յ�
#endif
				}
				else
					RcvStatus_UART1 = 0;
				break;
#if USE_ADDR_UART1
			case 3:
				((u8*)(&DestAddr_UART1))[1] = ibuf;			//Ŀ����վ��ַ

				SumChk += ibuf;
				XorChk ^= ibuf;
				RcvStatus_UART1 = 4;
				break;
			case 4:
				((u8*)(&DestAddr_UART1))[0] = ibuf;			//Ŀ���豸��ַ
				SumChk += ibuf;
				XorChk ^= ibuf;
				RcvStatus_UART1 = 5;
				break;
			case 5:
				((u8*)(&SourceAddr_UART1))[1] = ibuf;		//Դ��վ��ַ
				SumChk += ibuf;
				XorChk ^= ibuf;
				RcvStatus_UART1 = 6;
				break;
			case 6:
				((u8*)(&SourceAddr_UART1))[0] = ibuf;		//Դ���豸��ַ
				SumChk += ibuf;
				XorChk ^= ibuf;
				RcvStatus_UART1 = 7;
				break;
#endif
			case 7:
				Cmd_UART1 = ibuf;							//ָ������
				SumChk += ibuf;
				XorChk ^= ibuf;
				RcvStatus_UART1 = 8;
				break;
			case 8:
				SumChk += ibuf;
				XorChk ^= ibuf;
				if(ibuf)						//�������ݳ���
			    {
					RcvDatLen_UART1 = ibuf;		//��¼���ݳ���
					RcvStatus_UART1 = 9;
				}
				else							//���ݳ���Ϊ��
					RcvStatus_UART1 = 10;
				break;
			case 9:								//�������ݳ����ڵ�
				CmdDatBuf_UART1[pCmdDatBuf_UART1] = ibuf;
				SumChk += ibuf;
				XorChk ^= ibuf;
				pCmdDatBuf_UART1 += 1;
				RcvDatLen_UART1 -= 1;
				if(0 == RcvDatLen_UART1)
					RcvStatus_UART1 = 10;
				break;
			case 10:
				if(XorChk != ibuf)
					RcvStatus_UART1 = 0;
				else
					RcvStatus_UART1 = 11;
				break;
			case 11:
				if(SumChk != ibuf)
					RcvStatus_UART1 = 0;
				else
					RcvStatus_UART1 = 12;
				break;
			case 12:
				if(0x0D == ibuf)
				{
#if USE_ADDR_UART1
					if((DestAddr_UART1 == MMC_ADDR) && (SourceAddr_UART1 == 0))
#endif
					{
						CmdPending_UART1 = 1;
						Link_Active_UART1 = 1;
						Link_ActiveCnt_UART1 = 500;
					}

				}
				RcvStatus_UART1 = 0;
				break;
		}
	}
}


void DataTrans_UART1(CMD_TYPE Type,u8 Para1, u8 Para2)
{
	u8 SumChk_TX = 0,XorChk_TX = 0;							//���غ͡����У�����ֵ
	float fTemp;
	u8* u8pTemp;
	u8 u8Temp = 0;
	u16 u16Temp;

	AddtoBuf_UART1(0x7e);
	AddtoBuf_UART1(0xe7);
	AddtoBuf_UART1(0x7e);
#if USE_ADDR_UART1
	CheckAddtoBuf_UART1(((u8*)(&DAddr))[1]);
	CheckAddtoBuf_UART1(((u8*)(&DAddr))[0]);
	CheckAddtoBuf_UART1(((u8*)(&SAddr))[1]);
	CheckAddtoBuf_UART1(((u8*)(&SAddr))[0]);
#endif

	if(Type == CMD_DATREQ_R)
	{
		CheckAddtoBuf_UART1(Type);					//����

		if(Para1 == 0x0)
		{
			CheckAddtoBuf_UART1(79);					//���ȣ���Ҫ����ʵ�ʳ��������޸�
			CheckAddtoBuf_UART1(Para1);					//��ʾ��������
			CheckAddtoBuf_UART1(Para2);
			//Ua
			fTemp =  Ctl_LI1.pVSC->Va;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Ub
			fTemp =  Ctl_LI1.pVSC->Vb;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Uc
			fTemp =  Ctl_LI1.pVSC->Vc;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Ia
			// fTemp =  Ctl_LI1.pVSC->Ia;
			debug1 = Ctl_VSC1.Vac_Cmd;
			fTemp = debug1;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Ib
			// fTemp =  Ctl_LI1.pVSC->Ib;
			fTemp = debug2;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Ic
			fTemp =  Ctl_LI1.pVSC->Ic;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//P
			fTemp =  Ctl_LI1.pVSC->P;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Q
			fTemp =  Ctl_LI1.pVSC->Q;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//PF
			fTemp =  Ctl_LI1.pVSC->PF;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//f
			fTemp =  Ctl_LI1.pVSC->f;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Vdc
			fTemp =  Ctl_LI1.pVSC->Vdc;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Idc
			fTemp =  Ctl_LI1.pVSC->Idc;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//��ص�ѹ
			fTemp =  Ctl_LI1.tStackInfo.StackV;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//��ص���
			fTemp =  Ctl_LI1.tStackInfo.StackI;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Pdc
			fTemp =  Ctl_LI1.pVSC->Pdc;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//SOC
			fTemp =  Ctl_LI1.SOC*100.0f;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);

			//״̬
			CheckAddtoBuf_UART1(Ctl_LI1.MainStatus);
			//ģʽ
			CheckAddtoBuf_UART1((u8)Ctl_LI1.CtlMode);
			//VSC״̬
			CheckAddtoBuf_UART1((u8)Ctl_LI1.pVSC->MainStatus);
			//���״̬
			CheckAddtoBuf_UART1((u8)Ctl_LI1.ChgSts);
			//�������״̬
			CheckAddtoBuf_UART1((u8)Ctl_LI1.ChDischAllowedSts);
			//������
			u8Temp=0;
			if(IsCB())
				u8Temp |= 0x1;
			if(IsKM())
				u8Temp |= 0x2;
			// ����ΪʲôҪע�� 
//			if(Ctl_LI1.pVSC->AcPreChSts())
//				u8Temp |= 0x4;
//			if(Ctl_LI1.pVSC->DcContSts())
//				u8Temp |= 0x8;
//			if(Ctl_LI1.pVSC->DcPreChSts())
//				u8Temp |= 0x10;
			if(Ctl_LI1.MainStatus == RUN)
				u8Temp |= 0x20;
			if(Link_Active_UART1)
				u8Temp |= 0x40;
			if(Ctl_LI1.gSysErrFlag)
				u8Temp |= 0x80;
			CheckAddtoBuf_UART1(u8Temp);
			//ErrStatus
			u16Temp = (Ctl_LI1.gSysErrReg) & (Ctl_LI1.gErrMask);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);	//����״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			//����״̬
			CheckAddtoBuf_UART1((u8)Ctl_LI1.StartUpStatus);
			u16Temp = Ctl_LI1.StartFailReg & 0xFFFF;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);	//����ʧ�ܼĴ������֣�ָʾʧ�ܲ���
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = ((Ctl_LI1.StartFailReg)>>16) & 0xFFFF;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);	//����ʧ�ܼĴ������֣�ָʾʧ�ܲ���
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
		}
		else if(Para1 == 0x1)
		{//���ع���ģ����������
			CheckAddtoBuf_UART1(90);					//����
			CheckAddtoBuf_UART1(Para1);					//��ʾ��������
			CheckAddtoBuf_UART1(Para2);
			u8pTemp = (u8*)sOpt0.Dat->DAT;				//������˰�0��ģ����Ϣ
			u8 i = 3*4;
			while(i--)
			{
				CheckAddtoBuf_UART1(*u8pTemp);
				u8pTemp++;
			}
			//�����������
			u8pTemp = (u8*)(&(Ctl_LI1.tStackInfo.SOC));
			i = 76;//(u8*)(&(Ctl_LI1.tStackInfo.SOC))-(u8*)(&(Ctl_LI1.tStackInfo.SOC))+1;
			while(i--)
			{
				CheckAddtoBuf_UART1(*u8pTemp);
				u8pTemp++;
			}
		}
		else if(Para1 == 0x2)
		{//�����ڲ�����
			CheckAddtoBuf_UART1(38);								//����
			CheckAddtoBuf_UART1(Para1);								//��ʾ��������
			CheckAddtoBuf_UART1(Para2);
			u16Temp = (Ctl_LI1.gSysErrReg) & (Ctl_LI1.gErrMask);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//ϵͳ����״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (Ctl_LI1.pVSC->gSysErrReg);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//VSC����״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;//(Ctl_LI1.pDCDC->gSysErrReg);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//DC/DC����״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (u16)sDio0.Dat->all;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//DI״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (u16)sDio0.Cmd->all;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//DO״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (u16)(Ctl_LI1.pVSC->pRop->Cmd->all);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//VSC DO״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;//(u16)(Ctl_LI1.pDCDC->pRop->Cmd->all);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//DC/DC DO״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = hS_FpgaUpLink->CardValid;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//ͨ���忨״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = hS_FpgaUpLink->CardType7_0 & 0xFFFF; 			//0~3�忨����
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (hS_FpgaUpLink->CardType7_0>>16) & 0xFFFF; 	//4~7�忨����
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = hS_FpgaUpLink->CardType9_8 & 0xFFFF; 			//9~8�忨����
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = hS_FpgaUpLink->LvdsLinkSts & 0xFFFF; 			//LVDS��������״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (hS_FpgaUpLink->LvdsLinkSts>>16) & 0xFFFF; 	//LVDS��������״̬
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (u16)((10000000.0f/MAINFPGA_CLK)*hS_FpgaUpLink->CalcTime); 			//����ʱ��,=us*10
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
		}
	}
	else if(Type == CMD_START_R)
	{
		CheckAddtoBuf_UART1(Type);					//����
		CheckAddtoBuf_UART1(1);						//����
		CheckAddtoBuf_UART1(Para1);
	}
	else if(Type == CMD_RST_ERR_R)
	{
		CheckAddtoBuf_UART1(Type);					//����
		CheckAddtoBuf_UART1(1);						//����
		CheckAddtoBuf_UART1(Para1);
	}
	else if(Type == CMD_DATSET_R)
	{
		CheckAddtoBuf_UART1(Type);					//����
		CheckAddtoBuf_UART1(1);						//����
		CheckAddtoBuf_UART1(Para1);
	}
	else
	{
		return;
	}

	AddtoBuf_UART1(XorChk_TX);						//���У��
	AddtoBuf_UART1(SumChk_TX);						//��У��
	AddtoBuf_UART1(0x0d);							//��У��
	//ִ�з��ͳ���
	SetTxFIFOLoop_UART1();
}

void DataProc_UART1(void)
{
	float f32Temp;
//	tVSC_CTL *pVSCx_Ctl;

	if(!CmdPending_UART1)
		return;

	if(Cmd_UART1 == CMD_DATREQ)					//��ȡ��������������֡
	{
		if(CmdDatBuf_UART1[0] == 0x0)			//��ҳ��������������
		{
			DataTrans_UART1(CMD_DATREQ_R,CmdDatBuf_UART1[0],CmdDatBuf_UART1[1]);
		}
		else if(CmdDatBuf_UART1[0] == 0x1)
		{
			if(CmdDatBuf_UART1[1] < 6)
			{
				DataTrans_UART1(CMD_DATREQ_R,CmdDatBuf_UART1[0],CmdDatBuf_UART1[1]);
			}
		}
		else if(CmdDatBuf_UART1[0] == 0x2)
		{
			DataTrans_UART1(CMD_DATREQ_R,CmdDatBuf_UART1[0],CmdDatBuf_UART1[1]);
		}
	}
	else if(Cmd_UART1 == CMD_START)
	{//����ָ��
		if(CmdDatBuf_UART1[0] == 1)					//��������
			Ctl_LI1.MainStatus_Ref = START;
		else if(CmdDatBuf_UART1[0] == 0)
			Ctl_LI1.MainStatus_Ref = STOP;
		else
			Ctl_LI1.MainStatus_Ref = IDLE;
		DataTrans_UART1(CMD_START_R,0x1,0x0);
	}
	else if(Cmd_UART1 == CMD_RST_ERR)
	{//��λ
		if(CmdDatBuf_UART1[0] == 1)					//��λ����
		{
			if((Ctl_LI1.MainStatus == IDLE) || (Ctl_LI1.MainStatus == STOP) || (Ctl_LI1.MainStatus == _FAULT))
				Ctl_LI1.ClrErr();
		}
		DataTrans_UART1(CMD_RST_ERR_R,0x1,0x0);
	}
	else if(Cmd_UART1 == CMD_DATSET)
	{
		if((CmdDatBuf_UART1[0] == 0x0) && (CmdDatBuf_UART1[2] == 0x0C))
		{//ԭ����IPָ��

			DataTrans_UART1(CMD_DATSET_R,0x1,0x0);
		}
		else if((CmdDatBuf_UART1[0] == 0x1) && (CmdDatBuf_UART1[2] == 0x12))
		{//�������в���
			if((CmdDatBuf_UART1[1] == 0x0))
			{//����ģʽ����
				if((CmdDatBuf_UART1[3] >= (u8)IDLE_LI) || (CmdDatBuf_UART1[3] <= (u8)PQ_LI))					//MODE
				{
					Ctl_LI1.CtlMode_Ref = CmdDatBuf_UART1[3];
					Ctl_LI1.CtlMode = Ctl_LI1.CtlMode_Ref;
					switch(Ctl_LI1.CtlMode_Ref){
					//extern float P_Cmd,Q_Cmd,Vdc_Cmd,Id_Cmd,Iq_Cmd,Vac_Cmd,Fre_Cmd;
					case IDLE_LI:
						break;
					case CH_DISCH_LI:
						SetWordB0(f32Temp,CmdDatBuf_UART1[4]);
						SetWordB1(f32Temp,CmdDatBuf_UART1[5]);
						SetWordB2(f32Temp,CmdDatBuf_UART1[6]);
						SetWordB3(f32Temp,CmdDatBuf_UART1[7]);
						HardLimit(f32Temp,0,1);
						Ctl_LI1.SOC_Ref = f32Temp;
						break;
					case PQ_LI:
					// ����ʱ��Ҫ�л�������ģʽʱӦ�Ƚ�﮵�����ΪPQ����ģʽ��PQ������Ӧ��ֵ��Ȼ����ͨ�������·�ѡ��VSC����ģʽ�Ͳ���ֵ
						SetWordB0(f32Temp,CmdDatBuf_UART1[4]);
						SetWordB1(f32Temp,CmdDatBuf_UART1[5]);
						SetWordB2(f32Temp,CmdDatBuf_UART1[6]);
						SetWordB3(f32Temp,CmdDatBuf_UART1[7]);
						HardLimit(f32Temp,-1.0,1.0);
						Ctl_LI1.P_Ref = f32Temp;
						SetWordB0(f32Temp,CmdDatBuf_UART1[8]);
						SetWordB1(f32Temp,CmdDatBuf_UART1[9]);
						SetWordB2(f32Temp,CmdDatBuf_UART1[10]);
						SetWordB3(f32Temp,CmdDatBuf_UART1[11]);
						HardLimit(f32Temp,-1.0,1.0);
						Ctl_LI1.Q_Ref = f32Temp;
						break;
					default:
						break;
					}
				}
			}
			if((CmdDatBuf_UART1[1] == 0x1))
			{//����ģʽ����
				AuxCom(CmdDatBuf_UART1[3], (float*)(&CmdDatBuf_UART1[4]));
				// VSC����ģʽ����������·�
				Ctl_VSC1.CtlMode_Ref = (u16)VSC_CTLMODE;
				Ctl_VSC1.CtlMode = (u16)VSC_CTLMODE;
				switch(Ctl_VSC1.CtlMode){
					case PQCTL: // = 0
						// ���������ʿ���
						Ctl_VSC1.P_Cmd = VSC_PREF;
						Ctl_VSC1.Q_Cmd = VSC_QREF;
						break;
					case VQCTL: // = 1
						Ctl_VSC1.Vdc_Cmd = VSC_UDCREF;
						Ctl_VSC1.Q_Cmd = VSC_QREF;
						break;
					case IDQCTL: // = 2
						// ��ʱδ���壬�����������ģʽ
						break;
					case VACCTL: // = 3
						Ctl_VSC1.P_Cmd = VSC_PREF;
						Ctl_VSC1.Q_Cmd = VSC_QREF;
#ifdef QVLOOP
						Ctl_VSC1.Vac_Cmd = VSC_UACREF;
#endif
				}
			}
			DataTrans_UART1(CMD_DATSET_R,0x1,0x0);
		}
	}
	CmdPending_UART1 = 0;						//������������ٴν���
}

/****************************************************************************
*
*UART1Э�鳬ʱ������Ҫ���������ж�ʱ����
*
****************************************************************************/
void OTProc_UART1(void)
{
	if(RcvIntTimeCnt_UART1 < OT_MAX_UART1)
		RcvIntTimeCnt_UART1++;
	else
	{
		RcvIntTimeCnt_UART1 = 0;
		RcvStatus_UART1 = 0;
	}

	if(Link_ActiveCnt_UART1 > 0)
		Link_ActiveCnt_UART1--;
	else
		Link_Active_UART1 = 0;
}




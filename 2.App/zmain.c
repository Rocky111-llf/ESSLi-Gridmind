/**
  ******************************************************************************
  * @file    zmain.c
  * @author  RZ
  * @brief   zmain.c module
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

#include <stdio.h>
#include "sleep.h"
#include "xil_printf.h"

#include "Includings.h"
#include "ui_com.h"
#include "../src/platform.h"


extern int init_interrupt();
void BoardLedCtl(void);
void LedKeyCtl(void);
void SlowCtl(void);
void ExInit(void);


/*************************************************
 Function: SystemInit
 Description: ��ʼ������
 Input: ��
 Output: ��
 Return:
 Others:
 *************************************************/
void SystemInit(void)
{
    //�������ð忨�����ݰ忨���ͽ��б�������װλ�ð�
	SysCfg();
	//�忨���ӳ�ʼ�����򣬸��ݰ忨��ͬ����ʼ����ͬ
    ExInit();
    //����ģ���ʼ������Ҫ����Ƿ��ѹ�㡢����Ƶ�ʡ�����Ƕȵ�����
    PEMInit();
    //�Կ��жϴ����жϳ�ʼ��
    ProtocolInit_UART1();

    //����ģ���ʼ��
    VSCInit(&Ctl_VSC1);			//VSC��ʼ��
    LIInit(&Ctl_LI1);			//﮵索�ܿ�������ʼ��
	EMSInit(&EMS_Ctl);
    //Ĭ������ģʽ��VSC��DC/DC�ȹ�������ʱ����Ҫ���������ã�
//  Ctl_VSC1.CtlMode_Ref = VQCTL;				//VQCTL;PQCTL
//  Ctl_VSC1.StartMode_Ref = AC_PRECHG;			//AC_PRECHG
}

/*************************************************
 Function: Task1ms
 Description: 1ms��������
 Input: None
 Output: None
 Return: None
 Others:
 *************************************************/
void Task1ms(void)
{
	//Ǧ���ؿ�������
	LIMainFSM(&Ctl_LI1);
	//VSC��������
	VSCFastTask(&Ctl_VSC1);
	//�Կ���ĻЭ�鴦��
	ProtocolProc_UART1();
	//EMSЭ�����
	EMSPolling(&EMS_Ctl);
}

/*************************************************
 Function: Task20ms
 Description: 20ms��������
 Input: None
 Output: None
 Return: None
 Others:
 *************************************************/
void Task20ms(void)
{
	VSCMidTask(&Ctl_VSC1);
	BoardLedCtl();
	LedKeyCtl();
}

/*************************************************
 Function: Task500ms
 Description: 500ms��������
 Input: None
 Output: None
 Return: None
 Others:
 *************************************************/
void Task500ms(void)
{
	SlowCtl();
}

/*************************************************
 Function: AuxCom
 Description: �����·�����
 Input: None
 Output: None
 Return: None
 Others:
 *************************************************/
float ComDatF[5];
void AuxCom(u8 Group, float *Dat)
{
	ComDatF[0] = Dat[0];
	ComDatF[1] = Dat[1];
	ComDatF[2] = Dat[2];
	ComDatF[3] = Dat[3];
	ComDatF[4] = Dat[4];

	switch(Group){//������
	case 0:// �������Ʋ�����
		Ctl_VSC1.CtlMode_Ref = VACCTL;
		Ctl_VSC1.CtlMode = VACCTL;
		Ctl_VSC1.GFMCtlMode = ComDatF[0];
		switch(Ctl_VSC1.GFMCtlMode){
			case VFCTL:// =0
				Ctl_VSC1.Vac_Cmd = ComDatF[1];
				break;
			case DROOPCTL:// =1
				// Ĭ���޹��������ý�����ѹ�·�
				// Ctl_VSC1.Vac_Cmd = ComDatF[1];
				Ctl_VSC1.P_Cmd = ComDatF[2];
				Ctl_VSC1.Q_Cmd = ComDatF[3];
				break;
			case VSGCTL:
				// Ctl_VSC1.Vac_Cmd = ComDatF[1];
				Ctl_VSC1.P_Cmd = ComDatF[2];
				Ctl_VSC1.Q_Cmd = ComDatF[3];
				break;
			case UniversalGFM:
				// Ctl_VSC1.Vac_Cmd = ComDatF[1];
				Ctl_VSC1.P_Cmd = ComDatF[2];
				Ctl_VSC1.Q_Cmd = ComDatF[3];
				break;
			default:
				break;
		}
		break;
	case 1:// ���������ʿ���
		Ctl_VSC1.P_Cmd = ComDatF[0];
		Ctl_VSC1.Q_Cmd = ComDatF[1];
		break;
	case 2:// ������ֱ����ѹ����
		Ctl_VSC1.Vdc_Cmd = ComDatF[0];
		Ctl_VSC1.Q_Cmd = ComDatF[1];
		break;
	case 3:// ������������
		// ��ʱδ����
		break;
	default:
		break;
	}
}

/*************************************************
 Function: LedKeyCtl
 Description: ����LED���ƣ�RunLED��
 Input: ��
 Output: ��
 Return:
 Others:
 *************************************************/
void LedKeyCtl(void)
{
	if(Ctl_LI1.MainStatus == RUN)
		SysRunLedCtl(1);
	else
		SysRunLedCtl(0);

	if(Ctl_LI1.MainStatus == _FAULT)
		SysFaultLedCtl(1);
	else
		SysFaultLedCtl(0);
}

/*************************************************
 Function: SlowCtl
 Description: ���ٿ���
 Input: None
 Output: None
 Return: None
 Others:
 *************************************************/
void SlowCtl(void)
{
	//�������
	static u16 FAN_IDLECnt = 0;
	if(FANCtl == BOOL_TRUE)
	{
		if(Ctl_LI1.MainStatus != RUN)
		{
			if(FAN_IDLECnt < (FAN_IDLE_MAX*1000/MAIN_LOOP_TIMER3))
				FAN_IDLECnt++;
			else
			{
				FANCtl = BOOL_FALSE;
				SysFanCtl(0);
				FAN_IDLECnt = 0;
				Ctl_VSC1.CFanCtl(0);
			}
		}
	}
	else
		FAN_IDLECnt = 0;
}

/*************************************************
 Function: ExInit
 Description: ������ʼ��
 Input: None
 Output: None
 Return: None
 Others:
 *************************************************/
void ExInit(void)
{
//ģ�����������
	sAo0.Cmd->bit.AO_Bias_En = 1;
//���뿪��������
	sDio0.Cmd->all = 0x00;
//������������
//	sSens0.Cfg->bit.AdcCnvPoint = 300;
//	sSens0.Cfg->bit.FaultReset = 1;
}

/*************************************************
 Function: Main
 Description: �����򣬷Ǳ�Ҫ������
 Input: None
 Output: None
 Return: None
 Others:
 *************************************************/
int main()
{
	u16 Status;
	PlSoftwareReset();
	init_platform();
	usleep(300000);				//Wait for System Stable
	print("Hello GridMind!\n\r");
	print("Successfully ran the application!\n\r");
    BSPInit(MAINFPGA_CLK/INTFRE);
    usleep(500000);				//Wait for DIDO Stable

    SystemInit();						//����ϵͳ��ʼ������

	Status = init_interrupt();			//�����жϣ��������
	if(Status == XST_FAILURE)
	{
		CabineRunLed.all = 0xFFFFFFFF;
	}
    while(1)
    {
		if(MainTimer1.Alarm)		//1ms
		{
			TimerReload(&MainTimer1);
			Task1ms();
		}
		if(MainTimer2.Alarm)		//20ms
		{
			TimerReload(&MainTimer2);
			Task20ms();
		}
		if(MainTimer3.Alarm)		//500ms
		{
			TimerReload(&MainTimer3);
			Task500ms();
		}
    }
    cleanup_platform();
    return 0;
}


/*************************************************
 Function: BoardLedCtl
 Description: ���ػ���LED���ƣ�RunLED��
 Input: ��
 Output: ��
 Return:
 Others:
 *************************************************/
void BoardLedCtl(void)
{
	static u16 LedCnt = 0;
	if(LedCnt < 50) LedCnt++;
	else LedCnt = 0;
	if(LedCnt < 25)
		CabineRunLed.all = 0;
	else
	{
		if(LedCnt & 0x2)
			CabineRunLed.all = 0xFFFFFFFF;
		else
			CabineRunLed.all = 0x0 ;//| ((u32)0x1 << 10);
	}
	hS_FpgaDownLink->RunLed = CabineRunLed.all;
}






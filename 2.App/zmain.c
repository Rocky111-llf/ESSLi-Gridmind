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
 Description: 初始化程序
 Input: 无
 Output: 无
 Return:
 Others:
 *************************************************/
void SystemInit(void)
{
    //用于配置板卡，根据板卡类型进行变量及安装位置绑定
	SysCfg();
	//板卡附加初始化程序，根据板卡不同，初始化不同
    ExInit();
    //功率模块初始化，主要包括欠过压点、开关频率、移相角度等配置
    PEMInit();
    //显控中断串口中断初始化
    ProtocolInit_UART1();

    //工作模块初始化
    VSCInit(&Ctl_VSC1);			//VSC初始化
    LIInit(&Ctl_LI1);			//锂电储能控制器初始化
	EMSInit(&EMS_Ctl);
    //默认启动模式（VSC或DC/DC等孤立运行时，需要进行重配置）
//  Ctl_VSC1.CtlMode_Ref = VQCTL;				//VQCTL;PQCTL
//  Ctl_VSC1.StartMode_Ref = AC_PRECHG;			//AC_PRECHG
}

/*************************************************
 Function: Task1ms
 Description: 1ms运行任务
 Input: None
 Output: None
 Return: None
 Others:
 *************************************************/
void Task1ms(void)
{
	//铅酸电池快速任务
	LIMainFSM(&Ctl_LI1);
	//VSC快速任务
	VSCFastTask(&Ctl_VSC1);
	//显控屏幕协议处理
	ProtocolProc_UART1();
	//EMS协议相关
	EMSPolling(&EMS_Ctl);
}

/*************************************************
 Function: Task20ms
 Description: 20ms运行任务
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
 Description: 500ms运行任务
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
 Description: 串口下发辅助
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

	switch(Group){//参数组
	case 0:// 构网控制参数组
		Ctl_VSC1.CtlMode_Ref = VACCTL;
		Ctl_VSC1.CtlMode = VACCTL;
		Ctl_VSC1.GFMCtlMode = ComDatF[0];
		switch(Ctl_VSC1.GFMCtlMode){
			case VFCTL:// =0
				Ctl_VSC1.Vac_Cmd = ComDatF[1];
				break;
			case DROOPCTL:// =1
				// 默认无功环，弃用交流电压下发
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
	case 1:// 跟网定功率控制
		Ctl_VSC1.P_Cmd = ComDatF[0];
		Ctl_VSC1.Q_Cmd = ComDatF[1];
		break;
	case 2:// 跟网定直流电压控制
		Ctl_VSC1.Vdc_Cmd = ComDatF[0];
		Ctl_VSC1.Q_Cmd = ComDatF[1];
		break;
	case 3:// 跟网电流控制
		// 暂时未定义
		break;
	default:
		break;
	}
}

/*************************************************
 Function: LedKeyCtl
 Description: 机柜LED控制（RunLED）
 Input: 无
 Output: 无
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
 Description: 慢速控制
 Input: None
 Output: None
 Return: None
 Others:
 *************************************************/
void SlowCtl(void)
{
	//风机控制
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
 Description: 辅助初始化
 Input: None
 Output: None
 Return: None
 Others:
 *************************************************/
void ExInit(void)
{
//模拟输出板配置
	sAo0.Cmd->bit.AO_Bias_En = 1;
//开入开出板配置
	sDio0.Cmd->all = 0x00;
//传感器板配置
//	sSens0.Cfg->bit.AdcCnvPoint = 300;
//	sSens0.Cfg->bit.FaultReset = 1;
}

/*************************************************
 Function: Main
 Description: 主程序，非必要不更改
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

    SystemInit();						//调用系统初始化程序

	Status = init_interrupt();			//启动中断，放在最后
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
 Description: 主控机箱LED控制（RunLED）
 Input: 无
 Output: 无
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






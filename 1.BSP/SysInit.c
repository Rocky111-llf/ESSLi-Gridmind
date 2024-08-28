/**
  ******************************************************************************
  * @file    SysInit.c
  * @author  RZ
  * @brief   SysInit.c module
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

//#include "xgpio.h"
#include "xgpiops.h"
#include "sleep.h"
#include "xscugic.h"
#include "xil_mmu.h"

#include "Includings.h"
#include "Memory.h"
#include "SysCfg.h"

XGpioPs Gpio;

#define INT_CFG0_OFFSET 		0x00000C00
#define PL_INT_TYPE_MASK        0x03

#define PL_IRQ_ID				31

void GPIOInit(void);
void ParasInit(u32);
void SysCfg(void);

static XScuGic GicPs={0};
static XScuGic_Config *XScuGic_Cfg;

/*************************************************
 Function: SysCfg
 Description: ���ð忨��Ӧ��ϵ
 Input: ��
 Output: ��
 Return: 0:OK,Others:Fault
 Others:
 *************************************************/
void SysCfg(void)			//��ϵͳ�������ã���Ҫ�а忨��ӳ���
{
	SysCardBind(&sAo0,CARD_TYPE_AO,1);
	SysCardBind(&sRop0,CARD_TYPE_ROP,3);
	SysCardBind(&sOpt0,CARD_TYPE_OPT,4);
	SysCardBind(&sDio0,CARD_TYPE_DIDO,6);
}

/*************************************************
 Function: BSPInit
 Description: ���ð忨��Ӧ��ϵ
 Input: ��
 Output: ��
 Return: 0:OK,Others:Fault
 Others:
 *************************************************/
void BSPInit(u32 IntFreReg)
{
	SysEnvInit();
	Xil_SetTlbAttributes((INTPTR)0x3FF80000,0x14de2);				//1MB
	Xil_SetTlbAttributes(XPAR_BRAM_0_BASEADDR,0x14de2);				//1MB
	GPIOInit();
	ParasInit(IntFreReg);
}

/*************************************************
 Function: init_interrupt
 Description: �������ж�
 Input: ��
 Output: ��
 Return: XST_FAILURE��XST_SUCCESS
 Others:
 *************************************************/
int init_interrupt()
{
	int Status;

	XScuGic_Cfg = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	if (NULL == XScuGic_Cfg)
	{
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&GicPs,XScuGic_Cfg,XScuGic_Cfg->CpuBaseAddress);
	if( Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	XScuGic_CPUWriteReg(&GicPs, XSCUGIC_CONTROL_OFFSET, 0x04U);			//Bypass GIC

//	XScuGic_Connect(&GicPs,PL_IRQ_ID,(Xil_InterruptHandler)PL_IntrHandler,&GicPs);

	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)PL_IntrHandler/*XScuGic_InterruptHandler*/,&GicPs);
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/*************************************************
 Function: GPIOInit
 Description:
 Input: ��
 Output: ��
 Return: ��
 Others:
 *************************************************/
void GPIOInit(void)
{
	XGpioPs_Config* ConfigPtr;	//GPIO���ýṹ��ָ��

	//��������ID������������������Ϣ
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	//��ʼ��GPIO
	XGpioPs_CfgInitialize(&Gpio, ConfigPtr, ConfigPtr->BaseAddr);

	//��LED GPIO�ķ�������Ϊ���--1Ϊ���
	XGpioPs_SetDirectionPin(&Gpio, MIO7_PS_LED, 1);
	//����GPIO�����ʹ��--1ʹ��
	XGpioPs_SetOutputEnablePin(&Gpio, MIO7_PS_LED, 1);
}

/*************************************************
 Function: ParasInit
 Description: ���ð忨��Ӧ��ϵ
 Input: ��
 Output: ��
 Return: 0:OK,Others:Fault
 Others:
 *************************************************/
void ParasInit(u32 IntFreReg)
{
	hS_FpgaDownLink->SyncFreNum = IntFreReg;//MAINFPGA_CLK/INTFRE;
}

void PlSoftwareReset(void)
{
    Xil_Out32(SLCR_UNLOCK_ADDR, UNLOCK_KEY);  	//����

    Xil_Out32(FPGA_RST_CTRL, PL_RST_MASK);  	//��λ
    Xil_Out32(FPGA_RST_CTRL, PL_CLR_MASK);  	//����λ

    Xil_Out32(SLCR_LOCK_ADDR, LOCK_KEY);  		//����
}

void TimerProc(TIMER_SYS* timer1)
{
    if(timer1->Cnt!=0)
    {
        timer1->Cnt--;
    }
    else
    {
        timer1->Alarm = TRUE;
    }
}

void TimerReload(TIMER_SYS* timer1)
{
    timer1->Alarm = FALSE;
    timer1->Cnt = timer1->Reload;
}



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
 Description: 配置板卡对应关系
 Input: 无
 Output: 无
 Return: 0:OK,Others:Fault
 Others:
 *************************************************/
void SysCfg(void)			//对系统进行配置，主要有板卡的映射等
{
	SysCardBind(&sAo0,CARD_TYPE_AO,1);
	SysCardBind(&sRop0,CARD_TYPE_ROP,3);
	SysCardBind(&sOpt0,CARD_TYPE_OPT,4);
	SysCardBind(&sDio0,CARD_TYPE_DIDO,6);
}

/*************************************************
 Function: BSPInit
 Description: 配置板卡对应关系
 Input: 无
 Output: 无
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
 Description: 配置主中断
 Input: 无
 Output: 无
 Return: XST_FAILURE、XST_SUCCESS
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
 Input: 无
 Output: 无
 Return: 无
 Others:
 *************************************************/
void GPIOInit(void)
{
	XGpioPs_Config* ConfigPtr;	//GPIO配置结构体指针

	//根据器件ID，查找器件的配置信息
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	//初始化GPIO
	XGpioPs_CfgInitialize(&Gpio, ConfigPtr, ConfigPtr->BaseAddr);

	//把LED GPIO的方向设置为输出--1为输出
	XGpioPs_SetDirectionPin(&Gpio, MIO7_PS_LED, 1);
	//设置GPIO的输出使能--1使能
	XGpioPs_SetOutputEnablePin(&Gpio, MIO7_PS_LED, 1);
}

/*************************************************
 Function: ParasInit
 Description: 配置板卡对应关系
 Input: 无
 Output: 无
 Return: 0:OK,Others:Fault
 Others:
 *************************************************/
void ParasInit(u32 IntFreReg)
{
	hS_FpgaDownLink->SyncFreNum = IntFreReg;//MAINFPGA_CLK/INTFRE;
}

void PlSoftwareReset(void)
{
    Xil_Out32(SLCR_UNLOCK_ADDR, UNLOCK_KEY);  	//解锁

    Xil_Out32(FPGA_RST_CTRL, PL_RST_MASK);  	//复位
    Xil_Out32(FPGA_RST_CTRL, PL_CLR_MASK);  	//拉起复位

    Xil_Out32(SLCR_LOCK_ADDR, LOCK_KEY);  		//加锁
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



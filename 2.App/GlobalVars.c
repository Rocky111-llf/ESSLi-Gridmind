/**
  ******************************************************************************
  * @file    GlobalVars.c
  * @author  RZ
  * @brief   GlobalVars.c module
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

u32 u32TempGlobal;

S_FPGA_DOWNLINK *hS_FpgaDownLink;		//__attribute__((section(".cfg_wrSection")))
S_FPGA_UPLINK   *hS_FpgaUpLink;			//__attribute__((section(".cfg_rdSection")))

/********************�忨����********************/
S_OPTCARD  		sOpt0;
S_DIDOCARD 		sDio0;
S_AOCARD 		sAo0;
S_ROPCARD		sRop0;
/******************�忨�������******************/

U_RUNLED CabineRunLed;
//������ѭ����ʱ����ʱ��Ϊ��ʱ��
TIMER_SYS MainTimer1={0,MAIN_LOOP_TIMER1*INTFRE/1000-1,0};              //Լ1ms  (����Ϊ100us),������SysParas�ж���
TIMER_SYS MainTimer2={5,MAIN_LOOP_TIMER2*INTFRE/1000-1,0};              //Լ20ms
TIMER_SYS MainTimer3={105,MAIN_LOOP_TIMER3*INTFRE/1000-1,0};            //Լ500ms

BOOL_SYS FANCtl = BOOL_FALSE;

float debug1 = 0;
float debug2 = 0;


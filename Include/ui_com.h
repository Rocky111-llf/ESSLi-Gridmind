/**
  ******************************************************************************
  * @file    Uart1.h
  * @author  RZ
  * @brief   Header for System configuration
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

#ifndef _UI_COM_H_
#define _UI_COM_H_

#define USE_ADDR_UART1			(0)
#define OT_MAX_UART1			(10)
#define ACTIVE_CNT_UART1		(500)


typedef enum{
			 CMD_DATREQ = 0x1, CMD_DATREQ_R = 0x81,
			 CMD_DATSET = 0x2, CMD_DATSET_R = 0x82,
			 CMD_START  = 0x3, CMD_START_R  = 0x83,
			 CMD_ST_RUNMODE = 0x4, CMD_ST_RUNMODE_R =  0x84,
			 CMD_RUNORD = 0x5,	CMD_RUNORD_R = 0x85,
			 CMD_RST_ERR = 0x6,CMD_RST_ERR_R = 0x86
}CMD_TYPE;


void ProtocolInit_UART1(void);
void ProtocolProc_UART1(void);
void DataTrans_UART1(CMD_TYPE Type,u8 ,u8 );
void RcvRxDat_UART1(void);


#endif


/**
  ******************************************************************************
  * @file    Modbus.h
  * @author  RZ
  * @brief   Header for Modbus
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

#ifndef _MODBUS_H_
#define _MODBUS_H_

#include "Includings.h"

typedef enum{Idle_MB = 0, Txing_MB, WaitRply_MB, OT_MB, RplyErr_MB, RplyOK_MB} STATUS_MODBUS;

#define CRC(A)	{CRCIndex=CRCLo^((u8)A);CRCLo=CRCHi^TblCRCHi[CRCIndex];CRCHi=TblCRCLo[CRCIndex];}
#define TxWithCRC_MB(A)	{*hMB->pTxDat = (A);CRC(A);}													//A禁用指针、++等操作
#define Tx_MB(A)	(*hMB->pTxDat = (A))																//A禁用指针、++等操作

typedef struct
{
	u8 SlaveAddr;
	u8 OpCode;
	u8 TxReq;
	u16 StartAddr;
	u16 RegNum;
	u16 DatW[130];
	u16 CRC;

	u8 FSMSts;				//状态机
	u8 ExcepCode;
	u16 OTCnt;				//超时计数器
	u16 ErrCode;
	u8 RcvOK;
	u8 SlaveAddrR;
	u8 ByteNumR;
	u16 DatR[130];
	u16 CRCR;
	void (*RcvCallback)(void(*));

	void(* hParent);

	u16 u16Temp;
//参数设置全部用指针实现
	u32 *pSetBaud;
	u32 *pIsRxEmpty;
	u32 *pRxNum;
	u32 *pIsTxFull;
	u32 *pTxNum;
	u32 *pTxDat;
	u32 *pRxDat;
}tModbusCtl;

extern void ModbusInit(tModbusCtl *hMB);
extern void ModbusPoll(tModbusCtl *hMB);
extern void StartTxMB(tModbusCtl *hMB);
extern u8 IsBusyMB(tModbusCtl *hMB);

#endif



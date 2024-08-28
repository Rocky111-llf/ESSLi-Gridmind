/**
  ******************************************************************************
  * @file    CAN.h
  * @author  RZ
  * @brief   Header for CAN
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

#ifndef _CAN_H_
#define _CAN_H_

#include "xcanps.h"

typedef enum
{
	rate_1000kbps = 0,
	rate_800kbps,
	rate_500kbps,
	rate_250kbps,
	rate_125kbps,
	rate_100kbps,
	rate_50kbps,
	rate_20kbps
} S_CAN_BAUD;

typedef union
{
	u8 data_8[4];
	u16 data_16[2];
	u32 data_32;
} U_BIT8_16_32;

typedef struct {
  u8 BaudRate;
  u8 BaudRatePrescaler;
  u8 TimeSegment1;
  u8 TimeSegment2;
  u8 SyncJumpWidth;
} S_CAN_BIT_TIMING;

typedef struct {
	u8 data[8] __attribute__((aligned (4)));	//方便接收发送，采用4字节对齐/**< frame's data */
	u32 id;		/**< frame's ID */
	u8 extend_flag;
	u8 rtr;		/**< remote transmission request. (0 if not rtr frame, 1 if rtr frame) */
	u8 len;		/**< frame's length (0 to 8) */
} S_CAN_FRAME;

typedef struct {
	XCanPs CanPs;
	S_CAN_BAUD CanBaud;
	S_CAN_FRAME CanFrameTx,CanFrameRx;
	void (*RcvCallback)(void(*));
	void(* hParent);
}S_CANCTL;

extern void CANInit(S_CANCTL *hCAN);
extern u8 CANBusPoll(S_CANCTL *hCAN);
extern s32 CanTxProc(S_CANCTL *hCAN);

#endif



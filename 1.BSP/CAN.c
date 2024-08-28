/**
  ******************************************************************************
  * @file    CAN.c
  * @author  RZ
  * @brief   CAN.c module
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
#include "includings.h"

#define XCANPS_MAX_FRAME_SIZE_IN_WORDS (XCANPS_MAX_FRAME_SIZE / sizeof(u32))

u8 CANRxProc(S_CANCTL *hCAN);
u8 CANTxProc(S_CANCTL *hCAN);

S_CAN_BIT_TIMING can_bit_timing_config[]=
{
	{rate_1000kbps, 4,  15,  2, 0},
	{rate_800kbps,  24,  2,  0, 0},
	{rate_500kbps,  24,  5,  0, 0},
	{rate_250kbps,  49,  5,  0, 0},
	{rate_125kbps,  99,  5,  0, 0},
	{rate_100kbps,  124, 5,  0, 0},
	{rate_50kbps,   124, 12, 1, 0},
	{rate_20kbps,   249, 15, 2, 1}
};

u8 CANBusPoll(S_CANCTL *hCAN)
{
	return CANRxProc(hCAN);
}

void CANInit(S_CANCTL *hCAN)
{
	int i;
	XCanPs_Config *ConfigPtr;
	XCanPs *CanPsPtr = &(hCAN->CanPs);
	u8 BaudRate = hCAN->CanBaud;

	ConfigPtr = XCanPs_LookupConfig(XPAR_XCANPS_0_DEVICE_ID);
	XCanPs_CfgInitialize(CanPsPtr,ConfigPtr,ConfigPtr->BaseAddr);

	XCanPs_EnterMode(CanPsPtr, XCANPS_MODE_CONFIG);

	for(i=0; i<(sizeof(can_bit_timing_config)/sizeof(S_CAN_BIT_TIMING)); i++)
	{
		if(can_bit_timing_config[i].BaudRate == BaudRate)
		{
			XCanPs_SetBaudRatePrescaler(CanPsPtr, can_bit_timing_config[i].BaudRatePrescaler);
			XCanPs_SetBitTiming(CanPsPtr,  can_bit_timing_config[i].SyncJumpWidth,
					            can_bit_timing_config[i].TimeSegment2,
						        can_bit_timing_config[i].TimeSegment1);
			break;
		}

		if(i == ((sizeof(can_bit_timing_config)/sizeof(S_CAN_BIT_TIMING)) - 1))
		{
			xil_printf("Baud rate not found!\r\n");
		}
	}

	XCanPs_EnterMode(CanPsPtr, XCANPS_MODE_NORMAL);
}

s32 CanTxProc(S_CANCTL *hCAN)
{
	int Status;
	u32 s_id;
	u32 e_id;
	u32 RtrFlag,ExtFlag,Lenth;

	u32 TxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS] = {0};
	XCanPs *CanPsPtr = &(hCAN->CanPs);

	if(hCAN->CanFrameTx.extend_flag)
	{
		s_id = (hCAN->CanFrameTx.id >> 18)&0x000007ff;
		e_id = (hCAN->CanFrameTx.id) & 0x0003ffff;
		ExtFlag = 1;
	}
	else
	{
		s_id = (hCAN->CanFrameTx.id) & 0x000007ff;
		e_id = 0;
		ExtFlag = 0;
	}
	RtrFlag = hCAN->CanFrameTx.rtr;
	Lenth = hCAN->CanFrameTx.len;

	TxFrame[0] = (u32)XCanPs_CreateIdValue(s_id, RtrFlag, ExtFlag, e_id, 0);
	TxFrame[1] = (u32)XCanPs_CreateDlcValue(Lenth);
	TxFrame[2] = *((u32*)(&(hCAN->CanFrameTx.data[0])));		//(u32)(((u32)msg->data[3] << 24) | ((u32)msg->data[2] << 16) | ((u32)msg->data[1] << 8) | ((u32)msg->data[0]));
	TxFrame[3] = *((u32*)(&(hCAN->CanFrameTx.data[4])));		//(u32)(((u32)msg->data[7] << 24) | ((u32)msg->data[6] << 16) | ((u32)msg->data[5] << 8) | ((u32)msg->data[4]));

	Status = XCanPs_Send(CanPsPtr, TxFrame);

	return Status;
}

u8 CANRxProc(S_CANCTL *hCAN)
{
	u32 RxFram[XCANPS_MAX_FRAME_SIZE_IN_WORDS];
	u32 *pu32Temp;
	XCanPs *CanPsPtr = &(hCAN->CanPs);
	if(XCanPs_Recv(CanPsPtr, RxFram))	return 1;					//无数据
	hCAN->CanFrameRx.extend_flag = (u8)((RxFram[0] & XCANPS_IDR_IDE_MASK) >> XCANPS_IDR_IDE_SHIFT);

    if(hCAN->CanFrameRx.extend_flag)
    {
    	hCAN->CanFrameRx.id = ((u32)((RxFram[0] & XCANPS_IDR_ID2_MASK )>> XCANPS_IDR_ID2_SHIFT))
    			| ((u32)((RxFram[0] & XCANPS_IDR_ID1_MASK )>> 3));
    }
    else
    {
    	hCAN->CanFrameRx.id = (u16)((RxFram[0] & XCANPS_IDR_ID1_MASK )>> XCANPS_IDR_ID1_SHIFT);
    }

    hCAN->CanFrameRx.rtr = (u8)((RxFram[0] & XCANPS_IDR_SRR_MASK) >> XCANPS_IDR_SRR_SHIFT);
    hCAN->CanFrameRx.len = (u8)((RxFram[1] & XCANPS_DLCR_DLC_MASK) >> XCANPS_DLCR_DLC_SHIFT);

    pu32Temp = ((u32*)(&(hCAN->CanFrameRx.data[0])));
    *pu32Temp = RxFram[2];
    pu32Temp = ((u32*)(&(hCAN->CanFrameRx.data[4])));
    *pu32Temp = RxFram[3];

    hCAN->RcvCallback(hCAN->hParent);						//接收完成，执行回调函数
    return 0;
}

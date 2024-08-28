/**
  ******************************************************************************
  * @file    Modbus.c
  * @author  RZ
  * @brief   Modbus.c module
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

/*
#define SetBaud_CH0(A)		((hRS485_WR->Baud0.BAUDRATE) = ((u32)A<<8))			//设置波特率
#define IsRxEmpty_CH0()		(hRS485_RD->RxEmpty_CH0)							//RxFIFO已满？
#define RxNum_CH0()			(hRS485_RD->RxFFNum_CH0)							//
#define IsTxFull_CH0()		(hRS485_RD->TxFull_CH0)
#define TxNum_CH0()			(hRS485_RD->TxFFNum_CH0)
#define SetTx_CH0(A)		((hRS485_WR->TxD_CH0) = A)
#define GetRx_CH0()			((u8)(hRS485_RD->RxD_CH0))
 */

#include "Includings.h"

u8 ModbusRxProc(tModbusCtl *hMB);
u8 ModbusTxProc(tModbusCtl *hMB);

static u8 TblCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40 } ;
static u8 TblCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40 };

/*************************************************
 Function: ModbusInit
 Description: Modbus初始化
 Input: 无
 Output: 无
 Return:
 Others:
 *************************************************/
void ModbusInit(tModbusCtl *hMB)
{
	hMB->OTCnt = 0;
	hMB->ExcepCode = 0;
	hMB->TxReq = 0;
	hMB->ErrCode = 0;
	hMB->FSMSts = Idle_MB;
}

/*************************************************
 Function: StartTxMB
 Description: Modbus初始化
 Input: 无
 Output: 无
 Return:
 Others:
 *************************************************/
void StartTxMB(tModbusCtl *hMB)
{
	hMB->TxReq = 1;
	hMB->ErrCode = 0;
	hMB->RcvOK = 0;
}

/*************************************************
 Function: IsBusyMB()
 Description: 返回MB繁忙状态
 Input: 无
 Output: 无
 Return:
 Others:
 *************************************************/
u8 IsBusyMB(tModbusCtl *hMB)
{
	if((hMB->FSMSts) || (hMB->TxReq))
		return 1;
	else
		return 0;
}

#define UpdateMBSts(A)	{hMB->FSMSts = (A);hMB->OTCnt = 0;}
void ModbusPoll(tModbusCtl *hMB)
{
	u8 u8Temp;
	switch(hMB->FSMSts){
	case Idle_MB:
		if(hMB->TxReq)					//仅在空闲状态处理发送请求
		{
			if(ModbusTxProc(hMB))
			{
				//清空接收缓存
				hMB->u16Temp = RS485BUFLEN + 1;
				while((!(*hMB->pIsRxEmpty)) && (hMB->u16Temp--))	//防止死锁
				{
					u8Temp = *hMB->pRxDat;					//读取接收缓存
				}
				u8Temp = u8Temp;
				if(!(*hMB->pIsRxEmpty))						//考虑是否判断发送缓存？
					hMB->ErrCode |= 0x8;					//读缓存未能清空
				else
				{
					UpdateMBSts(Txing_MB);
				}
			}
			else
			{
				hMB->ErrCode |= 0x1;					//指令不支持
			}
			hMB->TxReq = 0;
		}
		break;
	case Txing_MB:
		if(*hMB->pTxNum > (RS485BUFLEN-2))				//剩下的未发送仅剩1个，指示发送正常进行
			UpdateMBSts(WaitRply_MB);
		if(hMB->OTCnt > 100)
			hMB->ErrCode |= 0x2;						//发送超时
		break;
	case WaitRply_MB:
		ModbusRxProc(hMB);
		if(hMB->RcvOK)
		{
			hMB->FSMSts = Idle_MB;
		}
		if(hMB->OTCnt > 200)
			hMB->ErrCode |= 0x4;						//接收超时
		break;
	default:
		break;
	}
	if(hMB->OTCnt < 5000)
		hMB->OTCnt++;
	if(hMB->ErrCode)
		hMB->FSMSts = Idle_MB;
}

u8 ModbusTxProc(tModbusCtl *hMB)
{
	u8 Temp;
	u8 CRCHi = 0xFF;
	u8 CRCLo = 0xFF;
	u8 CRCIndex;
	u8 i,j;
	u16 u16Temp;
	switch(hMB->OpCode){
	case 0x03:
		Temp = hMB->SlaveAddr;
		TxWithCRC_MB(Temp);			//从机地址
		TxWithCRC_MB(0x3);			//功能码
		u16Temp = hMB->StartAddr;
		Temp = u16Temp>>8;
		TxWithCRC_MB(Temp);			//开始地址高字节
		Temp = u16Temp;
		TxWithCRC_MB(Temp);			//开始地址低字节
		u16Temp = hMB->RegNum;
		Temp = u16Temp>>8;
		TxWithCRC_MB(Temp);			//寄存器数高字节
		Temp = u16Temp;
		TxWithCRC_MB(Temp);			//寄存器数低字节
		Tx_MB(CRCLo);
		Tx_MB(CRCHi);
		return 1;
		break;
	case 0x06:
		Temp = hMB->SlaveAddr;
		TxWithCRC_MB(Temp);			//从机地址
		TxWithCRC_MB(0x6);			//功能码
		u16Temp = hMB->StartAddr;
		Temp = u16Temp>>8;
		TxWithCRC_MB(Temp);			//开始地址高字节
		Temp = u16Temp;
		TxWithCRC_MB(Temp);			//开始地址低字节
		u16Temp = hMB->DatW[0];
		Temp = u16Temp>>8;
		TxWithCRC_MB(Temp);			//寄存器数据高字节
		Temp = u16Temp;
		TxWithCRC_MB(Temp);			//寄存器数据低字节
		Tx_MB(CRCLo);
		Tx_MB(CRCHi);
		return 1;
		break;
	case 0x10:
		Temp = hMB->SlaveAddr;
		TxWithCRC_MB(Temp);			//从机地址
		TxWithCRC_MB(0x10);			//功能码
		u16Temp = hMB->StartAddr;
		Temp = u16Temp>>8;
		TxWithCRC_MB(Temp);			//开始地址高地址
		Temp = u16Temp;
		TxWithCRC_MB(Temp);			//开始地址低地址
		u16Temp = hMB->RegNum;
		Temp = u16Temp>>8;
		TxWithCRC_MB(Temp);			//寄存器数高地址
		Temp = u16Temp;
		TxWithCRC_MB(Temp);			//寄存器数低地址
		Temp = u16Temp*2;
		TxWithCRC_MB(Temp);			//字节数地址
		i= hMB->RegNum;
		j = 0;
		while(i--)
		{
			u16Temp = hMB->DatW[j];
			Temp = u16Temp>>8;
			TxWithCRC_MB(Temp);			//寄存器数据高地址
			Temp = u16Temp;
			TxWithCRC_MB(Temp);			//寄存器数据低地址
			j++;
		}
		Tx_MB(CRCLo);
		Tx_MB(CRCHi);
		return 1;
		break;
	default:
		return 0;
		break;
	}
}

u8 ModbusRxProc(tModbusCtl *hMB)
{
	u8 u8Temp;				//接收数据缓存，等接收数量达到预设值后，合并进行处理
	u16 u16Temp,i;
	u16* pDat;
	u8 CRCHi = 0xFF;
	u8 CRCLo = 0xFF;
	u8 CRCIndex;
	switch(hMB->OpCode){
	case 0x03:
	{
		u16Temp = ((hMB->RegNum)<<1);
		if(*hMB->pRxNum >= (u16Temp + 5))
		{//已接收到的数据数量满足要求
			if((*hMB->pRxDat) != hMB->SlaveAddr)
			{
				hMB->ErrCode |= 0x100;						//接收地址错误
				break;
			}
			if((*hMB->pRxDat) != hMB->OpCode)
			{
				hMB->ErrCode |= 0x200;						//功能码错误,暂未处理0x80逻辑
				break;
			}
			if((*hMB->pRxDat) != u16Temp)
			{
				hMB->ErrCode |= 0x400;						//字节数错误
				break;
			}
			CRC(hMB->SlaveAddr);
			CRC(hMB->OpCode);
			CRC(u16Temp);
			i = hMB->RegNum;
			pDat = hMB->DatR;
			while(i--)
			{
				u8Temp = *hMB->pRxDat;
				CRC(u8Temp);
				((u8*)pDat)[1] = u8Temp;
				u8Temp = *hMB->pRxDat;
				CRC(u8Temp);
				((u8*)pDat)[0] = u8Temp;
				pDat++;
			}
			if(CRCLo == (*hMB->pRxDat))			//高低未一起比较为防止接收顺序错误
			{
				if(CRCHi == (*hMB->pRxDat))
				{
					hMB->RcvOK = 1;
					if(hMB->RcvCallback != 0)
						hMB->RcvCallback(hMB->hParent);		//执行回调函数
				}
				else
					hMB->ErrCode |= 0x800;		//校验错误
			}
			else
				hMB->ErrCode |= 0x800;			//校验错误
		}
		break;
	}
	case 0x06:
	{
		if(*hMB->pRxNum >= 8)
		{//已接收到的数据数量满足要求
			if((*hMB->pRxDat) != hMB->SlaveAddr)
			{
				hMB->ErrCode |= 0x100;						//接收地址错误
				break;
			}
			if((*hMB->pRxDat) != hMB->OpCode)
			{
				hMB->ErrCode |= 0x200;						//功能码错误,暂未处理0x80逻辑
				break;
			}
			CRC(hMB->SlaveAddr);
			CRC(hMB->OpCode);
			u8Temp = *hMB->pRxDat;
			CRC(u8Temp);
			((u8*)&u16Temp)[1] = u8Temp;
			u8Temp = *hMB->pRxDat;
			CRC(u8Temp);
			((u8*)&u16Temp)[0] = u8Temp;
			if(u16Temp != hMB->StartAddr)
			{
				hMB->ErrCode |= 0x1000;						//寄存器地址错误
				break;
			}
			u8Temp = *hMB->pRxDat;
			CRC(u8Temp);
			((u8*)&u16Temp)[1] = u8Temp;
			u8Temp = *hMB->pRxDat;
			CRC(u8Temp);
			((u8*)&u16Temp)[0] = u8Temp;
			if(u16Temp != hMB->DatW[0])
			{
				hMB->ErrCode |= 0x2000;						//寄存器值错误
				break;
			}
			if(CRCLo == (*hMB->pRxDat))			//高低未一起比较为防止接收顺序错误
			{
				if(CRCHi == (*hMB->pRxDat))
				{
					hMB->RcvOK = 1;
					if(hMB->RcvCallback != 0)
						hMB->RcvCallback(hMB->hParent);			//执行回调函数
				}
				else
					hMB->ErrCode |= 0x800;		//校验错误
			}
			else
				hMB->ErrCode |= 0x800;			//校验错误
		}
		break;
	}
	case 0x10:
	{
		if(*hMB->pRxNum >= 8)
		{//已接收到的数据数量满足要求
			if((*hMB->pRxDat) != hMB->SlaveAddr)
			{
				hMB->ErrCode |= 0x100;						//接收地址错误
				break;
			}
			if((*hMB->pRxDat) != hMB->OpCode)
			{
				hMB->ErrCode |= 0x200;						//功能码错误,暂未处理0x80逻辑
				break;
			}
			CRC(hMB->SlaveAddr);
			CRC(hMB->OpCode);
			u8Temp = *hMB->pRxDat;
			CRC(u8Temp);
			((u8*)&u16Temp)[1] = u8Temp;
			u8Temp = *hMB->pRxDat;
			CRC(u8Temp);
			((u8*)&u16Temp)[0] = u8Temp;
			if(u16Temp != hMB->StartAddr)
			{
				hMB->ErrCode |= 0x1000;						//寄存器地址错误
				break;
			}
			u8Temp = *hMB->pRxDat;
			CRC(u8Temp);
			((u8*)&u16Temp)[1] = u8Temp;
			u8Temp = *hMB->pRxDat;
			CRC(u8Temp);
			((u8*)&u16Temp)[0] = u8Temp;
			if(u16Temp != hMB->RegNum)
			{
				hMB->ErrCode |= 0x4000;						//寄存器数目错误
				break;
			}
			if(CRCLo == (*hMB->pRxDat))			//高低未一起比较为防止接收顺序错误
			{
				if(CRCHi == (*hMB->pRxDat))
				{
					hMB->RcvOK = 1;
					if(hMB->RcvCallback != 0)
						hMB->RcvCallback(hMB->hParent);			//执行回调函数
				}
				else
					hMB->ErrCode |= 0x800;		//校验错误
			}
			else
				hMB->ErrCode |= 0x800;			//校验错误
		}
		break;
	}
	default:
	{
		break;
	}}
	return hMB->RcvOK;
}



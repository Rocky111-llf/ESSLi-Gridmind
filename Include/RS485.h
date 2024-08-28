/**
  ******************************************************************************
  * @file    RS485.h
  * @author  RZ
  * @brief   Header for RS485
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

#ifndef _RS485_H_
#define _RS485_H_

#define RS485BUFLEN		(256)


typedef enum{RS485BAUD_9600 = 0, RS485BAUD_19200 = 1, RS485BAUD_38400 = 2, RS485BAUD_115200 = 3} RS485_BAUD;


//RS485写寄存器
typedef struct
{
	u32 TxD_CH0;				//CH0数据寄存器
	u32 Baud_CH0;				//CH0设置寄存器
	u32 TxD_CH1;				//CH1数据寄存器
	u32 Baud_CH1;				//CH1设置寄存器
	u32 TxD_CH2;				//CH2数据寄存器
	u32 Baud_CH2;				//CH2设置寄存器
	u32 TxD_CH3;				//CH3数据寄存器
	u32 Baud_CH3;				//CH3设置寄存器
}S_RS485_WR;

//RS485读寄存器
typedef struct
{
	u32 RxD_CH0;						//CH0数据寄存器
	u32 RxFFNum_CH0;					//CH0 RxFIFO可读数据量
	u32 RxEmpty_CH0;					//CH0 RxFIFO空标志
	u32 TxFFNum_CH0;					//CH0 TxFIFO可写数据量
	u32 TxFull_CH0;						//CH0 TxFIFO满标志
	u32 RxD_CH1;						//CH1数据寄存器
	u32 RxFFNum_CH1;					//CH1 RxFIFO可读数据量
	u32 RxEmpty_CH1;					//CH1 RxFIFO空标志
	u32 TxFFNum_CH1;					//CH1 TxFIFO可写数据量
	u32 TxFull_CH1;						//CH1 TxFIFO满标志
	u32 RxD_CH2;						//CH2数据寄存器
	u32 RxFFNum_CH2;					//CH2 RxFIFO可读数据量
	u32 RxEmpty_CH2;					//CH2 RxFIFO空标志
	u32 TxFFNum_CH2;					//CH2 TxFIFO可写数据量
	u32 TxFull_CH2;						//CH2 TxFIFO满标志
	u32 RxD_CH3;						//CH3数据寄存器
	u32 RxFFNum_CH3;					//CH3 RxFIFO可读数据量
	u32 RxEmpty_CH3;					//CH3 RxFIFO空标志
	u32 TxFFNum_CH3;					//CH3 TxFIFO可写数据量
	u32 TxFull_CH3;						//CH3 TxFIFO满标志
}S_RS485_RD;

extern S_RS485_WR* hRS485_WR;
extern S_RS485_RD* hRS485_RD;

#define Baud_CH0			(hRS485_WR->Baud_CH0)								//设置波特率
#define IsRxEmpty_CH0		(hRS485_RD->RxEmpty_CH0)							//RxFIFO已满？
#define RxNum_CH0			(hRS485_RD->RxFFNum_CH0)							//
#define IsTxFull_CH0		(hRS485_RD->TxFull_CH0)
#define TxNum_CH0			(hRS485_RD->TxFFNum_CH0)
#define TxDat_CH0			(hRS485_WR->TxD_CH0)
#define RxDat_CH0			(hRS485_RD->RxD_CH0)

#define Baud_CH1			(hRS485_WR->Baud_CH1)								//设置波特率
#define IsRxEmpty_CH1		(hRS485_RD->RxEmpty_CH1)							//RxFIFO已满？
#define RxNum_CH1			(hRS485_RD->RxFFNum_CH1)							//
#define IsTxFull_CH1		(hRS485_RD->TxFull_CH1)
#define TxNum_CH1			(hRS485_RD->TxFFNum_CH1)
#define SetTx_CH1			(hRS485_WR->TxD_CH1)
#define GetRx_CH1			(hRS485_RD->RxD_CH1)

#define Baud_CH2			(hRS485_WR->Baud_CH2)								//设置波特率
#define IsRxEmpty_CH2		(hRS485_RD->RxEmpty_CH2)							//RxFIFO已满？
#define RxNum_CH2			(hRS485_RD->RxFFNum_CH2)							//
#define IsTxFull_CH2		(hRS485_RD->TxFull_CH2)
#define TxNum_CH2			(hRS485_RD->TxFFNum_CH2)
#define SetTx_CH2			(hRS485_WR->TxD_CH2)
#define GetRx_CH2			(hRS485_RD->RxD_CH2)

#define Baud_CH3			(hRS485_WR->Baud_CH3)								//设置波特率
#define IsRxEmpty_CH3		(hRS485_RD->RxEmpty_CH3)							//RxFIFO已满？
#define RxNum_CH3			(hRS485_RD->RxFFNum_CH3)							//
#define IsTxFull_CH3		(hRS485_RD->TxFull_CH3)
#define TxNum_CH3			(hRS485_RD->TxFFNum_CH3)
#define SetTx_CH3			(hRS485_WR->TxD_CH3)
#define GetRx_CH3			(hRS485_RD->RxD_CH3)

#endif



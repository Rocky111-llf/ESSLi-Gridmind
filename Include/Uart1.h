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

#ifndef _UART1_H_
#define _UART1_H_

//UART1为本地TFT接口
#define UART1_BASEADDRESS 0xE0001000



#define RXBUFLEN_UART1			(512)
#define TXBUFLEN_UART1			(512)
#define FIFO_DEEPTH_UART1		(64)

#define CheckAddtoBuf_UART1(A) 	(u8Temp_UART1 = (A),SetTxDat_UART1(u8Temp_UART1), SumChk_TX += u8Temp_UART1, XorChk_TX ^= u8Temp_UART1)
#define AddtoBuf_UART1(A) 		(SetTxDat_UART1((A)))

#define INCX(A,B)	((A)<((B)-1)?((A)++):((A)=0))
#define INC_RXPTR_UART1(A) (INCX(A,RXBUFLEN_UART1))
#define INC_TXPTR_UART1(A) (INCX(A,TXBUFLEN_UART1))

extern u8 u8Temp_UART1;

void Init_UART1(void);
u8 SetTxDat_UART1(u8 Dat);
void SetTxFIFOLoop_UART1(void);

u8 GetRxDat_UART1(u8* Dat);
u16 LeftToRx_UART1();
void GetRxFIFOLoop_UART1(void);

#endif



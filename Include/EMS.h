/**
  ******************************************************************************
  * @file    EMS.h
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

#ifndef _EMS_H_
#define _EMS_H_

#define EMS_BASE	(0x00020000)
#define EMS_YK_BASE	(0x00020400) 			//共512字节，每个字节表示1个bit
#define EMS_YX_BASE	(0x00020600) 			//共512字节，每个字节表示1个bit
#define EMS_YC_BASE	(0x00020C00) 			//共1024字节，每2个字节表示1个变量
#define EMS_YT_BASE	(0x00021000) 			//共1024字节，每2个字节表示1个变量

#define EMS_YK_BUFLEN	512
#define EMS_YT_BUFLEN	512

#define EMS_YK_CMDBUF_ADDR_L	(0x00020004)
#define EMS_YK_CMDBUF_ADDR_H	(0x000203FC)
#define EMS_YT_CMDBUF_ADDR_L	(0x00020804)
#define EMS_YT_CMDBUF_ADDR_H	(0x00020BFC)

typedef struct{
	u8 Reset;
	u8 Start;
	u8 Stop;
	u8 EmStop;
	u8 rsvd_1[12];
	u8 StaticMode;
	u8 ChgMode;
	u8 PQMode;
}sEMS_YK_t;

typedef struct{
	u8 StopSts;
	u8 IdleSts;
	u8 StartSts;
	u8 RunSts;
	u8 FaultSts;
	u8 AlarmSts;
	u8 ChgSts;
	u8 DischgSts;
	u8 OnGridSts;
	u8 OffGridSts;
	u8 BMSComSts;
	u8 StaticMode;
	u8 ChgMode;
	u8 PQMode;
	u8 rsvd1[2];
	u8 VSCIdle;
	u8 VSCStart;
	u8 VSCRun;
	u8 VSCFault;
	u8 CBSts;
	u8 ContSts;
}sEMS_YX_t;

typedef struct{
	u16 Vab;
	u16 Vbc;
	u16 Vca;
	u16 Ia;
	u16 Ib;
	u16 Ic;
	s16 P;
	s16 Q;
	u16 f;
	s16 PF;
	s16 Vdc;
	s16 Idc;
	s16 Pdc;
	u16 FaultWord1;
	u16 FaultWord2;
	u16 BatVNorm;
	u16 BatAhNorm;
	u16 BatSNum;
	u16 BatIChgAllow;
	u16 BatIDisChgAllow;
	u16 BatPChgAllow;
	u16 BatPDisChgAllow;
	u16 BatCycNum;
	u16 SOH;
	u16 PCSFaultWord1;
	u16 PCSFaultWord2;	
	u16 SOC;
	u16 Vdca;
	u16 Vdcb;
	u16 Vdcc;
	u16 FaultWord_PEM_A;
	u16 FaultWord_PEM_B;
	u16 FaultWord_PEM_C;
	u16 rsvd2;
	s16 VBat;
	s16 IBat;
	u16 rsvd3[2];
	u16 CellVMaxNum;
	u16 CellVMax;
	u16 CellVMinNum;
	u16 CellVMin;
	u16 CellTMaxNum;
	u16 CellTMax;
	u16 CellTMinNum;
	u16 CellTMin;	
	u16 BMSFaultWord1;
	u16 BMSFaultWord2;
	u16 rsvd4[2];
	u16 CellV[68];
	u16 rsvd5[16];
	u16 CellT[20];
}sEMS_YC_t;

typedef struct{
	u16 OnOffGrid;
	u16 RunMode;
	u16 P_Cmd;
	u16 Q_Cmd;
	u16 PF_Cmd;
}sEMS_YT_t;

typedef struct{
	u16 Addr;
	u16 Num;
}sEMS_NUMADD_t;

typedef struct{
	u32 WrPtr;
	sEMS_NUMADD_t Dat[255];
}sEMS_BUFW_t;

typedef struct{
	u32 RdPtr;
	u16 AddrValidSts;
	sEMS_BUFW_t* EmsBufCtl;
}sEMS_YK_YT_BUFCTL_t;

typedef struct{
	u16 CtlSts;	
	sEMS_YX_t* EmsYX;
}sEMS_YXCTL_t;

typedef struct{
	u16 CtlSts;	
	sEMS_YC_t* EmsYC;
}sEMS_YCCTL_t;

typedef struct{
	sEMS_YK_YT_BUFCTL_t YKCtl;
	sEMS_YK_YT_BUFCTL_t YTCtl;
	sEMS_YXCTL_t YXCtl;				
	sEMS_YCCTL_t YCCtl;
}sEMS_CTL_t;

extern sEMS_CTL_t EMS_Ctl;

extern void EMSInit(sEMS_CTL_t* tEMSHandler);
extern void EMSPolling(sEMS_CTL_t* tEMSHandler);

#endif



/**
  ******************************************************************************
  * @file    ui_com.c
  * @author  RZ
  * @brief   Intelligent Screen Driver
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
#include "ui_com.h"

u8 CmdPending_UART1;					//接收命令待处理
u8 RcvStatus_UART1;						//接收状态机
u8 Cmd_UART1;							//数据类型
u8 pCmdDatBuf_UART1 = 0;				//数据存储区指针
u8 CmdDatBuf_UART1[256] __attribute__((aligned (4)));				//接收数据存储区,为了方便浮点，采用4字节对齐
static u8 RcvIntTimeCnt_UART1;

u8 SetRxDat_UART1(u8 Dat);
void DataRcv_UART1(void);
void DataProc_UART1(void);
void OTProc_UART1(void);

u8 Link_Active_UART1;
u16 Link_ActiveCnt_UART1 = 0;

extern void AuxCom(u8 Group, float *Dat);


/****************************************************************************
*
*UART1通讯协议相关变量初始化（UART1硬件自身已在main中初始化）
*
****************************************************************************/
void ProtocolInit_UART1(void)
{
	Init_UART1();
	RcvIntTimeCnt_UART1 = 0;
	RcvStatus_UART1 = 0;
	CmdPending_UART1 = 0;
	Cmd_UART1 = 0;
	Link_Active_UART1 = 0;
	Link_ActiveCnt_UART1 = 500;
}

void ProtocolProc_UART1(void)
{
	DataRcv_UART1();
	DataProc_UART1();
	SetTxFIFOLoop_UART1();
	OTProc_UART1();
}

/****************************************************************************
*
*UART1接受数据处理
*
****************************************************************************/
void DataRcv_UART1(void)
{
	u8 AntiDeadLockCnt;									//防死锁寄存器
	u8 ibuf;
	static u8 SumChk,XorChk,RcvDatLen_UART1;			//接收数据长度;

	GetRxFIFOLoop_UART1();								//将硬件FIFO中待读取

	if(CmdPending_UART1 != 0)							//上一次命令未处理，返回
		return;

	AntiDeadLockCnt = LeftToRx_UART1();					//最多处理缓冲区剩余数据
	while((!CmdPending_UART1) && (AntiDeadLockCnt--))
	{
		RcvIntTimeCnt_UART1 = 0;
		GetRxDat_UART1(&ibuf);
		switch (RcvStatus_UART1)							//帧状态机
		{
			case 0:
				CmdPending_UART1 = 0;
				pCmdDatBuf_UART1 = 0;						//接收有效数据区指针
				SumChk = 0;
				XorChk = 0;
				if(ibuf == 0x7E)							//判断帧头1
					RcvStatus_UART1 = 1;
				else
					RcvStatus_UART1 = 0;
				break;
			case 1:
				if(ibuf == 0xE7)							//判断帧头2
				{
					RcvStatus_UART1 = 2;
				}
				else if(ibuf == 0x7E)
					RcvStatus_UART1 = 1;
				else
					RcvStatus_UART1 = 0;
				break;
			case 2:
				if(ibuf == 0x7E)							//判断帧头3
				{
#if USE_ADDR_UART1
					RcvStatus_UART1 = 3;					//帧头收到
#else
					RcvStatus_UART1 = 7;					//帧头收到
#endif
				}
				else
					RcvStatus_UART1 = 0;
				break;
#if USE_ADDR_UART1
			case 3:
				((u8*)(&DestAddr_UART1))[1] = ibuf;			//目的子站地址

				SumChk += ibuf;
				XorChk ^= ibuf;
				RcvStatus_UART1 = 4;
				break;
			case 4:
				((u8*)(&DestAddr_UART1))[0] = ibuf;			//目的设备地址
				SumChk += ibuf;
				XorChk ^= ibuf;
				RcvStatus_UART1 = 5;
				break;
			case 5:
				((u8*)(&SourceAddr_UART1))[1] = ibuf;		//源子站地址
				SumChk += ibuf;
				XorChk ^= ibuf;
				RcvStatus_UART1 = 6;
				break;
			case 6:
				((u8*)(&SourceAddr_UART1))[0] = ibuf;		//源子设备地址
				SumChk += ibuf;
				XorChk ^= ibuf;
				RcvStatus_UART1 = 7;
				break;
#endif
			case 7:
				Cmd_UART1 = ibuf;							//指令类型
				SumChk += ibuf;
				XorChk ^= ibuf;
				RcvStatus_UART1 = 8;
				break;
			case 8:
				SumChk += ibuf;
				XorChk ^= ibuf;
				if(ibuf)						//非零数据长度
			    {
					RcvDatLen_UART1 = ibuf;		//记录数据长度
					RcvStatus_UART1 = 9;
				}
				else							//数据长度为零
					RcvStatus_UART1 = 10;
				break;
			case 9:								//接收数据长度内的
				CmdDatBuf_UART1[pCmdDatBuf_UART1] = ibuf;
				SumChk += ibuf;
				XorChk ^= ibuf;
				pCmdDatBuf_UART1 += 1;
				RcvDatLen_UART1 -= 1;
				if(0 == RcvDatLen_UART1)
					RcvStatus_UART1 = 10;
				break;
			case 10:
				if(XorChk != ibuf)
					RcvStatus_UART1 = 0;
				else
					RcvStatus_UART1 = 11;
				break;
			case 11:
				if(SumChk != ibuf)
					RcvStatus_UART1 = 0;
				else
					RcvStatus_UART1 = 12;
				break;
			case 12:
				if(0x0D == ibuf)
				{
#if USE_ADDR_UART1
					if((DestAddr_UART1 == MMC_ADDR) && (SourceAddr_UART1 == 0))
#endif
					{
						CmdPending_UART1 = 1;
						Link_Active_UART1 = 1;
						Link_ActiveCnt_UART1 = 500;
					}

				}
				RcvStatus_UART1 = 0;
				break;
		}
	}
}


void DataTrans_UART1(CMD_TYPE Type,u8 Para1, u8 Para2)
{
	u8 SumChk_TX = 0,XorChk_TX = 0;							//本地和、异或校验计算值
	float fTemp;
	u8* u8pTemp;
	u8 u8Temp = 0;
	u16 u16Temp;

	AddtoBuf_UART1(0x7e);
	AddtoBuf_UART1(0xe7);
	AddtoBuf_UART1(0x7e);
#if USE_ADDR_UART1
	CheckAddtoBuf_UART1(((u8*)(&DAddr))[1]);
	CheckAddtoBuf_UART1(((u8*)(&DAddr))[0]);
	CheckAddtoBuf_UART1(((u8*)(&SAddr))[1]);
	CheckAddtoBuf_UART1(((u8*)(&SAddr))[0]);
#endif

	if(Type == CMD_DATREQ_R)
	{
		CheckAddtoBuf_UART1(Type);					//命令

		if(Para1 == 0x0)
		{
			CheckAddtoBuf_UART1(79);					//长度，需要根据实际长度数据修改
			CheckAddtoBuf_UART1(Para1);					//表示运行数据
			CheckAddtoBuf_UART1(Para2);
			//Ua
			fTemp =  Ctl_LI1.pVSC->Va;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Ub
			fTemp =  Ctl_LI1.pVSC->Vb;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Uc
			fTemp =  Ctl_LI1.pVSC->Vc;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Ia
			// fTemp =  Ctl_LI1.pVSC->Ia;
			debug1 = Ctl_VSC1.Vac_Cmd;
			fTemp = debug1;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Ib
			// fTemp =  Ctl_LI1.pVSC->Ib;
			fTemp = debug2;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Ic
			fTemp =  Ctl_LI1.pVSC->Ic;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//P
			fTemp =  Ctl_LI1.pVSC->P;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Q
			fTemp =  Ctl_LI1.pVSC->Q;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//PF
			fTemp =  Ctl_LI1.pVSC->PF;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//f
			fTemp =  Ctl_LI1.pVSC->f;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Vdc
			fTemp =  Ctl_LI1.pVSC->Vdc;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Idc
			fTemp =  Ctl_LI1.pVSC->Idc;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//电池电压
			fTemp =  Ctl_LI1.tStackInfo.StackV;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//电池电流
			fTemp =  Ctl_LI1.tStackInfo.StackI;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//Pdc
			fTemp =  Ctl_LI1.pVSC->Pdc;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);
			//SOC
			fTemp =  Ctl_LI1.SOC*100.0f;
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[3]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[2]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&fTemp))[0]);

			//状态
			CheckAddtoBuf_UART1(Ctl_LI1.MainStatus);
			//模式
			CheckAddtoBuf_UART1((u8)Ctl_LI1.CtlMode);
			//VSC状态
			CheckAddtoBuf_UART1((u8)Ctl_LI1.pVSC->MainStatus);
			//电池状态
			CheckAddtoBuf_UART1((u8)Ctl_LI1.ChgSts);
			//禁充禁放状态
			CheckAddtoBuf_UART1((u8)Ctl_LI1.ChDischAllowedSts);
			//开关量
			u8Temp=0;
			if(IsCB())
				u8Temp |= 0x1;
			if(IsKM())
				u8Temp |= 0x2;
			// 这里为什么要注释 
//			if(Ctl_LI1.pVSC->AcPreChSts())
//				u8Temp |= 0x4;
//			if(Ctl_LI1.pVSC->DcContSts())
//				u8Temp |= 0x8;
//			if(Ctl_LI1.pVSC->DcPreChSts())
//				u8Temp |= 0x10;
			if(Ctl_LI1.MainStatus == RUN)
				u8Temp |= 0x20;
			if(Link_Active_UART1)
				u8Temp |= 0x40;
			if(Ctl_LI1.gSysErrFlag)
				u8Temp |= 0x80;
			CheckAddtoBuf_UART1(u8Temp);
			//ErrStatus
			u16Temp = (Ctl_LI1.gSysErrReg) & (Ctl_LI1.gErrMask);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);	//故障状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			//启动状态
			CheckAddtoBuf_UART1((u8)Ctl_LI1.StartUpStatus);
			u16Temp = Ctl_LI1.StartFailReg & 0xFFFF;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);	//启动失败寄存器低字，指示失败步骤
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = ((Ctl_LI1.StartFailReg)>>16) & 0xFFFF;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);	//启动失败寄存器低字，指示失败步骤
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
		}
		else if(Para1 == 0x1)
		{//返回功率模块运行数据
			CheckAddtoBuf_UART1(90);					//长度
			CheckAddtoBuf_UART1(Para1);					//表示运行数据
			CheckAddtoBuf_UART1(Para2);
			u8pTemp = (u8*)sOpt0.Dat->DAT;				//传输光纤板0的模块信息
			u8 i = 3*4;
			while(i--)
			{
				CheckAddtoBuf_UART1(*u8pTemp);
				u8pTemp++;
			}
			//传输组端数据
			u8pTemp = (u8*)(&(Ctl_LI1.tStackInfo.SOC));
			i = 76;//(u8*)(&(Ctl_LI1.tStackInfo.SOC))-(u8*)(&(Ctl_LI1.tStackInfo.SOC))+1;
			while(i--)
			{
				CheckAddtoBuf_UART1(*u8pTemp);
				u8pTemp++;
			}
		}
		else if(Para1 == 0x2)
		{//返回内部数据
			CheckAddtoBuf_UART1(38);								//长度
			CheckAddtoBuf_UART1(Para1);								//表示运行数据
			CheckAddtoBuf_UART1(Para2);
			u16Temp = (Ctl_LI1.gSysErrReg) & (Ctl_LI1.gErrMask);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//系统故障状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (Ctl_LI1.pVSC->gSysErrReg);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//VSC故障状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;//(Ctl_LI1.pDCDC->gSysErrReg);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//DC/DC故障状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (u16)sDio0.Dat->all;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//DI状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (u16)sDio0.Cmd->all;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//DO状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (u16)(Ctl_LI1.pVSC->pRop->Cmd->all);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//VSC DO状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;//(u16)(Ctl_LI1.pDCDC->pRop->Cmd->all);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//DC/DC DO状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = hS_FpgaUpLink->CardValid;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);				//通道板卡状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = hS_FpgaUpLink->CardType7_0 & 0xFFFF; 			//0~3板卡类型
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (hS_FpgaUpLink->CardType7_0>>16) & 0xFFFF; 	//4~7板卡类型
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = hS_FpgaUpLink->CardType9_8 & 0xFFFF; 			//9~8板卡类型
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = hS_FpgaUpLink->LvdsLinkSts & 0xFFFF; 			//LVDS下行连接状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (hS_FpgaUpLink->LvdsLinkSts>>16) & 0xFFFF; 	//LVDS上行连接状态
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = (u16)((10000000.0f/MAINFPGA_CLK)*hS_FpgaUpLink->CalcTime); 			//计算时间,=us*10
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
			u16Temp = 0;
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[1]);
			CheckAddtoBuf_UART1(((u8*)(&u16Temp))[0]);
		}
	}
	else if(Type == CMD_START_R)
	{
		CheckAddtoBuf_UART1(Type);					//命令
		CheckAddtoBuf_UART1(1);						//长度
		CheckAddtoBuf_UART1(Para1);
	}
	else if(Type == CMD_RST_ERR_R)
	{
		CheckAddtoBuf_UART1(Type);					//命令
		CheckAddtoBuf_UART1(1);						//长度
		CheckAddtoBuf_UART1(Para1);
	}
	else if(Type == CMD_DATSET_R)
	{
		CheckAddtoBuf_UART1(Type);					//命令
		CheckAddtoBuf_UART1(1);						//长度
		CheckAddtoBuf_UART1(Para1);
	}
	else
	{
		return;
	}

	AddtoBuf_UART1(XorChk_TX);						//异或校验
	AddtoBuf_UART1(SumChk_TX);						//和校验
	AddtoBuf_UART1(0x0d);							//和校验
	//执行发送程序
	SetTxFIFOLoop_UART1();
}

void DataProc_UART1(void)
{
	float f32Temp;
//	tVSC_CTL *pVSCx_Ctl;

	if(!CmdPending_UART1)
		return;

	if(Cmd_UART1 == CMD_DATREQ)					//获取到的是数据请求帧
	{
		if(CmdDatBuf_UART1[0] == 0x0)			//主页面运行数据请求
		{
			DataTrans_UART1(CMD_DATREQ_R,CmdDatBuf_UART1[0],CmdDatBuf_UART1[1]);
		}
		else if(CmdDatBuf_UART1[0] == 0x1)
		{
			if(CmdDatBuf_UART1[1] < 6)
			{
				DataTrans_UART1(CMD_DATREQ_R,CmdDatBuf_UART1[0],CmdDatBuf_UART1[1]);
			}
		}
		else if(CmdDatBuf_UART1[0] == 0x2)
		{
			DataTrans_UART1(CMD_DATREQ_R,CmdDatBuf_UART1[0],CmdDatBuf_UART1[1]);
		}
	}
	else if(Cmd_UART1 == CMD_START)
	{//开机指令
		if(CmdDatBuf_UART1[0] == 1)					//启动命令
			Ctl_LI1.MainStatus_Ref = START;
		else if(CmdDatBuf_UART1[0] == 0)
			Ctl_LI1.MainStatus_Ref = STOP;
		else
			Ctl_LI1.MainStatus_Ref = IDLE;
		DataTrans_UART1(CMD_START_R,0x1,0x0);
	}
	else if(Cmd_UART1 == CMD_RST_ERR)
	{//复位
		if(CmdDatBuf_UART1[0] == 1)					//复位命令
		{
			if((Ctl_LI1.MainStatus == IDLE) || (Ctl_LI1.MainStatus == STOP) || (Ctl_LI1.MainStatus == _FAULT))
				Ctl_LI1.ClrErr();
		}
		DataTrans_UART1(CMD_RST_ERR_R,0x1,0x0);
	}
	else if(Cmd_UART1 == CMD_DATSET)
	{
		if((CmdDatBuf_UART1[0] == 0x0) && (CmdDatBuf_UART1[2] == 0x0C))
		{//原设置IP指令

			DataTrans_UART1(CMD_DATSET_R,0x1,0x0);
		}
		else if((CmdDatBuf_UART1[0] == 0x1) && (CmdDatBuf_UART1[2] == 0x12))
		{//设置运行参数
			if((CmdDatBuf_UART1[1] == 0x0))
			{//控制模式设置
				if((CmdDatBuf_UART1[3] >= (u8)IDLE_LI) || (CmdDatBuf_UART1[3] <= (u8)PQ_LI))					//MODE
				{
					Ctl_LI1.CtlMode_Ref = CmdDatBuf_UART1[3];
					Ctl_LI1.CtlMode = Ctl_LI1.CtlMode_Ref;
					switch(Ctl_LI1.CtlMode_Ref){
					//extern float P_Cmd,Q_Cmd,Vdc_Cmd,Id_Cmd,Iq_Cmd,Vac_Cmd,Fre_Cmd;
					case IDLE_LI:
						break;
					case CH_DISCH_LI:
						SetWordB0(f32Temp,CmdDatBuf_UART1[4]);
						SetWordB1(f32Temp,CmdDatBuf_UART1[5]);
						SetWordB2(f32Temp,CmdDatBuf_UART1[6]);
						SetWordB3(f32Temp,CmdDatBuf_UART1[7]);
						HardLimit(f32Temp,0,1);
						Ctl_LI1.SOC_Ref = f32Temp;
						break;
					case PQ_LI:
					// 运行时需要切换跟构网模式时应先将锂电设置为PQ工作模式，PQ设置相应的值，然后再通过辅助下发选择VSC运行模式和参数值
						SetWordB0(f32Temp,CmdDatBuf_UART1[4]);
						SetWordB1(f32Temp,CmdDatBuf_UART1[5]);
						SetWordB2(f32Temp,CmdDatBuf_UART1[6]);
						SetWordB3(f32Temp,CmdDatBuf_UART1[7]);
						HardLimit(f32Temp,-1.0,1.0);
						Ctl_LI1.P_Ref = f32Temp;
						SetWordB0(f32Temp,CmdDatBuf_UART1[8]);
						SetWordB1(f32Temp,CmdDatBuf_UART1[9]);
						SetWordB2(f32Temp,CmdDatBuf_UART1[10]);
						SetWordB3(f32Temp,CmdDatBuf_UART1[11]);
						HardLimit(f32Temp,-1.0,1.0);
						Ctl_LI1.Q_Ref = f32Temp;
						break;
					default:
						break;
					}
				}
			}
			if((CmdDatBuf_UART1[1] == 0x1))
			{//控制模式设置
				AuxCom(CmdDatBuf_UART1[3], (float*)(&CmdDatBuf_UART1[4]));
				// VSC控制模式更改与参数下发
				Ctl_VSC1.CtlMode_Ref = (u16)VSC_CTLMODE;
				Ctl_VSC1.CtlMode = (u16)VSC_CTLMODE;
				switch(Ctl_VSC1.CtlMode){
					case PQCTL: // = 0
						// 跟网定功率控制
						Ctl_VSC1.P_Cmd = VSC_PREF;
						Ctl_VSC1.Q_Cmd = VSC_QREF;
						break;
					case VQCTL: // = 1
						Ctl_VSC1.Vdc_Cmd = VSC_UDCREF;
						Ctl_VSC1.Q_Cmd = VSC_QREF;
						break;
					case IDQCTL: // = 2
						// 暂时未定义，不用设置这个模式
						break;
					case VACCTL: // = 3
						Ctl_VSC1.P_Cmd = VSC_PREF;
						Ctl_VSC1.Q_Cmd = VSC_QREF;
#ifdef QVLOOP
						Ctl_VSC1.Vac_Cmd = VSC_UACREF;
#endif
				}
			}
			DataTrans_UART1(CMD_DATSET_R,0x1,0x0);
		}
	}
	CmdPending_UART1 = 0;						//处理完后允许再次接收
}

/****************************************************************************
*
*UART1协议超时处理，需要在主程序中定时调用
*
****************************************************************************/
void OTProc_UART1(void)
{
	if(RcvIntTimeCnt_UART1 < OT_MAX_UART1)
		RcvIntTimeCnt_UART1++;
	else
	{
		RcvIntTimeCnt_UART1 = 0;
		RcvStatus_UART1 = 0;
	}

	if(Link_ActiveCnt_UART1 > 0)
		Link_ActiveCnt_UART1--;
	else
		Link_Active_UART1 = 0;
}




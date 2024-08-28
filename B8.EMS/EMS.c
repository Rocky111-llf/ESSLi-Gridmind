/*****************************************************************************
  Copyright: 2019-2021, GridMind. Co., Ltd.
  File name: DCDC_GeneralCtl.c
  Description: DCDC基本控制
  Author: RZ
  Version: 版本
  Date: 完成日期
  History: 修改历史记录列表， 每条修改记录应包括修改日期、修改者及修改内容简述。
 *****************************************************************************/


#include "Includings.h"
#include "xil_mmu.h"

sEMS_CTL_t EMS_Ctl;

void EMS_YKProc(u16 Addr,u16 Num);		//遥控信号处理
void EMS_YTProc(u16 Addr,u16 Num);		//遥调信号处理
void EMS_YXProc(sEMS_CTL_t* tEMSHandler);						//遥信处理
void EMS_YCProc(sEMS_CTL_t* tEMSHandler);						//遥测处理


/*************************************************
  Function: EMSInit
  Description: EMS初始化
  Input: 无
  Output: 无
  Return: 无
  Others: //需要注意程序启动初期时候的值
 *************************************************/
void EMSInit(sEMS_CTL_t* tEMSHandler)
{
//关闭双核交互区缓存
	Xil_SetTlbAttributes(EMS_BASE,0x14de2);									//1MB
//遥控信号缓冲区初始化
	tEMSHandler->YKCtl.EmsBufCtl = (sEMS_BUFW_t*)0x00020000;
	tEMSHandler->YKCtl.RdPtr = tEMSHandler->YKCtl.EmsBufCtl->WrPtr;			//指针复归
	tEMSHandler->YKCtl.AddrValidSts = 1;
//遥调信号缓冲区初始化
	tEMSHandler->YTCtl.EmsBufCtl = (sEMS_BUFW_t*)0x00020800;
	tEMSHandler->YTCtl.RdPtr = tEMSHandler->YTCtl.EmsBufCtl->WrPtr;			//指针复归
	tEMSHandler->YTCtl.AddrValidSts = 1;
//遥信初始化
	tEMSHandler->YXCtl.CtlSts = 0;
	tEMSHandler->YXCtl.EmsYX = (sEMS_YX_t*)EMS_YX_BASE;
//遥测初始化
	tEMSHandler->YCCtl.CtlSts = 0;
	tEMSHandler->YCCtl.EmsYC = (sEMS_YC_t*)EMS_YC_BASE;
}

/*************************************************
  Function: EMSInit
  Description: EMS初始化
  Input: 无
  Output: 无
  Return: 无
  Others: //分别填充
 *************************************************/
void EMSPolling(sEMS_CTL_t* tEMSHandler)
{
	u32 u32Temp;
	sEMS_NUMADD_t *sEms_NumAddTemp;
//遥控信号处理
	u32Temp = tEMSHandler->YKCtl.EmsBufCtl->WrPtr;
	if(((u32Temp &0x3)==0) && (u32Temp>=EMS_YK_CMDBUF_ADDR_L) && (u32Temp<=EMS_YK_CMDBUF_ADDR_H)) 
	{//写地址范围正常
		if(tEMSHandler->YKCtl.AddrValidSts == 0)
		{
			tEMSHandler->YKCtl.RdPtr = tEMSHandler->YKCtl.EmsBufCtl->WrPtr - 4;			//指针复归，默认本次为有效数据
			tEMSHandler->YKCtl.AddrValidSts = 1;
		}
//		else
		{
			if(tEMSHandler->YKCtl.RdPtr < tEMSHandler->YKCtl.EmsBufCtl->WrPtr)
			{//存在新写入的数据
				tEMSHandler->YKCtl.RdPtr += 4;
				sEms_NumAddTemp = (sEMS_NUMADD_t *)tEMSHandler->YKCtl.RdPtr;
				EMS_YKProc(sEms_NumAddTemp->Addr,sEms_NumAddTemp->Num);
			}
			else if(tEMSHandler->YKCtl.RdPtr > tEMSHandler->YKCtl.EmsBufCtl->WrPtr)
			{
				if(tEMSHandler->YKCtl.RdPtr + 4 <= EMS_YK_CMDBUF_ADDR_H)
				{//缓冲区未翻转
					tEMSHandler->YKCtl.RdPtr += 4;
					sEms_NumAddTemp = (sEMS_NUMADD_t *)tEMSHandler->YKCtl.RdPtr;
					EMS_YKProc(sEms_NumAddTemp->Addr,sEms_NumAddTemp->Num);
				}
				else
				{
					tEMSHandler->YKCtl.RdPtr = EMS_YK_CMDBUF_ADDR_L;
					sEms_NumAddTemp = (sEMS_NUMADD_t *)tEMSHandler->YKCtl.RdPtr;
					EMS_YKProc(sEms_NumAddTemp->Addr,sEms_NumAddTemp->Num);
				}
			}
		}
	}
	else//写地址范围异常
		tEMSHandler->YKCtl.AddrValidSts = 0;
//遥调信号处理
	u32Temp = tEMSHandler->YTCtl.EmsBufCtl->WrPtr;
	if(((u32Temp &0x3)==0) && (u32Temp>=EMS_YT_CMDBUF_ADDR_L) && (u32Temp<=EMS_YT_CMDBUF_ADDR_H)) 
	{//写地址范围正常
		if(tEMSHandler->YTCtl.AddrValidSts == 0)
		{
			tEMSHandler->YTCtl.RdPtr = tEMSHandler->YTCtl.EmsBufCtl->WrPtr - 4;			//指针复归，默认本次为有效数据
			tEMSHandler->YTCtl.AddrValidSts = 1;
		}
//		else
		{
			if(tEMSHandler->YTCtl.RdPtr < tEMSHandler->YTCtl.EmsBufCtl->WrPtr)
			{//存在新写入的数据
				tEMSHandler->YTCtl.RdPtr += 4;
				sEms_NumAddTemp = (sEMS_NUMADD_t *)tEMSHandler->YTCtl.RdPtr;
				EMS_YTProc(sEms_NumAddTemp->Addr,sEms_NumAddTemp->Num);
			}
			else if(tEMSHandler->YTCtl.RdPtr > tEMSHandler->YTCtl.EmsBufCtl->WrPtr)
			{
				if(tEMSHandler->YTCtl.RdPtr + 4 <= EMS_YK_CMDBUF_ADDR_H)
				{//缓冲区未翻转
					tEMSHandler->YTCtl.RdPtr += 4;
					sEms_NumAddTemp = (sEMS_NUMADD_t *)tEMSHandler->YTCtl.RdPtr;
					EMS_YTProc(sEms_NumAddTemp->Addr,sEms_NumAddTemp->Num);
				}
				else
				{
					tEMSHandler->YTCtl.RdPtr = EMS_YK_CMDBUF_ADDR_L;
					sEms_NumAddTemp = (sEMS_NUMADD_t *)tEMSHandler->YTCtl.RdPtr;
					EMS_YTProc(sEms_NumAddTemp->Addr,sEms_NumAddTemp->Num);
				}
			}
		}
	}
	else//写地址范围异常
		tEMSHandler->YTCtl.AddrValidSts = 0;	
//遥信信号处理
	EMS_YXProc(tEMSHandler);
//遥测信号处理
	EMS_YCProc(tEMSHandler);
}

/*************************************************
  Function: EMS_YKProc
  Description: EMS遥控信号处理
  Input: 无
  Output: 无
  Return: 无
  Others: //分别填充
 *************************************************/
void EMS_YKProc(u16 Addr,u16 Num)
{
	u8 CmdTemp;
	u8* pu8Cmd = (u8*)EMS_YK_BASE;
	if(Addr > (EMS_YK_BUFLEN-1))	//首地址超出缓存范围
		return;
	if((Addr+Num) > EMS_YK_BUFLEN)	//总地址超出缓存范围
	{
		Num = EMS_YK_BUFLEN-Addr;
	}
	while(Num--)
	{
		CmdTemp = pu8Cmd[Addr];
		switch(Addr+1){
		case 1://故障清除
			if(CmdTemp == 1)
			{	
				if((Ctl_LI1.MainStatus == IDLE) || (Ctl_LI1.MainStatus == STOP) || (Ctl_LI1.MainStatus == _FAULT))
					Ctl_LI1.ClrErr();
			}
			break;
		case 2://设备启动
			if(CmdTemp == 1)
				Ctl_LI1.MainStatus_Ref = START;
			break;
		case 3://设备停机
			if(CmdTemp == 1)
				Ctl_LI1.MainStatus_Ref = STOP;
			break;
		case 4://设备急停
			if(CmdTemp == 1)
				Ctl_LI1.MainStatus_Ref = STOP;
			break;
		case 17:
			if(CmdTemp == 1)
			{
				Ctl_LI1.CtlMode_Ref = IDLE_LI;
				Ctl_LI1.CtlMode = Ctl_LI1.CtlMode_Ref;
			}	
			break;
		case 18:
			if(CmdTemp == 1)
			{
				Ctl_LI1.CtlMode_Ref = CH_DISCH_LI;
				Ctl_LI1.CtlMode = Ctl_LI1.CtlMode_Ref;
				Ctl_LI1.SOC_Ref = 1.0f;
			}	
			break;
		case 19:
			if(CmdTemp == 1)
			{
				Ctl_LI1.CtlMode_Ref = PQ_LI;
				Ctl_LI1.CtlMode = Ctl_LI1.CtlMode_Ref;
				Ctl_LI1.P_Ref = 0;
				Ctl_LI1.Q_Ref = 0;
			}	
			break;
		default:
			break;
		}
		Addr++;
	}
}

/*************************************************
  Function: EMS_YTProc
  Description: EMS遥调信号处理
  Input: 无
  Output: 无
  Return: 无
  Others: //分别填充
 *************************************************/
void EMS_YTProc(u16 Addr,u16 Num)		//遥调信号处理
{
	u16 DatTemp;
	u16* pu16Dat = (u16*)EMS_YT_BASE;
	float f32Temp;
	if(Addr < 512)
		return;						//地址范围错误
	else
		Addr -= 512;
	if(Addr > (EMS_YT_BUFLEN-1))	//首地址超出缓存范围
		return;
	if((Addr+Num) > EMS_YT_BUFLEN)	//总地址超出缓存范围
	{
		Num = EMS_YT_BUFLEN-Addr;
	}
	while(Num--)
	{
		DatTemp = pu16Dat[Addr];
		switch(Addr+1){
		case 1://并离网模式
			
			break;
		case 2://并网运行子模式
			if(DatTemp == 0)
			{
				Ctl_LI1.CtlMode_Ref = IDLE_LI;
				Ctl_LI1.CtlMode = Ctl_LI1.CtlMode_Ref;
			}	
			else if(DatTemp == 1)
			{
				Ctl_LI1.CtlMode_Ref = CH_DISCH_LI;
				Ctl_LI1.CtlMode = Ctl_LI1.CtlMode_Ref;
				Ctl_LI1.SOC_Ref = 1.0f;
			}	
			else if(DatTemp == 2)
			{
				Ctl_LI1.CtlMode_Ref = PQ_LI;
				Ctl_LI1.CtlMode = Ctl_LI1.CtlMode_Ref;
				Ctl_LI1.P_Ref = 0;
				Ctl_LI1.Q_Ref = 0;
			}	
			break;
		case 3://有功指令
			f32Temp = (10.0f*(1.0f/RATED_S))*((s16)DatTemp);			//0.01kW
			HardLimit(f32Temp,-1.0,1.0);
			Ctl_LI1.P_Ref = f32Temp;
			break;
		case 4://无功指令
			f32Temp = (10.0f*(1.0f/RATED_S))*((s16)DatTemp);			//0.01kW
			HardLimit(f32Temp,-1.0,1.0);
			Ctl_LI1.Q_Ref = f32Temp;
			break;
		case 5://功率因数
			
			break;
		default:
			break;
		}
		Addr++;
	}	
}

/*************************************************
  Function: EMS_YXProc
  Description: EMS遥信信号处理
  Input: 无
  Output: 无
  Return: 无
  Others: 若数量较多，可以分组
 *************************************************/
void EMS_YXProc(sEMS_CTL_t* tEMSHandler)						//遥信处理
{
	switch(tEMSHandler->YXCtl.CtlSts){
	case 0://当前每个时间片更新10个变量
		tEMSHandler->YXCtl.EmsYX->StopSts = (Ctl_LI1.MainStatus == STOP);
		tEMSHandler->YXCtl.EmsYX->IdleSts = (Ctl_LI1.MainStatus == IDLE);
		tEMSHandler->YXCtl.EmsYX->StartSts = (Ctl_LI1.MainStatus == START);
		tEMSHandler->YXCtl.EmsYX->RunSts = (Ctl_LI1.MainStatus == RUN);
		tEMSHandler->YXCtl.EmsYX->FaultSts = (Ctl_LI1.MainStatus == _FAULT);
		tEMSHandler->YXCtl.EmsYX->AlarmSts = (Ctl_LI1.MainStatus == _FAULT);
		tEMSHandler->YXCtl.EmsYX->ChgSts = (Ctl_LI1.Iopu > 0.1f);
		tEMSHandler->YXCtl.EmsYX->DischgSts = (Ctl_LI1.Iopu < -0.1f);
		tEMSHandler->YXCtl.EmsYX->OnGridSts = 1;
		tEMSHandler->YXCtl.EmsYX->OffGridSts = 0;
		tEMSHandler->YXCtl.CtlSts++;
		break;
	case 1:
		tEMSHandler->YXCtl.EmsYX->BMSComSts = ((Ctl_LI1.gSysErrReg & ERRLI_BMS_COM) == ERRLI_BMS_COM);
		tEMSHandler->YXCtl.EmsYX->StaticMode = (Ctl_LI1.CtlMode == IDLE_LI);
		tEMSHandler->YXCtl.EmsYX->ChgMode = (Ctl_LI1.CtlMode == CH_DISCH_LI);
		tEMSHandler->YXCtl.EmsYX->PQMode = (Ctl_LI1.CtlMode == PQ_LI);
		tEMSHandler->YXCtl.EmsYX->VSCIdle = (Ctl_LI1.pVSC->MainStatus == IDLE);
		tEMSHandler->YXCtl.EmsYX->VSCStart = (Ctl_LI1.pVSC->MainStatus == START);
		tEMSHandler->YXCtl.EmsYX->VSCRun = (Ctl_LI1.pVSC->MainStatus == RUN);
		tEMSHandler->YXCtl.EmsYX->VSCFault = (Ctl_LI1.pVSC->MainStatus == _FAULT);
		tEMSHandler->YXCtl.EmsYX->CBSts = (IsCB());
		tEMSHandler->YXCtl.EmsYX->ContSts = (IsKM());
		tEMSHandler->YXCtl.CtlSts++;
		break;
	default:
		if(tEMSHandler->YXCtl.CtlSts < 10) //总共10个时间片更新一次
			tEMSHandler->YXCtl.CtlSts++;
		else
			tEMSHandler->YXCtl.CtlSts = 0;
		break;
	}		
}

/*************************************************
  Function: EMS_YXProc
  Description: EMS遥测信号处理
  Input: 无
  Output: 无
  Return: 无
  Others: 若数量较多，可以分组
 *************************************************/
void EMS_YCProc(sEMS_CTL_t* tEMSHandler)						//遥测处理
{
	switch(tEMSHandler->YXCtl.CtlSts){
	case 0://当前每个时间片更新10个变量
		tEMSHandler->YCCtl.EmsYC->Vab = Ctl_LI1.pVSC->Va*10;
		tEMSHandler->YCCtl.EmsYC->Vbc = Ctl_LI1.pVSC->Vb*10;
		tEMSHandler->YCCtl.EmsYC->Vca = Ctl_LI1.pVSC->Vc*10;
		tEMSHandler->YCCtl.EmsYC->Ia = Ctl_LI1.pVSC->Ia*100;
		tEMSHandler->YCCtl.EmsYC->Ib = Ctl_LI1.pVSC->Ib*100;
		tEMSHandler->YCCtl.EmsYC->Ic = Ctl_LI1.pVSC->Ic*100;
		tEMSHandler->YCCtl.EmsYC->P = Ctl_LI1.pVSC->P*0.1f;
		tEMSHandler->YCCtl.EmsYC->Q = Ctl_LI1.pVSC->Q*0.1f;
		tEMSHandler->YCCtl.EmsYC->f = Ctl_LI1.pVSC->f*100;
		tEMSHandler->YCCtl.EmsYC->PF = Ctl_LI1.pVSC->PF*100;
		tEMSHandler->YCCtl.CtlSts++;
		break;
	case 1:
		tEMSHandler->YCCtl.EmsYC->Vdc = Ctl_LI1.pVSC->Vdc*10;
		tEMSHandler->YCCtl.EmsYC->Idc = Ctl_LI1.pVSC->Idc*100;
		tEMSHandler->YCCtl.EmsYC->Pdc = Ctl_LI1.pVSC->Pdc*0.1f;
		tEMSHandler->YCCtl.EmsYC->FaultWord1 = Ctl_LI1.gSysErrReg;
		tEMSHandler->YCCtl.EmsYC->FaultWord2 = 0;
		tEMSHandler->YCCtl.EmsYC->BatVNorm = Ctl_LI1.tStackInfo.RatedV;
		tEMSHandler->YCCtl.EmsYC->BatAhNorm = Ctl_LI1.tStackInfo.RatedAh;
		tEMSHandler->YCCtl.EmsYC->BatSNum = Ctl_LI1.tStackInfo.StringNum;
		tEMSHandler->YCCtl.EmsYC->BatIChgAllow = Ctl_LI1.tStackInfo.MaxChgI;
		tEMSHandler->YCCtl.EmsYC->BatIDisChgAllow = Ctl_LI1.tStackInfo.MaxDischI;
		tEMSHandler->YCCtl.EmsYC->BatPChgAllow = Ctl_LI1.tStackInfo.MaxChgP;
		tEMSHandler->YCCtl.EmsYC->BatPDisChgAllow = Ctl_LI1.tStackInfo.MaxDischP;
		tEMSHandler->YCCtl.EmsYC->BatCycNum = Ctl_LI1.tStackInfo.CycleNum;
		tEMSHandler->YCCtl.EmsYC->SOH = Ctl_LI1.tStackInfo.SOH;
		tEMSHandler->YCCtl.EmsYC->PCSFaultWord1 = Ctl_LI1.pVSC->gSysErrReg;
		tEMSHandler->YCCtl.EmsYC->PCSFaultWord2	= 0;
		tEMSHandler->YCCtl.EmsYC->SOC = Ctl_LI1.SOC*100;//0~1
		tEMSHandler->YCCtl.CtlSts++;
		break;
	case 2:
		tEMSHandler->YCCtl.EmsYC->Vdca = Ctl_LI1.pVSC->pHBDat_A->CapV*10;
		tEMSHandler->YCCtl.EmsYC->Vdcb = Ctl_LI1.pVSC->pHBDat_B->CapV*10;
		tEMSHandler->YCCtl.EmsYC->Vdcc = Ctl_LI1.pVSC->pHBDat_C->CapV*10;
		tEMSHandler->YCCtl.EmsYC->FaultWord_PEM_A = Ctl_LI1.pVSC->pHBDat_A->FaultWord;
		tEMSHandler->YCCtl.EmsYC->FaultWord_PEM_B = Ctl_LI1.pVSC->pHBDat_B->FaultWord;
		tEMSHandler->YCCtl.EmsYC->FaultWord_PEM_C = Ctl_LI1.pVSC->pHBDat_C->FaultWord;
		tEMSHandler->YCCtl.EmsYC->VBat = 10.0f*Ctl_LI1.tStackInfo.StackV;		//
		tEMSHandler->YCCtl.EmsYC->IBat = 10.0f*Ctl_LI1.tStackInfo.StackI;
		tEMSHandler->YCCtl.EmsYC->CellVMaxNum = Ctl_LI1.tStackInfo.CellVMaxNO;
		tEMSHandler->YCCtl.EmsYC->CellVMax = 1000.0f*Ctl_LI1.tStackInfo.CellVMax;
		tEMSHandler->YCCtl.CtlSts++;
		break;
	case 3:
		tEMSHandler->YCCtl.EmsYC->CellVMinNum = Ctl_LI1.tStackInfo.CellVMinNO;
		tEMSHandler->YCCtl.EmsYC->CellVMin = 1000.0f*Ctl_LI1.tStackInfo.CellVMin;
		tEMSHandler->YCCtl.EmsYC->CellTMaxNum = Ctl_LI1.tStackInfo.CellTMaxNO;
		tEMSHandler->YCCtl.EmsYC->CellTMax = Ctl_LI1.tStackInfo.CellTMax;
		tEMSHandler->YCCtl.EmsYC->CellTMinNum = Ctl_LI1.tStackInfo.CellTMinNO;
		tEMSHandler->YCCtl.EmsYC->CellTMin = Ctl_LI1.tStackInfo.CellTMin;
		tEMSHandler->YCCtl.EmsYC->BMSFaultWord1 = ((u16)Ctl_LI1.tStackInfo.ChgErrStage<<8)+Ctl_LI1.tStackInfo.ChgErrCode;
		tEMSHandler->YCCtl.EmsYC->BMSFaultWord1 = ((u16)Ctl_LI1.tStackInfo.DischErrStage<<8)+Ctl_LI1.tStackInfo.DischErrCode;
		tEMSHandler->YCCtl.CtlSts++;
		break;
	case 4:
		for(int i=0;i<68;i++){
			tEMSHandler->YCCtl.EmsYC->CellV[i] = 1000.0f*Ctl_LI1.tStackInfo.CELLV[i];
		}
		tEMSHandler->YCCtl.CtlSts++;
		break;
	case 5:
		for(int i=0;i<20;i++){
			tEMSHandler->YCCtl.EmsYC->CellT[i] = Ctl_LI1.tStackInfo.CELLT[i];
		}
		tEMSHandler->YCCtl.CtlSts++;
		break;
	default:
		if(tEMSHandler->YCCtl.CtlSts < 10) //总共10个时间片更新一次
			tEMSHandler->YCCtl.CtlSts++;
		else
			tEMSHandler->YCCtl.CtlSts = 0;
		break;
	}	
}

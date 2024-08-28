/*****************************************************************************
Copyright: 2019-2023, GridMind. Co., Ltd.
File name: VSCSeqCtl.c
Description: 顺序控制算法
Author: RZ
Version: 版本
Date: 完成日期
History: 修改历史记录列表， 每条修改记录应包括修改日期、修改者及修改内容简述。
*****************************************************************************/
#include "Includings.h"


#define SetSSDelay(A)	(tVSCHandler->StartUpDelayCnt = (A))
#define SSDelayOK()		(!tVSCHandler->StartUpDelayCnt)
#define SetSS(A)		(tVSCHandler->StartUpStatus = (A))

void VSCStopMachine(tVSC_CTL* tVSCHandler);
void VSCRunCheck(tVSC_CTL* tVSCHandler);

#define ACPRE_TIME		(3000)

//交流启动状态机状态枚举
typedef enum {
	SSAC_Prepare = 0,				/*准备状态*/
	SSAC_FPGACheck,					/*FPGA状态检查*/
	SSAC_StParaInit,				/*启动参数初始化*/
	SSAC_SWChk,						/*断路器状态检测*/
	SSAC_GridChk,					/*电网状态检测并持续，打开保护*/
	SSAC_HBCfg,						/*H桥配置及复位*/
	SSAC_HBChk,						/*H桥状态检测*/
	SSAC_PreChgDio,					/*合预充接触器，不控整流预充电*/
	SSAC_PreChgCtl,					/*可控整流*/
	SSAC_SWACDCOn,					/*合交流直流开关*/
	SSAC_FANChk,					/*风机启动，进入正常模式*/
	SSAC_StartFail					/*启动失败，务必跳开所有开关并闭锁*/
} ACSTART_STEPS;

//直流启动状态机状态枚举
typedef enum {
	SSDC_Prepare = 0,				/*准备状态*/
	SSDC_FPGACheck,					/*FPGA状态检查*/
	SSDC_StParaInit,				/*启动参数初始化*/
	SSDC_SWChk,						/*断路器状态检测*/
	SSDC_GridChk,					/*电网状态检测并持续，打开保护，如果是无源逆变则跳过此步*/
	SSDC_DcChk,						/*检测外部直流母线电压*/
	SSDC_HBCfg,						/*H桥配置及复位*/
	SSDC_HBChk,						/*H桥状态检测*/
	SSDC_PreChgDio,					/*合预充接触器，不控整流预充电*/
	SSDC_PreChgCtl,					/*可控整流*/
	SSDC_SWACDCOn,					/*合交流直流开关*/
	SSDC_FANChk,					/*风机启动*/
	SSDC_StartFail					/*启动失败，务必跳开所有开关并闭锁*/
} DCSTART_STEPS;

/****************************************************************************
*
*交流启动状态机
*
****************************************************************************/
/*启动流程：
预充完成后、闭锁模块，合直流侧开关
*/
void VSCStartUp_ACPreChg(tVSC_CTL* tVSCHandler)
{
	u16 u16Temp;
	u16 StateChanged = 0;

	if(tVSCHandler->StartUpStatusPre != tVSCHandler->StartUpStatus)
	{
		StateChanged = 1;
		tVSCHandler->StartUpStatusPre = tVSCHandler->StartUpStatus;
	}
	switch(tVSCHandler->StartUpStatus){
	case SSAC_Prepare:								//准备状态
		tVSCHandler->StartUpStatusPre = SSAC_Prepare;
		tVSCHandler->StartFailReg = 0;
		tVSCHandler->StartUpCheckEn = 0;
		tVSCHandler->StartCheckFlag = 0;

		SetSS(SSAC_FPGACheck);
		break;
	case SSAC_FPGACheck:							//板卡信息检测
		if(StateChanged)
		{//初次进入
			SetSSDelay(1000);
		}
		if(SSDelayOK())
		{
			u16Temp = SysChkPlatform();
			if(u16Temp != 0)
			{
				SetMSW(tVSCHandler->StartFailReg,u16Temp);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSAC_StartFail);
			}
			else
				SetSS(SSAC_StParaInit);
		}
		break;
	case SSAC_StParaInit:
		if(StateChanged)
		{//初次进入
			SetSSDelay(1000);
		}
		if(SSDelayOK())
		{
			VSCParasInit(tVSCHandler);
			tVSCHandler->gErrMask = (ERR_STARTFAIL);					//开启启动失败保护
			tVSCHandler->ClrErr();
			SetSS(SSAC_SWChk);
		}
		break;
	case SSAC_SWChk:									//
		if(StateChanged)
		{//初次进入
			SetSSDelay(1000);
			tVSCHandler->AcContCtl(0);
			tVSCHandler->DcContCtl(0);
			tVSCHandler->AcPreChCtl(0);
			tVSCHandler->DcPreChCtl(0);
		}
		if(SSDelayOK())
		{
			if(!((tVSCHandler->AcContSts())||(tVSCHandler->DcContSts())||(tVSCHandler->AcPreChSts())||(tVSCHandler->DcPreChSts())))		//开关初始位置正常
				SetSS(SSAC_GridChk);
			else
			{
				u16Temp = tVSCHandler->AcContSts() + (tVSCHandler->DcContSts()<<1) +(tVSCHandler->AcPreChSts()<<2) + (tVSCHandler->DcPreChSts()<<3);
				SetMSW(tVSCHandler->StartFailReg,u16Temp);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSAC_StartFail);
			}
		}
		break;
	case SSAC_GridChk:
		if(StateChanged)
		{//初次进入
			SetSSDelay(500);				//设置检测时间
			tVSCHandler->StartUpCheckEn |= (0x1|0x2);	//初次低压调试时可先屏蔽
		}
		if(SSDelayOK())
		{
			if(tVSCHandler->StartCheckFlag&(0x1|0x2))
			{
				u16Temp = 0;
				SetMSW(tVSCHandler->StartFailReg,u16Temp);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSAC_StartFail);
			}
			else
				SetSS(SSAC_HBCfg);
		}
		break;
	case SSAC_HBCfg:
		if(StateChanged)
		{//初次进入
			SetSSDelay(500);							//设置检测时间
			CfgCard();									//下发所有板卡配置
		}
		else
		{
			if(SSDelayOK())
				SetSS(SSAC_HBChk);
			else
			{
				u16Temp = (u16)tVSCHandler->pHBDat_A->CfgFlag + \
							   tVSCHandler->pHBDat_B->CfgFlag + \
							   tVSCHandler->pHBDat_C->CfgFlag;// +  tVSCHandler->pHBDat_N->CfgFlag;
				if(u16Temp != 3)
				{
					SetMSW(tVSCHandler->StartFailReg,u16Temp);
					SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
					SetSS(SSAC_StartFail);
				}
			}
		}
		break;
	case SSAC_HBChk:
		if(StateChanged)
		{//初次进入
			SetSSDelay(500);							//设置检测时间
			tVSCHandler->gEnHBOut = 0;
			HBBlk(0);									//释放全局闭锁
			tVSCHandler->pHBCmd_A->Mode = HB_MODE_RST;
			tVSCHandler->pHBCmd_B->Mode = HB_MODE_RST;
			tVSCHandler->pHBCmd_C->Mode = HB_MODE_RST;
			//tVSCHandler->pHBCmd_N->Mode = HB_MODE_RST;
		}
		else
		{
			if(SSDelayOK())			//所有模块应该处于复位状态，且模块有效
				SetSS(SSAC_PreChgDio);
			else
			{
				if(tVSCHandler->HBOK())
				{
					u16Temp = (u16)(tVSCHandler->pHBDat_A->HBMode!=HB_MODE_RST) + \
							  (u16)(tVSCHandler->pHBDat_B->HBMode!=HB_MODE_RST) + \
							  (u16)(tVSCHandler->pHBDat_C->HBMode!=HB_MODE_RST);// + (u16)(tVSCHandler->pHBDat_A->HBMode!=HB_MODE_RST);

					if(u16Temp != 0)
					{
						SetMSW(tVSCHandler->StartFailReg,1);
						SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
						SetSS(SSAC_StartFail);
					}
				}
				else
				{
					SetMSW(tVSCHandler->StartFailReg,0);
					SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
					SetSS(SSAC_StartFail);
				}
			}
		}
		break;
	case SSAC_PreChgDio:									//预充开启前，需要开启所有保护
		if(StateChanged)
		{//初次进入
			SetSSDelay(ACPRE_TIME);								//设置检测时间
			tVSCHandler->gErrMask |= (ERR_PEM_SW);						//开启模块保护
			tVSCHandler->StartUpCheckEn |= 0x4;
			tVSCHandler->AcPreChCtl(1);
		}
		if(SSDelayOK())
		{
			u16Temp = tVSCHandler->pHBDat_A->CapV;
			if((u16Temp > tVSCHandler->RMSInfo.RMS_Va*(RATED_ACV*sqrt2*ACPRECH_VOL_MIN)) && (u16Temp > tVSCHandler->RMSInfo.RMS_Vb*(RATED_ACV*sqrt2*ACPRECH_VOL_MIN)) && (u16Temp > tVSCHandler->RMSInfo.RMS_Vc*(RATED_ACV*sqrt2*ACPRECH_VOL_MIN)))
				SetSS(SSAC_SWACDCOn);
			else
			{
				SetMSW(tVSCHandler->StartFailReg,0);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSAC_StartFail);
			}
		}
		else if(tVSCHandler->StartUpDelayCnt == 1000)		//检查开关
		{
//			if(!tVSCHandler->AcPreChSts())
//			{
//				u16Temp = 0x1000;				//预充开关闭合失败
//				SetMSW(tVSCHandler->StartFailReg,u16Temp);
//				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
//				SetSS(SSAC_StartFail);
//			}
		}
		break;
	case SSAC_SWACDCOn:
		if(StateChanged)
		{//初次进入
			SetSSDelay(500);							//设置开关检测信息
			tVSCHandler->AcContCtl(1);					//合交流接触器
			tVSCHandler->AcPreChCtl(0);					//分交流预充
			tVSCHandler->DcContCtl(1);					//合直流接触器
		}
		if(SSDelayOK())
		{
//			if((tVSCHandler->AcContSts())&&(!tVSCHandler->AcPreChSts()))
				SetSS(SSAC_FANChk);
//			else
//			{
//				SetMSW(tVSCHandler->StartFailReg,0);
//				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
//				SetSS(SSAC_StartFail);
//			}
		}
		break;
	case SSAC_FANChk:
		tVSCHandler->gErrMask |= ERRx_EN_MASK;
		tVSCHandler->Vdc_Cmd = 1.0f;
		tVSCHandler->Vdc_Ref = tVSCHandler->DCV_Bus;				//缓启动
		tVSCHandler->MainStatus = RUN;								//进入运行状态
		tVSCHandler->pHBCmd_A->Mode = HB_MODE_PWM;					//模块设置为PWM模式
		tVSCHandler->pHBCmd_B->Mode = HB_MODE_PWM;
		tVSCHandler->pHBCmd_C->Mode = HB_MODE_PWM;
		//tVSCHandler->pHBCmd_N->Mode = HB_MODE_PWM;
		FANCtl = BOOL_TRUE;
		SysFanCtl(1);
		tVSCHandler->CFanCtl(1);
		break;
	case SSAC_StartFail:
		tVSCHandler->AcContCtl(0);
		tVSCHandler->AcPreChCtl(0);
		tVSCHandler->DcContCtl(0);
//		tVSCHandler->DcPreChCtl(0);
		tVSCHandler->gSysErrReg |= ERR_STARTFAIL;
		break;
	default:
		SetSS(SSAC_StartFail);
		break;
	}
	if(tVSCHandler->StartUpDelayCnt)
		tVSCHandler->StartUpDelayCnt--;

	if(tVSCHandler->StartCheckFlag)			//触发启动过程中保护
	{
		SetMSW(tVSCHandler->StartFailReg,tVSCHandler->StartCheckFlag);
		SetLSW(tVSCHandler->StartFailReg,SSAC_StartFail);
		SetSS(SSAC_StartFail);
	}
}


/*************************************************
 Function: StartUp_DCPreChg
 Description: 直流预充状态机
 Input: 无
 Output: 无
 Return: 0:OK,Others:Fault
 Others: 本次MMC板卡位置为 5678
 *************************************************/
void VSCStartUp_DCPreChg(tVSC_CTL* tVSCHandler)
{
	u16 u16Temp;
	u16 StateChanged = 0;
	if(tVSCHandler->StartUpStatusPre != tVSCHandler->StartUpStatus)
	{
		StateChanged = 1;
		tVSCHandler->StartUpStatusPre = tVSCHandler->StartUpStatus;
	}
	switch(tVSCHandler->StartUpStatus){
	case SSDC_Prepare:								//准备状态
		tVSCHandler->StartUpStatusPre = SSDC_Prepare;
		tVSCHandler->StartFailReg = 0;
		tVSCHandler->StartUpCheckEn = 0;
		tVSCHandler->StartCheckFlag = 0;

		SetSS(SSDC_FPGACheck);
		break;
	case SSDC_FPGACheck:							//板卡信息检测
		if(StateChanged)
		{//初次进入
			SetSSDelay(1000);
		}
		if(SSDelayOK())
		{
			u16Temp = SysChkPlatform();
			if(u16Temp != 0)
			{
				SetMSW(tVSCHandler->StartFailReg,u16Temp);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSDC_StartFail);
			}
			else
				SetSS(SSDC_StParaInit);
		}
		break;
	case SSDC_StParaInit:
		if(StateChanged)
		{//初次进入
			SetSSDelay(1000);
		}
		if(SSDelayOK())
		{
			VSCParasInit(tVSCHandler);
			tVSCHandler->gErrMask = (ERR_STARTFAIL);					//开启启动失败保护
			tVSCHandler->ClrErr();
			SetSS(SSDC_SWChk);
		}
		break;
	case SSDC_SWChk:						//
		if(StateChanged)
		{//初次进入
			SetSSDelay(1000);
			tVSCHandler->AcContCtl(0);
			tVSCHandler->DcContCtl(0);
			tVSCHandler->AcPreChCtl(0);
			tVSCHandler->DcPreChCtl(0);
		}
		if(SSDelayOK()) // 屏蔽交流断路器检测
		{
			// if(!((tVSCHandler->AcContSts())||(tVSCHandler->DcContSts())||(tVSCHandler->AcPreChSts())||(tVSCHandler->DcPreChSts())))		//开关初始位置正常
			// {
			// 	if(tVSCHandler->CtlMode == VACCTL)
					SetSS(SSDC_DcChk);
			// 	else
			// 		SetSS(SSDC_GridChk);
			// }
			// else
			// {
			// 	u16Temp = tVSCHandler->AcContSts() + (tVSCHandler->DcContSts()<<1) +(tVSCHandler->AcPreChSts()<<2) + (tVSCHandler->DcPreChSts()<<3);
			// 	SetMSW(tVSCHandler->StartFailReg,u16Temp);
			// 	SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
			// 	SetSS(SSDC_StartFail);
			// }
		}
		break;
	case SSDC_GridChk:
		if(StateChanged)
		{//初次进入
			SetSSDelay(500);				//设置检测时间
			tVSCHandler->StartUpCheckEn |= (0x1|0x2);	//初次低压调试时可先屏蔽
		}
		if(SSDelayOK())
		{
			if(tVSCHandler->StartCheckFlag&(0x1|0x2))
			{
				u16Temp = 0;
				SetMSW(tVSCHandler->StartFailReg,u16Temp);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSDC_StartFail);
			}
			else
				SetSS(SSDC_HBCfg);			//当前跳过直流电压检测，更改为直流预充电流检测
		}
		break;
	case SSDC_DcChk:
		if(StateChanged)
		{//初次进入
			SetSSDelay(500);				//设置检测时间
			// 屏蔽直流电压检测
			// tVSCHandler->StartUpCheckEn |= (0x8);		//开启直流电压检测
		}
		if(SSDelayOK())
		{
			// if(tVSCHandler->StartCheckFlag&(0x8))
			// {
			// 	u16Temp = 0;
			// 	SetMSW(tVSCHandler->StartFailReg,u16Temp);
			// 	SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
			// 	SetSS(SSDC_StartFail);
			// }
			// else
				SetSS(SSDC_HBCfg);
		}
		break;
	case SSDC_HBCfg:
		if(StateChanged)
		{//初次进入
			SetSSDelay(500);							//设置检测时间
			CfgCard();									//下发所有板卡配置
		}
		else
		{
			if(SSDelayOK())
				SetSS(SSDC_HBChk);
			else
			{
				u16Temp = (u16)tVSCHandler->pHBDat_A->CfgFlag + \
							   tVSCHandler->pHBDat_B->CfgFlag + \
							   tVSCHandler->pHBDat_C->CfgFlag;
							   //tVSCHandler->pHBDat_N->CfgFlag;
				if(u16Temp != 3)
				{
					SetMSW(tVSCHandler->StartFailReg,u16Temp);
					SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
					SetSS(SSAC_StartFail);
				}
			}
		}
		break;
	case SSDC_HBChk:
		if(StateChanged)
		{//初次进入
			SetSSDelay(500);							//设置检测时间
			tVSCHandler->gEnHBOut = 0;
			HBBlk(0);									//释放全局闭锁
			tVSCHandler->pHBCmd_A->Mode = HB_MODE_RST;
			tVSCHandler->pHBCmd_B->Mode = HB_MODE_RST;
			tVSCHandler->pHBCmd_C->Mode = HB_MODE_RST;
			//tVSCHandler->pHBCmd_N->Mode = HB_MODE_RST;
		}
		else
		{
			if(SSDelayOK())			//所有模块应该处于复位状态，且模块有效
				SetSS(SSDC_PreChgDio);
			// else
			// {
			// 	if(tVSCHandler->HBOK())
			// 	{
			// 		u16Temp = (u16)(tVSCHandler->pHBDat_A->HBMode==HB_MODE_RST) + \
			// 				  (u16)(tVSCHandler->pHBDat_B->HBMode==HB_MODE_RST) + \
			// 				  (u16)(tVSCHandler->pHBDat_C->HBMode==HB_MODE_RST);
			// 				  //(u16)(tVSCHandler->pHBDat_N->HBMode==HB_MODE_RST);

			// 		if(u16Temp != 3)
			// 		{
			// 			SetMSW(tVSCHandler->StartFailReg,1);
			// 			SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
			// 			SetSS(SSAC_StartFail);
			// 		}
			// 	}
			// 	else
			// 	{
			// 		SetMSW(tVSCHandler->StartFailReg,0);
			// 		SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
			// 		SetSS(SSDC_StartFail);
			// 	}
			// }
		}
		break;
	case SSDC_PreChgDio:									//预充开启前，需要开启所有保护
		if(StateChanged)
		{//初次进入
			SetSSDelay(3000);								//设置检测时间
			tVSCHandler->gErrMask |= (ERR_PEM_SW);			//开启模块保护
			tVSCHandler->StartUpCheckEn |= 0x4;
			tVSCHandler->DcPreChCtl(1);
		}
		if(SSDelayOK())
		{
			u16Temp = tVSCHandler->pHBDat_A->CapV;
			if((u16Temp > tVSCHandler->DCV_Bus*(RATED_DCV*DCPRECH_VOL_MIN)))
				SetSS(SSDC_SWACDCOn);
			else
			{
				SetMSW(tVSCHandler->StartFailReg,0);
				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
				SetSS(SSDC_StartFail);
			}
		}
		else if(tVSCHandler->StartUpDelayCnt == 2000)		//检查开关
		{
//			if(!tVSCHandler->DcPreChSts())
//			{
//				u16Temp = 0x1000;				//预充开关闭合失败
//				SetMSW(tVSCHandler->StartFailReg,u16Temp);
//				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
//				SetSS(SSDC_StartFail);
//			}
		}
		break;
	case SSDC_SWACDCOn:
		if(StateChanged)
		{//初次进入
			SetSSDelay(1000);							//设置开关检测信息
			tVSCHandler->AcContCtl(1);					//合交流接触器
			tVSCHandler->AcPreChCtl(0);					//分交流预充
			tVSCHandler->DcContCtl(1);					//合直流接触器
		}
		if(SSDelayOK())
		{
//			if((tVSCHandler->AcContSts())&&(tVSCHandler->DcContSts())&&(!tVSCHandler->AcPreChSts())&&(!tVSCHandler->DcPreChSts()))
				SetSS(SSDC_FANChk);
//			else
//			{
//				SetMSW(tVSCHandler->StartFailReg,0);
//				SetLSW(tVSCHandler->StartFailReg,tVSCHandler->StartUpStatus);
//				SetSS(SSDC_StartFail);
//			}
		}
		else if(tVSCHandler->StartUpDelayCnt == 500)
		{
			tVSCHandler->DcPreChCtl(0);					//延迟分直流预充
		}
		break;
	case SSDC_FANChk:
		tVSCHandler->gErrMask |= ERRx_EN_MASK;
		tVSCHandler->MainStatus = RUN;					//进入运行状态
		tVSCHandler->pHBCmd_A->Mode = HB_MODE_PWM;					//模块设置为PWM模式
		tVSCHandler->pHBCmd_B->Mode = HB_MODE_PWM;
		tVSCHandler->pHBCmd_C->Mode = HB_MODE_PWM;
		//tVSCHandler->pHBCmd_N->Mode = HB_MODE_PWM;
		FANCtl = BOOL_TRUE;
		SysFanCtl(1);
		tVSCHandler->CFanCtl(1);
		break;
	case SSDC_StartFail:
		tVSCHandler->AcContCtl(0);
		tVSCHandler->AcPreChCtl(0);
		tVSCHandler->DcContCtl(0);
		tVSCHandler->DcPreChCtl(0);
		tVSCHandler->gSysErrReg |= ERR_STARTFAIL;
		break;
	default:
		SetSS(SSDC_StartFail);
		break;
	}
	if(tVSCHandler->StartUpDelayCnt)
		tVSCHandler->StartUpDelayCnt--;

	if(tVSCHandler->StartCheckFlag)			//触发启动过程中保护
	{
		SetMSW(tVSCHandler->StartFailReg,tVSCHandler->StartCheckFlag);
		SetLSW(tVSCHandler->StartFailReg,SSDC_StartFail);
		SetSS(SSDC_StartFail);
	}
}

/*************************************************
 Function: AcStartUpCheck
 Description: 交流启动中间状态监控
 Input: 无
 Output: 无
 Return: 0:OK,Others:Fault
 Others: 用于交流启动过程中监控
 *************************************************/
void VSCAcStartUpCheck(tVSC_CTL* tVSCHandler)
{
	LimitProc(&tVSCHandler->FrePLL_Limit,tVSCHandler->PLLFre,START);
	LimitProc(&tVSCHandler->ACVINA_Limit,tVSCHandler->RMSInfo.RMS_Va,START);
	LimitProc(&tVSCHandler->ACVINA_Limit,tVSCHandler->RMSInfo.RMS_Vb,START);
	LimitProc(&tVSCHandler->ACVINA_Limit,tVSCHandler->RMSInfo.RMS_Vc,START);

	if(tVSCHandler->StartUpCheckEn & 0x1)		//锁相环检测
	{
		if(tVSCHandler->FrePLL_Limit.Result != LIMIT_OK)
			tVSCHandler->StartCheckFlag |= 0x1;
	}
	if(tVSCHandler->StartUpCheckEn & 0x2)		//输入电压检测
	{
		if((tVSCHandler->ACVINA_Limit.Result)+(tVSCHandler->ACVINB_Limit.Result)+(tVSCHandler->ACVINC_Limit.Result) != LIMIT_OK)
			tVSCHandler->StartCheckFlag |= 0x2;
	}
	if(tVSCHandler->StartUpCheckEn & 0x4)		//模块检测
	{//开启模块检测
		if(!tVSCHandler->HBOK())
			tVSCHandler->StartCheckFlag |= 0x4;
	}
}

/*************************************************
 Function: DcStartUpCheck
 Description: 直流启动中间状态监控
 Input: 无
 Output: 无
 Return: 0:OK,Others:Fault
 Others: 用于直流启动过程中监控
 *************************************************/
void VSCDcStartUpCheck(tVSC_CTL* tVSCHandler)
{
	LimitProc(&tVSCHandler->FrePLL_Limit,tVSCHandler->PLLFre,START);
	LimitProc(&tVSCHandler->ACVINA_Limit,tVSCHandler->RMSInfo.RMS_Va,START);
	LimitProc(&tVSCHandler->ACVINB_Limit,tVSCHandler->RMSInfo.RMS_Vb,START);
	LimitProc(&tVSCHandler->ACVINC_Limit,tVSCHandler->RMSInfo.RMS_Vc,START);
	LimitProc(&tVSCHandler->DCVPreCharge_Limit,tVSCHandler->DCV_Bus,START);

	if(tVSCHandler->StartUpCheckEn & 0x1)		//锁相环检测
	{
		if(tVSCHandler->FrePLL_Limit.Result != LIMIT_OK)
			tVSCHandler->StartCheckFlag |= 0x1;
	}
	if(tVSCHandler->StartUpCheckEn & 0x2)		//输入电压检测
	{
		if((tVSCHandler->ACVINA_Limit.Result)+(tVSCHandler->ACVINB_Limit.Result)+(tVSCHandler->ACVINC_Limit.Result) != LIMIT_OK)
			tVSCHandler->StartCheckFlag |= 0x2;
	}
	if(tVSCHandler->StartUpCheckEn & 0x4)		//模块检测
	{//开启模块检测
		if(!tVSCHandler->HBOK())
			tVSCHandler->StartCheckFlag |= 0x4;
	}
	if(tVSCHandler->StartUpCheckEn & 0x8)		//直流电压检测
	{//开启直流电压检测
		if(tVSCHandler->DCVPreCharge_Limit.Result != LIMIT_OK)
			tVSCHandler->StartCheckFlag |= 0x8;
	}
}

/****************************************************************************
*
*运行中检测
*
****************************************************************************/
void VSCRunCheck(tVSC_CTL* tVSCHandler)
{
	// 软件过欠压检测，孤网运行时需要屏蔽
	if((LimitProc(&tVSCHandler->ACVINA_Limit,tVSCHandler->RMSInfo.RMS_Va,RUN)+LimitProc(&tVSCHandler->ACVINB_Limit,tVSCHandler->RMSInfo.RMS_Vb,RUN)+LimitProc(&tVSCHandler->ACVINC_Limit,tVSCHandler->RMSInfo.RMS_Vc,RUN))!=LIMIT_OK)
	{
		// if(tVSCHandler->CtlMode != VACCTL){
		// 	tVSCHandler->gSysErrReg |= ERR_AC_UVOV_SW;
		// }
	}
	if(LimitProc(&tVSCHandler->FrePLL_Limit,tVSCHandler->PLLFre,RUN) != LIMIT_OK)
	{
		// if(tVSCHandler->CtlMode != VACCTL)
		// 	tVSCHandler->gSysErrReg |= ERR_PLL_SW;
	}
}

/****************************************************************************
*
*主状态机
*
****************************************************************************/
void VSCMainFSM(tVSC_CTL* tVSCHandler)
{
//主状态机，用于控制系统主工作状态跳转
	switch(tVSCHandler->MainStatus){
	case IDLE:
		break;
	case START:
		if(tVSCHandler->StartMode == DC_PRECHG)
		{
			VSCStartUp_DCPreChg(tVSCHandler);
			VSCDcStartUpCheck(tVSCHandler);
		}
		else
		{
			VSCStartUp_ACPreChg(tVSCHandler);
			VSCAcStartUpCheck(tVSCHandler);
		}
		break;
	case RUN:
		VSCRunCheck(tVSCHandler);
		break;
	case STOP:
		VSCStopMachine(tVSCHandler);
		break;
	case _FAULT:
//		tVSCHandler->AcBrkCtl(0);								//跳开交流断路器
		VSCStopMachine(tVSCHandler);
		tVSCHandler->MainStatus_Ref = IDLE;						//防止错误清除后立即启动
		break;
	default:
		tVSCHandler->StartUpStatus = 0;
		tVSCHandler->MainStatus = IDLE;
		break;
	}
	if(tVSCHandler->MainStatusPre != tVSCHandler->MainStatus)
	{
		tVSCHandler->MainStatusPre = tVSCHandler->MainStatus;
		if(tVSCHandler->MainStatus == START)
			tVSCHandler->StartMode = tVSCHandler->StartMode_Ref;	//启动前Latch启动模式
		tVSCHandler->StartUpStatus = 0;										//状态切换时，tVSCHandler->StartUpStatus复归
		tVSCHandler->RelayDelayCnt = 0;
		tVSCHandler->gEnHBOut = 0;
	}

}

/*************************************************
 Function: MainFSM_Init
 Description:
 Input: 无
 Output: 无
 Return:
 Others:
 *************************************************/
void VSCMainFSM_Init(tVSC_CTL* tVSCHandler)
{
	tVSCHandler->MainStatus_Ref = IDLE;
	tVSCHandler->MainStatus = IDLE;
	tVSCHandler->StartMode_Ref = AC_PRECHG;
	tVSCHandler->MainStatusPre = _FAULT-1;
}

void VSCStopMachine(tVSC_CTL* tVSCHandler)
{
//	HBBlk(1);										//闭锁H桥
	tVSCHandler->pHBCmd_A->Mode = HB_MODE_FBLK;
	tVSCHandler->pHBCmd_B->Mode = HB_MODE_FBLK;
	tVSCHandler->pHBCmd_C->Mode = HB_MODE_FBLK;
//	tVSCHandler->pHBCmd_N->Mode = HB_MODE_FBLK;
	if(tVSCHandler->RelayDelayCnt < 50000)
		tVSCHandler->RelayDelayCnt++;
	if(tVSCHandler->RelayDelayCnt > 100)							//至少延迟100ms
	{
		tVSCHandler->AcBrkCtl(0);									//交流跳闸复归
		tVSCHandler->AcContCtl(0);
		tVSCHandler->AcPreChCtl(0);
		if((RATED_DCI*tVSCHandler->DCI_Bus)<DCRELAY_OPENCURRENT_TH)
		{
			tVSCHandler->DcPreChCtl(0);
			tVSCHandler->DcContCtl(0);
			if(!(tVSCHandler->AcContSts() || tVSCHandler->AcPreChSts()))
			{
				if(tVSCHandler->gSysErrFlag)
					tVSCHandler->MainStatus = _FAULT;
				else
					tVSCHandler->MainStatus = IDLE;
			}
		}
	}
}



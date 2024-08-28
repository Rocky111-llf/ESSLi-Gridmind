#include "Includings.h"

void LIParasInit(tLI_CTL* tLIHandler);
void LILocalParasUpdate(tLI_CTL*);
void LIFaultDet(tLI_CTL*);
void LIStartUp(tLI_CTL* tLIHandler);
void LIRunProc(tLI_CTL* tLIHandler);
void LIStopMachine(tLI_CTL* tLIHandler);
void LIBMSCom(tLI_CTL* tLIHandler);
void LIBMSDatRcv(void(*tLIHandlerVoid));

#define ERRMSK_ALWAYS	(ERRLI_EMER_HW|ERRLI_BMS_HW|ERRLI_BMS_COM)

//直流启动状态机状态枚举
typedef enum {
	SSLI_Prepare = 0,			/*准备状态*/
	SSLI_StParaInit,			/*启动参数初始化*/
	SSLI_SWCtl,					/*系统开关控制*/
	SSLI_VSCStart,				/*启动VSC*/
	SSLI_RSVD1,					/*DCDC预留*/
	SSLI_StartOK,				/*启动成功*/
	SSLI_StartFail				/*启动失败*/
} LISTART_STEPS;

#define SetSSDelay(A)	(tLIHandler->StartUpDelayCnt = (A))
#define SSDelayOK()		(!tLIHandler->StartUpDelayCnt)
#define SetSS(A)		(tLIHandler->StartUpStatus = (A))

float PLimTempLI;

/*************************************************
 Function: LIMainFSM
 Description: LI主状态机
 Input: LI句柄
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void LIMainFSM(tLI_CTL* tLIHandler)
{
//主状态机，用于控制系统主工作状态跳转
	switch(tLIHandler->MainStatus){
	case IDLE:
		break;
	case START:
		LIStartUp(tLIHandler);
		break;
	case RUN:
		LIRunProc(tLIHandler);
		break;
	case STOP:
		LIStopMachine(tLIHandler);
		break;
	case _FAULT:
		LIStopMachine(tLIHandler);
		tLIHandler->MainStatus_Ref = IDLE;									//防止错误清除后立即启动
		break;
	default:
		tLIHandler->StartUpStatus = 0;
		tLIHandler->MainStatus = IDLE;
		break;
	}
	if(tLIHandler->MainStatusPre != tLIHandler->MainStatus)
	{
		tLIHandler->MainStatusPre = tLIHandler->MainStatus;
		tLIHandler->StartUpStatus = 0;										//状态切换时，tLIHandler->StartUpStatus复归
	}
	LILocalParasUpdate(tLIHandler);
	LIFaultDet(tLIHandler);
}

/*************************************************
 Function: LIInit
 Description: LI初始化
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void LIInit(tLI_CTL* tLIHandler)
{
	tLIHandler->Init();
	tLIHandler->tCANCtl.RcvCallback = LIBMSDatRcv;
	CANInit(&tLIHandler->tCANCtl);
	tLIHandler->CANOTCnt = 500;
	tLIHandler->BMS_ComOK = 1;
	LIParasInit(tLIHandler); // 初始化了两次
}
/*************************************************
 Function: LIParasInit
 Description: LI主状态机
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void LIParasInit(tLI_CTL* tLIHandler)
{
//PID
	tLIHandler->P_PID = tLIHandler->P_PID_INIT;
	tLIHandler->P_Ref = 0;

	tLIHandler->VLimH_PID = tLIHandler->VO_PID_INIT;
	tLIHandler->VLimH_PID.Ref = tLIHandler->V_Max * tLIHandler->Vb_Reci;
	tLIHandler->VLimH_PID.OutMin = -0.1f;
	tLIHandler->VLimL_PID = tLIHandler->VO_PID_INIT;
	tLIHandler->VLimL_PID.Ref = tLIHandler->V_Min * tLIHandler->Vb_Reci;
	tLIHandler->VLimL_PID.OutMax = 0.1f;
	tLIHandler->Vo_PID = tLIHandler->VO_PID_INIT;
	tLIHandler->Io_PID = tLIHandler->IO_PID_INIT;
//LIMIT
	tLIHandler->Vo_Limit = tLIHandler->VO_LIMITREF;
	tLIHandler->Io_Limit = tLIHandler->IO_LIMITREF;

	tLIHandler->IoLimH = 1.2f;
	tLIHandler->IoLimL = -1.2f;

	tLIHandler->CtlMode_Ref = IDLE_LI;
	tLIHandler->CtlMode = IDLE_LI;

	tLIHandler->ChgSts = CHG_IDLE_LI;

	tLIHandler->CtlModePre = tLIHandler->CtlMode+1;

	tLIHandler->gErrMask = ERRMSK_ALWAYS;

	tLIHandler->ChgNotAllowedFlag = 0;				//标志清零
	tLIHandler->DischNotAllowedFlag = 0;
	tLIHandler->ChDischAllowedSts = 0;
}

/*************************************************
 Function: LIMainFSM_Init
 Description:
 Input: 无
 Output: 无
 Return:
 Others:
 *************************************************/
void LIMainFSM_Init(tLI_CTL* tLIHandler)
{
	tLIHandler->MainStatus_Ref = IDLE;
	tLIHandler->MainStatus = IDLE;
	tLIHandler->MainStatusPre = _FAULT-1;
	tLIHandler->CtlMode_Ref = IDLE_LI;
	tLIHandler->CtlMode = IDLE_LI;
}

/*************************************************
 Function: LIStartUp
 Description: LI启动
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void LIStartUp(tLI_CTL* tLIHandler)
{
	u16 u16Temp;
	u16 StateChanged = 0;
	if(tLIHandler->StartUpStatusPre != tLIHandler->StartUpStatus)
	{
		StateChanged = 1;
		tLIHandler->StartUpStatusPre = tLIHandler->StartUpStatus;
	}
	switch(tLIHandler->StartUpStatus){
	case SSLI_Prepare:								//准备状态
		tLIHandler->StartUpStatusPre = SSLI_Prepare;
		tLIHandler->StartFailReg = 0;
		SetSS(SSLI_StParaInit);
		break;
	case SSLI_StParaInit:
		if(StateChanged)
		{//初次进入
			SetSSDelay(1000);
		}
		if(SSDelayOK())
		{
			LIParasInit(tLIHandler);
			tLIHandler->gErrMask |= (ERRLI_STARTFAIL);					//开启启动失败保护
			tLIHandler->ClrErr();
			SetSS(SSLI_SWCtl);
		}
		break;
	case SSLI_SWCtl:
		if(StateChanged)
		{//初次进入
			SetSSDelay(2000); 
			if(IsCB())								//检查断路器
			{
				KMCtl(1);							//合接触器
			}
			else
			{
				u16Temp = 0x1;						//预充开关闭合失败
				SetMSW(tLIHandler->StartFailReg,u16Temp);
				SetLSW(tLIHandler->StartFailReg,tLIHandler->StartUpStatus);
				SetSS(SSLI_StartFail);
			}
		}
		if(SSDelayOK())
		{
			u16Temp = 0x0;
			if(!IsKM())
			{
				u16Temp |= 0x2;						//接触器闭合失败
			}
			if(!IsCB())								//再次检查断路器，防止涌流退出
			{
				u16Temp |= 0x1;						//断路器跳闸
			}
			if(u16Temp)
			{
				SetMSW(tLIHandler->StartFailReg,u16Temp);
				SetLSW(tLIHandler->StartFailReg,tLIHandler->StartUpStatus);
				SetSS(SSLI_StartFail);
			}
			else
				SetSS(SSLI_VSCStart);
		}
		break;
	case SSLI_VSCStart:
		if(StateChanged)
		{//初次进入
			SetSSDelay(10000);
			if(tLIHandler->pVSC != 0)										//指针非空
			{
				tLIHandler->pVSC->CtlMode_Ref = IDQCTL;						//电流控制
				tLIHandler->pVSC->StartMode_Ref = DC_PRECHG;				//直流预充
				tLIHandler->pVSC->MainStatus_Ref = START;					//启动VSC
			}
		}
		if(SSDelayOK())
		{
			if(tLIHandler->pVSC->MainStatus != RUN)
			{
				u16Temp = 0;												//VSC启动失败
				SetMSW(tLIHandler->StartFailReg,u16Temp);
				SetLSW(tLIHandler->StartFailReg,tLIHandler->StartUpStatus);
				SetSS(SSLI_StartFail);
			}
			else
			{
					SetSS(SSLI_StartOK);
			}
		}
		break;
	case SSLI_StartOK:
		tLIHandler->gErrMask |= ERRLIx_EN_MASK;
		tLIHandler->MainStatus = RUN;
		break;
	case SSLI_StartFail:
		tLIHandler->gSysErrReg |= ERR_STARTFAIL;
		break;
	default:
		break;
	}
	if(tLIHandler->StartUpDelayCnt)
		tLIHandler->StartUpDelayCnt--;
}
/*************************************************
 Function: LIRunProc
 Description: LI主状态机
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
#define SOC_RAMP_STEP		(0.05f)
void LIRunProc(tLI_CTL* tLIHandler)//选择控制模式
{
	u8 CtlModeChanged=0;
	if(tLIHandler->CtlModePre != tLIHandler->CtlMode)
	{
		CtlModeChanged = 1;
		tLIHandler->CtlModePre = tLIHandler->CtlMode;
	}

	switch(tLIHandler->CtlMode){
	case IDLE_LI:
		tLIHandler->Io_PID.Ref = 0.0f;				//工作于直流模式
		tLIHandler->Io_PID.FeedBack = tLIHandler->Iopu;
		PIDProc(&tLIHandler->Io_PID);
		tLIHandler->pVSC->Id_Cmd = tLIHandler->Io_PID.Out;
		tLIHandler->pVSC->Iq_Cmd = 0.0f;
		tLIHandler->ChgSts = CHG_IDLE_LI;
		tLIHandler->pVSC->CtlMode = IDQCTL;
		break;
	case CH_DISCH_LI:								//充电模式
		if(CtlModeChanged)
		{//初次进入
			tLIHandler->pVSC->Id_Cmd = 0.0f;
			tLIHandler->pVSC->Iq_Cmd = 0.0f;
			tLIHandler->Vo_PID.I = 0.0f;
			tLIHandler->Io_PID.I = 0.0f;
			if(tLIHandler->Vopu < tLIHandler->ChgV_TC2CCpu)		//低于最低电压
				tLIHandler->ChgSts = CHG_TC_LI;
			else if(tLIHandler->Vopu < tLIHandler->ChgV_CVpu)
			{
				tLIHandler->ChgSts = CHG_CC_LI;
				tLIHandler->Io_PID.Ref = 0.0f;
			}
			else
				tLIHandler->ChgSts = CHG_CV_LI;
			tLIHandler->pVSC->CtlMode = IDQCTL;
		}
		switch(tLIHandler->ChgSts){
		case CHG_TC_LI:
			tLIHandler->Io_PID.Ref = tLIHandler->ChgI_TCpu;		//涓流电流
			tLIHandler->Io_PID.FeedBack = tLIHandler->Iopu;
			PIDProc(&tLIHandler->Io_PID);
			tLIHandler->pVSC->Id_Cmd = tLIHandler->Io_PID.Out;
			if(tLIHandler->Vopu >= tLIHandler->ChgV_TC2CCpu)
				tLIHandler->ChgSts = CHG_CC_LI;
			break;
		case CHG_CC_LI:
			//电流缓爬
			if((tLIHandler->Io_PID.Ref + 0.001f) < tLIHandler->ChgI_CCpu)
				tLIHandler->Io_PID.Ref += 0.001f;
			else
				tLIHandler->Io_PID.Ref = tLIHandler->ChgI_CCpu;
			tLIHandler->Io_PID.FeedBack = tLIHandler->Iopu;
			PIDProc(&tLIHandler->Io_PID);
			tLIHandler->pVSC->Id_Cmd = tLIHandler->Io_PID.Out;
			if(tLIHandler->Vopu >= tLIHandler->ChgV_CVpu)
			{
				tLIHandler->ChgSts = CHG_CV_LI;
				//设置电压闭环I
				tLIHandler->Vo_PID.I = tLIHandler->Io_PID.Out/tLIHandler->Vo_PID.Kp;
			}
			break;
		case CHG_CV_LI:
			tLIHandler->Vo_PID.Ref = tLIHandler->ChgV_CVpu;
			tLIHandler->Vo_PID.FeedBack = tLIHandler->Vopu;
			PIDProc(&tLIHandler->Vo_PID);
			tLIHandler->pVSC->Id_Cmd = tLIHandler->Vo_PID.Out;
			if(tLIHandler->Iopu < tLIHandler->ChgI_CV2OKpu)
			{
				tLIHandler->ChgSts = CHG_OK_LI;
			}
			break;
		case CHG_OK_LI:
			tLIHandler->Io_PID.Ref = 0.0f;				//工作于直流模式
			tLIHandler->Io_PID.FeedBack = tLIHandler->Iopu;
			PIDProc(&tLIHandler->Io_PID);
			tLIHandler->pVSC->Id_Cmd = tLIHandler->Io_PID.Out;
			tLIHandler->pVSC->Iq_Cmd = 0.0f;
			break;
		default:
			tLIHandler->pVSC->Id_Cmd = 0.0f;
			tLIHandler->pVSC->Iq_Cmd = 0.0f;
			break;
		}
		if(tLIHandler->ChgNotAllowedFlag)									//禁充触发
		{
			tLIHandler->ChDischAllowedSts |= 0x1;							//禁充标识
			tLIHandler->CtlMode = IDLE_LI;									//强制状态为空闲
			tLIHandler->CtlMode_Ref = IDLE_LI;
		}
		if(!(tLIHandler->DischNotAllowedFlag))								//处理禁放逻辑
			tLIHandler->ChDischAllowedSts = tLIHandler->ChDischAllowedSts & 0xFFFD;		//清除禁放状态
		break;
	case PQ_LI:
		if(CtlModeChanged)
		{//初次进入，默认VSC运行在IDQ控制模式
			tLIHandler->VLimH_PID.I = 0;
			tLIHandler->VLimL_PID.I = 0;
			tLIHandler->pVSC->Id_Cmd = 0.0f;
			tLIHandler->pVSC->Iq_Cmd = 0.0f;
			tLIHandler->pVSC->CtlMode = IDQCTL;
			tLIHandler->ChgSts = CHG_PQ_LI;
			// 构网控制模式初始化，默认工作在VF控制
			tLIHandler->pVSC->GFMCtlMode = VFCTL;
			tLIHandler->pVSC->GFMCtlMode_Pre = 0XFF-1;
		}
		// 此处通过两个PID限制Idref间接限制锂电池的输出电压值，考虑通过一个判断使VSC的其他运行模式也有此限制，其他运行模式下不应该在此赋Idref和Iqref值
		tLIHandler->VLimH_PID.FeedBack = tLIHandler->Vopu;
		tLIHandler->VLimL_PID.FeedBack = tLIHandler->Vopu;
		PIDProc(&tLIHandler->VLimH_PID);
		PIDProc(&tLIHandler->VLimL_PID);
		float IdCmdTemp = 0;
		// 其他运行模式，直接读取Idref值
		if(tLIHandler->pVSC->CtlMode == IDQCTL){
			IdCmdTemp = tLIHandler->P_Ref/tLIHandler->pVSC->UGrid.P2R.d;
		}else{
			IdCmdTemp = tLIHandler->pVSC->Id_Cmd;
		}
		HardLimit(IdCmdTemp,tLIHandler->VLimL_PID.Out,tLIHandler->VLimH_PID.Out);
		if(tLIHandler->ChgNotAllowedFlag)			//禁充触发
			tLIHandler->ChDischAllowedSts |= 0x1;	//禁充标识
		if(tLIHandler->DischNotAllowedFlag)			//禁放触发
			tLIHandler->ChDischAllowedSts |= 0x2;	//禁放禁充
		if(tLIHandler->ChDischAllowedSts & 0x1)		//处于禁充状态
		{
			if(IdCmdTemp > 0)
				IdCmdTemp = 0;
			else if(!(tLIHandler->ChgNotAllowedFlag))
				tLIHandler->ChDischAllowedSts = tLIHandler->ChDischAllowedSts & 0xFFFE;		//清除禁充状态
		}
		if(tLIHandler->ChDischAllowedSts & 0x2)		//处于禁放状态
		{
			if(IdCmdTemp < 0)
				IdCmdTemp = 0;
			else if(!(tLIHandler->DischNotAllowedFlag))
				tLIHandler->ChDischAllowedSts = tLIHandler->ChDischAllowedSts & 0xFFFD;		//清除禁放状态
		}
		tLIHandler->pVSC->Id_Cmd = IdCmdTemp;
		// Iqref同样执行不同运行模式的判断
		if(tLIHandler->pVSC->CtlMode == IDQCTL){
			tLIHandler->pVSC->Iq_Cmd = -tLIHandler->Q_Ref/tLIHandler->pVSC->UGrid.P2R.d;
		}else{
			tLIHandler->pVSC->Iq_Cmd = tLIHandler->pVSC->Iq_Cmd; // 不变
		}
		break;
	default:
		tLIHandler->CtlMode = IDLE_LI;
		break;
	}
}
/*************************************************
 Function: LIStopMachine
 Description: LI主状态机
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void LIStopMachine(tLI_CTL* tLIHandler)
{
	tLIHandler->pVSC->MainStatus_Ref = STOP;
	tLIHandler->ChgSts = CHG_IDLE_LI;
	tLIHandler->CtlMode = IDLE_LI;
	if((tLIHandler->pVSC->MainStatus == IDLE)||(tLIHandler->pVSC->MainStatus == _FAULT))
	{
		tLIHandler->MainStatus = IDLE;
	}
}

/*************************************************
 Function: LIStopMachine
 Description: LI主状态机
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void LILocalParasUpdate(tLI_CTL* tLIHandler)		//本地数据更新
{
	u8 i = 10;
	LIBMSCom(tLIHandler);
	while((!CANBusPoll(&tLIHandler->tCANCtl)) && (i--))		//CANbus轮询任务,，单次最多处理10帧，根据BMS返回数据量进行微调

	tLIHandler->ErrStatus = tLIHandler->gSysErrReg;
	tLIHandler->Vo = tLIHandler->pVSC->Vdc;
	tLIHandler->Io = tLIHandler->pVSC->Idc;
	tLIHandler->Vopu = tLIHandler->Vo*tLIHandler->Vb_Reci;
	tLIHandler->Iopu = tLIHandler->Io*tLIHandler->Ib_Reci;
	if(tLIHandler->BMS_ComOK)
		tLIHandler->SOC = tLIHandler->tStackInfo.SOC;
	else
		tLIHandler->SOC = 0;

	if(tLIHandler->MainStatus_Ref == START)
	{
		if(tLIHandler->MainStatus == IDLE && (tLIHandler->gErrClr == 0))			//只有在IDLE状态才能切换进入启动
		{
			tLIHandler->MainStatus = START;
			tLIHandler->CtlMode = tLIHandler->CtlMode_Ref;
		}
	}
	else if(tLIHandler->MainStatus_Ref == STOP)
	{
		if(tLIHandler->MainStatus == START || tLIHandler->MainStatus == RUN)		//非错误状态下可以进入停机状态
			tLIHandler->MainStatus = STOP;
	}
}

/*************************************************
 Function: LIBMSCom
 Description: LI BMS 通信
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void LIBMSCom(tLI_CTL* tLIHandler)						//处理发送数据
{
	u16 LocalCANCtlCnt;
	LocalCANCtlCnt = tLIHandler->CANCtlCnt;
	if(LocalCANCtlCnt < 2000) LocalCANCtlCnt++;
	else
		LocalCANCtlCnt = 0;

	if(LocalCANCtlCnt == 0)
	{
	}
	else if(LocalCANCtlCnt == 300)
	{
	}
	else if(LocalCANCtlCnt == 600)
	{
	}
	else if(LocalCANCtlCnt == 1000)
	{
	}
	tLIHandler->CANCtlCnt = LocalCANCtlCnt;
}

/*************************************************
 Function: LIBMSDatRcv
 Description: LI BMS 接收数据处理
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void LIBMSDatRcv(void(*tLIHandlerVoid))						//本地数据更新
{
	tLI_CTL* tLIHandler = (tLI_CTL*)tLIHandlerVoid;
	u16 u16Temp;
	u32 RcvID = tLIHandler->tCANCtl.CanFrameRx.id;
	switch(RcvID){
	case 0x18102701:
		tLIHandler->tStackInfo.HeartBeatPre = tLIHandler->tStackInfo.HeartBeat;
		tLIHandler->tStackInfo.SOC = 0.01f*tLIHandler->tCANCtl.CanFrameRx.data[0];
		tLIHandler->tStackInfo.SOH = tLIHandler->tCANCtl.CanFrameRx.data[1];
		tLIHandler->tStackInfo.HeartBeat = tLIHandler->tCANCtl.CanFrameRx.data[6];
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[2];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[3];
		tLIHandler->tStackInfo.StackV = 0.1f*u16Temp;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[4];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[5];
		tLIHandler->tStackInfo.StackI = 0.1f*u16Temp - 3200;
		tLIHandler->CANOTCnt = 500;
		break;
	case 0x18112701:
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[0];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[1];
		tLIHandler->tStackInfo.CellVMax = 0.001f*u16Temp;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[2];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[3];
		tLIHandler->tStackInfo.CellVMaxNO = u16Temp+1;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[4];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[5];
		tLIHandler->tStackInfo.CellVMin = 0.001f*u16Temp;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[6];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[7];
		tLIHandler->tStackInfo.CellVMinNO = u16Temp+1;
		break;
	case 0x18122701:
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[0];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[1];
		tLIHandler->tStackInfo.CellTMax = (s16)u16Temp-40;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[2];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[3];
		tLIHandler->tStackInfo.CellTMaxNO = u16Temp+1;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[4];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[5];
		tLIHandler->tStackInfo.CellTMin = (s16)u16Temp-40;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[6];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[7];
		tLIHandler->tStackInfo.CellTMinNO = u16Temp+1;
		break;
	case 0x18132701:
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[0];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[1];
		tLIHandler->tStackInfo.StringNum = u16Temp;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[2];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[3];
		tLIHandler->tStackInfo.RatedV = 0.1f*u16Temp;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[4];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[5];
		tLIHandler->tStackInfo.RatedAh = 0.1f*u16Temp;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[6];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[7];
		tLIHandler->tStackInfo.CycleNum = u16Temp;
		break;
	case 0x18142701:
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[0];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[1];
		tLIHandler->tStackInfo.MaxChgI = 0.1f*u16Temp;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[2];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[3];
		tLIHandler->tStackInfo.MaxDischI = 0.1f*u16Temp;
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[4];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[5];
		tLIHandler->tStackInfo.MaxChgP = 100.0f*u16Temp;			//W
		((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[6];
		((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[7];
		tLIHandler->tStackInfo.MaxDischP = 100.0f*u16Temp;			//W
		break;
	case 0x18152701:
		tLIHandler->tStackInfo.ChgAllow = tLIHandler->tCANCtl.CanFrameRx.data[0];
		tLIHandler->tStackInfo.DischAllow = tLIHandler->tCANCtl.CanFrameRx.data[1];
		tLIHandler->tStackInfo.ChgErrStage = tLIHandler->tCANCtl.CanFrameRx.data[2];
		tLIHandler->tStackInfo.ChgErrCode = tLIHandler->tCANCtl.CanFrameRx.data[3];
		tLIHandler->tStackInfo.DischErrStage = tLIHandler->tCANCtl.CanFrameRx.data[4];
		tLIHandler->tStackInfo.DischErrCode = tLIHandler->tCANCtl.CanFrameRx.data[5];
		tLIHandler->tStackInfo.ChgStatus = tLIHandler->tCANCtl.CanFrameRx.data[6];			//0：静置，01：充电；02：放电
		tLIHandler->tStackInfo.OtherErrCode = tLIHandler->tCANCtl.CanFrameRx.data[7];
		tLIHandler->ChgNotAllowedFlag = !tLIHandler->tStackInfo.ChgAllow;
		tLIHandler->DischNotAllowedFlag = !tLIHandler->tStackInfo.DischAllow;
		break;
	default:
		if((RcvID & 0xFFFF) == 0x2701)
		{
			u16Temp = (tLIHandler->tCANCtl.CanFrameRx.id >> 16);
			if((u16Temp >= 0x1830) && (u16Temp <= 0x187F))
			{
				u16 IndexBase = (u16Temp - 0x1830) << 2;
				if(IndexBase < (CELL_V_INDEX_NUM-1-3))			//超出缓存区间
				{
					((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[0];
					((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[1];
					tLIHandler->tStackInfo.CELLV[IndexBase] = 0.001f*u16Temp;
					((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[2];
					((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[3];
					tLIHandler->tStackInfo.CELLV[IndexBase+1] = 0.001f*u16Temp;
					((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[4];
					((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[5];
					tLIHandler->tStackInfo.CELLV[IndexBase+2] = 0.001f*u16Temp;
					((u8*)(&u16Temp))[1] = tLIHandler->tCANCtl.CanFrameRx.data[6];
					((u8*)(&u16Temp))[0] = tLIHandler->tCANCtl.CanFrameRx.data[7];
					tLIHandler->tStackInfo.CELLV[IndexBase+3] = 0.001f*u16Temp;
				}
			}
			else if((u16Temp >= 0x1880) && (u16Temp <= 0x189F))
			{
				u16 IndexBase = (u16Temp - 0x1880) << 3;
				if(IndexBase < (CELL_T_INDEX_NUM-1-7))			//超出缓存区间
				{
					tLIHandler->tStackInfo.CELLT[IndexBase] = (s16)tLIHandler->tCANCtl.CanFrameRx.data[0]-40;
					tLIHandler->tStackInfo.CELLT[IndexBase+1] = (s16)tLIHandler->tCANCtl.CanFrameRx.data[1]-40;
					tLIHandler->tStackInfo.CELLT[IndexBase+2] = (s16)tLIHandler->tCANCtl.CanFrameRx.data[2]-40;
					tLIHandler->tStackInfo.CELLT[IndexBase+3] = (s16)tLIHandler->tCANCtl.CanFrameRx.data[3]-40;
					tLIHandler->tStackInfo.CELLT[IndexBase+4] = (s16)tLIHandler->tCANCtl.CanFrameRx.data[4]-40;
					tLIHandler->tStackInfo.CELLT[IndexBase+5] = (s16)tLIHandler->tCANCtl.CanFrameRx.data[5]-40;
					tLIHandler->tStackInfo.CELLT[IndexBase+6] = (s16)tLIHandler->tCANCtl.CanFrameRx.data[6]-40;
					tLIHandler->tStackInfo.CELLT[IndexBase+7] = (s16)tLIHandler->tCANCtl.CanFrameRx.data[7]-40;
				}
			}
		}
		break;
	}
}

/*************************************************
 Function: LIStopMachine
 Description: LI主状态机
 Input: 无
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void LIFaultDet(tLI_CTL* tLIHandler)
{
	u16 Status = tLIHandler->MainStatus;
	if((Status == RUN) || (Status == START))
	{
		LimitProc(&tLIHandler->Vo_Limit,tLIHandler->Vopu,Status);
		LimitProc(&tLIHandler->Io_Limit,tLIHandler->Iopu,Status);
	}
	//输出过压欠压
	if((tLIHandler->Vo_Limit.Result == LIMIT_STH)||tLIHandler->Vo_Limit.Result == LIMIT_RUNH||tLIHandler->Vo_Limit.Result == LIMIT_RUNL)
		tLIHandler->gSysErrReg |= ERRLI_OV_SW;
	//输出过流
	if(tLIHandler->Io_Limit.Result)
		tLIHandler->gSysErrReg |= ERRLI_OC_SW;
	//BMS干接点
	if(tLIHandler->BMSDrySts())
		tLIHandler->gSysErrReg |= ERRLI_BMS_HW;
	//急停开关
	if(tLIHandler->EmSwSts())
		tLIHandler->gSysErrReg |= ERRLI_EMER_HW;
	//VSC故障
	if(tLIHandler->pVSC->MainStatus == _FAULT)
		tLIHandler->gSysErrReg |= ERRLI_VSC_SW;
	//BMS通信故障，包括BMS对RSP
	if(tLIHandler->CANOTCnt)
	{
		tLIHandler->CANOTCnt--;
		tLIHandler->BMS_ComOK = 1;
	}
	else
		tLIHandler->BMS_ComOK = 0;
	if(!(tLIHandler->BMS_ComOK))			//通信丢失
		tLIHandler->gSysErrReg |= ERRLI_BMS_COM;
	//BMS测量告警
	if((tLIHandler->tStackInfo.ChgErrStage == 3) || (tLIHandler->tStackInfo.DischErrStage == 3))			//BMS三级故障，需停机
	{
		tLIHandler->gSysErrReg |= ERRLI_BMS_ALARM;
		tLIHandler->BMSAlarmLatched[0] = tLIHandler->tStackInfo.ChgErrCode;
		tLIHandler->BMSAlarmLatched[1] = tLIHandler->tStackInfo.DischErrCode;
	}


	if((tLIHandler->gSysErrReg) & ((tLIHandler->gErrMask)|ERRMSK_ALWAYS))		//故障信号在掩码范围内(急停、板连接总是有效)
	{
		if(!(tLIHandler->gSysErrFlag))
		{
			tLIHandler->gSysErrFlag = 1;
			tLIHandler->gErrTriped = tLIHandler->gSysErrReg;
		}
		tLIHandler->MainStatus = _FAULT;		//进入故障状态
	}
	if(tLIHandler->gErrClr)
	{
		tLIHandler->gErrClr--;
		tLIHandler->gSysErrReg = 0;
		tLIHandler->gSysErrFlag = 0;
		tLIHandler->gErrTriped = 0;
		tLIHandler->pVSC->ClrErr();
		tLIHandler->BMSAlarmLatched[0]  = 0;
		tLIHandler->BMSAlarmLatched[1]  = 0;
		tLIHandler->ChDischAllowedSts = 0;
		LimitStatusReset(&tLIHandler->Vo_Limit,LIMIT_OK);
		LimitStatusReset(&tLIHandler->Io_Limit,LIMIT_OK);
	}
}


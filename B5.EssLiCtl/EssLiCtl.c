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

//ֱ������״̬��״̬ö��
typedef enum {
	SSLI_Prepare = 0,			/*׼��״̬*/
	SSLI_StParaInit,			/*����������ʼ��*/
	SSLI_SWCtl,					/*ϵͳ���ؿ���*/
	SSLI_VSCStart,				/*����VSC*/
	SSLI_RSVD1,					/*DCDCԤ��*/
	SSLI_StartOK,				/*�����ɹ�*/
	SSLI_StartFail				/*����ʧ��*/
} LISTART_STEPS;

#define SetSSDelay(A)	(tLIHandler->StartUpDelayCnt = (A))
#define SSDelayOK()		(!tLIHandler->StartUpDelayCnt)
#define SetSS(A)		(tLIHandler->StartUpStatus = (A))

float PLimTempLI;

/*************************************************
 Function: LIMainFSM
 Description: LI��״̬��
 Input: LI���
 Output: ��
 Return: ��
 Others: //�ֱ����
 *************************************************/
void LIMainFSM(tLI_CTL* tLIHandler)
{
//��״̬�������ڿ���ϵͳ������״̬��ת
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
		tLIHandler->MainStatus_Ref = IDLE;									//��ֹ�����������������
		break;
	default:
		tLIHandler->StartUpStatus = 0;
		tLIHandler->MainStatus = IDLE;
		break;
	}
	if(tLIHandler->MainStatusPre != tLIHandler->MainStatus)
	{
		tLIHandler->MainStatusPre = tLIHandler->MainStatus;
		tLIHandler->StartUpStatus = 0;										//״̬�л�ʱ��tLIHandler->StartUpStatus����
	}
	LILocalParasUpdate(tLIHandler);
	LIFaultDet(tLIHandler);
}

/*************************************************
 Function: LIInit
 Description: LI��ʼ��
 Input: ��
 Output: ��
 Return: ��
 Others: //�ֱ����
 *************************************************/
void LIInit(tLI_CTL* tLIHandler)
{
	tLIHandler->Init();
	tLIHandler->tCANCtl.RcvCallback = LIBMSDatRcv;
	CANInit(&tLIHandler->tCANCtl);
	tLIHandler->CANOTCnt = 500;
	tLIHandler->BMS_ComOK = 1;
	LIParasInit(tLIHandler); // ��ʼ��������
}
/*************************************************
 Function: LIParasInit
 Description: LI��״̬��
 Input: ��
 Output: ��
 Return: ��
 Others: //�ֱ����
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

	tLIHandler->ChgNotAllowedFlag = 0;				//��־����
	tLIHandler->DischNotAllowedFlag = 0;
	tLIHandler->ChDischAllowedSts = 0;
}

/*************************************************
 Function: LIMainFSM_Init
 Description:
 Input: ��
 Output: ��
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
 Description: LI����
 Input: ��
 Output: ��
 Return: ��
 Others: //�ֱ����
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
	case SSLI_Prepare:								//׼��״̬
		tLIHandler->StartUpStatusPre = SSLI_Prepare;
		tLIHandler->StartFailReg = 0;
		SetSS(SSLI_StParaInit);
		break;
	case SSLI_StParaInit:
		if(StateChanged)
		{//���ν���
			SetSSDelay(1000);
		}
		if(SSDelayOK())
		{
			LIParasInit(tLIHandler);
			tLIHandler->gErrMask |= (ERRLI_STARTFAIL);					//��������ʧ�ܱ���
			tLIHandler->ClrErr();
			SetSS(SSLI_SWCtl);
		}
		break;
	case SSLI_SWCtl:
		if(StateChanged)
		{//���ν���
			SetSSDelay(2000); 
			if(IsCB())								//����·��
			{
				KMCtl(1);							//�ϽӴ���
			}
			else
			{
				u16Temp = 0x1;						//Ԥ�俪�رպ�ʧ��
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
				u16Temp |= 0x2;						//�Ӵ����պ�ʧ��
			}
			if(!IsCB())								//�ٴμ���·������ֹӿ���˳�
			{
				u16Temp |= 0x1;						//��·����բ
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
		{//���ν���
			SetSSDelay(10000);
			if(tLIHandler->pVSC != 0)										//ָ��ǿ�
			{
				tLIHandler->pVSC->CtlMode_Ref = IDQCTL;						//��������
				tLIHandler->pVSC->StartMode_Ref = DC_PRECHG;				//ֱ��Ԥ��
				tLIHandler->pVSC->MainStatus_Ref = START;					//����VSC
			}
		}
		if(SSDelayOK())
		{
			if(tLIHandler->pVSC->MainStatus != RUN)
			{
				u16Temp = 0;												//VSC����ʧ��
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
 Description: LI��״̬��
 Input: ��
 Output: ��
 Return: ��
 Others: //�ֱ����
 *************************************************/
#define SOC_RAMP_STEP		(0.05f)
void LIRunProc(tLI_CTL* tLIHandler)//ѡ�����ģʽ
{
	u8 CtlModeChanged=0;
	if(tLIHandler->CtlModePre != tLIHandler->CtlMode)
	{
		CtlModeChanged = 1;
		tLIHandler->CtlModePre = tLIHandler->CtlMode;
	}

	switch(tLIHandler->CtlMode){
	case IDLE_LI:
		tLIHandler->Io_PID.Ref = 0.0f;				//������ֱ��ģʽ
		tLIHandler->Io_PID.FeedBack = tLIHandler->Iopu;
		PIDProc(&tLIHandler->Io_PID);
		tLIHandler->pVSC->Id_Cmd = tLIHandler->Io_PID.Out;
		tLIHandler->pVSC->Iq_Cmd = 0.0f;
		tLIHandler->ChgSts = CHG_IDLE_LI;
		tLIHandler->pVSC->CtlMode = IDQCTL;
		break;
	case CH_DISCH_LI:								//���ģʽ
		if(CtlModeChanged)
		{//���ν���
			tLIHandler->pVSC->Id_Cmd = 0.0f;
			tLIHandler->pVSC->Iq_Cmd = 0.0f;
			tLIHandler->Vo_PID.I = 0.0f;
			tLIHandler->Io_PID.I = 0.0f;
			if(tLIHandler->Vopu < tLIHandler->ChgV_TC2CCpu)		//������͵�ѹ
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
			tLIHandler->Io_PID.Ref = tLIHandler->ChgI_TCpu;		//�������
			tLIHandler->Io_PID.FeedBack = tLIHandler->Iopu;
			PIDProc(&tLIHandler->Io_PID);
			tLIHandler->pVSC->Id_Cmd = tLIHandler->Io_PID.Out;
			if(tLIHandler->Vopu >= tLIHandler->ChgV_TC2CCpu)
				tLIHandler->ChgSts = CHG_CC_LI;
			break;
		case CHG_CC_LI:
			//��������
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
				//���õ�ѹ�ջ�I
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
			tLIHandler->Io_PID.Ref = 0.0f;				//������ֱ��ģʽ
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
		if(tLIHandler->ChgNotAllowedFlag)									//���䴥��
		{
			tLIHandler->ChDischAllowedSts |= 0x1;							//�����ʶ
			tLIHandler->CtlMode = IDLE_LI;									//ǿ��״̬Ϊ����
			tLIHandler->CtlMode_Ref = IDLE_LI;
		}
		if(!(tLIHandler->DischNotAllowedFlag))								//��������߼�
			tLIHandler->ChDischAllowedSts = tLIHandler->ChDischAllowedSts & 0xFFFD;		//�������״̬
		break;
	case PQ_LI:
		if(CtlModeChanged)
		{//���ν��룬Ĭ��VSC������IDQ����ģʽ
			tLIHandler->VLimH_PID.I = 0;
			tLIHandler->VLimL_PID.I = 0;
			tLIHandler->pVSC->Id_Cmd = 0.0f;
			tLIHandler->pVSC->Iq_Cmd = 0.0f;
			tLIHandler->pVSC->CtlMode = IDQCTL;
			tLIHandler->ChgSts = CHG_PQ_LI;
			// ��������ģʽ��ʼ����Ĭ�Ϲ�����VF����
			tLIHandler->pVSC->GFMCtlMode = VFCTL;
			tLIHandler->pVSC->GFMCtlMode_Pre = 0XFF-1;
		}
		// �˴�ͨ������PID����Idref�������﮵�ص������ѹֵ������ͨ��һ���ж�ʹVSC����������ģʽҲ�д����ƣ���������ģʽ�²�Ӧ���ڴ˸�Idref��Iqrefֵ
		tLIHandler->VLimH_PID.FeedBack = tLIHandler->Vopu;
		tLIHandler->VLimL_PID.FeedBack = tLIHandler->Vopu;
		PIDProc(&tLIHandler->VLimH_PID);
		PIDProc(&tLIHandler->VLimL_PID);
		float IdCmdTemp = 0;
		// ��������ģʽ��ֱ�Ӷ�ȡIdrefֵ
		if(tLIHandler->pVSC->CtlMode == IDQCTL){
			IdCmdTemp = tLIHandler->P_Ref/tLIHandler->pVSC->UGrid.P2R.d;
		}else{
			IdCmdTemp = tLIHandler->pVSC->Id_Cmd;
		}
		HardLimit(IdCmdTemp,tLIHandler->VLimL_PID.Out,tLIHandler->VLimH_PID.Out);
		if(tLIHandler->ChgNotAllowedFlag)			//���䴥��
			tLIHandler->ChDischAllowedSts |= 0x1;	//�����ʶ
		if(tLIHandler->DischNotAllowedFlag)			//���Ŵ���
			tLIHandler->ChDischAllowedSts |= 0x2;	//���Ž���
		if(tLIHandler->ChDischAllowedSts & 0x1)		//���ڽ���״̬
		{
			if(IdCmdTemp > 0)
				IdCmdTemp = 0;
			else if(!(tLIHandler->ChgNotAllowedFlag))
				tLIHandler->ChDischAllowedSts = tLIHandler->ChDischAllowedSts & 0xFFFE;		//�������״̬
		}
		if(tLIHandler->ChDischAllowedSts & 0x2)		//���ڽ���״̬
		{
			if(IdCmdTemp < 0)
				IdCmdTemp = 0;
			else if(!(tLIHandler->DischNotAllowedFlag))
				tLIHandler->ChDischAllowedSts = tLIHandler->ChDischAllowedSts & 0xFFFD;		//�������״̬
		}
		tLIHandler->pVSC->Id_Cmd = IdCmdTemp;
		// Iqrefͬ��ִ�в�ͬ����ģʽ���ж�
		if(tLIHandler->pVSC->CtlMode == IDQCTL){
			tLIHandler->pVSC->Iq_Cmd = -tLIHandler->Q_Ref/tLIHandler->pVSC->UGrid.P2R.d;
		}else{
			tLIHandler->pVSC->Iq_Cmd = tLIHandler->pVSC->Iq_Cmd; // ����
		}
		break;
	default:
		tLIHandler->CtlMode = IDLE_LI;
		break;
	}
}
/*************************************************
 Function: LIStopMachine
 Description: LI��״̬��
 Input: ��
 Output: ��
 Return: ��
 Others: //�ֱ����
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
 Description: LI��״̬��
 Input: ��
 Output: ��
 Return: ��
 Others: //�ֱ����
 *************************************************/
void LILocalParasUpdate(tLI_CTL* tLIHandler)		//�������ݸ���
{
	u8 i = 10;
	LIBMSCom(tLIHandler);
	while((!CANBusPoll(&tLIHandler->tCANCtl)) && (i--))		//CANbus��ѯ����,��������ദ��10֡������BMS��������������΢��

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
		if(tLIHandler->MainStatus == IDLE && (tLIHandler->gErrClr == 0))			//ֻ����IDLE״̬�����л���������
		{
			tLIHandler->MainStatus = START;
			tLIHandler->CtlMode = tLIHandler->CtlMode_Ref;
		}
	}
	else if(tLIHandler->MainStatus_Ref == STOP)
	{
		if(tLIHandler->MainStatus == START || tLIHandler->MainStatus == RUN)		//�Ǵ���״̬�¿��Խ���ͣ��״̬
			tLIHandler->MainStatus = STOP;
	}
}

/*************************************************
 Function: LIBMSCom
 Description: LI BMS ͨ��
 Input: ��
 Output: ��
 Return: ��
 Others: //�ֱ����
 *************************************************/
void LIBMSCom(tLI_CTL* tLIHandler)						//����������
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
 Description: LI BMS �������ݴ���
 Input: ��
 Output: ��
 Return: ��
 Others: //�ֱ����
 *************************************************/
void LIBMSDatRcv(void(*tLIHandlerVoid))						//�������ݸ���
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
		tLIHandler->tStackInfo.ChgStatus = tLIHandler->tCANCtl.CanFrameRx.data[6];			//0�����ã�01����磻02���ŵ�
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
				if(IndexBase < (CELL_V_INDEX_NUM-1-3))			//������������
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
				if(IndexBase < (CELL_T_INDEX_NUM-1-7))			//������������
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
 Description: LI��״̬��
 Input: ��
 Output: ��
 Return: ��
 Others: //�ֱ����
 *************************************************/
void LIFaultDet(tLI_CTL* tLIHandler)
{
	u16 Status = tLIHandler->MainStatus;
	if((Status == RUN) || (Status == START))
	{
		LimitProc(&tLIHandler->Vo_Limit,tLIHandler->Vopu,Status);
		LimitProc(&tLIHandler->Io_Limit,tLIHandler->Iopu,Status);
	}
	//�����ѹǷѹ
	if((tLIHandler->Vo_Limit.Result == LIMIT_STH)||tLIHandler->Vo_Limit.Result == LIMIT_RUNH||tLIHandler->Vo_Limit.Result == LIMIT_RUNL)
		tLIHandler->gSysErrReg |= ERRLI_OV_SW;
	//�������
	if(tLIHandler->Io_Limit.Result)
		tLIHandler->gSysErrReg |= ERRLI_OC_SW;
	//BMS�ɽӵ�
	if(tLIHandler->BMSDrySts())
		tLIHandler->gSysErrReg |= ERRLI_BMS_HW;
	//��ͣ����
	if(tLIHandler->EmSwSts())
		tLIHandler->gSysErrReg |= ERRLI_EMER_HW;
	//VSC����
	if(tLIHandler->pVSC->MainStatus == _FAULT)
		tLIHandler->gSysErrReg |= ERRLI_VSC_SW;
	//BMSͨ�Ź��ϣ�����BMS��RSP
	if(tLIHandler->CANOTCnt)
	{
		tLIHandler->CANOTCnt--;
		tLIHandler->BMS_ComOK = 1;
	}
	else
		tLIHandler->BMS_ComOK = 0;
	if(!(tLIHandler->BMS_ComOK))			//ͨ�Ŷ�ʧ
		tLIHandler->gSysErrReg |= ERRLI_BMS_COM;
	//BMS�����澯
	if((tLIHandler->tStackInfo.ChgErrStage == 3) || (tLIHandler->tStackInfo.DischErrStage == 3))			//BMS�������ϣ���ͣ��
	{
		tLIHandler->gSysErrReg |= ERRLI_BMS_ALARM;
		tLIHandler->BMSAlarmLatched[0] = tLIHandler->tStackInfo.ChgErrCode;
		tLIHandler->BMSAlarmLatched[1] = tLIHandler->tStackInfo.DischErrCode;
	}


	if((tLIHandler->gSysErrReg) & ((tLIHandler->gErrMask)|ERRMSK_ALWAYS))		//�����ź������뷶Χ��(��ͣ��������������Ч)
	{
		if(!(tLIHandler->gSysErrFlag))
		{
			tLIHandler->gSysErrFlag = 1;
			tLIHandler->gErrTriped = tLIHandler->gSysErrReg;
		}
		tLIHandler->MainStatus = _FAULT;		//�������״̬
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


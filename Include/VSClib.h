/**
  ******************************************************************************
  * @file    VSCLib.h
  * @author  RZ
  * @brief   Header for VSC Control
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
#ifndef _VSCLIB_H_
#define _VSCLIB_H_

//ϵͳ����״̬ö�ٶ��壬���С����������С�ͣ������·������
#ifndef SYSSTATUS_DEFINED
#define SYSSTATUS_DEFINED
typedef enum {
	IDLE = 0, START = 1, RUN = 2, STOP = 3, MAINTANCE = 5,_FAULT = 0xFF
} SYSSTATUS;
#endif

//ö�ٶ���
typedef enum{PQCTL = 0,VQCTL = 1,IDQCTL = 2,VACCTL = 3} CTLMODE;
typedef enum{DC_PRECHG = 0,AC_PRECHG = 1} STARTMODE;
typedef enum{VFCTL = 0,DROOPCTL = 1,VSGCTL = 2,UniversalGFM=3} GFMMODE;

//�ű۵����ṹ�嶨��
typedef struct {
	float AH;
	float BH;
	float CH;
	float AL;
	float BL;
	float CL;
} S_IARM;

//�ű۵����ṹ�嶨��
typedef struct {
	float RMS_Va;
	float RMS_Vb;
	float RMS_Vc;
	float RMS_Ia;
	float RMS_Ib;
	float RMS_Ic;
} RMSInfo_t;

typedef struct
{
//���лض�ֵ����������
	float Va,Vb,Vc;						//ʵ��ֵ
	float Ia,Ib,Ic;						//ʵ��ֵ
	float P,Q,PF,f;						//ʵ��ֵ
	float Vdc,Idc,Pdc;					//ʵ��ֵ
	float Id,Iq;						//����ֵ
	float PSet,QSet,VdcSet;				//ʵ��ֵ
	SYSSTATUS MainStatus_Ref;			//RUN IDLE ...etc
	SYSSTATUS MainStatus;				//RUN IDLE ...etc
	CTLMODE CtlMode_Ref;
	CTLMODE CtlMode;					//PQ/VQ/Id&Iq Control Mode
	// ��ӹ����Ͳ���״̬��־λ
	u16 ISLANDED; // ��־λ 1 -> ���� --- 0 -> ����
	// ��ӹ�������ģʽ
	GFMMODE GFMCtlMode;
	GFMMODE GFMCtlMode_Pre;
	STARTMODE StartMode_Ref;
	STARTMODE StartMode;
	u16 StartUpStatus;
	u16 ErrStatus;						//Is Error Occurred
	u16 DI;
	u16 DO;
	u32 Protect[4];
	u32 FrameNum;

//ϵͳ�����ڲ����б���
	PARA3S2S2R UGrid,UConv,IGrid;					//������ѹ
	PARA3S2S2R Iconv,Ucap;
	float Omega,deltaOmega,Vmag,deltaVmag,Theta,PLLFre;
	SINCOS_t Theta_GridSincos;
	float ThetaPhase;
	SINCOS_t ThetaPhase_GridSincos;
	float DCV_Bus,DCI_Bus;							//ֱ������ֵ
	float P_Ref,Q_Ref,Vdc_Ref,Vac_Ref;
	float Id_Ref;									//ʵ��ʹ�õ��ڲ��ο�
	float Iq_Ref;
	float Idc_Ref;
	PID ThetaPID,IdPID,IqPID,VBusPID,P_PID,Q_PID,Vc_PID,IBusPID,Id2PID,Iq2PID;
	// ���ӽ�����ѹDQ��PID
	PID Vd_PID,Vq_PID;
	float mdc_RefOutLoop;
	float mdc;
	s16 PWM_A,PWM_B,PWM_C;
	u16 gErrMask,gSysErrReg,gSysErrFlag;
	u16 gErrClr,gEnHBOut,gErrTriped;
	float PQSlopStep,VdcSlopStep,VacSlopStep;
//�����м����
	LPF AC_VARMS_G,AC_VBRMS_G,AC_VCRMS_G;           //������ѹ��Чֵ
	LPF AC_IARMS_G,AC_IBRMS_G,AC_ICRMS_G;           //����������Чֵ
	LPF DC_PWM_A,DC_PWM_B,DC_PWM_C;
	float P_AC_AVG,Q_AC_AVG,P_DC_AVG,S_AC_AVG;
	float P_AC,Q_AC,P_DC;
	RMSInfo_t RMSInfo;
	float P_Cmd,Q_Cmd,Vdc_Cmd,Id_Cmd,Iq_Cmd,Vac_Cmd,Fre_Cmd;		//���յ���ָ����ڲ��ο�����Ҫʹ��Slope
	PARA3S2S2R I2arm,Uccsc;
	S_IARM sIarm;
	s16 *ADC_Va,*ADC_Vb,*ADC_Vc,*ADC_Ia,*ADC_Ib,*ADC_Ic,*ADC_Vdc,*ADC_Idc;

	LIMIT ACVINA_Limit;								//��ֵ�жϱ���
	LIMIT ACVINB_Limit;
	LIMIT ACVINC_Limit;
	LIMIT DCVPreCharge_Limit;
	LIMIT FrePLL_Limit;

	const PID THETA_PID_INIT;
	const PID PQ_PID_INIT;
	const PID IDQ_PID_INIT;
	const PID VBUS_PID_INIT;
	const PID VC_PID_INIT;
	const PID IBUS_PID_INIT;
	const PID IDQ2_PID_INIT;

	const LPF AC_RMSREF;
	const LPF DC_PWMREF;

	const LIMIT ACVIN_LIMITREF;
	const LIMIT DCVPRECHG_LIMITREF;
	const LIMIT FREPLL_LIMITREF;
//ģ����ƼĴ���
	S_OPTCARD_CMD *pHBCmd_A,*pHBCmd_B,*pHBCmd_C,*pHBCmd_N;
	S_OPTCARD_DAT *pHBDat_A,*pHBDat_B,*pHBDat_C,*pHBDat_N;
	S_ROPCARD *pRop;

//���������߼���ظ�������
	SYSSTATUS MainStatusPre;					//����״̬�߼�����
	u16 StartUpStatusPre;						//����״̬�߼�����
	u32 StartFailReg;							//��16λ����ʧ����Ϣ����16λ����ָʾʧ�ܻ���
	u16 StartUpDelayCnt;						//�������������е��ӳٿ���
	u16 StartUpCheckEn;
	u16 StartCheckFlag;
	u16 RelayDelayCnt;
	u16 OutLoopCnt;
//���ز����뷵��
	void (*AcBrkCtl)(u16);						//������·������
	u16 (*AcBrkSts)(void);						//������·��״̬����
	void (*AcContCtl)(u16);						//�����Ӵ�������
	u16 (*AcContSts)(void);						//�����Ӵ���״̬����
	void (*AcPreChCtl)(u16);					//����Ԥ�����
	u16 (*AcPreChSts)(void);					//����Ԥ��״̬����
	void (*DcContCtl)(u16);						//ֱ���Ӵ�������
	u16 (*DcContSts)(void);						//ֱ���Ӵ���״̬����
	void (*DcPreChCtl)(u16);					//ֱ��Ԥ�����
	u16 (*DcPreChSts)(void);					//ֱ��Ԥ��״̬����
	u16 (*HBOK)(void);							//����ģ��״̬����
	u16 (*EmSwSts)(void);						//ROP��ͣ����
	u16 (*RstSwSts)(void);						//ROP���ϸ�λ����
	void (*CFanCtl)(u16);						//���ȿ���
	void (*CFaultLedCtl)(u16);					//ֱ���Ӵ�������
	void (*ClrErr)(void);						//�������
//����������
	u16 Flag3P4W;
//��ʼ������
	void (*Init)(void);							//��ʼ�����ƺ���

}tVSC_CTL;

extern void VSCParasInit(tVSC_CTL*);
extern void VSCFastTask(tVSC_CTL*);
extern void VSCMidTask(tVSC_CTL*);
extern void VSCInit(tVSC_CTL*);
extern void svgen(tVSC_CTL*);
extern void VSCFaultDet(tVSC_CTL*);
extern void VSCSysCtl(tVSC_CTL*);
extern void VSCControlLoop(tVSC_CTL*);
extern void VSCAnalogNormaliz(tVSC_CTL*);
extern void VSCRMS_PQCalc(tVSC_CTL*);

#endif



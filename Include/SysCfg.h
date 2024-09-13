/**
  ******************************************************************************
  * @file    SysCfg.h
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

#ifndef _SYSCFG_H_
#define _SYSCFG_H_

//��ѭ����ʱ��
#define MAIN_LOOP_TIMER1    (1L)           		//������ѭ����ʱ������λ����
#define MAIN_LOOP_TIMER2    (20L)
#define MAIN_LOOP_TIMER3    (500L)

#define INTFRE				(10000)				//�ж�Ƶ��
#define H_NUM				(4)					//ģ����
#define H_FRE				(20000)				//��ģ��Ƶ�ʣ�ָ���Ų�Ƶ��
#define CAR_SYNC_PRD		(1)					//=N*(INTFRE/H_FRE),NΪ��֤N*(INTFRE/H_FRE)Ϊ��������Сֵ

/*�������*/
#define RATED_S             (3.0e3)
#define RATED_ACV			(110.0f) // ��ߵ�ѹ��Ϊ110V
#define RATED_ACI           ((float)(RATED_S/RATED_ACV/sqrt3))
#define RATED_DCV			(200.0f)
#define RATED_DCI			((float)(RATED_S/RATED_DCV))

#define RATED_F     		(50.0f)				//�Ƶ��
#define RATED_FMIN			(30.0f)				//��СƵ��
#define RATED_FMAX			(70.0f)				//���Ƶ��

//�����
#define Larm                (1.5e-3)			//�����翹��4.0mH
#define Rs					(0.5f)				//�ű۵翹������
#define Cbus				(6.0e-3)			//��ģ�����

#define I_DC_LIM			1.4f				//ֱ���������ƻ�����ֵ

//ģ��������Ϣ
#define H_UV_THR			(0)					//H��Ƿѹ��
#define H_OV_THR			(300)				//H�Ź�ѹ��
#define H_ERR_MSK			(0x0)				//H�Ź�������λ

#define HBBlk(A) (A?(hS_FpgaDownLink->GlobalBlock = 0xA7):(hS_FpgaDownLink->GlobalBlock = 0x7A))			//H�ſ��ٱ���

#define MDCEQUALONE			1
#define FAN_IDLE_MAX		(180L)					//����ͣ����ת��ʱ�䣨��λ���룩

/**********����ϵͳ���**********/
#define OUTLOOPDIV  		(10)					//�⻷��Ƶ��
//ֱ�Ӳ������Ҫ����ThetaMaxΪ2��N���ݣ�ʹ�ò�ֵ����Ϊ2pi
#define THETAMAX 			(2.0f*PI)
//ThetaMIN�̶�Ϊ0
#define THETAMIN 			((u16)0)
#define THETA30DEG			(30.0f*(THETAMAX-THETAMIN)/360)
#define THETA180DEG     	(180.0f*(THETAMAX-THETAMIN)/360)

//��������ز���
#define CURLOOP_TS			0.0004f				//������ʱ�䳣��0.4ms//����400Hz
#define CURLOOP_K			((float)(1.0f/CURLOOP_TS/NORM_Z))
#define CURLOOP_Tf			512//312.0f
#define CURLOOP_Kp			0.002//0.003f

#define PLLFRE_ARRAY_SIZE 	2000				//���໷�˲����
#define OMEGAMAX 			(THETAMAX*RATED_FMAX/INTFRE)
#define OMEGAMIN 			(THETAMAX*RATED_FMIN/INTFRE)

#define PQAVGCOFF			AVG20ms_10kHz	//�����˲�

//��Чֵ���LPF����
#define RMSLPF3dBf  		1.0f            //-3dB 0.2Hz
#define RMSLPFt     		((float)(1.0f/2.0f/PI/RMSLPF3dBf))
#define RMSLPFCOFF1 		((float)(1.0f/(1.0f+INTFRE*RMSLPFt)))
#define RMSLPFCOFF2 		((float)(1.0f-RMSLPFCOFF1))

//PWM DC LPF����
#define DCPWMLPF3dBf  		0.1f            //-3dB 0.1Hz
#define DCPWMLPFt     		((float)(1.0f/2.0f/PI/DCPWMLPF3dBf))
#define DCPWMLPFCOFF1 		((float)(1.0f/(1.0f+INTFRE*DCPWMLPFt)))
#define DCPWMLPFCOFF2 		((float)(1.0f-DCPWMLPFCOFF1))

//�ؼ����в���ֵ
#define DCRELAY_OPENCURRENT_TH		(0.3f)				//ֱ��Ԥ���̵����г�������ֵ��Ĭ��0.3A
#define ACPRECH_VOL_MIN				(0.75f)				//0.8���ߵ�ѹ��ֵ
#define DCPRECH_VOL_MIN				(0.9f)				//0.9������ֱ����ѹ

//20240908��ӹ����⻷����
//�´�ϵ��

#define Omega0 (2*PI*50.0)
#define E0 ((float)(110.0)*sqrt(2)/sqrt3)

// �����迹
#define Rv 0.0f
#define Lv 100.0e-3f
#define Xv (2*PI*50.0*Lv)

#define Dp_Droop 0.2f
#define Dq_Droop 0.05f

//VSG����,20240814��,���ƽṹ�ĳ�����ֵ���ƣ������ʱ����ۻ�
#define Jvsg (0.5f)  		//VSG�Ĺ���
#define Dpvsg (10.0f) 	//VSG�й�����
#define Dqvsg (500.0f)  // VSG�޹�����
#define Kqvsg (15.0f)   // VSG�޹�����

//LPF Droop����,20240814��
#define Dp_LPF (1.0f/Omega0/Dqvsg) //20240814���
#define Dq_LPF (1.0f/Dqvsg)        //20240814���
#define omegap (Dpvsg/Jvsg)    //20240814���
#define omegaq (Dqvsg/Kqvsg)   //20240814���

//ͳһ��������,20240814��,��VSGΪ��
#define UniA (1.0f/Jvsg/Omega0)//Droop A=Dp*��; LPF Droop A=Dp*omegap;
#define UniB (Dpvsg/Jvsg)//Droop B=�� ; LPF Droop B=omegap;
#define UniC (1.0f/Kqvsg)//Droop C=�ޣ�LPF Droop C=Dq*omegaq
#define UniD (Dqvsg/Kqvsg)//Droop D=�ޣ�LPF Droop D=omegaq

/**********����ϵͳ��ؽ���**********/

/**********���϶�����************/
//���϶���
#define ERR_AC_OV_HW			(1<<0)		   //bit0: Ӳ��������ѹ
#define ERR_AC_OC_HW			(1<<1)         //bit1: Ӳ����������
#define ERR_DC_OV_HW			(1<<2)         //bit2: Ӳ��ֱ����ѹ
#define ERR_DC_OC_HW			(1<<3)         //bit3: Ӳ��ֱ������
#define ERR_BOARDLINK_HW		(1<<4)         //bit4: Ӳ��������ӹ���
#define ERR_EMER_HW				(1<<5)         //bit5: Ӳ����ͣ����
#define ERR_MOD_OV_HW			(1<<6)         //bit6: ģ�����ѹ
#define ERR_STARTFAIL			(1<<7)         //bit7: ����ʧ��
#define ERR_AC_UVOV_SW			(1<<8)         //bit8: ���������Ƿѹ
#define ERR_DC_UVOV_SW			(1<<9)         //bit9: ���ֱ����Ƿѹ
#define ERR_PLL_SW				(1<<10)        //bit10:������໷�쳣
#define ERR_PEM_SW				(1<<11)        //bit11:����ģ�����
#define ERR_CFG_SW				(1<<12)        //bit12:���ù��ϣ���������⣩

#define ERR0_EN					1
#define ERR1_EN					1
#define ERR2_EN					1
#define ERR3_EN					1
#define ERR4_EN					1
#define ERR5_EN					1
#define ERR6_EN					0
#define ERR7_EN					1
#define ERR8_EN					1
#define ERR9_EN					1
#define ERR10_EN				1
#define ERR11_EN				1
#define ERR12_EN				0
#define ERR13_EN				0
#define ERR14_EN				0
#define ERR15_EN				0

#define ERRx_EN_MASK		(ERR0_EN+(ERR1_EN<<1)+(ERR2_EN<<2)+(ERR3_EN<<3)+(ERR4_EN<<4)+(ERR5_EN<<5)+(ERR6_EN<<6)+(ERR7_EN<<7)+(ERR8_EN<<8)+(ERR9_EN<<9)+(ERR10_EN<<10)+(ERR11_EN<<11)+(ERR12_EN<<12)+(ERR13_EN<<13)+(ERR14_EN<<14)+(ERR15_EN<<15))
/**********���϶���������************/

/**********ADC************/
//ADC����
#define ADC_RANGE       	(ADC_RANGE10V)
#define ADCBITS         	(16)
#define ADC_BASE        	((float)(1L<<(ADCBITS-1)))
//����ϵ��
#define ACV_TRANS_G 		(1.0f)								//�ⲿ������ѹ������
#define ACI_TRANS_G 		(1.0f)								//�ⲿ��������������

//ϵ����ʱ��û������
#define ACVRATIO	((float)(10.0*20.3/32768))				//V/20.3=Vadc
#define ACIRATIO	((float)(10.0*5/32768))					//I/5=Vadc
#define DCVRATIO	((float)(10.0*30.3/32768))				//V/30.3=Vadc
#define DCIRATIO	((float)(10.0*5/32768))					//I/5=Vadc
/**********ADC����************/

/**********�����Զ�����ֵ���Ǳ�Ҫ���Ķ�************/
#define RATED_ACVP	(RATED_ACV*1.414)			//PeakValue
#define RATED_ACIP	(RATED_ACI*1.414)
//��һ����
#define NORM_V	((float)(RATED_ACV)/sqrt3*sqrt2) 		//AC��ѹ���ۻ�׼ֵ
#define NORM_I	((float)(RATED_ACI)*sqrt2)				//AC�������ۻ�׼
#define NORM_I_ARM	((NORM_I/2.0f)+(RATED_DCI/3.0f))
#define NORM_S	(RATED_S)
#define NORM_Z	((float)(RATED_ACV*RATED_ACV/RATED_S))
#define H_FRE_REG			((u16)(HB_CLK/H_FRE/4))

#define AVG20ms_10kHz		(INTFRE/50.0f)				//1�׵�ͨ�˲�����@10kHz����Ƶ��
#define AVG10ms_10kHz		(INTFRE/100.0f)
#define AVG5ms_10kHz		(INTFRE/200.0f)
#define AVG1ms_10kHz		(INTFRE/1000.0f)

#define AVG1000ms_1kHz	((float)INTFRE/10.0f/1.0f)		//1�׵�ͨ�˲�����@1kHz����Ƶ��
#define AVG10ms_1kHz	(((float)INTFRE/10.0f/100.0f))

#define ADC_RANGE5V     (5.0f)
#define ADC_RANGE10V    (10.0f)
/**********�����Զ�����ֵ������************/


/****************����****************/
extern void SysCfg(void);
extern void PlSoftwareReset(void);
extern u16 SysChkPlatform(void);
extern u16 SysCardBind(void* Handle,u16 CardType,u16 Slot);


#endif

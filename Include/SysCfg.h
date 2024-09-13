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

//主循环定时器
#define MAIN_LOOP_TIMER1    (1L)           		//三层主循环定时器，单位毫秒
#define MAIN_LOOP_TIMER2    (20L)
#define MAIN_LOOP_TIMER3    (500L)

#define INTFRE				(10000)				//中断频率
#define H_NUM				(4)					//模块数
#define H_FRE				(20000)				//子模块频率，指半桥侧频率
#define CAR_SYNC_PRD		(1)					//=N*(INTFRE/H_FRE),N为保证N*(INTFRE/H_FRE)为整数的最小值

/*额定参数表*/
#define RATED_S             (3.0e3)
#define RATED_ACV			(110.0f) // 额定线电压改为110V
#define RATED_ACI           ((float)(RATED_S/RATED_ACV/sqrt3))
#define RATED_DCV			(200.0f)
#define RATED_DCI			((float)(RATED_S/RATED_DCV))

#define RATED_F     		(50.0f)				//额定频率
#define RATED_FMIN			(30.0f)				//最小频率
#define RATED_FMAX			(70.0f)				//最大频率

//逆变器
#define Larm                (1.5e-3)			//并网电抗器4.0mH
#define Rs					(0.5f)				//桥臂电抗器内阻
#define Cbus				(6.0e-3)			//子模块电容

#define I_DC_LIM			1.4f				//直流电流限制环给定值

//模块配置信息
#define H_UV_THR			(0)					//H桥欠压点
#define H_OV_THR			(300)				//H桥过压点
#define H_ERR_MSK			(0x0)				//H桥故障屏蔽位

#define HBBlk(A) (A?(hS_FpgaDownLink->GlobalBlock = 0xA7):(hS_FpgaDownLink->GlobalBlock = 0x7A))			//H桥快速闭锁

#define MDCEQUALONE			1
#define FAN_IDLE_MAX		(180L)					//风扇停机后转动时间（单位：秒）

/**********控制系统相关**********/
#define OUTLOOPDIV  		(10)					//外环分频比
//直接查表是需要留意ThetaMax为2的N次幂，使用插值函数为2pi
#define THETAMAX 			(2.0f*PI)
//ThetaMIN固定为0
#define THETAMIN 			((u16)0)
#define THETA30DEG			(30.0f*(THETAMAX-THETAMIN)/360)
#define THETA180DEG     	(180.0f*(THETAMAX-THETAMIN)/360)

//电流环相关参数
#define CURLOOP_TS			0.0004f				//电流环时间常数0.4ms//带宽400Hz
#define CURLOOP_K			((float)(1.0f/CURLOOP_TS/NORM_Z))
#define CURLOOP_Tf			512//312.0f
#define CURLOOP_Kp			0.002//0.003f

#define PLLFRE_ARRAY_SIZE 	2000				//锁相环滤波深度
#define OMEGAMAX 			(THETAMAX*RATED_FMAX/INTFRE)
#define OMEGAMIN 			(THETAMAX*RATED_FMIN/INTFRE)

#define PQAVGCOFF			AVG20ms_10kHz	//功率滤波

//有效值检测LPF参数
#define RMSLPF3dBf  		1.0f            //-3dB 0.2Hz
#define RMSLPFt     		((float)(1.0f/2.0f/PI/RMSLPF3dBf))
#define RMSLPFCOFF1 		((float)(1.0f/(1.0f+INTFRE*RMSLPFt)))
#define RMSLPFCOFF2 		((float)(1.0f-RMSLPFCOFF1))

//PWM DC LPF参数
#define DCPWMLPF3dBf  		0.1f            //-3dB 0.1Hz
#define DCPWMLPFt     		((float)(1.0f/2.0f/PI/DCPWMLPF3dBf))
#define DCPWMLPFCOFF1 		((float)(1.0f/(1.0f+INTFRE*DCPWMLPFt)))
#define DCPWMLPFCOFF2 		((float)(1.0f-DCPWMLPFCOFF1))

//关键运行参数值
#define DCRELAY_OPENCURRENT_TH		(0.3f)				//直流预充电继电器切除电流阈值，默认0.3A
#define ACPRECH_VOL_MIN				(0.75f)				//0.8倍线电压峰值
#define DCPRECH_VOL_MIN				(0.9f)				//0.9倍输入直流电压

//20240908添加功率外环参数
//下垂系数

#define Omega0 (2*PI*50.0)
#define E0 ((float)(110.0)*sqrt(2)/sqrt3)

// 虚拟阻抗
#define Rv 0.0f
#define Lv 100.0e-3f
#define Xv (2*PI*50.0*Lv)

#define Dp_Droop 0.2f
#define Dq_Droop 0.05f

//VSG参数,20240814改,控制结构改成有名值控制，输出的时候标幺化
#define Jvsg (0.5f)  		//VSG的惯量
#define Dpvsg (10.0f) 	//VSG有功阻尼
#define Dqvsg (500.0f)  // VSG无功阻尼
#define Kqvsg (15.0f)   // VSG无功惯量

//LPF Droop参数,20240814改
#define Dp_LPF (1.0f/Omega0/Dqvsg) //20240814添加
#define Dq_LPF (1.0f/Dqvsg)        //20240814添加
#define omegap (Dpvsg/Jvsg)    //20240814添加
#define omegaq (Dqvsg/Kqvsg)   //20240814添加

//统一构网控制,20240814改,以VSG为例
#define UniA (1.0f/Jvsg/Omega0)//Droop A=Dp*∞; LPF Droop A=Dp*omegap;
#define UniB (Dpvsg/Jvsg)//Droop B=∞ ; LPF Droop B=omegap;
#define UniC (1.0f/Kqvsg)//Droop C=∞；LPF Droop C=Dq*omegaq
#define UniD (Dqvsg/Kqvsg)//Droop D=∞；LPF Droop D=omegaq

/**********控制系统相关结束**********/

/**********故障定义区************/
//故障定义
#define ERR_AC_OV_HW			(1<<0)		   //bit0: 硬件交流过压
#define ERR_AC_OC_HW			(1<<1)         //bit1: 硬件交流过流
#define ERR_DC_OV_HW			(1<<2)         //bit2: 硬件直流过压
#define ERR_DC_OC_HW			(1<<3)         //bit3: 硬件直流过流
#define ERR_BOARDLINK_HW		(1<<4)         //bit4: 硬件板间连接故障
#define ERR_EMER_HW				(1<<5)         //bit5: 硬件急停开关
#define ERR_MOD_OV_HW			(1<<6)         //bit6: 模块过电压
#define ERR_STARTFAIL			(1<<7)         //bit7: 启动失败
#define ERR_AC_UVOV_SW			(1<<8)         //bit8: 软件交流过欠压
#define ERR_DC_UVOV_SW			(1<<9)         //bit9: 软件直流过欠压
#define ERR_PLL_SW				(1<<10)        //bit10:软件锁相环异常
#define ERR_PEM_SW				(1<<11)        //bit11:功率模块故障
#define ERR_CFG_SW				(1<<12)        //bit12:配置故障（不单独检测）

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
/**********故障定义区结束************/

/**********ADC************/
//ADC配置
#define ADC_RANGE       	(ADC_RANGE10V)
#define ADCBITS         	(16)
#define ADC_BASE        	((float)(1L<<(ADCBITS-1)))
//增益系数
#define ACV_TRANS_G 		(1.0f)								//外部交流电压互感器
#define ACI_TRANS_G 		(1.0f)								//外部交流电流互感器

//系数暂时还没有修正
#define ACVRATIO	((float)(10.0*20.3/32768))				//V/20.3=Vadc
#define ACIRATIO	((float)(10.0*5/32768))					//I/5=Vadc
#define DCVRATIO	((float)(10.0*30.3/32768))				//V/30.3=Vadc
#define DCIRATIO	((float)(10.0*5/32768))					//I/5=Vadc
/**********ADC结束************/

/**********参数自动计算值，非必要不改动************/
#define RATED_ACVP	(RATED_ACV*1.414)			//PeakValue
#define RATED_ACIP	(RATED_ACI*1.414)
//归一化用
#define NORM_V	((float)(RATED_ACV)/sqrt3*sqrt2) 		//AC电压标幺基准值
#define NORM_I	((float)(RATED_ACI)*sqrt2)				//AC电流标幺基准
#define NORM_I_ARM	((NORM_I/2.0f)+(RATED_DCI/3.0f))
#define NORM_S	(RATED_S)
#define NORM_Z	((float)(RATED_ACV*RATED_ACV/RATED_S))
#define H_FRE_REG			((u16)(HB_CLK/H_FRE/4))

#define AVG20ms_10kHz		(INTFRE/50.0f)				//1阶低通滤波参数@10kHz运行频率
#define AVG10ms_10kHz		(INTFRE/100.0f)
#define AVG5ms_10kHz		(INTFRE/200.0f)
#define AVG1ms_10kHz		(INTFRE/1000.0f)

#define AVG1000ms_1kHz	((float)INTFRE/10.0f/1.0f)		//1阶低通滤波参数@1kHz运行频率
#define AVG10ms_1kHz	(((float)INTFRE/10.0f/100.0f))

#define ADC_RANGE5V     (5.0f)
#define ADC_RANGE10V    (10.0f)
/**********参数自动计算值，结束************/


/****************函数****************/
extern void SysCfg(void);
extern void PlSoftwareReset(void);
extern u16 SysChkPlatform(void);
extern u16 SysCardBind(void* Handle,u16 CardType,u16 Slot);


#endif

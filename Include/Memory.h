/**
  ******************************************************************************
  * @file    Memory.h
  * @author  RZ
  * @brief   Header for Memory.c module
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
#ifndef _MEMORY_H_
#define _MEMORY_H_

typedef struct{
	u16 B0:1;       	// B0
	u16 B1:1;       	// B1
	u16 B2:1;       	// B2
	u16 B3:1;       	// B3
	u16 B4:1;       	// B4
	u16 B5:1;       	// B5
	u16 B6:1;       	// B6
	u16 B7:1;       	// B7
	u16 B8:1;       	// B8
	u16 B9:1;       	// B9
	u16 MAIN:1;       	// B10
	u16 rsvd:5;
	u16 rsvd1:16;      	// 31:11 reserved 保留
}S_RUNLED_BITS;
typedef union{
	u32 all;
	S_RUNLED_BITS bit;
}U_RUNLED;

//下发至主控FPG的控制数据A
typedef struct
{
	u32 SyncFreNum;						//决定控制周期和中断频率，50MHz时钟，5000时表示10kHz
	u32 CMDUpdate;						//写1时，FPGA开始往光纤板下发控制数据
	u32 ConfigFlush;					//写1时，对应板卡进行配置数据下发
	u32 rsvd1;							//--//LVDS训练标志，上行占高16bit，下行占低16bit，每个插槽对应一个bit；为1表示下发训练数据，为0表示下发正常数据
	u32 GlobalBlock;					//全局紧急闭锁控制，写入0xA7生效，写入0x7A解除。
	u32 RunLed;							//LED控制
}S_FPGA_DOWNLINK;

//主控FPGA上传的数据
typedef struct
{
	u32 CardValid;						//板卡是否插上标志
	u32 CardType7_0;					//板卡类型0_7槽位的板卡类型
	u32 CardType9_8;					//板卡类型8_9槽位的板卡类型
	u32 LvdsLinkSts;					//Lvds上下行链路连接情况
	u32 CalcTime;						//计算时间
	u32 HandShake;
	u32 ADCSamPoint;
	u32 TestDat[8];
}S_FPGA_UPLINK;

/*********光纤卡定义*********/
typedef struct		//配置机构体
{
	u32 add_sub_flag1;					//同步字方向寄存器
	u32 add_sub_flag2;					//同步字方向寄存器
	s32 sync_data[20];					//同步字数据
	u32 HBAmp;							//H桥幅值寄存器
	u32 OVthr;							//过压基准值
	u32 UVthr;							//欠压基准值
	u32 ErrMsk;							//故障掩码寄存器
	u32 CarSyncPrd;						//载波同步次数
}S_OPTCARD_CFG;
typedef union		//配置联合体
{
	u32 all[27];						//所有寄存器
	S_OPTCARD_CFG bit;					//按照结构体分解
}U_OPTCARD_CFG;

typedef struct		//指令结构体
{
	s16 ModWave:16;    	// 15:0    调制波
	u16 Bypass:1;		// 16 旁路控制（无效）
	u16 Mode:3;        	// 19:17   运行模式
	u16 rsvd0:12;      	// 31:19 reserved 保留
}S_OPTCARD_CMD;

typedef union		//指令联合体
{
	u32 CMD[20];						//所有寄存器
	S_OPTCARD_CMD bit[20];				//按照结构体分解
}U_OPTCARD_CMD;

typedef struct		//数据结构体
{
	u16 CapV:11;       	// 10:0 电容电压
	u16 HBMode:3;      	// 13:11 运行模式
	u16 DownLink:1;    	// 14 下行有效
	u16 CfgFlag:1;     	// 15 配置有效
	u16 FaultWord:10;  	// 25:16 故障字
	u16 FaultSts:2;    	// 27:26 故障状态
	u16 UpLink:1;      	// 28 上行有效
	u16 rsvd1:3;      	// 31:29 reserved 保留
}S_OPTCARD_DAT;

typedef union		//数据联合体
{
	u32 DAT[21];						//所有寄存器，最后DAT[20]为状态信息
	S_OPTCARD_DAT bit[20];				//按照结构体分解
}U_OPTCARD_DAT;

typedef struct		//数据结构体
{
	U_OPTCARD_CFG* Cfg;
	U_OPTCARD_CMD* Cmd;
	U_OPTCARD_DAT* Dat;
	u16 InitOK;
}S_OPTCARD;
/*********光纤卡结束*********/

/*********传感器卡定义*********/
typedef struct		//配置机构体
{
	u32 AdcCnvPoint;
	u32 FaultReset;
}S_SENSORCARD_CFG;
typedef union		//配置联合体
{
	u32 all[2];						//所有寄存器
	S_SENSORCARD_CFG bit;			//按照结构体分解
}U_SENSORCARD_CFG;

typedef struct		//指令结构体
{
	s32 rsvd;
}S_SENSORCARD_CMD;

typedef union		//指令联合体
{
	u32 all;						//所有寄存器
	S_SENSORCARD_CMD bit;			//按照结构体分解
}U_SENSORCARD_CMD;

typedef struct		//数据结构体
{
	s16 ADdat[16];
	u32 Fault;
}S_SENSORCARD_DAT;

typedef union		//数据联合体
{
	u32 all[9];						//所有寄存器
	S_SENSORCARD_DAT bit;				//按照结构体分解
}U_SENSORCARD_DAT;

typedef struct		//数据结构体
{
	U_SENSORCARD_CFG* Cfg;
	U_SENSORCARD_CMD* Cmd;
	U_SENSORCARD_DAT* Dat;
	u16 InitOK;
}S_SENSORCARD;
/*********传感器卡结束*********/

/*********开入开出卡定义*********/
typedef struct		//配置机构体
{
	u32 rsvd;
}S_DIDOCARD_CFG;
typedef union		//配置联合体
{
	u32 all;						//所有寄存器
	S_DIDOCARD_CFG bit;					//按照结构体分解
}U_DIDOCARD_CFG;

typedef struct		//指令结构体
{
	u16 DO0:1;       	// DO0
	u16 DO1:1;       	// DO1
	u16 DO2:1;       	// DO2
	u16 DO3:1;       	// DO3
	u16 DO4:1;       	// DO4
	u16 DO5:1;       	// DO5
	u16 DO6:1;       	// DO6
	u16 DO7:1;       	// DO7
	u16 DO8:1;       	// DO8
	u16 DO9:1;       	// DO9
	u16 DO10:1;       	// DO10
	u16 DO11:1;       	// DO11
	u16 DO12:1;       	// DO12
	u16 DO13:1;       	// DO13
	u16 DO14:1;       	// DO14
	u16 DO15:1;       	// DO15
	u16 rsvd:16;      	// 31:16 reserved 保留
}S_DIDOCARD_CMD;

typedef union		//指令联合体
{
	u32 all;						//所有寄存器
	S_DIDOCARD_CMD bit;				//按照结构体分解
}U_DIDOCARD_CMD;

typedef struct		//数据结构体
{
	u16 DI0:1;       	// DI0
	u16 DI1:1;       	// DI1
	u16 DI2:1;       	// DI2
	u16 DI3:1;       	// DI3
	u16 DI4:1;       	// DI4
	u16 DI5:1;       	// DI5
	u16 DI6:1;       	// DI6
	u16 DI7:1;       	// DI7
	u16 DI8:1;       	// DI8
	u16 DI9:1;       	// DI9
	u16 DI10:1;       	// DI10
	u16 DI11:1;       	// DI11
	u16 DI12:1;       	// DI12
	u16 DI13:1;       	// DI13
	u16 DI14:1;       	// DI14
	u16 DI15:1;       	// DI15
	u16 rsvd:16;      	// 31:16 reserved 保留
}S_DIDOCARD_DAT;

typedef union		//数据联合体
{
	u32 all;						//所有寄存器
	S_DIDOCARD_DAT bit;				//按照结构体分解
}U_DIDOCARD_DAT;

typedef struct		//数据结构体
{
	U_DIDOCARD_CFG* Cfg;
	U_DIDOCARD_CMD* Cmd;
	U_DIDOCARD_DAT* Dat;
	u16 InitOK;
}S_DIDOCARD;
/*********开入开出卡结束*********/

/*********AO卡定义*********/
typedef struct		//配置机构体
{
	u32 rsvd1;
	u32 rsvd2;
}S_AOCARD_CFG;
typedef union		//配置联合体
{
	u32 all[2];						//所有寄存器
	S_AOCARD_CFG bit;				//按照结构体分解
}U_AOCARD_CFG;

typedef struct		//指令结构体
{
	s16 AOdat[32];
	u32 AO_Bias_En;
}S_AOCARD_CMD;

typedef union		//指令联合体
{
	u32 all[17];					//所有寄存器
	S_AOCARD_CMD bit;				//按照结构体分解
}U_AOCARD_CMD;

typedef struct		//数据结构体
{
	s32 rsvd1;
}S_AOCARD_DAT;

typedef union		//数据联合体
{
	u32 all;						//所有寄存器
	S_AOCARD_DAT bit;				//按照结构体分解
}U_AOCARD_DAT;

typedef struct		//数据结构体
{
	U_AOCARD_CFG* Cfg;
	U_AOCARD_CMD* Cmd;
	U_AOCARD_DAT* Dat;
	u16 InitOK;
}S_AOCARD;
/*********AO卡结束*********/

/*********ROP卡定义*********/
typedef struct		//配置机构体
{
	u32 AdcCnvPoint;
}S_ROPCARD_CFG;
typedef union		//配置联合体
{
	u32 all[1];						//所有寄存器
	S_ROPCARD_CFG bit;			//按照结构体分解
}U_ROPCARD_CFG;

typedef struct		//指令结构体
{
	u16 DO0:1;       	// DO0
	u16 DO1:1;       	// DO1
	u16 DO2:1;       	// DO2
	u16 DO3:1;       	// DO3
	u16 DO4:1;       	// DO4
	u16 DO5:1;       	// DO5
	u16 DO6:1;       	// DO6
	u16 DO7:1;       	// DO7
	u16 DO8:1;       	// DO8
	u16 DO9:1;       	// DO9
	u16 DO10:1;       	// DO10
	u16 DO11:1;       	// DO11
	u16 DO12:1;       	// DO12
	u16 DO13:1;       	// DO13
	u16 DO14:1;       	// DO14
	u16 DO15:1;       	// DO15
	u16 rsvd:16;      	// 31:16 reserved 保留
}S_ROPCARD_DOBITS;

typedef union		//指令结构体
{
	u32 all;
	S_ROPCARD_DOBITS bit;
}U_ROPCARD_CMD_DO;

typedef struct		//指令结构体
{
	U_ROPCARD_CMD_DO DO;
}S_ROPCARD_CMD;

typedef union		//指令联合体
{
	u32 all;					//所有寄存器
	S_ROPCARD_CMD bit;			//按照结构体分解
}U_ROPCARD_CMD;

typedef struct		//指令结构体
{
	u16 DI0:1;       	// DI0
	u16 DI1:1;       	// DI1
	u16 DI2:1;       	// DI2
	u16 DI3:1;       	// DI3
	u16 DI4:1;       	// DI4
	u16 DI5:1;       	// DI5
	u16 DI6:1;       	// DI6
	u16 DI7:1;       	// DI7
	u16 DI8:1;       	// DI8
	u16 DI9:1;       	// DI9
	u16 DI10:1;       	// DI10
	u16 DI11:1;       	// DI11
	u16 DI12:1;       	// DI12
	u16 DI13:1;       	// DI13
	u16 DI14:1;       	// DI14
	u16 DI15:1;       	// DI15
	u16 rsvd:16;      	// 31:16 reserved 保留
}S_ROPCARD_DIBITS;

typedef union		//指令结构体
{
	u32 all;
	S_ROPCARD_DIBITS bit;
}U_ROPCARD_CMD_DI;

typedef struct		//数据结构体
{
	s16 ADdat[16];
	U_ROPCARD_CMD_DI DI;
}S_ROPCARD_DAT;

typedef union		//数据联合体
{
	u32 all[9];						//所有寄存器
	S_ROPCARD_DAT bit;				//按照结构体分解
}U_ROPCARD_DAT;

typedef struct		//数据结构体
{
	U_ROPCARD_CFG* Cfg;
	U_ROPCARD_CMD* Cmd;
	U_ROPCARD_DAT* Dat;
	u16 InitOK;
}S_ROPCARD;
/*********ROP卡结束*********/

void ddr3_wr_burst(u32 * src_data_ptr,u32 * wr_ddr3_ptr,u32 burst_word_len);
void ddr3_rd_burst(u32 * dst_data_ptr,u32 * rd_ddr3_ptr,u32 burst_word_len);
extern u16 SysEnvInit(void);

#endif

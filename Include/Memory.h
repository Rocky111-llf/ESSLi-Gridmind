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
	u16 rsvd1:16;      	// 31:11 reserved ����
}S_RUNLED_BITS;
typedef union{
	u32 all;
	S_RUNLED_BITS bit;
}U_RUNLED;

//�·�������FPG�Ŀ�������A
typedef struct
{
	u32 SyncFreNum;						//�����������ں��ж�Ƶ�ʣ�50MHzʱ�ӣ�5000ʱ��ʾ10kHz
	u32 CMDUpdate;						//д1ʱ��FPGA��ʼ�����˰��·���������
	u32 ConfigFlush;					//д1ʱ����Ӧ�忨�������������·�
	u32 rsvd1;							//--//LVDSѵ����־������ռ��16bit������ռ��16bit��ÿ����۶�Ӧһ��bit��Ϊ1��ʾ�·�ѵ�����ݣ�Ϊ0��ʾ�·���������
	u32 GlobalBlock;					//ȫ�ֽ����������ƣ�д��0xA7��Ч��д��0x7A�����
	u32 RunLed;							//LED����
}S_FPGA_DOWNLINK;

//����FPGA�ϴ�������
typedef struct
{
	u32 CardValid;						//�忨�Ƿ���ϱ�־
	u32 CardType7_0;					//�忨����0_7��λ�İ忨����
	u32 CardType9_8;					//�忨����8_9��λ�İ忨����
	u32 LvdsLinkSts;					//Lvds��������·�������
	u32 CalcTime;						//����ʱ��
	u32 HandShake;
	u32 ADCSamPoint;
	u32 TestDat[8];
}S_FPGA_UPLINK;

/*********���˿�����*********/
typedef struct		//���û�����
{
	u32 add_sub_flag1;					//ͬ���ַ���Ĵ���
	u32 add_sub_flag2;					//ͬ���ַ���Ĵ���
	s32 sync_data[20];					//ͬ��������
	u32 HBAmp;							//H�ŷ�ֵ�Ĵ���
	u32 OVthr;							//��ѹ��׼ֵ
	u32 UVthr;							//Ƿѹ��׼ֵ
	u32 ErrMsk;							//��������Ĵ���
	u32 CarSyncPrd;						//�ز�ͬ������
}S_OPTCARD_CFG;
typedef union		//����������
{
	u32 all[27];						//���мĴ���
	S_OPTCARD_CFG bit;					//���սṹ��ֽ�
}U_OPTCARD_CFG;

typedef struct		//ָ��ṹ��
{
	s16 ModWave:16;    	// 15:0    ���Ʋ�
	u16 Bypass:1;		// 16 ��·���ƣ���Ч��
	u16 Mode:3;        	// 19:17   ����ģʽ
	u16 rsvd0:12;      	// 31:19 reserved ����
}S_OPTCARD_CMD;

typedef union		//ָ��������
{
	u32 CMD[20];						//���мĴ���
	S_OPTCARD_CMD bit[20];				//���սṹ��ֽ�
}U_OPTCARD_CMD;

typedef struct		//���ݽṹ��
{
	u16 CapV:11;       	// 10:0 ���ݵ�ѹ
	u16 HBMode:3;      	// 13:11 ����ģʽ
	u16 DownLink:1;    	// 14 ������Ч
	u16 CfgFlag:1;     	// 15 ������Ч
	u16 FaultWord:10;  	// 25:16 ������
	u16 FaultSts:2;    	// 27:26 ����״̬
	u16 UpLink:1;      	// 28 ������Ч
	u16 rsvd1:3;      	// 31:29 reserved ����
}S_OPTCARD_DAT;

typedef union		//����������
{
	u32 DAT[21];						//���мĴ��������DAT[20]Ϊ״̬��Ϣ
	S_OPTCARD_DAT bit[20];				//���սṹ��ֽ�
}U_OPTCARD_DAT;

typedef struct		//���ݽṹ��
{
	U_OPTCARD_CFG* Cfg;
	U_OPTCARD_CMD* Cmd;
	U_OPTCARD_DAT* Dat;
	u16 InitOK;
}S_OPTCARD;
/*********���˿�����*********/

/*********������������*********/
typedef struct		//���û�����
{
	u32 AdcCnvPoint;
	u32 FaultReset;
}S_SENSORCARD_CFG;
typedef union		//����������
{
	u32 all[2];						//���мĴ���
	S_SENSORCARD_CFG bit;			//���սṹ��ֽ�
}U_SENSORCARD_CFG;

typedef struct		//ָ��ṹ��
{
	s32 rsvd;
}S_SENSORCARD_CMD;

typedef union		//ָ��������
{
	u32 all;						//���мĴ���
	S_SENSORCARD_CMD bit;			//���սṹ��ֽ�
}U_SENSORCARD_CMD;

typedef struct		//���ݽṹ��
{
	s16 ADdat[16];
	u32 Fault;
}S_SENSORCARD_DAT;

typedef union		//����������
{
	u32 all[9];						//���мĴ���
	S_SENSORCARD_DAT bit;				//���սṹ��ֽ�
}U_SENSORCARD_DAT;

typedef struct		//���ݽṹ��
{
	U_SENSORCARD_CFG* Cfg;
	U_SENSORCARD_CMD* Cmd;
	U_SENSORCARD_DAT* Dat;
	u16 InitOK;
}S_SENSORCARD;
/*********������������*********/

/*********���뿪��������*********/
typedef struct		//���û�����
{
	u32 rsvd;
}S_DIDOCARD_CFG;
typedef union		//����������
{
	u32 all;						//���мĴ���
	S_DIDOCARD_CFG bit;					//���սṹ��ֽ�
}U_DIDOCARD_CFG;

typedef struct		//ָ��ṹ��
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
	u16 rsvd:16;      	// 31:16 reserved ����
}S_DIDOCARD_CMD;

typedef union		//ָ��������
{
	u32 all;						//���мĴ���
	S_DIDOCARD_CMD bit;				//���սṹ��ֽ�
}U_DIDOCARD_CMD;

typedef struct		//���ݽṹ��
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
	u16 rsvd:16;      	// 31:16 reserved ����
}S_DIDOCARD_DAT;

typedef union		//����������
{
	u32 all;						//���мĴ���
	S_DIDOCARD_DAT bit;				//���սṹ��ֽ�
}U_DIDOCARD_DAT;

typedef struct		//���ݽṹ��
{
	U_DIDOCARD_CFG* Cfg;
	U_DIDOCARD_CMD* Cmd;
	U_DIDOCARD_DAT* Dat;
	u16 InitOK;
}S_DIDOCARD;
/*********���뿪��������*********/

/*********AO������*********/
typedef struct		//���û�����
{
	u32 rsvd1;
	u32 rsvd2;
}S_AOCARD_CFG;
typedef union		//����������
{
	u32 all[2];						//���мĴ���
	S_AOCARD_CFG bit;				//���սṹ��ֽ�
}U_AOCARD_CFG;

typedef struct		//ָ��ṹ��
{
	s16 AOdat[32];
	u32 AO_Bias_En;
}S_AOCARD_CMD;

typedef union		//ָ��������
{
	u32 all[17];					//���мĴ���
	S_AOCARD_CMD bit;				//���սṹ��ֽ�
}U_AOCARD_CMD;

typedef struct		//���ݽṹ��
{
	s32 rsvd1;
}S_AOCARD_DAT;

typedef union		//����������
{
	u32 all;						//���мĴ���
	S_AOCARD_DAT bit;				//���սṹ��ֽ�
}U_AOCARD_DAT;

typedef struct		//���ݽṹ��
{
	U_AOCARD_CFG* Cfg;
	U_AOCARD_CMD* Cmd;
	U_AOCARD_DAT* Dat;
	u16 InitOK;
}S_AOCARD;
/*********AO������*********/

/*********ROP������*********/
typedef struct		//���û�����
{
	u32 AdcCnvPoint;
}S_ROPCARD_CFG;
typedef union		//����������
{
	u32 all[1];						//���мĴ���
	S_ROPCARD_CFG bit;			//���սṹ��ֽ�
}U_ROPCARD_CFG;

typedef struct		//ָ��ṹ��
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
	u16 rsvd:16;      	// 31:16 reserved ����
}S_ROPCARD_DOBITS;

typedef union		//ָ��ṹ��
{
	u32 all;
	S_ROPCARD_DOBITS bit;
}U_ROPCARD_CMD_DO;

typedef struct		//ָ��ṹ��
{
	U_ROPCARD_CMD_DO DO;
}S_ROPCARD_CMD;

typedef union		//ָ��������
{
	u32 all;					//���мĴ���
	S_ROPCARD_CMD bit;			//���սṹ��ֽ�
}U_ROPCARD_CMD;

typedef struct		//ָ��ṹ��
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
	u16 rsvd:16;      	// 31:16 reserved ����
}S_ROPCARD_DIBITS;

typedef union		//ָ��ṹ��
{
	u32 all;
	S_ROPCARD_DIBITS bit;
}U_ROPCARD_CMD_DI;

typedef struct		//���ݽṹ��
{
	s16 ADdat[16];
	U_ROPCARD_CMD_DI DI;
}S_ROPCARD_DAT;

typedef union		//����������
{
	u32 all[9];						//���мĴ���
	S_ROPCARD_DAT bit;				//���սṹ��ֽ�
}U_ROPCARD_DAT;

typedef struct		//���ݽṹ��
{
	U_ROPCARD_CFG* Cfg;
	U_ROPCARD_CMD* Cmd;
	U_ROPCARD_DAT* Dat;
	u16 InitOK;
}S_ROPCARD;
/*********ROP������*********/

void ddr3_wr_burst(u32 * src_data_ptr,u32 * wr_ddr3_ptr,u32 burst_word_len);
void ddr3_rd_burst(u32 * dst_data_ptr,u32 * rd_ddr3_ptr,u32 burst_word_len);
extern u16 SysEnvInit(void);

#endif

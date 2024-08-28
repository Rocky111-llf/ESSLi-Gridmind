/*****************************************************************************
Copyright: 2019-2021, GridMind. Co., Ltd.
File name: VSC_GeneralCtl.c
Description: VSC��������
Author: RZ
Version: �汾
Date: �������
History: �޸���ʷ��¼�б� ÿ���޸ļ�¼Ӧ�����޸����ڡ��޸��߼��޸����ݼ�����
*****************************************************************************/

#include "sleep.h"
#include "Includings.h"

#define EN_PHASE_SHIFT	0

//�����������
void PhShftCalc(s16 TriPrd);
//����ģ�����ã����漰�����·�
void PEMCfg(void);

/*************************************************
 Function: PhShftCalc
 Description: �������ǲ����ڼ����������
 Input: TriPrd��Ϊ���ǲ���ֵ������<32768
 Output: ��
 Return: ��
 Others: //�ֱ����
 *************************************************/
void PEMInit(void)
{
	PhShftCalc(H_FRE_REG);
	PEMCfg();
}

/*************************************************
 Function: PhShftCalc
 Description: �������ǲ����ڼ����������
 Input: TriPrd��Ϊ���ǲ���ֵ������<32768
 Output: ��
 Return: ��
 Others: //�ֱ����
 *************************************************/
void PhShftCalc(s16 TriPrd)
{
#if EN_PHASE_SHIFT
	u16 i;
	s16 StepTemp,StepBias;
	s16 SumTemp;

//N=4����λƫ�����ݼ��㣬������й��˿�0������
	StepTemp = (s16)((2.0/H_NUM)*(float)TriPrd);			//����ƽû������
	StepBias = StepTemp/2;
	SumTemp = TriPrd-50;					//��ʼΪ���ǲ���
	for(i=0;i<H_NUM;i++)
	{
		sOpt0.Cfg->bit.sync_data[i] = SumTemp;
		sOpt0.Cfg->bit.sync_data[i+(H_NUM*2)] = SumTemp;

		sOpt0.Cfg->bit.sync_data[i+H_NUM] = SumTemp-StepBias;
		sOpt0.Cfg->bit.sync_data[i+(H_NUM*3)] = SumTemp-StepBias;

		SumTemp -= StepTemp;
	}
//��λ������������
	sOpt0.Cfg->bit.add_sub_flag1 = 0xFFFF;
	sOpt0.Cfg->bit.add_sub_flag2 = 0x000F;
#endif
	s16 StepBias;
	StepBias = TriPrd-50;
	//VSC1
	sOpt0.Cfg->bit.sync_data[0] = StepBias;
	sOpt0.Cfg->bit.sync_data[1] = StepBias;
	sOpt0.Cfg->bit.sync_data[2] = StepBias;
	//DCDC,180deg Phase Shift
	sOpt0.Cfg->bit.sync_data[3] = StepBias;
	sOpt0.Cfg->bit.sync_data[4] = -StepBias;
	//
//	sOpt0.Cfg->bit.sync_data[5] = StepBias;
//	sOpt0.Cfg->bit.sync_data[6] = StepBias;
//	sOpt0.Cfg->bit.sync_data[7] = -StepBias;

	sOpt0.Cfg->bit.add_sub_flag1 = 0xFFEF;			//Bit4 ��
	sOpt0.Cfg->bit.add_sub_flag2 = 0x000F;
}

/*************************************************
 Function: PhShftCalc
 Description: �������ǲ����ڼ����������
 Input: TriPrd��Ϊ���ǲ���ֵ������<32768
 Output: ��
 Return: ��
 Others: //�ֱ����
 *************************************************/
void PEMCfg(void)
{
	u16 i;
	sOpt0.Cfg->bit.OVthr = H_OV_THR;			//��ѹ������
	sOpt0.Cfg->bit.UVthr = H_UV_THR;			//Ƿѹ������
	sOpt0.Cfg->bit.HBAmp = H_FRE_REG;			//H��Ƶ������
	sOpt0.Cfg->bit.ErrMsk = H_ERR_MSK;			//��������λ����

	sOpt0.Cfg->bit.CarSyncPrd = CAR_SYNC_PRD;	//�ز�ͬ�����ڣ�

	//ģ��ȫ�����ڸ�λ״̬
	for(i=0;i<20;i++)
	{
		sOpt0.Cmd->bit[i].Mode = HB_MODE_RST;
	}
	//ֱ������һ��
	CfgCard();
	usleep(1000);								//�ȴ��������
}

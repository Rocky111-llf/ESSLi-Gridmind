/*****************************************************************************
Copyright: 2019-2021, GridMind. Co., Ltd.
File name: VSC_GeneralCtl.c
Description: VSC基本控制
Author: RZ
Version: 版本
Date: 完成日期
History: 修改历史记录列表， 每条修改记录应包括修改日期、修改者及修改内容简述。
*****************************************************************************/

#include "sleep.h"
#include "Includings.h"

#define EN_PHASE_SHIFT	0

//移相参数计算
void PhShftCalc(s16 TriPrd);
//功率模块配置，不涉及参数下发
void PEMCfg(void);

/*************************************************
 Function: PhShftCalc
 Description: 根据三角波周期计算移相参数
 Input: TriPrd，为三角波峰值（正）<32768
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void PEMInit(void)
{
	PhShftCalc(H_FRE_REG);
	PEMCfg();
}

/*************************************************
 Function: PhShftCalc
 Description: 根据三角波周期计算移相参数
 Input: TriPrd，为三角波峰值（正）<32768
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void PhShftCalc(s16 TriPrd)
{
#if EN_PHASE_SHIFT
	u16 i;
	s16 StepTemp,StepBias;
	s16 SumTemp;

//N=4，相位偏移数据计算，这里仅有光纤卡0的配置
	StepTemp = (s16)((2.0/H_NUM)*(float)TriPrd);			//两电平没有相移
	StepBias = StepTemp/2;
	SumTemp = TriPrd-50;					//起始为三角波顶
	for(i=0;i<H_NUM;i++)
	{
		sOpt0.Cfg->bit.sync_data[i] = SumTemp;
		sOpt0.Cfg->bit.sync_data[i+(H_NUM*2)] = SumTemp;

		sOpt0.Cfg->bit.sync_data[i+H_NUM] = SumTemp-StepBias;
		sOpt0.Cfg->bit.sync_data[i+(H_NUM*3)] = SumTemp-StepBias;

		SumTemp -= StepTemp;
	}
//相位增减方向配置
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

	sOpt0.Cfg->bit.add_sub_flag1 = 0xFFEF;			//Bit4 ↑
	sOpt0.Cfg->bit.add_sub_flag2 = 0x000F;
}

/*************************************************
 Function: PhShftCalc
 Description: 根据三角波周期计算移相参数
 Input: TriPrd，为三角波峰值（正）<32768
 Output: 无
 Return: 无
 Others: //分别填充
 *************************************************/
void PEMCfg(void)
{
	u16 i;
	sOpt0.Cfg->bit.OVthr = H_OV_THR;			//过压点配置
	sOpt0.Cfg->bit.UVthr = H_UV_THR;			//欠压点配置
	sOpt0.Cfg->bit.HBAmp = H_FRE_REG;			//H桥频率配置
	sOpt0.Cfg->bit.ErrMsk = H_ERR_MSK;			//故障屏蔽位配置

	sOpt0.Cfg->bit.CarSyncPrd = CAR_SYNC_PRD;	//载波同步周期；

	//模块全部置于复位状态
	for(i=0;i<20;i++)
	{
		sOpt0.Cmd->bit[i].Mode = HB_MODE_RST;
	}
	//直接配置一次
	CfgCard();
	usleep(1000);								//等待配置完成
}

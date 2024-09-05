/****************************************************************************
*文件名   ：AlgoGen.c
*描述     ：
*硬件平台 ：
*软件版本 ：
*作者     ：Radio Zuo
*时间     ：
*修订者   ：
*修改时间 ：
*说明     ：
*****************************************************************************/

#ifndef _ALGOCOMM_H_
#define _ALGOCOMM_H_


#define FAST_MATH_TABLE_SIZE 1024

extern const float SINTBL[];
extern const float COSTBL[];


#define LPFCALC_MACRO		1

#if LPFCALC_MACRO
#define LPFProc(A) do{															\
		(A)->OutPre = (A)->In*(A)->Coff1 + (A)->OutPre*(A)->Coff2;				\
		(A)->OutPre1 = (A)->OutPre*(A)->Coff1 + (A)->OutPre1*(A)->Coff2;		\
		(A)->Out = (A)->OutPre1;												\
}while(0)
#endif

typedef struct
{
	float Ref;
	float FeedBack;
	float Err;
	float ErrPre;
	float Kp;
	float Ki;
	float Kd;
	float I;
	float Out;
	float	OutMax;
	float	OutMin;
	float	Tf;
	s16  UF;
}PID;

typedef struct
{
	float In;
	float Out;
	float AttCoff;				//1/e^(T/RC)
}PEAKDET;

typedef struct
{
	float STH;
	float STL;
	float RUNH;
	float RUNL;
	u16 StatusCntMax;
	u16 StatusCnt;
	u16 Result;
	u16 StatusPre;
	u16 Status;
}LIMIT;

typedef struct
{
	float In;
	float Out;
	float OutPre;
	float OutPre1;
	float Coff1;
	float Coff2;
}LPF;

//越限判断枚举定义
enum {
    LIMIT_STH = 1,
    LIMIT_STL = 2,
    LIMIT_RUNH = 3,
    LIMIT_RUNL = 4,
    LIMIT_OK = 0,
    LIMIT_ERR = 0xff
};

//最大最小及和结构体定义
typedef struct
{
    float Max;
    float Min;
    float Sum;
    float Avg;
    u16 IndexofMax;
    u16 IndexofMin;
}MAXMINSUM_t;

//最大最小及和结构体定义
typedef struct
{
    float Max;
    float Min;
}RANGEJUDGE_t;

void PIDProc(PID* l_PID);
void PIDProc_Int_Sepa(PID* l_PID);
void PIDProc_D(PID* l_PID);
void PeakDetect(PEAKDET *v);
#if !LPFCALC_MACRO
void LPFProc(LPF *v);
#endif
u16 LimitProc(LIMIT* limit1,float in,u16 Mode);
void LimitStatusReset(LIMIT* limit1,u16 StatusInit);
float FastSqrt(float number);
float FastSqrt_Int(float number);
float FastSqrt2(float number,float * ResultRecipr);
float FastSqrt2_Int(float number,float * ResultRecipr);
void arm_sin_cos_f32_1(float x,SINCOS_t *pSinCos);
//void LocalParasUpdate(tVSC_CTL* tVSCHandler);
//void GetOmegaTheta(tVSC_CTL* tVSCHandler);
void VcAvgCalc(void);
//void svgen(PARA2S *v);

#endif


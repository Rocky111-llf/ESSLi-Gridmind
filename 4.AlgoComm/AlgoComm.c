/****************************************************************************
*�ļ���   ��AlgoGen.c
*����     ��
*Ӳ��ƽ̨ ��
*����汾 ��
*����     ��Radio Zuo
*ʱ��     ��
*�޶���   ��
*�޸�ʱ�� ��
*˵��     ��
*****************************************************************************/
#include <stdio.h>
#include "Includings.h"
#include "math.h"
/****************************************************************************
*
*PID�������
*
****************************************************************************/
#define ERR_THRE 0.01f
void PIDProc(PID* l_PID)
{
/*
	float Ref;
	float FeedBack;
	float Err;
	float ErrPre;
	float Kp;
	float Ki;
	float Kd;
	double I;
	float Out;
	float	OutMax;
	float	OutMin;
	float	Tf;
	s8  UF;
*/
	// float Temp;
	// l_PID->ErrPre = l_PID->	Err;
	// l_PID->Err = l_PID->Ref - l_PID->FeedBack;
	// if(((l_PID->UF==-1)&&(l_PID->Err>0.0f))||((l_PID->UF==1)&&(l_PID->Err<0.0f))||l_PID->UF==0)
	// 	l_PID->I += l_PID->Err*l_PID->Ki;
	// Temp = (l_PID->Kp*(l_PID->Err*l_PID->Tf + l_PID->I));
	// if(Temp < l_PID->OutMin)
	// {
	// 	l_PID->Out = l_PID->OutMin;
	// 	l_PID->UF = -1;
	// }
	// else if(Temp > l_PID->OutMax)
	// {
	// 	l_PID->Out = l_PID->OutMax;
	// 	l_PID->UF = 1;
	// }
	// else
	// {
	// 	l_PID->Out = Temp;
	// 	l_PID->UF = 0;
	// }
	float Temp;
	//�������
    l_PID->ErrPre=l_PID->Err;
    l_PID->Err=l_PID->Ref-l_PID->FeedBack;
    //���ֿ��ƣ�����ʹ�û��ַ���ʱӦ�� 
	if(Ctl_VSC1.CtlMode == PQCTL){
		if(((l_PID->UF==-1)&&(l_PID->Err>0.0f))||((l_PID->UF==1)&&(l_PID->Err<0.0f))||l_PID->UF==0){
			if(fabs(l_PID->Err)>ERR_THRE){
				debug1+=1;
				l_PID->I+=l_PID->Err*l_PID->Ki; // ������������ֵʱ���ۻ�������
			}
		}
	}else{
		// ��ʹ�û��ַ������ʱ��ֱ���ۻ�������
		if(((l_PID->UF==-1)&&(l_PID->Err>0.0f))||((l_PID->UF==1)&&(l_PID->Err<0.0f))||l_PID->UF==0){
			l_PID->I+=l_PID->Err*l_PID->Ki;
		}
	}
	// ��ֹ�������
    if(l_PID->I>l_PID->OutMax){
        l_PID->I=l_PID->OutMax;
    }else if(l_PID->I<l_PID->OutMin){
        l_PID->I=l_PID->OutMin;
    }
	// ����������
    Temp=l_PID->Kp*(l_PID->Err*l_PID->Tf+l_PID->I);
	// ����޷�
    if(Temp<l_PID->OutMin){
        l_PID->Out=l_PID->OutMin;
        l_PID->UF=-1;
    }else if(Temp>l_PID->OutMax){
        l_PID->Out=l_PID->OutMax;
        l_PID->UF=1;
    }else{
        l_PID->Out=Temp;
        l_PID->UF=0;
    }
}

/****************************************************************************
*
*PID�������
*
****************************************************************************/
void PIDProc_D(PID* l_PID)
{
/*
	float Ref;
	float FeedBack;
	float Err;
	float ErrPre;
	float Kp;
	float Ki;
	float Kd;
	double I;
	float Out;
	float	OutMax;
	float	OutMin;
	float	Tf;
	s8  UF;
*/
	float Temp;
	l_PID->ErrPre = l_PID->	Err;
	l_PID->Err = l_PID->Ref - l_PID->FeedBack;
	if(((l_PID->UF==-1)&&(l_PID->Err>0.0f))||((l_PID->UF==1)&&(l_PID->Err<0.0f))||l_PID->UF==0)
		l_PID->I += l_PID->Err*l_PID->Ki;
	Temp = (l_PID->Kp*(l_PID->Err*l_PID->Tf + l_PID->I) + l_PID->Kd*(l_PID->Err-l_PID->ErrPre));
	if(Temp < l_PID->OutMin)
	{
		l_PID->Out = l_PID->OutMin;
		l_PID->UF = -1;
	}
	else if(Temp > l_PID->OutMax)
	{
		l_PID->Out = l_PID->OutMax;
		l_PID->UF = 1;
	}
	else
	{
		l_PID->Out = Temp;
		l_PID->UF = 0;
	}
}

/****************************************************************************
*
*��ֵ�첨����
*
****************************************************************************/
void PeakDetect(PEAKDET *v)
{
	if(v->In > v->Out)
		v->Out = v->In;
	else
		v->Out *= v->AttCoff;
	if(v->Out > 1.0f)
		v->Out = 1.0f;
}

/****************************************************************************
*
*��ͨ�˲���
*
****************************************************************************/
#if !(LPFCALC_MACRO)
void LPFProc(LPF *v)
{
	v->Out = v->In*v->Coff1 + v->OutPre*v->Coff2;
	v->OutPre = v->Out;
}
#endif

/*************************************************
 Function: LPFProc
 Description: ��ͨ�˲�����λ
 Input: LPF*
 Output: None
 Return: None
 Others: None
 *************************************************/
void LPFReset(LPF *v)
{
	v->OutPre = 0.0f;
	v->OutPre1 = 0.0f;
	v->Out = 0.0f;
}

/****************************************************************************
*
*��ֵ�жϳ���
*
****************************************************************************/
u16 LimitProc(LIMIT* limit1,float in,u16 Mode)
{
	//�ж�ʵʱStatus�����ݵ�ǰ����������������״̬
	    if(Mode == START)
	    {//����״̬
	        if(in > limit1->STH)
	            limit1->Status = LIMIT_STH;
	        else if(in < limit1->STL)
	            limit1->Status = LIMIT_STL;
	        else
	            limit1->Status = LIMIT_OK;
	    }
	    else if(Mode == RUN)
	    {//����״̬
	        if(in > limit1->RUNH)
	            limit1->Status = LIMIT_RUNH;
	        else if(in < limit1->RUNL)
	            limit1->Status = LIMIT_RUNL;
	        else
	            limit1->Status = LIMIT_OK;
	    }
	    else
	        limit1->Status = LIMIT_ERR;

	    if(limit1->Status != limit1->Result)					//���Ŀǰ�����������������ڵ���������˸ı�
	    {
	    	if(limit1->Status != limit1->StatusPre)				//�����ǰ״̬����һ������״̬�����˱仯
	    	{
	    		limit1->StatusCnt = 0;							//�ۻ�״̬����
	    	}
	    	else												//�����ǰ״̬����һ������״̬�����˱仯
	    	{
	    		if(limit1->StatusCnt < limit1->StatusCntMax)	//�����״̬������ʱ�������ֵ
				{
					limit1->StatusCnt++;						//������ֵ
				}
				else											//������ֵ
				{
					limit1->Result = limit1->Status;			//��״̬���õ����״̬
				}
	    	}
	    }
	    limit1->StatusPre = limit1->Status;
	    return limit1->Result;
}

/*************************************************
 Function: LimitStatusReset
 Description: ��ֵ����״̬����
 Input: LIMIT*
 Output: None
 Return: None
 Others: None
 *************************************************/
void LimitStatusReset(LIMIT* limit1,u16 StatusInit)
{
//	limit1->Status = LIMIT_OK;
	limit1->StatusCnt = 0;
	limit1->StatusPre = StatusInit;
	limit1->Result = StatusInit;
}

/*************************************************
 Function: FastSqrt
 Description: ���ٿ���,������
 Input: x,�����ȸ���
 Output: None
 Return: sqrt(x),�����ȸ���
 Others: None
 *************************************************/
float FastSqrt(float number)
{
    long i;
    float x, y;
    const float f = 1.5F;
    x = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    y  = y * ( f - ( x * y * y ) );
    return number * y;
}
/*************************************************
 Function: FastSqrt
 Description: ���ٿ���,������
 Input: x,�����ȸ���
 Output: None
 Return: sqrt(x),�����ȸ���
 Others: None
 *************************************************/
float FastSqrt2(float number,float * ResultRecipr)
{
    long i;
    float x, y;
    const float f = 1.5F;
    x = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    y  = y * ( f - ( x * y * y ) );
    *ResultRecipr = y;						//�������
    return number * y;						//���
}

/****************************************************************************
*
*ͬʱ����sin��cos������arm_sin_f32��arm_sin_f32
*
****************************************************************************/
void arm_sin_cos_f32_1(float x,SINCOS_t *pSinCos)
{
  float sinVal, cosVal, fract, in;                   /* Temporary input, output variables */
  u16 index, index_cos;                             /* Index variable */
  float a, b, c, d;                                  /* "Two" nearest output values */
  s32 n;
  float findex;

  /* input x is in radians */
  /* Scale input to [0 1] range from [0 2*PI] , divide input by 2*pi */
  in = x * 0.159154943092f;

  /* Calculation of floor value of input */
  n = (int32_t) in;

  /* Make negative values towards -infinity */
  if (in < 0.0f)
  {
    n--;
  }

  /* Map input value to [0 1] */
  in = in - (float) n;

  /* Calculation of index of the table */
  findex = (float)FAST_MATH_TABLE_SIZE * in;
  index = (u16)findex;

  /* when "in" is exactly 1, we need to rotate the index down to 0 */
  if (index >= FAST_MATH_TABLE_SIZE) {
    index = 0;
    findex -= (float)FAST_MATH_TABLE_SIZE;
  }

  /* fractional value calculation */
  fract = findex - (float) index;

  /* Read two nearest values of input value from the sin table */
  a = SINTBL[index];
  b = SINTBL[index+1];

  index_cos = index + (FAST_MATH_TABLE_SIZE>>2);			//ƫ��2PI/4
  if(index_cos >= FAST_MATH_TABLE_SIZE)
	  index_cos -= FAST_MATH_TABLE_SIZE;
  c = SINTBL[index_cos];
  d = SINTBL[index_cos+1];

  /* Linear interpolation process */
  sinVal = (1.0f - fract) * a + fract * b;
  cosVal = (1.0f - fract) * c + fract * d;

  pSinCos->Cos = cosVal;
  pSinCos->Sin = sinVal;
}

/*************************************************
 Function: GetOmegaTheta
 Description: ���໷
 Input: �������ྲֹ����ϵ
 Output: Omega��Theta
 Return: None
 Others: None
 *************************************************/
void GetOmegaTheta(tVSC_CTL* tVSCHandler)
{
	arm_sin_cos_f32_1(tVSCHandler->Theta,&tVSCHandler->Theta_GridSincos);
	tVSCHandler->ThetaPID.Ref = tVSCHandler->UGrid.P2S.beta*tVSCHandler->Theta_GridSincos.Cos;
	tVSCHandler->ThetaPID.FeedBack = tVSCHandler->UGrid.P2S.alpha*tVSCHandler->Theta_GridSincos.Sin;
	PIDProc(&tVSCHandler->ThetaPID);

	tVSCHandler->Omega = tVSCHandler->ThetaPID.Out;					//PID���ΪOmega
	tVSCHandler->Theta += tVSCHandler->Omega;							//�ۼӵõ����
    if(tVSCHandler->Theta > THETAMAX)
    {
    	tVSCHandler->Theta -= THETAMAX;
	}
	else if(tVSCHandler->Theta < THETAMIN)
		tVSCHandler->Theta += THETAMAX;

    tVSCHandler->PLLFre = (1.0f*(PLLFRE_ARRAY_SIZE-1)/PLLFRE_ARRAY_SIZE)*tVSCHandler->PLLFre + (1.0f/THETAMAX/PLLFRE_ARRAY_SIZE)*(INTFRE*tVSCHandler->Omega);
}

/*************************************************
 Function: svgen
 Description: SVPWM����
 Input: ���ྲֹ����ϵ��ѹָ��
 Output: tmr1��tmr2��tmr3������PWM�Ĵ���
 Return: None
 Others: None
 *************************************************/
void svgen(tVSC_CTL* tVSCHandler)
{
	float Ta,Tb,Tc;
	float U1,U2;
	float X,Y,Z;
	u16 sector=0;                                               // sector=a+2b+4c ����״̬��ʾ ע�⣺setor��ֵ1~6����������˳���Ӧ
	U1 = sqrt3 * tVSCHandler->UConv.P2S.alpha;
	U2 = -tVSCHandler->UConv.P2S.beta;
	X = U2;
	Y = (U1 + U2)*0.5f;
	Z = (U2 - U1)*0.5f;
	if(Y<0)
	{
		if(Z<0) sector = 5;
		else
		{
			if(X>0) sector = 3;
			else sector = 4;
		}
	}
	else
	{

		if(Z<0)
		{
			if(X>0) sector = 1;
			else sector = 6;
		}
		else sector = 2;
	}

   switch(sector){						//Sector 1 == 4 2==5 3==6 ,combine the cases will be OK
     case 1:
       Ta = (1.0f-Y)*0.5f;
	   Tc = Ta-Z;
	   Tb = 1.0f-Ta;
	   break;
     case 2:
	   Tc = (1.0f-X)*0.5f;
	   Ta = Tc+Z;
	   Tb = 1.0f-Tc;
       break;
     case 3:
	   Tc = (1.0f-Z)*0.5f;
	   Tb = Tc+X;
	   Ta = 1.0f-Tc;
	   break;
	 case 4:
       Tb = (1.0f+Y)*0.5f;
	   Tc = Tb-X;
	   Ta = 1.0f-Tb;
	   break;
	 case 5:
       Tb = (1.0f+X)*0.5f;
	   Ta = Tb-Y;
	   Tc = 1.0f-Tb;
       break;
     case 6:
       Ta = (1.0f+Z)*0.5f;
       Tb = Ta+Y;
       Tc = 1.0f-Ta;
       break;
     default:							//sector=0��sector=7ʱ����
       Ta=0.5f;
       Tb=0.5f;
       Tc=0.5f;
   }
   Ta = (0.5f-Ta)*2.0f;					//���㵽-1~1
   Tb = (0.5f-Tb)*2.0f;
   Tc = (0.5f-Tc)*2.0f;
   HardLimit(Ta,-1,1);
   HardLimit(Tb,-1,1);
   HardLimit(Tc,-1,1);
   tVSCHandler->PWM_A = Ta *H_FRE_REG;
   tVSCHandler->PWM_B = Tb *H_FRE_REG;
   tVSCHandler->PWM_C = Tc *H_FRE_REG;
}



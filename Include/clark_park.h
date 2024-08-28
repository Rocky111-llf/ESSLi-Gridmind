#ifndef _CLARK_PARK_H_
#define _CLARK_PARK_H_

typedef struct
{
   float as;                // ���ֹ������ϵ���ź�
   float bs;           
   float alpha;             // ���ֹ����ϵ���ź�
   float beta;
}CLARKE;

typedef struct
{
   float alpha;           // ����ֹƽ������ϵ�µ�ѹ�ź�
   float beta;
   float ang;             // �����Ƕ� �����Ƕ�=��е�Ƕ�*������
   float d;              // ��ת����ϵ�µ�ѹ�ź�
   float q;
}PARK;

//���Һ����ҽṹ�嶨��
typedef struct
{
    float Sin;
    float Cos;
}SINCOS_t;

typedef struct
{
	float a;
	float b;
	float c;
}PARA3S;

typedef struct
{
	float alpha;
	float beta;
}PARA2S;

typedef struct
{
	float d;
	float q;
}PARA2R;

typedef struct
{
	PARA3S P3S;
	PARA2S P2S;
	PARA2R P2R;
}PARA3S2S2R;

extern void clarke(PARA3S *v1,PARA2S *v2);
extern void iclarke(PARA2S *v1,PARA3S *v2);
extern void park(PARA2S *v1,PARA2R *v2,SINCOS_t *SC);
extern void ipark(PARA2R *v1,PARA2S *v2,SINCOS_t *SC);
extern void _3sto2r(PARA3S *v1,PARA2R *v2,SINCOS_t *SC);
extern void _2rto3s(PARA2R *v1,PARA3S *v2,SINCOS_t *SC);

#endif


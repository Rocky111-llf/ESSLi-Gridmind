#ifndef _CLARK_PARK_H_
#define _CLARK_PARK_H_

typedef struct
{
   float as;                // 三项静止坐标轴系下信号
   float bs;           
   float alpha;             // 两项静止坐标系下信号
   float beta;
}CLARKE;

typedef struct
{
   float alpha;           // 两静止平面坐标系下电压信号
   float beta;
   float ang;             // 电气角度 电气角度=机械角度*极对数
   float d;              // 旋转坐标系下电压信号
   float q;
}PARK;

//正弦和余弦结构体定义
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


//***************************************************/
//�ļ�����clarke_park.c
//���ܣ������˱任�����˱仯�Ӻ���
//˵����������Ϣ���ýṹ�壬ʹ��ʱ�ı�ṹ��ָ�뼴�ɸı�������
//     ����park����ʵ�����˱仯
//     ����clarke����ʵ�ֿ����˱仯
//****************************************************/


#include "float.h"
#include "clark_park.h"



//3S to 2S
void clarke(PARA3S *v1,PARA2S *v2)
{
	v2->alpha = v1->a;
	v2->beta = (v1->a + 2.0f * v1->b)*(0.57735026918963f);       // 1/sqrt(3) = 0.57735026918963
}
//2S to 3S
void iclarke(PARA2S *v1,PARA3S *v2)
{
	v2->a = v1->alpha;
	v2->b = 0.5f*(1.7320508075*v1->beta - v1->alpha);
	v2->c = -v2->a - v2->b;
}
//2S to 2R
void park(PARA2S *v1,PARA2R *v2,SINCOS_t *SC)
{
	v2->d=v1->alpha*SC->Cos + v1->beta*SC->Sin;      //�õ�������ת����ϵ��d���ѹ
	v2->q=v1->beta*SC->Cos - v1->alpha*SC->Sin;      //�õ�������ת����ϵ��q���ѹ
}
//2R to 2S
void ipark(PARA2R *v1,PARA2S *v2,SINCOS_t *SC)
{ 
	v2->alpha=v1->d*SC->Cos-v1->q*SC->Sin;      //�õ���ֹƽ������ϵ��d���ѹ
	v2->beta=v1->q*SC->Cos+v1->d*SC->Sin;      //�õ���ֹƽ������ϵ��q���ѹ
}
//3S to 2R
void _3sto2r(PARA3S *v1,PARA2R *v2,SINCOS_t *SC)
{
    float fbeta= (v1->a + 2.0f * v1->b)*(0.57735026918963f);
    v2->d = v1->a*SC->Cos + fbeta*SC->Sin;
    v2->q = fbeta*SC->Cos - v1->a*SC->Sin;
}
//2R to 3S
void _2rto3s(PARA2R *v1,PARA3S *v2,SINCOS_t *SC)
{
    v2->a = v1->d*SC->Cos-v1->q*SC->Sin;
    v2->b = 0.5f*(1.7320508075f*(v1->q*SC->Cos+v1->d*SC->Sin) - v2->a);
    v2->c = -v2->a - v2->b;
}



#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>  
#define ALL 100000
int main ()
{
    pid_t fpid; 
    while (1)
    {
        srand(time(0));
	double X[ALL],Y[ALL];
	int inround=0,index;
	for(index=0;index<ALL;index++)
	{
    
		X[index]=(double)rand()/(double)RAND_MAX;
		Y[index]=(double)rand()/(double)RAND_MAX;
		if(X[index]*X[index]+Y[index]*Y[index]<=1)
			inround++;
	/*	printf("%lf 	%lf\n",X[index],Y[index]); */
	}
	//计算平均数一定程度上检验生成的随机数是否合理// 
	for(index=1;index<ALL;index++)
	{
    
		X[0]+=X[index];
		Y[0]+=Y[index];
	}
    }
    
	
	return 0;	
}
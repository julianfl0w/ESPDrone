#define MAX 20
#include "driver/qmath.h"
double intcos(int n, double x)
{
    if( n > MAX )
         return 1;
    return 1-x*x/( (2*n-1)*2*n ) * intcos(n+1, x); 
}

double intsin(int n, double x)
{
    if( n > MAX )
         return 1;
    return x - x*x*intsin(n+1, x)/((2*n+1)*2*n);
}

double intarcsin(int n, double x)
{   
    if( n > MAX )
         return 0;
    double res = x*x*intarcsin(n+1,x)*(n*2-1)*(n*2-1)/((n*2)*(n*2+1));
    return x + res;
}

double qcos(double x){
	return intcos(1, x);
}

double qsin(double x){
	return intsin(1, x);
}

double qarcsin(double x)
{
	return intarcsin(1, x);
}

double qarccos(double x){ 
    return 1.57079632679 - qarcsin(x); 
}


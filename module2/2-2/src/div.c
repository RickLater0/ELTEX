#include <math.h>

double calc(double a, double b){
    if(b != 0)
        return a / b;
    return NAN;
}
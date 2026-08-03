#include <math.h>

double calc(double a, double b){
    if(b != 0)
        return fmod(a, b);
    return NAN;
}
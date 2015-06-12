#ifndef __PWMMotor_h__
#define __PWMMotor_h__
#include <string>
#include "mbed.h"

void init_pwms();
void write_pwm(float aa, float ab, float ba, float bb, int delay);

#endif

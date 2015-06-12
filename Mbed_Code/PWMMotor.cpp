#include "PWMMotor.h"

PwmOut AA(A5);
PwmOut AB(A4);
PwmOut BA(D3);
PwmOut BB(D9);

//PWM output channel
void init_pwms()
{
    AA.period_ms(20.0f);
    AB.period_ms(20.0f);
    BA.period_ms(20.0f);
    BB.period_ms(20.0f);
    
    AA.write(0.00f);   
    AB.write(0.00f);   
    BA.write(0.00f);   
    BB.write(0.00f); 
}
void write_pwm(float aa, float ab, float ba, float bb, int delay)
{
    std::fprintf(stderr, "write_pwm");
    AA.write(aa);   
    AB.write(ab);   
    BA.write(ba);   
    BB.write(bb);
    wait_ms(delay);
    AA.write(0.00f);   
    AB.write(0.00f);   
    BA.write(0.00f);   
    BB.write(0.00f); 
}

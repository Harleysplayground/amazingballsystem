#include "flexmotor.h"
#include <stdio.h>
#include <stdlib.h>
#include <p33Fxxxx.h>
#include <libpic30.h>
#include <stdint.h>

void motor_init(int chan) {
    //Timer OC8
    T2CONbits.TON = 0;
    T2CONbits.TCS = 0;
    T2CONbits.TGATE = 0;
    TMR2 = 0x00;
    T2CONbits.TCKPS = 0b10;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 0;
    PR2 = 4000; //20ms


}

void motor_set_duty(int chan, int duty_us) {

    //    int duty_x = duty_us / 5;
    int duty_y = 4000 - duty_us / 5;
    if (chan) { //OC7
        TRISDbits.TRISD7 = 0;
        OC7R = duty_y; //5ms
        OC7RS = duty_y;
        OC7CON = 0x0006;
        T2CONbits.TON = 1;
    } else { //OC8
        TRISDbits.TRISD7 = 0;
        OC8R = duty_y; //5ms
        OC8RS = duty_y;
        OC8CON = 0x0006;
        T2CONbits.TON = 1;
    }

}
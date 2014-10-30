#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>
#include "lcd.h"

#include "flextouch.h"

uint8_t current_pin;

void touch_init() {
    CLEARBIT(TRISEbits.TRISE1); //I/O pin set to output
    CLEARBIT(TRISEbits.TRISE2); //I/O pin set to output
    CLEARBIT(TRISEbits.TRISE3); //I/O pin set to output
}

void touch_select_dim(uint8_t dim) {
    if (dim == 0) {
        CLEARBIT(LATEbits.LATE1);
        SETBIT(LATEbits.LATE2);
        SETBIT(LATEbits.LATE3);
        current_pin = 15;
    } else {
        SETBIT(LATEbits.LATE1);
        CLEARBIT(LATEbits.LATE2);
        CLEARBIT(LATEbits.LATE3);
        current_pin = 9;
    }
}

uint16_t touch_adc() {
    AD1CHS0bits.CH0SA = current_pin; //set ADC to Sample pin
    SETBIT(AD1CON1bits.SAMP); //start to sample
    while (!AD1CON1bits.DONE); //wait for conversion to finish
    CLEARBIT(AD1CON1bits.DONE);

    return ADC1BUF0;
}
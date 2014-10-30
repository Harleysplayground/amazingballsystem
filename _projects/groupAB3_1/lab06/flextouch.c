#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>
#include "lcd.h"

#include "flextouch.h"

uint8_t current_pin;

void touch_init() {
    CLEARBIT(AD1CON1bits.ADON);
    SETBIT(TRISBbits.TRISB15); //set TRISE RE8 to input
    SETBIT(TRISBbits.TRISB9); //set TRISE RE8 to input
    CLEARBIT(AD1PCFGLbits.PCFG15); //set AD1 AN20 input pin as analog
    CLEARBIT(AD1PCFGLbits.PCFG9); //set AD1 AN20 input pin as analog
    CLEARBIT(AD1CON1bits.AD12B); //set 10b Operation Mode
    AD1CON1bits.FORM = 0; //set integer output
    AD1CON1bits.SSRC = 0x7; //set automatic conversion
    AD1CON2 = 0; //not using scanning sampling
    CLEARBIT(AD1CON3bits.ADRC); //internal clock source
    AD1CON3bits.SAMC = 0x1F; //sample-to-conversion clock = 31Tad
    AD1CON3bits.ADCS = 0x2; //Tad = 3Tcy (Time cycles)
    //Leave AD1CON4 at its default value
    SETBIT(AD1CON1bits.ADON);

    CLEARBIT(TRISEbits.TRISE1); //I/O pin set to output
    CLEARBIT(TRISEbits.TRISE2); //I/O pin set to output
    CLEARBIT(TRISEbits.TRISE3); //I/O pin set to output
}

void touch_select_dim(uint8_t dim) {
    if (dim == 0) { // for x
        CLEARBIT(LATEbits.LATE1);
        SETBIT(LATEbits.LATE2);
        SETBIT(LATEbits.LATE3);
        current_pin = 15;
    } else { // for y
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
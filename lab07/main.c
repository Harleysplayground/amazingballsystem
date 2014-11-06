#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "flextouch.h"
#include "flexmotor.h"

#define LED4_TRIS TRISAbits.TRISA10
#define LED4_PORT PORTAbits.RA10
#define LED4 10

#define LED1_TRIS TRISAbits.TRISA4
#define LED1_PORT PORTAbits.RA4
#define LED1 4

#define LED2_TRIS TRISAbits.TRISA5
#define LED2_PORT PORTAbits.RA5
#define LED2 5

#define LED3_TRIS TRISAbits.TRISA9
#define LED3_PORT PORTAbits.RA9
#define LED3 9

#define TRIGGER_THRESH 10000


/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);

/* global variables */
uint16_t xpos;
double Kp, Ki, Kd;
double previous_error = 0;
double integral = 0;
double derivative;
double output;

void __attribute__((__interrupt__)) _T3Interrupt() {
    xpos = touch_adc();
    double error = 450.0 - xpos;
    integral += error * 0.05;
    derivative = (error - previous_error) / 0.05;
    output = Kp * error + Ki * integral + Kd * derivative;
    previous_error = error;

    int duty = 1500 + output * 1.5;
    motor_set_duty(0, duty);

    TOGGLELED(LED1_PORT);
    IFS0bits.T3IF = 0;
}

void main() {
    //Init LCD
    __C30_UART = 1;
    lcd_initialize();
    lcd_clear();

    CLEARLED(LED4_TRIS);
    CLEARLED(LED1_TRIS);
    CLEARLED(LED2_TRIS);
    CLEARLED(LED3_TRIS);

    lcd_locate(0, 0);
    lcd_printf("Group No.1");

    touch_init();
    touch_select_dim(0);
    motor_init(0);
    motor_set_duty(1, 1800);

    Kp = 1.0;
    Ki = 0.0;
    Kd = 0.3;

    /* timer2 to 50 ms */
    T3CONbits.TON = 0;
    T3CONbits.TCS = 0;
    T3CONbits.TGATE = 0;
    TMR3 = 0;
    T3CONbits.TCKPS = 0b11;
    PR3 = 2500;
    IPC2bits.T3IP = 0x01;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    T3CONbits.TON = 1;

    int tick = 0;

    lcd_locate(0, 1);
    lcd_printf("Kp = %2.1f ", Kp);
    lcd_printf("Ki = %2.1f ", Ki);
    lcd_locate(0, 2);
    lcd_printf("Kd = %2.1f ", Kd);
    lcd_locate(0, 3);
    lcd_printf("X set to %04u", 450);

    while (1) {
        if (++tick == 10000) {
            tick = 0;
            lcd_locate(0, 4);
            lcd_printf("Xpos = %04u", xpos);
            lcd_locate(0, 5);
            lcd_printf("D_x = %7.3f", derivative);
            lcd_locate(0, 6);
            lcd_printf("I_x = %7.3f", integral);
            lcd_locate(0, 7);
            lcd_printf("F_x = %7.3f", output);
        }
    }
}


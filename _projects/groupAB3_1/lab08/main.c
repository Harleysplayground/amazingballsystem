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
double previous_error[2] = {0, 0};
double integral = 0;
double derivative;
double output;
int dim = 1;
double motor[2] = {0.0, 0.0};
uint8_t button_A_state;
uint16_t button_A_state_count = 0;
uint8_t stage = 0;

void debounce() {
    /* debouncing algorithm */
    if (PORTEbits.RE8 == button_A_state) {
        button_A_state_count = 0;
    } else {
        if (++button_A_state_count > TRIGGER_THRESH) {
            /* flipping the state!!! */
            button_A_state = PORTEbits.RE8;
            button_A_state_count = 0;
        }
    }

    if (button_A_state == 0) {
        SETLED(LED1_PORT);
    } else {
        CLEARLED(LED1_PORT);
    }
}

unsigned int adc(uint8_t port) {
    AD2CHS0bits.CH0SA = port; //set ADC to Sample AN20 pin
    SETBIT(AD2CON1bits.SAMP); //start to sample
    while (!AD2CON1bits.DONE); //wait for conversion to finish
    CLEARBIT(AD2CON1bits.DONE);

    if (button_A_state == 0) stage++;
    return ADC2BUF0;
}

void joystick() {


    /* -------- INIT ADC -------- */
    AD1PCFGHbits.PCFG20 = 1;
    TRISEbits.TRISE8 = 1;
    TRISDbits.TRISD10 = 1;

    button_A_state = PORTEbits.RE8; //debouncing

    CLEARBIT(AD2CON1bits.ADON);
    SETBIT(TRISBbits.TRISB4); //set TRISE RE8 to input
    SETBIT(TRISBbits.TRISB5); //set TRISE RE8 to input
    CLEARBIT(AD2PCFGLbits.PCFG4); //set AD1 AN20 input pin as analog
    CLEARBIT(AD2PCFGLbits.PCFG5); //set AD1 AN20 input pin as analog
    CLEARBIT(AD2CON1bits.AD12B); //set 10b Operation Mode
    AD2CON1bits.FORM = 0; //set integer output
    AD2CON1bits.SSRC = 0x7; //set automatic conversion
    AD2CON2 = 0; //not using scanning sampling
    CLEARBIT(AD2CON3bits.ADRC); //internal clock source
    AD2CON3bits.SAMC = 0x1F; //sample-to-conversion clock = 31Tad
    AD2CON3bits.ADCS = 0x2; //Tad = 3Tcy (Time cycles)
    //Leave AD1CON4 at its default value
    SETBIT(AD2CON1bits.ADON);

    //int tick;
    unsigned int min_x, max_x, min_y, max_y, tick;
    int duty_x, duty_y;
    while (stage < 11) {
        debounce();
        if (++tick == 10000) {
            TOGGLELED(LED2_PORT);

            if (stage == 0) {
                //MUST HAVE! clear conversion done bit
                max_x = adc(4);
                lcd_locate(0, 1);
                lcd_printf("X max: %u", max_x);

            } else if (stage == 1 && button_A_state == 1) stage++;

            else if (stage == 2) {
                min_x = adc(4);
                lcd_locate(0, 2);
                lcd_printf("X min: %u", min_x);

            } else if (stage == 3 && button_A_state == 1) stage++;

            if (stage == 4) {
                max_y = adc(5);
                lcd_locate(0, 3);
                lcd_printf("Y max: %u", max_y);

            } else if (stage == 5 && button_A_state == 1) stage++;

            else if (stage == 6) {
                min_y = adc(5);
                lcd_locate(0, 4);
                lcd_printf("Y min: %u", min_y);

            } else if (stage == 7 && button_A_state == 1) stage++;


            else if (stage == 8) {
                motor[0] = 1.0 * adc(4);
                //TODO: Calculation

                motor[0] = (motor[0] - min_x)*685.0/(max_x - min_x) + 73.0;

                lcd_locate(0, 5);

                lcd_printf("Duty X: %04u", motor[0]);

            } else if (stage == 9 && button_A_state == 1) stage++;

            else if (stage == 10) {
                motor[1] = 1.0 * adc(5);
                //TODO: Calculation

                motor[1] = (motor[1] - min_y)*574.0/(max_y - min_y) + 104.0;

                lcd_locate(0, 6);
                lcd_printf("Duty Y: %04u", motor[1]);

            } else if (stage == 11 && button_A_state == 1) stage++;

        }
    }
}

void __attribute__((__interrupt__)) _T3Interrupt() {
    // touch_select_dim(dim ^= 1);

    xpos = touch_adc();
    double error = motor[dim] - xpos;
    integral += error * 0.05;
    derivative = (error - previous_error[dim]) / 0.05;
    output = Kp * error + Ki * integral + Kd * derivative;
    previous_error[dim] = error;

    int duty = 1500 + output * 1.5;
    motor_set_duty(dim, duty);
    touch_select_dim(dim ^= 1);


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

    //   lcd_locate(0, 0);
    //  lcd_printf("Group No.1");
    joystick();

    touch_init();
    touch_select_dim(1);
    motor_init(0);
    // motor_set_duty(1200, 1200);

    Kp = 0.4;
    Ki = 0.0;
    Kd = 0.2;



    /* timer3 to 50 ms */
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
    /*
        lcd_locate(0, 1);
        lcd_printf("Kp = %2.1f ", Kp);
        lcd_printf("Ki = %2.1f ", Ki);
        lcd_locate(0, 2);
        lcd_printf("Kd = %2.1f ", Kd);
        lcd_locate(0, 3);
        lcd_printf("X set to %04u", 450);
     */
    while (1) {
        if (++tick == 10000) {
            tick = 0;
            // touch_select_dim(dim ^= 1);
            /*lcd_locate(0, 4);
            lcd_printf("Xpos = %04u", xpos);
            lcd_locate(0, 5);
            lcd_printf("D_x = %7.3f", derivative);
            lcd_locate(0, 6);
            lcd_printf("I_x = %7.3f", integral);
            lcd_locate(0, 7);
            lcd_printf("F_x = %7.3f", output);*/
        }
    }
}


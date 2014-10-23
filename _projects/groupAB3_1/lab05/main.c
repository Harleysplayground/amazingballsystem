#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>
#include "flexmotor.h"
#include "lcd.h"

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

void main() {
    //Init LCD
    __C30_UART = 1;
    lcd_initialize();
    lcd_clear();

    CLEARLED(LED4_TRIS);
    CLEARLED(LED1_TRIS);
    CLEARLED(LED2_TRIS);
    CLEARLED(LED3_TRIS);

    AD1PCFGHbits.PCFG20 = 1;
    TRISEbits.TRISE8 = 1;
    TRISDbits.TRISD10 = 1;

    button_A_state = PORTEbits.RE8; //debouncing

    lcd_locate(0, 0);
    lcd_printf("Group No.1");

    /* -------- INIT ADC -------- */

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

    int tick;
    unsigned int min_x, max_x, min_y, max_y, motor_x, motor_y;
    int duty_x, duty_y;
    while (1) {
        debounce();

        if (++tick == 10000) {

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
                motor_x = adc(4);
                //TODO: Calculation

                duty_x = 900 + ((float)motor_x - min_x)*1200/(max_x - min_x);
                motor_init(0);
                motor_set_duty(1,duty_x);


               
                lcd_locate(0, 5);

                lcd_printf("Duty X: %04u", duty_x);

            } else if (stage == 9 && button_A_state == 1) stage++;

            else if (stage == 10) {
                motor_y = adc(5);
                //TODO: Calculation
                
                duty_y = 900 + ((float)motor_y - min_y)*1200/(max_y - min_y);
                motor_set_duty(0,duty_y);


                lcd_locate(0, 6);
                lcd_printf("Duty Y: %04u", duty_y);

            } else if (stage == 11 && button_A_state == 1) stage++;

            tick = 0;
        }
    }




 /*   int duty = 900;// (900 - 2100)


    
    motor_init(0);
    motor_set_duty(1,duty);
    while(1);*/
}

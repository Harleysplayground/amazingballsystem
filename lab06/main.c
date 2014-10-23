#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "flextouch.h"

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

void main() {
    //Init LCD
    __C30_UART = 1;
    lcd_initialize();
    lcd_clear();
    lcd_locate(0, 0);
    lcd_printf("Group No.1");

    /* -------- INIT ADC -------- */

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

    touch_init();
    touch_select_dim(1);

    int tick = 0;

    while (1) {
        if (++tick == 20000) {
            tick = 0;
            lcd_locate(0, 1);
            lcd_printf("chan: %05d", touch_adc());
        }
    }
}

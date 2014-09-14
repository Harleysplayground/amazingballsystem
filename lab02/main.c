#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

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


/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);

uint16_t minute = 0;
uint16_t second = 0;
uint16_t msecond = 0;

void __attribute__((__interrupt__)) _T2Interrupt() {
    TOGGLELED(LED1_PORT);
    msecond += 2;
    if (msecond >= 1000) {
        msecond = 0;
        second++;
        if (second >= 60) {
            second = 0;
            minute++;
        }
    }
    IFS0bits.T2IF = 0;
}

void __attribute__((__interrupt__)) _T1Interrupt() {
    TOGGLELED(LED2_PORT);
    IFS0bits.T1IF = 0;
}

void __attribute__((__interrupt__)) _INT1Interrupt() {
    msecond = 0;
    second = 0;
    minute = 0;
    IFS1bits.INT1IF = 0;
}

void main() {
    //Init LCD
    __C30_UART = 1;
    lcd_initialize();
    lcd_clear();
    lcd_locate(0, 0);
    lcd_printf("Timer: 00:00.000");

    CLEARLED(LED4_TRIS);
    CLEARLED(LED1_TRIS);
    CLEARLED(LED2_TRIS);

    uint16_t tick = 0;

    /* Trigger & interrupt */
    AD1PCFGHbits.PCFG20 = 1;
    TRISEbits.TRISE8 = 1;
    INTCON2bits.INT1EP = 1;
    IPC5bits.INT1IP = 1;
    IFS1bits.INT1IF = 0;
    IEC1bits.INT1IE = 1;

    /* LED1 on Timer2*/
    T2CONbits.TON = 0;
    T2CONbits.TCS = 0;
    T2CONbits.TGATE = 0;
    TMR2 = 0;
    T2CONbits.TCKPS = 0b11;
    PR2 = 100;
    IPC1bits.T2IP = 0x01;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    T2CONbits.TON = 1;

    /* LED2 on Timer1 */
    __builtin_write_OSCCONL(OSCCONL | 2);
    T1CONbits.TON = 0;
    T1CONbits.TCS = 1;
    T1CONbits.TSYNC = 0;
    T1CONbits.TCKPS = 0b00;
    TMR1 = 0;
    PR1 = 32767;
    IPC0bits.T1IP = 0x02;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CONbits.TON = 1;

    /* Timer3 */
    T3CONbits.TON = 0;
    T3CONbits.TCS = 0;
    T3CONbits.TGATE = 0;
    TMR3 = 0;
    T3CONbits.TCKPS = 0;
    PR3 = 65535;
    T3CONbits.TON = 1;

    while (1) {
        TOGGLELED(LED4_PORT);

        if (++tick == 2000) {
            tick = 0;
            lcd_locate(0, 0);
            lcd_printf("Timer: %02d:%02d.%03d", minute, second, msecond);

            uint16_t timer_read = TMR3;
            TMR3 = 0;
            float milis = timer_read / 2000.0 / 12800.0;
            timer_read /= 2000;
            lcd_locate(0, 1);
            lcd_printf("%02u cyc, %2.4f ms", timer_read, milis);
        }
    }
}
#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "flexserial.h"
#include "crc16.h"


/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);

int timeout = 0;
int error = 0;

/*
void __attribute__((__interrupt__)) _T1Interrupt(void) {
    timeout = 1;
    IFS0bits.T1IF = 0;
}
 */

int main(int argc, char** argv) {
    uint16_t crc = 0;
    uint8_t buffer1[4];
    uint8_t buffer2[256];
    uint8_t length;

    __C30_UART = 1;
    lcd_initialize();
    lcd_clear();
    lcd_locate(0, 0);

    uart2_init();

    /*
    __builtin_write_OSCCONL(OSCCONL | 2);
    T1CONbits.TON = 0; //Disable Timer
    T1CONbits.TCS = 1; //Select external clock
    T1CONbits.TSYNC = 0; //Disable Synchronization
    T1CONbits.TCKPS = 0b00; //Select 1:1 Prescaler
    TMR1 = 0x00; //Clear timer register
    PR1 = 32767; //Load the period value
    IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
    IFS0bits.T1IF = 0; // Clear Timer1 Interrupt Flag
    //   IEC0bits.T1IE = 1; // Enable Timer1 interrupt
    T1CONbits.TON = 1; // Start Timer
     */

    lcd_locate(0, 0);
    lcd_printf("U2INIT");
    lcd_locate(0, 1);
    while (1) {
        uint8_t serialbuf = 0;
        if (uart2_getc(&serialbuf)) {
            lcd_printf("%x ", serialbuf);
            uart2_putc(1);
        }
    }

    while (0) {
        crc = 0;

        while (!uart2_getc(&buffer1[0]));

        //TMR1 = 0x00;
        //IEC0bits.T1IE = 1;

        while (!uart2_getc(&buffer1[1]));
        while (!uart2_getc(&buffer1[2]));
        while (!uart2_getc(&buffer1[3]));

        length = buffer1[3];

        int i;
        for (i = 0; i < length; i++) {
            while (!uart2_getc(&buffer2[i]) && (!timeout));
            crc_update(crc, buffer2[i]);
        }
        //IEC0bits.T1IE = 0;

        lcd_locate(0, 0);
        lcd_printf("Errors: %d", error);

        if (timeout == 1) {
            uart2_putc(0);
            error++;
            timeout = 0;

            continue;
        }

        if ((buffer1[1] != ((crc >> 8) & 0xFF)) || (buffer1[2] != (crc & 0xFF))) {
            uart2_putc(0);
            error++;

            continue;
        }

        uart2_putc(1);
        buffer2[length] = 0;
        //lcd_locate(0, 1);
        //lcd_printf("Message: %s", buffer2);
    }
    return(0);
}
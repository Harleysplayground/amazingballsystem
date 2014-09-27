#include "flexserial.h"
#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>
#include "lcd.h"

void uart2_init(uint16_t baud) {

    /* Stop UART port */
    CLEARBIT(U1MODEbits.UARTEN); //Disable UART for configuration
    /* Disable Interrupts */
    IEC0bits.U1RXIE = 0;
    IEC0bits.U1TXIE = 0;
    /* Clear Interrupt flag bits */
    IFS0bits.U1RXIF = 0;
    IFS0bits.U1TXIF = 0;
    /* Set IO pins */
    TRISFbits.TRISF2 = 1; //set as input UART1 RX pin
    TRISFbits.TRISF3 = 0; //set as output UART1 TX pin

    /* baud rate */
    // use the following equation to compute the proper
    // setting for a specific baud rate
    U1MODEbits.BRGH = 0; //Set low speed baud rate
    U1BRG = (uint32_t) 800000 / baud - 1;

    /* Operation settings and start port */
    U1MODE = 0; // 8-bit, no parity and, 1 stop bit
    U1MODEbits.RTSMD = 0; //select simplex mode
    U1MODEbits.UEN = 0; //select simplex mode
    U1MODE |= 0x00;
    U1MODEbits.UARTEN = 1; //enable UART
    U1STA = 0;
    U1STAbits.UTXEN = 1; //enable UART TX



}

int uart2_putc(uint8_t c) {


    while (U1STAbits.UTXBF);
    U1TXREG = c;
    while (!U1STAbits.TRMT);


    return 0;


}

uint8_t uart2_getc(uint8_t* buf) {

   

    if (U1STAbits.OERR) {
        U1STAbits.OERR = 0;
    }

    if (U1STAbits.URXDA) {
        *buf = U1RXREG & 0x00FF;
        return 1;
    }


    return 0;
}
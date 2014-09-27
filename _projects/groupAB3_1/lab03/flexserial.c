#include "flexserial.h"
#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>

void uart2_init() {
    CLEARBIT(U2MODEbits.UARTEN); //Disable UART for configuration

    IEC1bits.U2RXIE = 0;
    IEC1bits.U2TXIE = 0;

    IFS1bits.U2RXIF = 0;
    IFS1bits.U2TXIF = 0;

    TRISFbits.TRISF4 = 1; //set as input UART1 RX pin
    TRISFbits.TRISF5 = 0; //set as output UART1 TX pin

    U2MODEbits.BRGH = 0; //Set low speed baud rate
    U2BRG = (uint32_t) 800000 / 9600 - 1;

    U2MODE = 0; // 8-bit, no parity and, 1 stop bit
    U2MODEbits.RTSMD = 0; //select simplex mode
    U2MODEbits.UEN = 0; //select simplex mode
    U2MODE |= 0x00;
    U2MODEbits.UARTEN = 1; //enable UART
    U2STA = 0;
    U2STAbits.UTXEN = 1; //enable UART TX
}

int uart2_putc(uint8_t c) {
    while (U2STAbits.UTXBF);
    U2TXREG = c;
    while (!U2STAbits.TRMT);
    return 0;
}

uint8_t uart2_getc(uint8_t* buf) {
    if (U2STAbits.OERR) {
        U2STAbits.OERR = 0;
    }
    if (U2STAbits.URXDA) {
        *buf = U2RXREG & 0x00FF;
        return 1;
    }
    return 0;
}
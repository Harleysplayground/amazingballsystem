/* 
 * File:   flexserial.h
 * Author: hzheng13
 *
 * Created on September 27, 2014, 1:37 PM
 */

#ifndef FLEXSERIAL_H
#define	FLEXSERIAL_H

#include <p33Fxxxx.h>
#include "types.h"
#include <stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif




uint8_t uart2_getc(uint8_t* buf);
int uart2_putc(uint8_t c);
void uart2_init(uint16_t baud);

#endif	/* FLEXSERIAL_H */


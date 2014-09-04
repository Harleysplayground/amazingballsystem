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


void main(){
	//Init LCD
	__C30_UART=1;	
	lcd_initialize();
	lcd_clear();
	lcd_locate(0,0);
	lcd_printf("Group 01");

        lcd_locate(0, 1);
        lcd_printf("Matt Wallick");

        lcd_locate(0, 2);
        lcd_printf("Haolin Zheng");

        lcd_locate(0, 3);
        lcd_printf("Albert Sun");

        lcd_locate(0, 4);
        lcd_printf("Counter: 0x00 (000)");

        
        uint16_t tick = 0;
        uint8_t count = 0;

        CLEARLED(LED4_TRIS);
        CLEARLED(LED1_TRIS);
        CLEARLED(LED2_TRIS);
        CLEARLED(LED3_TRIS);

        AD1PCFGHbits.PCFG20 = 1;
        TRISEbits.TRISE8 = 1;
        TRISDbits.TRISD10 = 1;
        
        uint8_t button_A_state = PORTEbits.RE8;
        uint16_t button_A_state_count = 0;
	
	while(1){
            if (++tick == 50000) {
                tick = 0;
                TOGGLELED(LED4_PORT);
            }

            /* debouncing algorithm */
            if (PORTEbits.RE8 == button_A_state) {
                button_A_state_count = 0;
            } else {
                if (++button_A_state_count > TRIGGER_THRESH) {
                    /* flipping the state!!! */
                    button_A_state = PORTEbits.RE8;
                    button_A_state_count = 0;

                    if (button_A_state == 0) {
                        lcd_locate(11, 4);
                        lcd_printf("%02x", ++count);
                        lcd_locate(15, 4);
                        lcd_printf("%03d", count);
                    }
                }
            }

            if (button_A_state == 0) {
                SETLED(LED1_PORT);
            } else {
                CLEARLED(LED1_PORT);
            }

            if (PORTDbits.RD10 == 0) {
                SETLED(LED2_PORT);
            } else {
                CLEARLED(LED2_PORT);
            }

            if (PORTDbits.RD10 != PORTEbits.RE8) {
                SETLED(LED3_PORT);
            } else {
                CLEARLED(LED3_PORT);
            }
	}
}


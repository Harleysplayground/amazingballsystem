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

    touch_init();

    int tick = 0;
    int dim = 0;
    int sample = 0;
    uint16_t pos_hist[2][5] = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0}
    };

    touch_select_dim(dim);

    while (1) {
        if (++tick == 10000) {
            tick = 0;
            pos_hist[dim][sample] = touch_adc(dim);
            touch_select_dim(dim ^= 1);
            sample += dim;

            if (sample == 5) {
                sample = 0;
                int j, x1, x2, x3, y1, y2, y3;
                x1 = pos_hist[0][0];
                y1 = pos_hist[1][0];
                x2 = x1;
                y2 = y1;
                x3 = x2;
                y3 = y2;

                for (j = 1; j < 5; j++) {
                    if (x1 > pos_hist[0][j])
                        x1 = pos_hist[0][j];
                    if (y1 > pos_hist[1][j])
                        y1 = pos_hist[1][j];
                }

                for (j = 1; j < 5; j++) {
                    if (x2 > x1)
                        x2 = pos_hist[0][j];
                    if (y2 > y1)
                        y2 = pos_hist[1][j];
                }

                for (j = 1; j < 5; j++) {
                    if (x3 > x2)
                        x3 = pos_hist[0][j];
                    if (y3 > y2)
                        y3 = pos_hist[1][j];
                }
                lcd_locate(0, 1);
                lcd_printf("x: %05d", x3);
                lcd_locate(0, 2);
                lcd_printf("y: %05d", y3);
            }
        }
    }
}

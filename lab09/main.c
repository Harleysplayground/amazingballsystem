#include "lcd.h"
#include "types.h"
#include "led.h"
#include "flexmotor.h"
#include "flextouch.h"
#include "performance.h"
#include "math.h"

#include <p33Fxxxx.h>

#define FCY 12800000UL

// control task frequency (Hz)
#define RT_FREQ 50

//setpoint parameters
#define SPEED 0.1  // tested up to .12!
#define RADIUS 100//350
#define CENTER_X 500.0//1650
#define CENTER_Y 120.0//1350

// Servo defines
#define MAX_DUTY_MICROSEC 2100
#define MIN_DUTY_MICROSEC 900
#define SERVO_PWM_PERIOD_MICROSEC 20000
#define INIT_DUTYX_MICROSEC 1410    // leveled plate on X axis: 1410
#define INIT_DUTYY_MICROSEC 1400     // leveled plate on Y axis: 1400
#define MOTOR_X_CHAN 0
#define MOTOR_Y_CHAN 1

// Touch screen definitions
#define X_DIM 0
#define Y_DIM 1

uint16_t TOUCH_MIN_X = 340;
uint16_t TOUCH_MAX_X = 3020;
uint16_t TOUCH_MIN_Y = 454;
uint16_t TOUCH_MAX_Y = 2657;

// do not change position of this include
#include <libpic30.h>

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);


// control setpoint
double Xpos_set = 200.0; //1650.0;
double Ypos_set = 200.0; //1550.0;

// raw, unfiltered X and Y position of the ball
volatile uint16_t Xpos, Ypos;
volatile uint8_t start = 0;
volatile uint8_t select = X_DIM;
volatile uint8_t deadline_miss = 0;


double Kp, Ki, Kd;
double previous_error[2] = {0.0, 0.0};
double integral = 0.0;
double derivative[2];
int dim = 1;

double pidX_controller(uint16_t Xp) {
    double pid;
    //Implement PID X
    double error = Xpos_set - Xp;
    integral += error * 0.05;
    derivative[0] = (error - previous_error[0]) / 0.05;
    pid = Kp * error + Ki * integral + Kd * derivative[0];
    previous_error[0] = error;

    return pid;
}

double pidY_controller(uint16_t Yp) {
    double pid;
    //Implement PID Y
    double error = Ypos_set - Yp;
    integral += error * 0.05;
    derivative[1] = (error - previous_error[1]) / 0.05;
    pid = Kp * error + Ki * integral + Kd * derivative[1];
    previous_error[1] = error;
    return pid;
}


// Configure the real-time task timer and its interrupt.

void timers_initialize() {

    //Set Timer1 to generate an interrupt every 10ms (100Hz) ==> PR1=500
    CLEARBIT(T1CONbits.TON); //Disable Timer
    CLEARBIT(T1CONbits.TCS); //Select internal instruction cycle clock
    CLEARBIT(T1CONbits.TGATE); //Disable Gated Timer mode
    T1CONbits.TCKPS = 0b11; //Select 1:256 Prescaler
    PR1 = 500; //Load the period value ==> running at 100Hz now!
    TMR1 = 0x00; //Clear timer register
    IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
    CLEARBIT(IFS0bits.T1IF); // Clear Timer1 Interrupt Flag
    SETBIT(IEC0bits.T1IE); // Enable Timer1 interrupt
    SETBIT(T1CONbits.TON); // Start Timer
}

int main() {
    uint8_t start_r, old_IPL;
    uint8_t hz50_scaler, hz5_scaler, hz1_scaler, sec;
    uint32_t tick = 0;

    hz50_scaler = hz5_scaler = hz1_scaler = sec = 1;

    touch_init();
    touch_select_dim(1);

    __delay_ms(200);
    lcd_initialize(); // Initialize the LCD

    motor_init(0);

    Kp = 0.5;
    Ki = 0.0;
    Kd = 0.4;

    lcd_clear();
    lcd_locate(0, 0);
    lcd_printf("-- Ball position: --");

    timers_initialize(); // Initialize timers

    while (1) {
        start_r = 0;
        while (!start_r) {
            // disable all maskable interrupts
            SET_AND_SAVE_CPU_IPL(old_IPL, 7);
            start_r = start;

            // enable all maskable interrupts
            RESTORE_CPU_IPL(old_IPL);
        }

        // Periodic real-time task code starts here!!!
        double pidX, pidY;
        uint16_t duty_us_x, duty_us_y;

        // 50Hz control task
        if (hz50_scaler == 0) {
            calcQEI(Xpos_set, Xpos, Ypos_set, Ypos);
            Xpos_set = CENTER_X + RADIUS * cos(tick * SPEED);
            tick++;

            pidX = pidX_controller(Xpos);

            //Convert PID to motor duty cycle (900-2100 us)
            duty_us_x = 1500 + pidX * 1.5;


            // setMotorDuty is a wrapper function that calls your motor_set_duty
            // implementation in flexmotor.c. The 2nd parameter expects a value
            // between 900-2100 us
            setMotorDuty(0, duty_us_x);
            Ypos_set = CENTER_Y + RADIUS * sin(tick * SPEED);
            pidY = pidY_controller(Ypos);
            duty_us_y = 1500 + pidY * 1.5;
            setMotorDuty(1, duty_us_y);
        }

        // 5Hz display task
        if (hz5_scaler == 0) {
            //lcd_locate(0, 1);
            //lcd_printf("Xf(t)=%u", Xpos);

            if (deadline_miss >= 1) {
                lcd_locate(0, 6);
                lcd_printf("%4d d_misses!!!", deadline_miss);
            }
        }

        // 1Hz seconds display task
        if (hz1_scaler == 0) {
            lcd_locate(0, 7);
            lcd_printf("QEI: %5u", getQEI());
            sec++;
        }

        hz50_scaler = (hz50_scaler + 1) % 2;
        hz5_scaler = (hz5_scaler + 1) % 20;
        hz1_scaler = (hz1_scaler + 1) % 100;

        start = 0;
    }

    return 0;
}

// This ISR will execute whenever Timer1 has a compare match.
// it kicks off the periodic execution of user code and performs I/O
// Min period: 10msec due to X,Y switch time for touchscreen

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0; // clear interrupt flag

    if (start == 1)
        deadline_miss++;

    if (select == X_DIM) {
        //read 5 samples from X-dimension and set Xpos as the median
        Xpos = touch_adc();
        touch_select_dim(Y_DIM);
        select = Y_DIM;
    } else {
        //read 5 samples from Y-dimension and set Ypos as the median
        Ypos = touch_adc();
        touch_select_dim(X_DIM);
        select = X_DIM;
    }

    start = 1;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "linuxanalog.h"

uint16_t lower, upper;

void signal_handler(int val)
{
    dac(lower);
    uint16_t temp = lower;
    lower = upper;
    upper = temp;
}

void signal_setup()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = signal_handler;
    if (sigaction(SIGALRM, &action, NULL) != 0) {
        perror("signal_setup");
        exit(1);
    }
}

int main()
{
    float low, high, freq, freq_max;
    struct timespec res;

    clock_getres(CLOCK_REALTIME, &res);
    freq_max = 2e9 / res.tv_nsec;
    
    printf("Low voltage (-5 to 5 or quit): "); scanf("%f", &low); if (low < -5.0 || low > 5.0) exit(1);
    printf("High voltage (-5 to 5 or quit): "); scanf("%f", &high); if (high < -5.0 || high > 5.0) exit (1);
    printf("Frequency (0 to %f or quit): ", freq_max); scanf("%f", &freq); if (freq < 0.0 || freq > freq_max) exit(1);

    lower = (low + 5) * 4095 / 10;
    upper = (high + 5) * 4095 / 10;

    signal_setup();

    timer_t timer1;
    if (timer_create(CLOCK_REALTIME, NULL, &timer1) != 0) {
        perror("Timer1");
        exit(1);
    }

    struct itimerspec timer1_time;
    timer1_time.it_value.tv_sec = 0; // 0 seconds
    timer1_time.it_value.tv_nsec = 500000000; // 0.5 seconds (5e8 nanoseconds)
    timer1_time.it_interval.tv_sec = 0;
    timer1_time.it_interval.tv_nsec = 5e8 / freq;

    if (timer_settime(timer1, 0, &timer1_time, NULL) != 0)
    {
        perror("timer_settime");
        exit(1);
    }

    printf("\nInitializing DAC hardware...\n");
    das1602_init();

    uint32_t counter = 0;
    while (1)
    {
        if ((counter % 100000000) == 0) printf("Counter: %u\n", counter);
        counter++;
    }

    return 0;
}

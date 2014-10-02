#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "linuxanalog.h"

void signal_handler(int val)
{
    printf("TEST: signal received\n");
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
    signal_setup();

    printf("\nInitializing DAC hardware...\n");
    das1602_init();

    printf("TEST: start\n");
    while(1);

    return 0;
}

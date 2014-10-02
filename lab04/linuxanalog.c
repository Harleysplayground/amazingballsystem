#include <sys/io.h>
#include "linuxanalog.h"

#define BADR0 0xCCC0
#define BADR1 0xCC40
#define BADR2 0xCC60
#define BADR3 0xCC80
#define BADR4 0xCCA0

#define DAS1602INITVEC 0x0027 /* binary 0000 0000 0010 0111 */

void das1602_init()
{
    outw(DAS1602INITVEC, BADR1 + 8);
    outw(1, BADR2 + 4);
}

void dac(uint16_t value)
{
    outw(value, BADR4 + 0);
    int i = 0;
    while(i < 10000000); //TODO
}

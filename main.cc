// Linux hyperspectral software
// Author: sumingrui

#include <signal.h>
#include "bus.h"

int searching;

// 终止信号:ctrl+c
void sig_handler(int signum)
{
    if (SIGINT == signum)
    {
        searching = 0;
    }
    return;
}

int main()
{
    searching = 1;
    signal(SIGINT, sig_handler);
    HSIC::Bus bus;
    bus.ObjSearch();

    return 0;
}
/*
 * main.c
 * This is the entry point for the Air Drums application
 * Author: Shannon, Lauren
 */

#include "drums.h"

int main(void)
{
#ifdef RXDEBUG
    airdrums_debug();
#else
    #ifdef RECEIVER
	airdrums_receiver();
    #else
	airdrums_stick();
    #endif
#endif
	return 0;
}


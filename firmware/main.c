/*
 * main.c
 * This is the entry point for the Air Drums application
 * Author: Shannon, Lauren
 */

#include "drums.h"

int main(void)
{
#ifdef RECEIVER
	airdrums_receiver();
#else
	airdrums_stick();
#endif
	return 0;
}


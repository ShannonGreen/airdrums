/*
 * airdrums_receiver.c
 *
 *  Created on: 20/10/2013
 *      Author: Shannon
 */

#include <msp430.h>

#include "nrf.h"
#include "uart.h"

void airdrums_receiver() {
	    WDTCTL  = WDTPW + WDTHOLD; // Stop WDT

	    __delay_cycles(10000);
	    // Clock rate 1MHz
	    if (CALBC1_1MHZ==0xFF) { // If calibration constant erased
	    	while(1);            // do not load, trap CPU!!
	    }
	    DCOCTL = 0; // Select lowest DCOx and MODx settings
	    BCSCTL1 = CALBC1_1MHZ; // Set DCO
	    DCOCTL = CALDCO_1MHZ;

		uart_init();
		nrfInit();

		__delay_cycles(10000);
		nrfStartRX('!'); // start RX with address '!'
		__delay_cycles(10000);

		P1OUT &= ~1;
		P1DIR |= 1;

		volatile char a;
		char drum, velocity;
		int midiNote;
		while(1) {
			while(!nrfDataReady());
			a = nrfGetRXByte();
			velocity = 100;
			drum = (a & 0x7);

			switch (drum) {
			case 0:
				midiNote = 38;
				break;
			case 1:
				midiNote = 42;
				break;
			default:
				midiNote = 56;
			}

			uart_putc(153);
			uart_putc(midiNote);
			uart_putc(velocity);

			uart_putc(137);
			uart_putc(midiNote);
			uart_putc(velocity);

	    	P1OUT ^= 1;
		}
}


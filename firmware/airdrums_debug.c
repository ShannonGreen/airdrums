/*
 * airdrums_debug.c
 *
 *  Created on: 20/10/2013
 *      Author: Shannon
 */

#include <msp430.h>

#include "nrf.h"
#include "uart.h"

void airdrums_debug() {
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

        volatile char a;

        while(1) {
            while(!nrfDataReady());
            a = nrfGetRXByte();
            //art_putc(a);
            //uart_putc("\n");
        }
}


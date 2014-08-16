/*
 * spi.c
 *
 *  Created on: 07/10/2013
 *      Author: Shannon
 */
 #include <msp430.h>


#ifndef RECEIVER
#define SCLKPIN    BIT4
#define MISOPIN    BIT1
#define MOSIPIN    BIT2
#define CSPIN      BIT5
#define CTL0	   UCA0CTL0
#define CTL1	   UCA0CTL1
#define TXBUF      UCA0TXBUF
#define RXBUF      UCA0RXBUF
#define TXIFG	   UCA0TXIFG
#else
#define SCLKPIN    BIT5
#define MISOPIN    BIT6
#define MOSIPIN    BIT7
#define CSPIN      BIT4
#define CTL0	   UCB0CTL0
#define CTL1	   UCB0CTL1
#define TXBUF      UCB0TXBUF
#define RXBUF      UCB0RXBUF
#define TXIFG      UCB0TXIFG
#endif
/**
 * SPI functions
 */

void spiInit(void) {
	// Set up SPI on USCIA.

	/*	The recommended USCI initialization/re-configuration process is:
	1. Set UCSWRST (BIS.B #UCSWRST,&UCxCTL1)
	2. Initialize all USCI registers with UCSWRST=1 (including UCxCTL1)
	3. Configure ports
	4. Clear UCSWRST via software (BIC.B #UCSWRST,&UCxCTL1)
	5. Enable interrupts (optional) via UCxRXIE and/or UCxTXIE
	*/

	// Set reset bit
	CTL1 |= UCSWRST;

	// Set up control register appropriately.
	// Clock phase: transmit on second edge
	// Master mode
	// Synchronous mode
	// MSB first transmission
	// Use SMCLK
	CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;
	CTL1 |= UCSSEL_2;

	// Set up port select, direction, etc
	P1SEL |= (MISOPIN | MOSIPIN | SCLKPIN);
	P1SEL2 |= (MISOPIN | MOSIPIN | SCLKPIN);

	// Since we're using a GPIO for CS
	// we set it up here too.
	// Directions for other pins are handled by USCI.
	P1DIR |= CSPIN;

	// Clear reset bit, all systems go.
	CTL1 &= ~UCSWRST;

}

unsigned char spiSend(unsigned char byte) {
	TXBUF = byte;
	// wait for TX
	while (!(IFG2 & TXIFG));
	return RXBUF;
}

void spiSetCS(int val) {
	if (val) {
		P1OUT &= ~(CSPIN);
	} else {
		P1OUT |= (CSPIN);
	}
}

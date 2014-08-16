#include <msp430.h>

#include "drums.h"
#include "i2c.h"
#include "MPU6050.h"

#define MAX_TX_SIZE 5
#define TIMEOUT 20

int RX = 0;
volatile byte MSData[MAX_TX_SIZE] = {0};

void init_i2c(void) {
    RPT_Flag = 0;
    RX = 0;
}

//-------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move received data from the I2C slave
// to the MSP430 memory. It is structured such that it can be used to receive
// any 2+ number of bytes by pre-loading RXByteCtr with the byte count.
//-------------------------------------------------------------------------------

#ifndef __GNUC__
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
#else
static void __attribute__((__interrupt__(USCIAB0TX_VECTOR))) USCIAB0TX_ISR(void)
#endif
{

    if(RX == 1){                                    // Master Recieve?
        RXByteCtr--;                                // Decrement RX byte counter
        if (RXByteCtr){
            *PRxData++ = UCB0RXBUF;                 // Move RX data to address PRxData
        } else {
            if(RPT_Flag == 0){
                UCB0CTL1 |= UCTXSTP;                // No Repeated Start: stop condition
            }
            if(RPT_Flag == 1){                      // if Repeated Start: do nothing
                RPT_Flag = 0;
            }
            *PRxData = UCB0RXBUF;                   // Move final RX data to PRxData
            __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
        }
    } else {                                        // Master Transmit

        if (TXByteCtr) {                            // Check TX byte counter
            UCB0TXBUF = *(PTxData++);               // Load TX buffer

            TXByteCtr--;                            // Decrement TX byte counter
        } else {
            if(RPT_Flag == 1){
                RPT_Flag = 0;
                __bic_SR_register_on_exit(CPUOFF);

            } else {
                UCB0CTL1 |= UCTXSTP;                // I2C stop condition
                IFG2 &= ~UCB0TXIFG;                 // Clear USCI_B0 TX int flag
                __bic_SR_register_on_exit(CPUOFF);  // Exit LPM0
            }
        }
    }
}

void Setup_TX(void){
    __dint();
    RX = 0;
    IE2 &= ~UCB0RXIE;                               // Disable RX interrupt
    int i = 0;
    while (UCB0CTL1 & UCTXSTP){                     // Ensure stop condition got sent
        if(i > TIMEOUT){
            WDTCTL = 0;                             //if spinning too long, assume clock stretching and reset
        }
        i++;
    }

    UCB0CTL1 |= UCSWRST;                            // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;           // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;                  // Use SMCLK, keep SW reset
    UCB0BR0 = 12;                                   // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = MPU6050_I2C_ADDRESS;                // Slave Address is 048h
    UCB0CTL1 &= ~UCSWRST;                           // Clear SW reset, resume operation
    IE2 |= UCB0TXIE;                                // Enable TX interrupt
}

void Setup_RX(void){
    __dint();


    RX = 1;
    IE2 &= ~UCB0TXIE;
    UCB0CTL1 |= UCSWRST;                            // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;           // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;                  // Use SMCLK, keep SW reset
    UCB0BR0 = 12;                                   // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = MPU6050_I2C_ADDRESS;                // Slave Address is 048h
    UCB0CTL1 &= ~UCSWRST;                           // Clear SW reset, resume operation
    IE2 |= UCB0RXIE;                                // Enable RX interrupt
}

void Transmit(void){
    PTxData = (byte *)MSData;                       // TX array start address

    int i = 0;
    while (UCB0CTL1 & UCTXSTP){                     // Ensure stop condition got sent
        if(i > TIMEOUT){
            WDTCTL = 0;                             //if spinning too long, assume clock stretching and reset
        }
        i++;
    }
    UCB0CTL1 |= UCTR + UCTXSTT;                     // I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);                // Enter LPM0 w/ interrupts
}

void Receive(void){
    PRxData = (byte *)&ga_data;                     // Start of RX buffer
    RXByteCtr--;                                    // Load RX byte counter  <----- not sure this line should have --
    int i = 0;
    while (UCB0CTL1 & UCTXSTP){                     // Ensure stop condition got sent
        if(i > TIMEOUT){
            WDTCTL = 0;                             //if spinning too long, assume clock stretching and reset
        }
        i++;
    }
    UCB0CTL1 |= UCTXSTT;                            // I2C start condition
    __bis_SR_register(CPUOFF + GIE);                // Enter LPM0 w/ interrupts
}

void MPU6050_write_byte(byte reg_addr, byte data){
    MSData[0] = reg_addr;
    MSData[1] = data;
    TXByteCtr = 2;

    //Transmit process
    Setup_TX();

    RPT_Flag = 0;
    Transmit();
    int i = 0;
    while (UCB0CTL1 & UCTXSTP){                     // Ensure stop condition got sent
        if(i > TIMEOUT){
            WDTCTL = 0;                             //if spinning too long, assume clock stretching and reset
        }
        i++;
    }
    //P1OUT = 0x01;
}

void MPU6050_read(byte reg_addr, int size){
    //send address of read to mpu
    MSData[0] = reg_addr;
    TXByteCtr = 1;

    Setup_TX();
    RPT_Flag = 1;
    Transmit();

    //Read
    RPT_Flag = 0;
    RXByteCtr = size;
    Setup_RX();
    Receive();
    int i = 0;
    while (UCB0CTL1 & UCTXSTP){                     // Ensure stop condition got sent
        if(i > TIMEOUT){
            WDTCTL = 0;                             //if spinning too long, assume clock stretching and reset
        }
        i++;
    }
}

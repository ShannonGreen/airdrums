/*
 * nrf.c
 *
 *  Created on: 07/10/2013
 *      Author: Shannon
 */

/* TODO
 * + Add delays as spec'd (maybe not necessary?)
 * + Power down when not transmitting/receiving
 * +
 */

#include "spi.h"
#include "nrf.h"

#include <msp430.h>

void nrfInit() {
    unsigned const char initConfig = RF24_PWR_UP | RF24_EN_CRC;

    // Init SPI
    spiInit();

    // CE pin direction
    P2DIR |= NRF_CE;

    // CE
    P2OUT &= ~NRF_CE;

    // Power up
    nrfWriteReg(RF24_CONFIG, initConfig);
}

void nrfStartTX(unsigned char address) {
    unsigned char config = nrfReadReg(RF24_CONFIG);
    config &= ~RF24_PRIM_RX;
    nrfWriteReg(RF24_CONFIG, config);

    // set attenuation to 0dB, xmit rate to 1MHz
    nrfWriteReg(RF24_RF_SETUP, 0x06);

    // pick channel
    nrfWriteReg(RF24_RF_CH, CHANNEL);

    // set TX address (8 bits only for now)
    nrfWriteReg(RF24_TX_ADDR, address);

    // set up P0 for ACK
    nrfWriteReg(RF24_RX_ADDR_P0, address);

    //nrfWriteReg(RF24_RX_PW_P0, 1);

    // clear MAX_RT and TX_DS flags
    nrfWriteReg(RF24_STATUS, RF24_MAX_RT | RF24_TX_DS);

    // DISABLE AUTOACK completely for now
    nrfWriteReg(RF24_EN_AA, 0);

    // set up payload size on P1
    nrfWriteReg(RF24_RX_PW_P1, 1);
}

void nrfStartRX(unsigned char address) {
    unsigned char config = nrfReadReg(RF24_CONFIG);
    config |= RF24_PRIM_RX;
    nrfWriteReg(RF24_CONFIG, config);

    // set attenuation to 0dB, xmit rate to 1MHz
    nrfWriteReg(RF24_RF_SETUP, 0x06);

    nrfWriteReg(RF24_RF_CH, CHANNEL);

    // set TX address (8 bits only for now)
    nrfWriteReg(RF24_TX_ADDR, address);

    // set up P0 for ACK
    nrfWriteReg(RF24_RX_ADDR_P0, address);

    nrfWriteReg(RF24_RX_PW_P0, 1);

    // clear MAX_RT and TX_DS flags
    nrfWriteReg(RF24_STATUS, RF24_MAX_RT | RF24_TX_DS);

    // DISABLE AUTOACK completely for now
    nrfWriteReg(RF24_EN_AA, 0);

    // flush FIFO
    spiSetCS(1);
    spiSend(RF24_FLUSH_TX);
    spiSetCS(0);

    // CE
    P2OUT |= NRF_CE;
}



void nrfStandby() {
    P2OUT &= ~NRF_CE;
}

unsigned char nrfReadReg(unsigned char reg) {
    unsigned char ret;
    spiSetCS(1);
    spiSend(RF24_R_REGISTER | reg);
    ret = spiSend(RF24_NOP);
    spiSetCS(0);
    return ret;
}

void nrfWriteReg(unsigned char reg, unsigned char val) {
    spiSetCS(1);
    spiSend(RF24_W_REGISTER | reg);
    spiSend(val);
    spiSetCS(0);
}

void nrfTXByte(unsigned char byte) {

    // standby
    P2OUT &= ~NRF_CE;

    // flush FIFO
    spiSetCS(1);
    spiSend(RF24_FLUSH_TX);
    spiSetCS(0);

    // Transmit payload
    spiSetCS(1);
    spiSend(RF24_W_TX_PAYLOAD);
    spiSend(byte);
    spiSetCS(0);

    // start transmission
    P2OUT |= NRF_CE;
}

unsigned char nrfGetStatus() {
    unsigned char ret;
    spiSetCS(1);
    ret = spiSend(RF24_NOP);
    spiSetCS(0);
    return ret;
}

unsigned char nrfGetRXByte() {
    unsigned char ret;
    spiSetCS(1);
    spiSend(RF24_R_RX_PAYLOAD);
    ret = spiSend(RF24_NOP);
    spiSetCS(0);

    nrfWriteReg(RF24_STATUS, RF24_RX_DR);
    return ret;
}

int nrfDataReady() {
    unsigned char status = nrfGetStatus();
    if (status & RF24_RX_DR) {
        // First check status bit
        return 1;
    } else {
        // If RX_DR isn't set we still need
        // to check the FIFO queue.
        status = nrfReadReg(RF24_FIFO_STATUS);
        return !(status & RF24_RX_EMPTY);
    }
}









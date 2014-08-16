/*
 * spi.h
 *
 *  Created on: 06/10/2013
 *      Author: Shannon
 */

#ifndef SPI_H_
#define SPI_H_

void spiInit(void);
void spiSetCS(int val);
unsigned char spiSend(unsigned char byte);

#endif /* SPI_H_ */

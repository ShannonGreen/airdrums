CC=msp430-gcc
CFLAGS=-mmcu=msp430g2553
DBG=mspdebug rf2500
OUTDIR=build
SOURCES_RX=main.c airdrums_receiver.c nrf.c spi.c uart.c
SOURCES_TX=main.c airdrums_stick.c i2c.c nrf.c spi.c

all: rx_build tx_build

rx: rx_build
	$(DBG) "prog $(OUTDIR)/rx" run

tx: tx_build
	$(DBG) "prog $(OUTDIR)/tx" run

rx_build: $(SOURCES_RX)
	$(CC) $(CFLAGS) -DRECEIVER $(SOURCES_RX) -o $(OUTDIR)/rx

tx_build: $(SOURCES_TX)
	$(CC) $(CFLAGS) -DTRANSMITTER $(SOURCES_TX) -o $(OUTDIR)/tx
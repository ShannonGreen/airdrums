CC=msp430-gcc
CFLAGS=-mmcu=msp430g2553
DBG=mspdebug rf2500
OUTDIR=build
SOURCES_COMMON=main.c nrf.c spi.c uart.c i2c.c
SOURCES_RX=$(SOURCES_COMMON) airdrums_receiver.c
SOURCES_TX=$(SOURCES_COMMON) airdrums_stick.c

all: rx_build tx_build

rx: rx_build
	$(DBG) "prog $(OUTDIR)/rx"

tx: tx_build
	$(DBG) "prog $(OUTDIR)/tx"

rx_build: clean $(SOURCES_RX)
	@mkdir -p build
	$(CC) $(CFLAGS) -DRECEIVER $(SOURCES_RX) -o $(OUTDIR)/rx

tx_build: clean $(SOURCES_TX)
	@mkdir -p build
	$(CC) $(CFLAGS) -DTRANSMITTER $(SOURCES_TX) -o $(OUTDIR)/tx

tx_debug: clean $(SOURCES_COMMON) airdrums_debug_tx.c
	@mkdir -p build
	$(CC) $(CFLAGS) -DTXDEBUG $(SOURCES_COMMON) airdrums_debug_tx.c -o $(OUTDIR)/txdebug
	$(DBG) "prog $(OUTDIR)/txdebug"

rx_debug: clean $(SOURCES_COMMON) airdrums_debug_rx.c
	@mkdir -p build
	$(CC) $(CFLAGS) -DRXDEBUG $(SOURCES_COMMON) airdrums_debug_rx.c -o $(OUTDIR)/rxdebug
	$(DBG) "prog $(OUTDIR)/rxdebug"

clean:
	@rm -f build/*
	@rm -f *.pyc
	@rm -f *.o
	@rm -f *.out
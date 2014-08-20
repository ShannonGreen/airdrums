CC=msp430-gcc
CFLAGS=-mmcu=msp430g2553
DBG=mspdebug rf2500
OUTDIR=build
SOURCES_RX=main.c airdrums_receiver.c nrf.c spi.c uart.c airdrums_debug.c
SOURCES_TX=main.c airdrums_stick.c i2c.c nrf.c spi.c

all: rx_build tx_build

rx: rx_build
	$(DBG) "prog $(OUTDIR)/rx" run

tx: tx_build
	$(DBG) "prog $(OUTDIR)/tx" run

rx_build: clean $(SOURCES_RX)
	@mkdir -p build
	$(CC) $(CFLAGS) -DRECEIVER $(SOURCES_RX) -o $(OUTDIR)/rx

tx_build: clean $(SOURCES_TX)
	@mkdir -p build
	$(CC) $(CFLAGS) -DTRANSMITTER $(SOURCES_TX) -o $(OUTDIR)/tx

tx_debug: clean $(SOURCES_TX)
	@mkdir -p build
	$(CC) $(CFLAGS) -DTXDEBUG $(SOURCES_TX) -o $(OUTDIR)/txdebug
	$(DBG) "prog $(OUTDIR)/txdebug" run

rx_debug: clean $(SOURCES_RX)
	@mkdir -p build
	$(CC) $(CFLAGS) -DRXDEBUG $(SOURCES_RX) -o $(OUTDIR)/rxdebug
	$(DBG) "prog $(OUTDIR)/rxdebug" run

clean:
	@rm -f build/*
	@rm -f *.pyc
	@rm -f *.o
	@rm -f *.out
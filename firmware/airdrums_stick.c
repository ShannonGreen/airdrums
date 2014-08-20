#include <msp430.h>

#include "drums.h"
#include "i2c.h"
#include "MPU6050.h"

#define MAGNITUDE_THRESHOLD 20000
#define DIRECTION_THRESHOLD 10000
#define SAMPLE_COUNT 10
#define DRIFT_CYCLE_LENGTH 20
#define STILL_COUNT_MAX 80

#define PROCESS_RATE_HZ 300

byte highCount = 0;
int pos = 0;
int isDown = FALSE;
int drift;
int driftCount = 0;
int hitDetected = 0;
int stillCounter = 0;
byte debugCounter = 0;

void process_data();
void findDrift();

// Circular buffer size.
// Try to keep this a power of 2 for fast modulos.
#define BUFFER_SIZE 16

void circ_init();
void circ_add(int item);
int circ_getelem(int index);

int circ_gyro[BUFFER_SIZE] = {0};
int circ_index;

void circ_init() {
    circ_index = 0;
}

void circ_add(int item) {
    circ_gyro[circ_index] = item;
    if (++circ_index == BUFFER_SIZE) circ_index = 0;
}

int circ_getelem(int index) {
    return circ_gyro[(index + circ_index) % BUFFER_SIZE];
}

int main(void) {
    airdrums_stick();
    return 0;
}

void airdrums_stick(void){

    //initial setup
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT

    P1SEL &= 0;
    P1SEL2 &= 0;
    P1SEL |= BIT6 + BIT7; // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7; // Assign I2C pins to USCI_B0

    // LED pin
    P1DIR |= 0x01;
    P1OUT = 0x00;

    // start up i2c bus
    init_i2c();

    //intialise some globals
    highCount = 0;
    isDown = FALSE;
    driftCount=0;

    // Give the nrf plenty of time to start up;
    // it gets pissy if you talk to it too early.

    __delay_cycles(10000);
    nrfInit();
    __delay_cycles(10000);
    // Start TX with address '!'.
    // Hard coded because why not
    nrfStartTX('!');
    __delay_cycles(10000);

    //transmit stuff to set up mpu6050
    //set sensitivity levels
    MPU6050_write_byte(MPU6050_GYRO_CONFIG, MPU6050_FS_SEL_2000);
    MPU6050_write_byte(MPU6050_ACCEL_CONFIG, MPU6050_AFS_SEL_16G);

    //wake sensor up by clearing sleep bit
    MPU6050_write_byte (MPU6050_PWR_MGMT_1, 0);

    // initialise circular buffer
    circ_init();

    // Do some quick and dirty drift correction
    findDrift();

    // Set up timer interrupts for main loop
    CCTL0 = CCIE;                    // CCR0 interrupt enabled
    TACTL = TASSEL_2 + MC_1 + ID_3;  // SMCLK/8, upmode
    CCR0 =  (1000000 / 8 / PROCESS_RATE_HZ); // Set process rate
    // CCR0 = 417; // 300 Hz

    _BIS_SR(CPUOFF + GIE); // Enter LPM0 w/ interrupt
    while(1);
}


//

#ifndef __GNUC__
#pragma vector = TIMERA0_VECTOR
__interrupt void TIMERA0_ISR(void)
#else
static void __attribute__((__interrupt__(TIMER0_A0_VECTOR))) timer0_a0_isr(void)
#endif
{
        MPU6050_read(MPU6050_ACCEL_XOUT_H, sizeof(ga_data));

        // Reverse byte ordering of accel/gyro data
        SWAP (ga_data.reg.x_accel_h, ga_data.reg.x_accel_l);
        SWAP (ga_data.reg.y_accel_h, ga_data.reg.y_accel_l);
        SWAP (ga_data.reg.z_accel_h, ga_data.reg.z_accel_l);
        SWAP (ga_data.reg.x_gyro_h, ga_data.reg.x_gyro_l);
        SWAP (ga_data.reg.y_gyro_h, ga_data.reg.y_gyro_l);
        SWAP (ga_data.reg.z_gyro_h, ga_data.reg.z_gyro_l);

        process_data();

}

void process_data() {

    /*
     * A general outline of the algorithm is as follows:
     * 1. a. Increment stillCounter if no z-axis rotation detected.
     *    b. If we've been still for long enough, reset postition to 0.
     * 2. a. Integrate z gyro for h position. Add the drift correction factor if necessary.
     *    b. Determine which drum corresponds to our current position.
     * 3. a. If z accel is above our threshold, increment highCount. Otherwise reset it.
     *    b. Check which direction x gyro is moving.
     *    c. If highCount > q, xgyro > r, and we haven't had a hit in past samples, boom.
     */

    byte drum;

    if((int)(ga_data.value.z_gyro)/1000 == 0){
        stillCounter++;
    } else {
        stillCounter = 0;
    }

    if (stillCounter >= STILL_COUNT_MAX){
        pos = 0;
        stillCounter = 0;
    } else if (driftCount == DRIFT_CYCLE_LENGTH){
        pos += (int)(ga_data.value.z_gyro)/1000-drift;
        driftCount = 0;
    } else {
        pos += (int)(ga_data.value.z_gyro)/1000;
        drift++;
    }

    // HIT DETECTION BEYOND THIS POINT
    //pos += (char)ga_data.reg.z_gyro_h;
    if (pos <= -3000) {
        drum = 0; // out of range
    } else if (pos <= -1000) {
        drum = 1;
    } else if (pos <= 1000) {
        drum = 2;
    } else if (pos <= 3000) {
        drum = 3;
    } else {
        drum = 0; // out of range
    }

    if (ga_data.value.z_accel > MAGNITUDE_THRESHOLD) {
        // increment the high sample counter
        highCount++;
    } else {
        highCount = 0;
        hitDetected = FALSE;
    }

    if (ga_data.value.x_gyro < -DIRECTION_THRESHOLD) {
        isDown = TRUE;
    } else if (ga_data.value.x_gyro > DIRECTION_THRESHOLD) {
        isDown = FALSE;
    }

    circ_add(pos);

    if ((highCount > SAMPLE_COUNT) && isDown && (!hitDetected)) {
        highCount = 0;
        isDown = FALSE;
        hitDetected = TRUE;
        int velocity = ((ga_data.value.z_accel - MAGNITUDE_THRESHOLD)) * 31 / (MAGNITUDE_THRESHOLD);
        if (velocity > 31) velocity = 31;
        if (velocity < 0) velocity = 0;

        // transmit!
#ifndef TXDEBUG
        nrfTXByte((velocity << 3) | drum);
#endif


        // A drum hit was detected. This is great, but
        // it has the potential to screw with our gyro -
        // they don't handle shock very well.
        // Fortunately we've been keeping a buffer of gyro values.
        // We'll restore the value from a few samples back and
        // see if that helps.

        //pos = circ_getelem(BUFFER_SIZE - 1);

    }

#ifdef TXDEBUG
    if (debugCounter == 100) {
        nrfTXByte(pos / 20);
        debugCounter = 0;
    }
#endif
}

void findDrift(void){
    drift = 0;
    int i;

    for(i = 0; i < DRIFT_CYCLE_LENGTH; i++){
        MPU6050_read(MPU6050_ACCEL_XOUT_H, sizeof(ga_data));
        SWAP (ga_data.reg.z_gyro_h, ga_data.reg.z_gyro_l);
        drift += (int)(ga_data.value.z_gyro)/1000;
    }
}
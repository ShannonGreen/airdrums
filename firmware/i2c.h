#ifndef _I2C_H_
#define _I2C_H_

#include "drums.h"

void init_i2c(void);
void Setup_TX(void);
void Setup_RX(void);
void Transmit(void);
void Receive(void);
void MPU6050_write_byte(byte reg_addr, byte data);
void MPU6050_read(byte reg_addr, int size);

byte *PTxData;                                      // Pointer to TX data
byte *PRxData;                                      // Pointer to RX data
byte RPT_Flag;
unsigned int RXByteCtr;
unsigned int TXByteCtr;

typedef union accel_t_gyro_union
{
  struct
  {
    byte x_accel_h;
    byte x_accel_l;
    byte y_accel_h;
    byte y_accel_l;
    byte z_accel_h;
    byte z_accel_l;
    byte t_h;
    byte t_l;
    byte x_gyro_h;
    byte x_gyro_l;
    byte y_gyro_h;
    byte y_gyro_l;
    byte z_gyro_h;
    byte z_gyro_l;
    byte extra_h;
    byte extra_l;
  } reg;
  struct
  {
    int x_accel;
    int y_accel;
    int z_accel;
    int temperature;
    int x_gyro;
    int y_gyro;
    int z_gyro;
    int extra;

  } value;
} accel_gyro_union;

accel_gyro_union ga_data;

#endif /* _I2C_H_ */

/*
 * bme280.c
 *
 *  Created on: Jun 14, 2016
 *      Author: spiri
 *
 *  Designed for TI-RTOS and tested on the CC1310 LaunchPad
 *
 */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/I2C.h>

#include "bme280.h"


static I2C_Handle i2cbus;
static Uint8 i2cAddr;
static Uint8 _bme280_calibration[33];

Void BME280_init(I2C_Handle hand, Uint8 addr)
{
	i2cbus = hand;
	i2cAddr = addr;
}

Bool BME280_open()
{
	I2C_Transaction txn;
	Uint8 regAddr;
	Uint8 readBuf[42];
	txn.readBuf = readBuf;
	txn.writeBuf = &regAddr;
	txn.writeCount = 1;
	txn.slaveAddress = i2cAddr;

	// Find Chip ID
	regAddr = 0xD0; // chip_id[7:0]
	txn.readCount = 1;
	if (!I2C_transfer(i2cbus, &txn)) {
		System_printf("Error: BME280_open() could not perform I2C transfer\r\n");
		System_flush();
		return false;
	}
	if (readBuf[0] != 0x60) { // Not a BME280?
		System_printf("Error: BME280_open() read I2C bus for CHIP_ID and found invalid ID!\r\n");
		System_flush();
		return false;
	}

	regAddr = BME280_REG_CALIB00;
	txn.readBuf = &_bme280_calibration[0];
	txn.readCount = 26;
	I2C_transfer(i2cbus, &txn);
	regAddr = BME280_REG_CALIB26;
	txn.readBuf = &_bme280_calibration[26];
	txn.readCount = 6;
	I2C_transfer(i2cbus, &txn);

	return true;
}

Bool BME280_close()
{
	// nothing actually to do here
	return true;
}

Void BME280_setAddress(Uint8 memAddress)
{
	I2C_Transaction txn;
	Uint8 regAddr = memAddress;

	txn.readBuf = NULL;
	txn.readCount = 0;
	txn.writeBuf = &regAddr;
	txn.writeCount = 1;
	txn.slaveAddress = i2cAddr;

	I2C_transfer(i2cbus, &txn);
}

Void BME280_writeReg(Uint8 memAddress, Uint8 value)
{
	I2C_Transaction txn;
	Uint8 wrBuf[2];

	txn.readBuf = NULL;
	txn.readCount = 0;
	txn.writeBuf = wrBuf;
	txn.writeCount = 2;
	txn.slaveAddress = i2cAddr;

	wrBuf[0] = memAddress;
	wrBuf[1] = value;

	I2C_transfer(i2cbus, &txn);
}

Uint8 BME280_readReg(Uint8 memAddress)
{
	I2C_Transaction txn;
	Uint8 regAddr = memAddress;
	Uint8 rdBuf;

	txn.readBuf = &rdBuf;
	txn.readCount = 1;
	txn.writeBuf = &regAddr;
	txn.writeCount = 1;
	txn.slaveAddress = i2cAddr;

	I2C_transfer(i2cbus, &txn);
	return rdBuf;
}

Uint16 BME280_readWord(Uint8 memAddress)
{
	I2C_Transaction txn;
	Uint8 regAddr = memAddress;
	Uint8 rdBuf[2];

	txn.readBuf = &rdBuf;
	txn.readCount = 2;
	txn.writeBuf = &regAddr;
	txn.writeCount = 1;
	txn.slaveAddress = i2cAddr;

	I2C_transfer(i2cbus, &txn);
	return ((Uint16)rdBuf[0] << 8) | (Uint16)rdBuf[1];
}

Uint32 BME280_readWord20(Uint8 memAddress)
{
	I2C_Transaction txn;
	Uint8 regAddr = memAddress;
	Uint8 rdBuf[3];

	txn.readBuf = &rdBuf;
	txn.readCount = 3;
	txn.writeBuf = &regAddr;
	txn.writeCount = 1;
	txn.slaveAddress = i2cAddr;

	I2C_transfer(i2cbus, &txn);
	return ((Uint32)rdBuf[0] << 12) | ((Uint32)rdBuf[1] << 4) | ((Uint32)rdBuf[2] >> 4);
}

static BME280_RawData _rawData;
BME280_RawData * BME280_readMeasurements()
{
	I2C_Transaction txn;
	Uint8 regAddr = BME280_REG_PRESSURE;
	Uint8 rdBuf[8];

	txn.readBuf = &rdBuf;
	txn.readCount = 8;
	txn.writeBuf = &regAddr;
	txn.writeCount = 1;
	txn.slaveAddress = i2cAddr;

	I2C_transfer(i2cbus, &txn);

	// Fan out results
	_rawData.humidity_raw = ((Uint16)rdBuf[6] << 8) | (Uint16)rdBuf[7];
	_rawData.temperature_raw = ((Uint32)rdBuf[3] << 12) | ((Uint32)rdBuf[4] << 4) | ((Uint32)rdBuf[5] >> 4);
	_rawData.pressure_raw = ((Uint32)rdBuf[0] << 12) | ((Uint32)rdBuf[1] << 4) | ((Uint32)rdBuf[2] >> 4);

	return &_rawData;
}

BME280_RawData * BME280_read()
{
	BME280_writeReg(BME280_REG_CTRL_HUM, BME280_CTRL_HUM_OSRS__4);
	BME280_writeReg(BME280_REG_CTRL_MEAS, BME280_CTRL_MEAS_MODE_FORCED | BME280_CTRL_MEAS_OSRS_T__4 | BME280_CTRL_MEAS_OSRS_P__4);

	while (BME280_readReg(BME280_REG_STATUS) & (BME280_STATUS_MEASURING | BME280_STATUS_IM_UPDATE)) {
		Task_sleep(100);  // Poll every 100ms until complete
	}

	return BME280_readMeasurements();
}

/* Calibration positions */
#define BME280_CALIBOFFSET_U16LE_dig_T1          0
#define BME280_CALIBOFFSET_S16LE_dig_T2          2
#define BME280_CALIBOFFSET_S16LE_dig_T3          4
#define BME280_CALIBOFFSET_U16LE_dig_P1          6
#define BME280_CALIBOFFSET_S16LE_dig_P2          8
#define BME280_CALIBOFFSET_S16LE_dig_P3          10
#define BME280_CALIBOFFSET_S16LE_dig_P4          12
#define BME280_CALIBOFFSET_S16LE_dig_P5          14
#define BME280_CALIBOFFSET_S16LE_dig_P6          16
#define BME280_CALIBOFFSET_S16LE_dig_P7          18
#define BME280_CALIBOFFSET_S16LE_dig_P8          20
#define BME280_CALIBOFFSET_S16LE_dig_P9          22
#define BME280_CALIBOFFSET_U8_dig_H1             25
#define BME280_CALIBOFFSET_S16LE_dig_H2          26
#define BME280_CALIBOFFSET_U8_dig_H3             28
#define BME280_CALIBOFFSET_S16LE_dig_H4          29
#define BME280_CALIBOFFSET_S16LE_dig_H5          30
#define BME280_CALIBOFFSET_S8_dig_H6             32

#define BME280_dig_T1 ( _bme280_compute_U16LE(_bme280_calibration[BME280_CALIBOFFSET_U16LE_dig_T1], _bme280_calibration[BME280_CALIBOFFSET_U16LE_dig_T1+1]) )
#define BME280_dig_T2 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_T2], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_T2+1]) )
#define BME280_dig_T3 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_T3], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_T3+1]) )
#define BME280_dig_P1 ( _bme280_compute_U16LE(_bme280_calibration[BME280_CALIBOFFSET_U16LE_dig_P1], _bme280_calibration[BME280_CALIBOFFSET_U16LE_dig_P1+1]) )
#define BME280_dig_P2 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P2], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P2+1]) )
#define BME280_dig_P3 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P3], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P3+1]) )
#define BME280_dig_P4 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P4], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P4+1]) )
#define BME280_dig_P5 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P5], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P5+1]) )
#define BME280_dig_P6 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P6], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P6+1]) )
#define BME280_dig_P7 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P7], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P7+1]) )
#define BME280_dig_P8 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P8], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P8+1]) )
#define BME280_dig_P9 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P9], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_P9+1]) )
#define BME280_dig_H1 ( (Uint8)_bme280_calibration[BME280_CALIBOFFSET_U8_dig_H1] )
#define BME280_dig_H2 ( _bme280_compute_S16LE(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_H2], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_H2+1]) )
#define BME280_dig_H3 ( (Uint8)_bme280_calibration[BME280_CALIBOFFSET_U8_dig_H3] )
#define BME280_dig_H4 ( _bme280_compute_H4(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_H4], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_H4+1]) )
#define BME280_dig_H5 ( _bme280_compute_H5(_bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_H5], _bme280_calibration[BME280_CALIBOFFSET_S16LE_dig_H5+1]) )
#define BME280_dig_H6 ( (Int8)_bme280_calibration[BME280_CALIBOFFSET_S8_dig_H6] )

inline Int16 _bme280_compute_S16LE(Uint8 r0, Uint8 r1)
{
	Int16 value = 0;
	Uint16 w0 = (Uint16)r0, w1 = (Uint16)r1;

	value = w1 << 8 | w0;
	return value;
}

inline Uint16 _bme280_compute_U16LE(Uint8 r0, Uint8 r1)
{
	Uint16 value = 0;
	Uint16 w0 = (Uint16)r0, w1 = (Uint16)r1;

	value = w1 << 8 | w0;
	return value;
}

inline Int16 _bme280_compute_H4(Uint8 e4, Uint8 e5)
{
	Int16 value = 0;
	// No sign extension necessary per empirical testing...
	value |= ((Uint16)e4) << 4;
	value |= e5 & 0x0F;
	return value;
}

inline Int16 _bme280_compute_H5(Uint8 e5, Uint8 e6)
{
	Int16 value = 0;
	// No sign extension necessary per empirical testing...
	value |= ((Uint16)e6) << 4;
	value |= e5 >> 4;
	return value;
}

/* These compensation equations are derived from BME280 datasheet pseudocode, page 23 & 24 */
static Int32 _t_fine;

// Output degrees Celsius with 0.01C resolution.  Divide by 100 for whole degrees.
Int32 BME280_compensated_Temperature(BME280_RawData *rd)
{
	Int32 adc_T = rd->temperature_raw; // No sign extension will be performed as the raw value is expected to be positive.

	Int32 var1, var2, T;

	var1 = ((((adc_T >> 3) - ((Int32)BME280_dig_T1 << 1))) * ((Int32)BME280_dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) - (Int32)BME280_dig_T1) * ((adc_T >> 4) - (Int32)BME280_dig_T1)) >> 12) * (Int32)BME280_dig_T3) >> 14;
	_t_fine = var1 + var2;
	T = (_t_fine * 5 + 128) >> 8;

	return T;
}

// Pressure in Pascals as unsigned 32-bit integer in Q24.8 format; divide by 256 for whole Pascals
Uint32 BME280_compensated_Pressure(BME280_RawData *rd)
{
	Int32 adc_P = rd->pressure_raw; // No sign extension will be performed as the raw value is expected to be positive.

	Int64 var1, var2, p;
	var1 = (Int64)_t_fine - 128000;
	var2 = var1 * var1 * (Int64)BME280_dig_P6;
	var2 = var2 + ((var1 * (Int64)BME280_dig_P5) << 17);
	var2 = var2 + (((Int64)BME280_dig_P3) >> 8) + ((var1 * (Int64)BME280_dig_P2) << 12);
	var1 = ((var1 * var1 * (Int64)BME280_dig_P3) >> 8) + ((var1 * (Int64)BME280_dig_P2) << 12);
	var1 = (((((Int64)1) << 47) + var1)) * ((Int64)BME280_dig_P1) >> 33;
	if (var1 == 0) {
		return 0;  // avoid exception caused by divide by zero
	}
	p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((Int64)BME280_dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((Int64)BME280_dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((Int64)BME280_dig_P7) << 4);
	return (Uint32)p;
}

// Humidity in %relativehumidity as unsigned 32-bit integer in Q22.10 format; divide by 1024 for whole %RH
Uint32 BME280_compensated_Humidity(BME280_RawData *rd)
{
	Int32 adc_H = (Uint32)rd->humidity_raw; // No sign extension will be performed as the raw value is expected to be positive.

	Int32 v_x1_u32r;
	v_x1_u32r = _t_fine - ((Int32)76800);
	v_x1_u32r = (((((adc_H << 14) - (((Int32)BME280_dig_H4) << 20) - (((Int32)BME280_dig_H5) * v_x1_u32r)) \
			+ ((Int32)16384)) >> 15) * (((((((v_x1_u32r * ((Int32)BME280_dig_H6)) >> 10) \
			* (((v_x1_u32r * ((Int32)BME280_dig_H3)) >> 11) + ((Int32)32768))) >> 10) + ((Int32)2097152)) \
			* ((Int32)BME280_dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) \
			* ((Int32)BME280_dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (Uint32)(v_x1_u32r >> 12);
}

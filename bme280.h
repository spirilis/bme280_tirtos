/*
 * bme280.h
 *
 *  Created on: Jun 14, 2016
 *      Author: spiri
 *
 *  Designed for TI-RTOS and tested on the CC1310 LaunchPad
 *
 */

#ifndef BME280_H_
#define BME280_H_

/* Bosch Sensortec BME280 API using TI Drivers I2C */
#include <ti/drivers/I2C.h>

#define BOSCH_SENSORTEC_BME280_I2CSLAVE_DEFAULT 0x77

/* Data types */
typedef struct {
	Uint16 humidity_raw;
	Uint32 temperature_raw;
	Uint32 pressure_raw;
} BME280_RawData;

/* Basic API */
Void BME280_init(I2C_Handle, Uint8 slaveaddr);
Bool BME280_open();
Bool BME280_close();
BME280_RawData *BME280_read(); // Runs BME280 in Forced mode with 4x oversampling, no IIR filter on Pressure.

/* Numeric interpretation/compensation API for extracting results */

// Output degrees Celsius with 0.01C resolution.  Divide by 100 for whole degrees.
// **** This MUST be run once before Pressure or Humidity can be calculated! ****
Int32 BME280_compensated_Temperature(BME280_RawData *);

// Pressure in Pascals as unsigned 32-bit integer in Q24.8 format; divide by 256 for whole Pascals
Uint32 BME280_compensated_Pressure(BME280_RawData *);

// Humidity in %relativehumidity as unsigned 32-bit integer in Q22.10 format; divide by 1024 for whole %RH
Uint32 BME280_compensated_Humidity(BME280_RawData *);



/* Internal API */
Void BME280_setAddress(Uint8 memAddress);
Void BME280_writeReg(Uint8 memAddress, Uint8 value);
Uint8 BME280_readReg(Uint8 memAddress);
Uint16 BME280_readWord(Uint8 memAddress); // Interprets Big-Endian format of the BME280
Uint32 BME280_readWord20(Uint8 memAddress); // Interprets Big-Endian with four LSB bits present in MSB of last byte
BME280_RawData *BME280_readMeasurements();


/* Register defines and constants from BME280 datasheet */
#define BME280_REG_ID  0xD0
#define BME280_REG_CALIB00 0x88
#define BME280_REG_RESET 0xE0
#define BME280_REG_CALIB26 0xE1
#define BME280_REG_CTRL_HUM 0xF2
#define BME280_REG_STATUS 0xF3
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG 0xF5

#define BME280_REG_PRESSURE 0xF7
#define BME280_REG_PRES_MSB 0xF7
#define BME280_REG_PRES_LSB 0xF8
#define BME280_REG_PRES_XLSB 0xF9

#define BME280_REG_TEMPERATURE 0xFA
#define BME280_REG_TEMP_MSB 0xFA
#define BME280_REG_TEMP_LSB 0xFB
#define BME280_REG_TEMP_XLSB 0xFC

#define BME280_REG_HUMIDITY 0xFD
#define BME280_REG_HUM_MSB 0xFD
#define BME280_REG_HUM_LSB 0xFE

#define BME280_CHIPID 0x60
#define BME280_CTRL_HUM_OSRS__SKIPPED (0)
#define BME280_CTRL_HUM_OSRS__1       (1)
#define BME280_CTRL_HUM_OSRS__2       (2)
#define BME280_CTRL_HUM_OSRS__4       (3)
#define BME280_CTRL_HUM_OSRS__8       (4)
#define BME280_CTRL_HUM_OSRS__16      (5)

#define BME280_STATUS_MEASURING 0x04
#define BME280_STATUS_IM_UPDATE 0x01

#define BME280_CTRL_MEAS_OSRS_T_SKIPPED (0 << 5)
#define BME280_CTRL_MEAS_OSRS_T__1      (1 << 5)
#define BME280_CTRL_MEAS_OSRS_T__2      (2 << 5)
#define BME280_CTRL_MEAS_OSRS_T__4      (3 << 5)
#define BME280_CTRL_MEAS_OSRS_T__8      (4 << 5)
#define BME280_CTRL_MEAS_OSRS_T__16     (5 << 5)

#define BME280_CTRL_MEAS_OSRS_P_SKIPPED (0 << 2)
#define BME280_CTRL_MEAS_OSRS_P__1      (1 << 2)
#define BME280_CTRL_MEAS_OSRS_P__2      (2 << 2)
#define BME280_CTRL_MEAS_OSRS_P__4      (3 << 2)
#define BME280_CTRL_MEAS_OSRS_P__8      (4 << 2)
#define BME280_CTRL_MEAS_OSRS_P__16     (5 << 2)

#define BME280_CTRL_MEAS_MODE_SLEEP     (0)
#define BME280_CTRL_MEAS_MODE_FORCED    (1)
#define BME280_CTRL_MEAS_MODE_NORMAL    (3)

#define BME280_CONFIG_SPI3WIRE              (1)

#define BME280_CONFIG_IIR_FILTER_COEF__OFF  (0)
#define BME280_CONFIG_IIR_FILTER_COEF__2    (1)
#define BME280_CONFIG_IIR_FILTER_COEF__4    (2)
#define BME280_CONFIG_IIR_FILTER_COEF__8    (3)
#define BME280_CONFIG_IIR_FILTER_COEF__16   (4)

#define BME280_CONFIG_STANDBY_TIME__0_5     (0 << 5)
#define BME280_CONFIG_STANDBY_TIME__62_5    (1 << 5)
#define BME280_CONFIG_STANDBY_TIME__125     (2 << 5)
#define BME280_CONFIG_STANDBY_TIME__250     (3 << 5)
#define BME280_CONFIG_STANDBY_TIME__500     (4 << 5)
#define BME280_CONFIG_STANDBY_TIME__1000    (5 << 5)
#define BME280_CONFIG_STANDBY_TIME__10      (6 << 5)
#define BME280_CONFIG_STANDBY_TIME__20      (7 << 5)


#endif /* BME280_H_ */

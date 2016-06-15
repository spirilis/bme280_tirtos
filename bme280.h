/*
 * @file bme280.h
 * @brief BME280 Library Header
 * @headerfile <>
 * @details Bosch Sensortec BME280 driver library for TI-RTOS using I2C bus
 *
 * @author Eric Brundick
 * @date 2016
 * @version 100
 * @copyright (C) 2016 Eric Brundick spirilis at linux dot com
 *  @n Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
 *  @n (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge,
 *  @n publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to
 *  @n do so, subject to the following conditions:
 *  @n
 *  @n The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  @n
 *  @n THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  @n OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 *  @n BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 *  @n OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *  @n
 *  @n Parts of this codebase derive from BOSCH SENSORTEC calibration compensation example code and is provided by BOSCH with no
 *  @n implied warranty.  The end-user assumes all responsibility for the performance of this codebase.
 *  @n BOSCH SENSORTEC also states in their datasheet the end-user bears all risk for the use of this product and they do not consider
 *  @n the product suitable for life-sustaining or security sensitive systems.
 *  @n
 *  @n A copy of the BME280 product datasheet may be found on BOSCH SENSORTEC's product page:
 *  @n https://www.bosch-sensortec.com/bst/products/all_products/bme280
 *
 */

#ifndef BME280_H_
#define BME280_H_

/* Bosch Sensortec BME280 API using TI Drivers I2C */
#include <ti/drivers/I2C.h>
#include <ti/sysbios/knl/Semaphore.h>

/// @brief Default I2C Slave address for the BME280
#define BOSCH_SENSORTEC_BME280_I2CSLAVE_DEFAULT 0x77

// If this is defined, BME280_readMeasurements() will System_printf(".\r\n"); every time the
// STATUS register is found to have MEASURING==1 or IM_UPDATE==1
//#define BME280_DEBUG_STATUS_POLLING 1

/* Data types */
/// @brief Holds raw register values for measurements
/// @details This struct type is returned in pointer form by any BME280 API calls
///          which pull measurement data from the device; it is used as a parameter
///          for the compensation computation functions that derive usable values
///          for the measurements.
typedef struct {
	Uint16 humidity_raw;
	Uint32 temperature_raw;
	Uint32 pressure_raw;
} BME280_RawData;

/// @brief Callback function for BME280_periodic() mode (see bottom of this header)
typedef void(*BME280_Callback)(BME280_RawData *);

/* Basic API */
Void BME280_init(I2C_Handle, Uint8 slaveaddr); /// @brief Driver initialization
Bool BME280_open();                            /// @brief Make contact with the chip and read calibration registers
Bool BME280_close();                           /// @brief Reset chip
BME280_RawData *BME280_read();                 /// @brief Initiate a Forced measurement, poll to completion, read & return raw data
/// @brief Collect current data
/// @details This will first poll the STATUS register to ascertain no measurements are in progress; if they are, it
///          will perform Task_sleep() and poll again.  Since this uses Task_sleep(), this function must ALWAYS
///          be run within Task context e.g. not within a Swi or a Clock callback.
///          The STATUS register poll will start with a 2ms sleep and double the time until <timeout> is exceeded.
///          When timeout = 0, it will poll indefinitely.
BME280_RawData *BME280_readMeasurements(Uint16 timeout);

/* Numeric interpretation/compensation API for extracting results */

/// @brief Compute Temperature from BME280_RawData struct
/// @details Output degrees Celsius with 0.01C resolution.  Divide by 100 for whole degrees.
///          This function needs to be run before computing Pressure or Humidity to compute
///          the _t_fine constant used by the Pressure and Humidity compensation functions below.
Int32 BME280_compensated_Temperature(BME280_RawData *);

/// @brief Compute Pressure from BME280_RawData struct
/// @details Pressure in Pascals as unsigned 32-bit integer in Q24.8 format; divide by 256 for whole Pascals
Uint32 BME280_compensated_Pressure(BME280_RawData *);

/// @brief Compute Relative Humidity from BME280_RawData struct
/// @details Humidity in %relativehumidity as unsigned 32-bit integer in Q22.10 format; divide by 1024 for whole %RH
Uint32 BME280_compensated_Humidity(BME280_RawData *);

/* Look at the bottom of this header file for the Periodic Polling API. */


/* Internal API */
Void BME280_setAddress(Uint8 memAddress);
Void BME280_writeReg(Uint8 memAddress, Uint8 value);
Uint8 BME280_readReg(Uint8 memAddress);
Uint16 BME280_readWord(Uint8 memAddress); // Interprets Big-Endian format of the BME280
Uint32 BME280_readWord20(Uint8 memAddress); // Interprets Big-Endian with four LSB bits present in MSB of last byte


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

#define BME280_RESET_ASSERT (0xB6)

/// @brief Enum for available periodic sensing timeframes
typedef enum {
	BME280_EVERY_62_5MS = BME280_CONFIG_STANDBY_TIME__62_5,
	BME280_EVERY_125MS = BME280_CONFIG_STANDBY_TIME__125,
	BME280_EVERY_250MS = BME280_CONFIG_STANDBY_TIME__250,
	BME280_EVERY_500MS = BME280_CONFIG_STANDBY_TIME__500,
	BME280_EVERY_1000MS = BME280_CONFIG_STANDBY_TIME__1000,
	BME280_EVERY_10MS = BME280_CONFIG_STANDBY_TIME__10,
	BME280_EVERY_20MS = BME280_CONFIG_STANDBY_TIME__20
} BME280_Period;

/// @brief Used internally to resolve BME280_Period enum to millisecond count
inline static Uint16 BME280_period_to_millis(BME280_Period p) {
	switch (p) {
	case BME280_EVERY_62_5MS:
		return 63;
	case BME280_EVERY_125MS:
		return 125;
	case BME280_EVERY_250MS:
		return 250;
	case BME280_EVERY_500MS:
		return 500;
	case BME280_EVERY_1000MS:
		return 1000;
	case BME280_EVERY_10MS:
		return 10;
	case BME280_EVERY_20MS:
		return 20;
	default:
		return 0;
	}
}

/* Periodic "NORMAL" mode API */
Semaphore_Handle BME280_periodic(BME280_Period);  /// @brief Initiate Normal (periodic) measurement mode
Void BME280_stop();                               /// @brief Halt periodic operation

#endif /* BME280_H_ */

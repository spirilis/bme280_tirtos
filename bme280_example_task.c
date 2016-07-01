/*
 * @file bme280_example_task.c
 * @brief BME280 Library RTOS example
 * @headerfile <bme280.h>
 * @details Example TI-RTOS task code for utilizing BME280 library
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
 */

#include <string.h>
#include <stdio.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/I2C.h>

/* Board Header files */
#include "Board.h"

#include "bme280.h"

static char ubuf[256];

void bme280_Example_Task(UArg arg0, UArg arg1)
{
    I2C_Params i2cParam;
    I2C_Handle i2c;

    /* Open I2C bus driver */
    I2C_Params_init(&i2cParam);
    i2cParam.transferMode = I2C_MODE_BLOCKING;
    i2cParam.transferCallbackFxn = NULL;
    i2cParam.bitRate = I2C_400kHz;
    i2c = I2C_open(0, &i2cParam);  // Open I2C instance #0 with 400KHz bus speed
    if (i2c == NULL) {
    	System_abort("I2C0: Failure opening port\r\n");
    }

    System_printf("Successfully initialized I2C driver.\r\n");
    System_flush();

    // Init the BME280 API
    BME280_init(i2c, BOSCH_SENSORTEC_BME280_I2CSLAVE_DEFAULT);
    if (!BME280_open()) {
    	System_printf("ERROR opening BME280_open()\r\n");
    	System_flush();
    } else {
    	System_printf("BME280 opened; polling every 500ms\r\n");
    	System_flush();
    }

    while(1) {

		// Read & interpret results, spitting to CIO console
		BME280_RawData *rd = BME280_read();
		Int32 tempC = BME280_compensated_Temperature(rd);
		sprintf(ubuf, "Temp: %d C (%d F), humidity: %d%%%%, Pressure: %u hPa\r\n", \
						tempC / 100,
						((tempC * 9) / 5) / 100 + 32,
						BME280_compensated_Humidity(rd) / 1024,
						(BME280_compensated_Pressure(rd) / 256) / 1000);
		System_printf(ubuf);
		System_flush();

		// Wait 500ms and poll again
		Task_sleep(500);

    }
}

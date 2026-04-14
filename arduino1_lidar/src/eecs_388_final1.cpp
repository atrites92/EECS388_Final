/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

#include "eecs_388_lib.h"

/******************************************************************************
 *   Function: setup() - Initializes the Arduino System
 *      Pre condition: 
 *          Hardware must be properly connected (sensors, LEDS, etc.)
 *      Post condition: 
 *          Runs initialization calls one time on power up
 *          Input/Output (IO) pins are configured
 *******************************************************************************/
void setup() 
{
    uart_init();

    return;
}

/******************************************************************************
 *   Function: loop() - Main execution loop
 *      Pre condition: 
 *          setup() has been executed and system is initialized
 *      Post condition: 
 *          Performs a single iteration of the system's function
 *          Repeates indefinetely unless the board is reset or powered off
 *******************************************************************************/
void loop() {
    uint16_t dist = 0;              /* LIDAR distance data is 16 bits. */
    uint8_t dataFrame[9];

    ser_printline("Setup completed.");
    ser_write( '\n' );

    while( 1 ) { 
        //Read 9 bytes from TFmini sensor:
        // 'Y', 'Y', dist_L, dist_H, str_L, str_H, reserved, rawQual, chkSum
        uint64_t chkSumConfirm = 0;
        for (int i=0; i < 9; i++){
            dataFrame[i] = ser_read();
            if (i < 8){
                chkSumConfirm += dataFrame[i];
            }
        }

        //Get distance
        if ('Y' == dataFrame[0] && 'Y' == dataFrame[1]) {
            dist = dataFrame[2] | (dataFrame[3] << 8);
            ser_printf("%d", dist);
        }

        //Checksum Confirmation
        uint8_t chkSumLSB = (chkSumConfirm & 0xFF);
        ser_printf("%d", dataFrame[8]);
        ser_printf("%d", chkSumLSB);
        if (dataFrame[8] == chkSumLSB){
            ser_printline("Checksum Confirmed");
        } else {
            ser_printline("Checksum Failed");
        }
        //Send distance to Arduino 2
        ser_write(dataFrame[2]);
        ser_write(dataFrame[3]);
    }
    return;
}
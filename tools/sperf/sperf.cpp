/*
    Copyright 2017-2019 (C) Alexey Dynda

    This file is part of Tiny Protocol Library.

    Protocol Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Protocol Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Protocol Library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "serial_api.h"
#include "proto/light/tiny_light.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if ( argc < 2)
    {
        printf("Usage: sperf Serial\n");
        printf("        COM1, COM2 ...  for Windows\n");
        printf("        /dev/ttyS0, /dev/ttyS1 ...  for Linux\n");
        return 1;
    }

    SerialHandle hPort = OpenSerial(argv[1], SERIAL_115200);

    if ( hPort == INVALID_SERIAL )
    {
        printf("Error opening serial port\n");
        return 1;
    }

    uint8_t outBuffer[4096]{};
    uint8_t inBuffer[4096]{};
    int sync_frame_len = 0;
    STinyLightData tiny;
    /* Initialize tiny light protocol */
    tiny_light_init(&tiny, SerialSend, SerialReceive, hPort);
    /* Wait for remote side to be ready */
    strcpy((char *)outBuffer, "SYNC FRAME");
    sync_frame_len = (int)strlen((char *)outBuffer);
    printf("Syncing\n");
    // SYNC STEP is required, because in case of using /dev/ttyUSB0 of Arduino
    // the boards goes offline state for 2 seconds
    for (;;)
    {
        /* Send single byte to remote device */
        if ( tiny_light_send( &tiny, outBuffer, sync_frame_len ) < 0 )
        {
            continue;
        }
        /* Check if we receive byte back */
        int err = tiny_light_read( &tiny, inBuffer, sizeof(inBuffer) );
        if ( err == sync_frame_len && inBuffer[0] == 'S' )
        {
            break;
        }
    };
    /* Start speed test */
    printf("Test is in progress\n");
    time_t startTs = time(NULL);
    uint32_t bytesSent = 0;
    uint32_t bytesReceived = 0;
    /* Run test for 15 seconds */
    while ( time(NULL) < (startTs + 15) )
    {
        /* Prepare data to send */
        sprintf((char *)outBuffer, "%u TEST IS RUNNING. MIDDLE SIZE PKT", bytesSent);
        int len = strlen((char *)outBuffer) + 1;
        /* Send data to remote side */
        tiny_light_send( &tiny, outBuffer, len );
        /* Wait until the data are sent back to us by remote side */
        int temp;
        for(;;)
        {
            temp = tiny_light_read( &tiny, inBuffer, sizeof(inBuffer) );
            if (temp == sync_frame_len)
            {
                // if this is sync frame just skip it
                continue;
            }
            break;
        };
        if ( temp == len)
        {
            bytesSent += len;
            bytesReceived += len;
        }
        else
        {
            printf("ERROR: %i bytes received. expected %i\n", temp, len);
            break;
        }
    }
    tiny_light_close(&tiny);
    CloseSerial(hPort);

    // We summarize TX and RX speed, because we're working in half duplex mode
    // When we receive something, we do not transmit anything.
    printf("Registered speed: %u baud\n", (bytesSent + bytesReceived) * 8 / 9);
    return 0;
}

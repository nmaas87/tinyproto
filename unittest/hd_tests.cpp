/*
    Copyright 2017 (C) Alexey Dynda

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


#include <functional>
#include <CppUTest/TestHarness.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "helpers/tiny_hd_helper.h"


TEST_GROUP(HdTests)
{
    void setup()
    {
        // ...
    }

    void teardown()
    {
        // ...
    }
};

#if 1
TEST(HdTests, TinyHd_Send)
{
    uint8_t      txbuf[128];
    FakeWire     line1;
    FakeWire     line2;
    FakeChannel  channel1( &line1, &line2 );
    FakeChannel  channel2( &line2, &line1 );
    uint16_t     nreceived = 0;
    uint16_t     nsent = 0;

    TinyHelperHd helper1( &channel1,
                          sizeof(txbuf),
                          [&nreceived](uint16_t uid, uint8_t *buf, int len)->void
                          {
//                              printf("received uid:%04X, len: %d\n", uid, len);
                              nreceived++;
                          });
    TinyHelperHd helper2( &channel2, sizeof(txbuf) );

    helper1.run(true);

    // sent 4 packets small packets with ACK
    for (nsent = 0; nsent < 4; nsent++)
    {
        txbuf[0] = 0xAA;
        txbuf[1] = 0xFF;
//        printf("sending %d bytes\n", 2);
        int result = helper2.send_wait_ack( txbuf, 2 );
        CHECK( result >= 0 );
    }
    // sleep for 2 ms befoe last frame arrives
    usleep(2000);
    CHECK_EQUAL( 4, nreceived );
}

#endif

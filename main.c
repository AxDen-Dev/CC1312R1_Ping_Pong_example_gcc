/*
 * Copyright (c) 2015-2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

/* Board Header files */
#include "ti_drivers_config.h"

#include "Protocol.h"
#include "SensorTask.h"
#include "RadioTask.h"

uint8_t radio_init;
uint8_t mac_address[MAC_ADDRESS_SIZE];
uint8_t packet_receive;
int8_t rssi;
radio_packet_protocol_t receive_radio_packet;

#ifdef EXAMPLE_RX

uint8_t payload_index;
uint8_t payload_size;
uint8_t payload[PAYLOAD_SIZE];

#endif

/*
 *  ======== main ========
 */
int main(void)
{

    radio_init = RADIO_INIT_WAIT;

    memset(mac_address, 0x00, sizeof(mac_address));

    packet_receive = 0x00;

    rssi = 0;
    memset(&receive_radio_packet, 0x00, sizeof(radio_packet_protocol_t));

#ifdef EXAMPLE_RX

    payload_index = 0;
    payload_size = 0;
    memset(payload, 0x00, sizeof(payload));

#endif

    Board_initGeneral();

    SensorTask_init();

    RadioTask_init();

    /* Start BIOS */
    BIOS_start();

    return (0);
}

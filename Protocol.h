
#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "stdio.h"
#include "easylink/EasyLink.h"

#define EXAMPLE_TX
//#define EXAMPLE_RX

#define MAC_ADDRESS_SIZE 8

#define TX_TIMEOUT_COUNT 100

#define PAYLOAD_SIZE 51

typedef enum
{

    RADIO_INIT_WAIT = 0x00, RADIO_INIT_SUCCESS

} RADIO_INIT;

typedef union
{

    struct
    {

        uint8_t company_id[2];
        uint8_t device_id[2];
        uint8_t mac_address[8];
        uint8_t control_number;
        uint8_t paylaod[51];

    } Packet;

    uint8_t buffer[64];

} radio_packet_protocol_t;

#endif /* PROTOCOL_H_ */

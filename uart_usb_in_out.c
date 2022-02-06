
#include "uart_usb_in_out.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "Protocol.h"

#ifdef CC1312R1

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

#endif

#define UART_BUFFER_SIZE 512

static uint16_t uart_tx_buffer_length = 0x00;
static uint8_t uart_tx_buffer[UART_BUFFER_SIZE] = { 0x00 };

static UART2_Handle uart2;

void set_uart_usb_in_out_uart2_instance(UART2_Handle uart)
{

    uart2 = uart;

}

static void uart_usb_in_out_data_send(uint8_t *data, uint16_t size)
{

    if (uart2 != NULL)
    {

        UART2_write(uart2, data, size, NULL);

    }

}

void set_uart_usb_in_out_write_string(char *data)
{

    uart_tx_buffer_length = 0;
    memset(uart_tx_buffer, 0x00, sizeof(uart_tx_buffer));

    uart_tx_buffer_length = sprintf((char*) uart_tx_buffer, "%s\r\n", data);

    uart_usb_in_out_data_send(uart_tx_buffer, uart_tx_buffer_length);

}

void set_uart_usb_in_out_find_i2c_address(uint8_t address)
{

    uart_tx_buffer_length = 0;
    memset(uart_tx_buffer, 0x00, sizeof(uart_tx_buffer));

    uart_tx_buffer_length = sprintf((char*) uart_tx_buffer,
                                    "I2C Address : %02X\r\n", address);

    uart_usb_in_out_data_send(uart_tx_buffer, uart_tx_buffer_length);

}

void set_uart_usb_in_out_mac_address(uint8_t *mac_address)
{

    uart_tx_buffer_length = 0;
    memset(uart_tx_buffer, 0x00, sizeof(uart_tx_buffer));

    uart_tx_buffer_length = sprintf((char*) uart_tx_buffer, "MAC Address : ");

    for (uint8_t i = 0; i < 8; i++)
    {

        uart_tx_buffer_length += sprintf(
                (char*) uart_tx_buffer + uart_tx_buffer_length, "%02X",
                mac_address[i]);

    }

    uart_tx_buffer_length += sprintf(
            (char*) uart_tx_buffer + uart_tx_buffer_length, "\r\n");

    uart_usb_in_out_data_send(uart_tx_buffer, uart_tx_buffer_length);

}

void set_uart_usb_in_out_voltage(int16_t voltage)
{

    uart_tx_buffer_length = 0;
    memset(uart_tx_buffer, 0x00, sizeof(uart_tx_buffer));

    uart_tx_buffer_length = sprintf((char*) uart_tx_buffer,
                                    "Voltage : %d V\r\n", voltage);

    uart_usb_in_out_data_send(uart_tx_buffer, uart_tx_buffer_length);

}

void set_uart_usb_in_out_payload(radio_packet_protocol_t receive_radio_packet,
                                 int8_t rssi)
{

    uart_tx_buffer_length = 0;
    memset(uart_tx_buffer, 0x00, sizeof(uart_tx_buffer));

    uart_tx_buffer_length = sprintf((char*) uart_tx_buffer,
                                    "\r\nPacket Receive\r\n");

    uint16_t company_id = receive_radio_packet.Packet.company_id[0] << 8;
    company_id |= receive_radio_packet.Packet.company_id[1];

    uart_tx_buffer_length += sprintf(
            (char*) uart_tx_buffer + uart_tx_buffer_length,
            "Company ID : %d\r\n", company_id);

    uint16_t device_id = receive_radio_packet.Packet.device_id[0] << 8;
    device_id |= receive_radio_packet.Packet.device_id[1];

    uart_tx_buffer_length += sprintf(
            (char*) uart_tx_buffer + uart_tx_buffer_length,
            "Device ID : %d\r\n", device_id);

    uart_tx_buffer_length += sprintf(
            (char*) uart_tx_buffer + uart_tx_buffer_length, "MAC Address : ");

    for (uint8_t i = 0; i < 8; i++)
    {

        uart_tx_buffer_length += sprintf(
                (char*) uart_tx_buffer + uart_tx_buffer_length, "%02X",
                receive_radio_packet.Packet.mac_address[i]);

    }

    uart_tx_buffer_length += sprintf(
            (char*) uart_tx_buffer + uart_tx_buffer_length, "\r\n");

    uart_tx_buffer_length += sprintf(
            (char*) uart_tx_buffer + uart_tx_buffer_length,
            "Control Number : %d\r\n",
            receive_radio_packet.Packet.control_number);

    uart_tx_buffer_length += sprintf(
            (char*) uart_tx_buffer + uart_tx_buffer_length, "RSSI : %d\r\n",
            rssi);

    uart_tx_buffer_length += sprintf(
            (char*) uart_tx_buffer + uart_tx_buffer_length,
            "Payload : %s\r\n", receive_radio_packet.Packet.paylaod);

    uart_usb_in_out_data_send(uart_tx_buffer, uart_tx_buffer_length);

}

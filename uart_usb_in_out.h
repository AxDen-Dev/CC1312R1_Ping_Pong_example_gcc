
#ifndef UART_USB_IN_OUT_H_
#define UART_USB_IN_OUT_H_

#include <stdint.h>
#include <stdbool.h>
#include "board_define.h"
#include "Protocol.h"

#include <ti/drivers/I2C.h>
#include <ti/drivers/UART2.h>
#include <ti/drivers/PIN.h>
#include "ti_drivers_config.h"

void set_uart_usb_in_out_uart2_instance(UART2_Handle uart);

void set_uart_usb_in_out_write_string(char *data);

void set_uart_usb_in_out_find_i2c_address(uint8_t address);

void set_uart_usb_in_out_mac_address(uint8_t *mac_address);

void set_uart_usb_in_out_voltage(int16_t voltage);

void set_uart_usb_in_out_payload(radio_packet_protocol_t receive_radio_packet,
                                 int8_t rssi);

#endif /* UART_USB_IN_OUT_H_ */

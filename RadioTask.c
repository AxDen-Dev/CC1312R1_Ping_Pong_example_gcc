
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Clock.h>

/* Drivers */
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>

#include <ti/drivers/power/PowerCC26XX.h>

/* Board Header files */
#include "ti_drivers_config.h"

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)

/* EasyLink API Header files */
#include "easylink/EasyLink.h"

#include "RadioTask.h"
#include "SensorTask.h"
#include "Protocol.h"
#include "board_define.h"

#define RADIO_TASK_STACK_SIZE 1024
#define RADIO_TASK_TASK_PRIORITY   3

#define RADIO_TASK_EVENT_ALL                  0xFFFFFFFF
#define RADIO_TASK_EVENT_ACK (uint32_t)(1 << 1)
#define RADIO_TASK_EVENT_ACK_TIMEOUT (uint32_t)(1 << 2)

#define ACK_TIME_OUT 160

const uint8_t addressFilterValue[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,
0x0F};

const int8_t TX_POWER = 10;

const uint16_t COMPANY_ID = 0x0000;
const uint16_t DEVICE_ID = 0x0000;

static Task_Params radioTaskParams;
Task_Struct radioTask; /* not static so you can see in ROV */
static uint8_t radioTaskStack[RADIO_TASK_STACK_SIZE];

Event_Struct radioEvent; /* not static so you can see in ROV */
static Event_Handle radioEventHandle;

Semaphore_Struct radioAccessSem; /* not static so you can see in ROV */
static Semaphore_Handle radioAccessSemHandle;

static EasyLink_TxPacket txPacket;
static radio_packet_protocol_t radio_packet_protocol;

extern uint8_t radio_init;
extern uint8_t mac_address[MAC_ADDRESS_SIZE];
extern uint8_t packet_receive;
extern int8_t rssi;
extern radio_packet_protocol_t receive_radio_packet;

#ifdef EXAMPLE_RX

extern uint8_t payload_index;
extern uint8_t payload_size;
extern uint8_t payload[PAYLOAD_SIZE];

#endif

#ifdef EXAMPLE_RX

static void wait_ms(uint32_t wait)
{

    Task_sleep(wait * 1000 / Clock_tickPeriod);

}

#endif

static void sendPacketCallback(uint8_t *payload, uint8_t payload_size)
{

#ifdef EXAMPLE_TX

    memset(radio_packet_protocol.Packet.paylaod, 0x00,
           sizeof(radio_packet_protocol.Packet.paylaod));

    memcpy(radio_packet_protocol.Packet.paylaod, payload, payload_size);

    Semaphore_post(radioAccessSemHandle);

#endif

}

static void rxDoneCallback(EasyLink_RxPacket *rxPacket, EasyLink_Status status)
{

    if (status == EasyLink_Status_Success)
    {

        if (rxPacket->len <= sizeof(receive_radio_packet.buffer))
        {

            rssi = rxPacket->rssi;

            memcpy(receive_radio_packet.buffer, rxPacket->payload,
                   rxPacket->len);

        }

        Event_post(radioEventHandle, RADIO_TASK_EVENT_ACK);

    }
    else
    {

        Event_post(radioEventHandle, RADIO_TASK_EVENT_ACK_TIMEOUT);

    }

}

#ifdef EXAMPLE_TX

static void radio_sensor_data_packet_send(radio_packet_protocol_t radio_packet)
{

    txPacket.len = 0;
    txPacket.absTime = 0;
    memset(txPacket.dstAddr, 0x00, sizeof(txPacket.dstAddr));
    memset(txPacket.payload, 0x00, sizeof(txPacket.payload));

    memcpy(txPacket.dstAddr, addressFilterValue, 8);

    memcpy(txPacket.payload, radio_packet.buffer, sizeof(radio_packet.buffer));
    txPacket.len = sizeof(radio_packet.buffer);

    EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut,
                     EasyLink_ms_To_RadioTime(ACK_TIME_OUT));

    if (EasyLink_transmit(&txPacket) != EasyLink_Status_Success)
    {

        SysCtrlSystemReset();

    }

    if (EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success)
    {

        SysCtrlSystemReset();

    }

}

#endif

#ifdef EXAMPLE_RX

static void radio_sensor_data_packet_send(radio_packet_protocol_t radio_packet)
{

    txPacket.len = 0;
    txPacket.absTime = 0;
    memset(txPacket.dstAddr, 0x00, sizeof(txPacket.dstAddr));
    memset(txPacket.payload, 0x00, sizeof(txPacket.payload));

    memcpy(txPacket.dstAddr, receive_radio_packet.Packet.mac_address, 8);

    memcpy(txPacket.payload, radio_packet.buffer, sizeof(radio_packet.buffer));
    txPacket.len = sizeof(radio_packet.buffer);

    if (EasyLink_transmit(&txPacket) != EasyLink_Status_Success)
    {

        SysCtrlSystemReset();

    }

}

#endif

static void radioTaskFunction(UArg arg0, UArg arg1)
{

    EasyLink_Params easyLink_params;
    EasyLink_Params_init(&easyLink_params);
    easyLink_params.ui32ModType = EasyLink_Phy_5kbpsSlLr;

    if (EasyLink_init(&easyLink_params) != EasyLink_Status_Success)
    {

        SysCtrlSystemReset();

    }

    EasyLink_getIeeeAddr(mac_address);

    EasyLink_setFrequency(920000000);

#ifdef EXAMPLE_TX

    EasyLink_enableRxAddrFilter(mac_address, 8, 1);

#endif

#ifdef EXAMPLE_RX

    EasyLink_enableRxAddrFilter((uint8_t*) addressFilterValue, 8, 1);

#endif

    SensorTask_registerPacketSendRequestCallback(sendPacketCallback);

    radio_packet_protocol.Packet.company_id[0] = COMPANY_ID >> 8;
    radio_packet_protocol.Packet.company_id[1] = COMPANY_ID;

    radio_packet_protocol.Packet.device_id[0] = DEVICE_ID >> 8;
    radio_packet_protocol.Packet.device_id[1] = DEVICE_ID;

    memcpy(radio_packet_protocol.Packet.mac_address, mac_address, 8);

    radio_packet_protocol.Packet.control_number = 0;

    radio_init = RADIO_INIT_SUCCESS;

#ifdef EXAMPLE_RX

    if (EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success)
    {

        SysCtrlSystemReset();

    }

#endif

    while (1)
    {

#ifdef EXAMPLE_TX

        Semaphore_pend(radioAccessSemHandle, BIOS_WAIT_FOREVER);

        EasyLink_setRfPower(TX_POWER);

        radio_sensor_data_packet_send(radio_packet_protocol);

        uint32_t events = Event_pend(radioEventHandle, 0,
        RADIO_TASK_EVENT_ALL,
                                     BIOS_WAIT_FOREVER);

        if (events == RADIO_TASK_EVENT_ACK)
        {

            packet_receive = 0x01;

        }

#endif

#ifdef EXAMPLE_RX

        uint32_t events = Event_pend(radioEventHandle, 0,
        RADIO_TASK_EVENT_ALL,
                                     BIOS_WAIT_FOREVER);

        if (events == RADIO_TASK_EVENT_ACK)
        {

            EasyLink_setRfPower(TX_POWER);

            memset(radio_packet_protocol.Packet.paylaod, 0x00,
                   sizeof(radio_packet_protocol.Packet.paylaod));

            if (payload_size == 0)
            {

                payload_size = sprintf(
                        (char*) radio_packet_protocol.Packet.paylaod, "%s",
                        "Pong");

            }
            else
            {

                memcpy(radio_packet_protocol.Packet.paylaod, payload,
                       payload_size);

            }

            payload_index = 0;

            payload_size = 0;
            memset(payload, 0x00, PAYLOAD_SIZE);

            radio_sensor_data_packet_send(radio_packet_protocol);

            packet_receive = 0x01;

        }

        wait_ms(500);

        if (EasyLink_receiveAsync(rxDoneCallback, 0) != EasyLink_Status_Success)
        {

            SysCtrlSystemReset();

        }

#endif

    }

}

void RadioTask_init(void)
{

    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    Semaphore_construct(&radioAccessSem, 0, &semParam);
    radioAccessSemHandle = Semaphore_handle(&radioAccessSem);

    Event_Params eventParam;
    Event_Params_init(&eventParam);
    Event_construct(&radioEvent, &eventParam);
    radioEventHandle = Event_handle(&radioEvent);

    Task_Params_init(&radioTaskParams);
    radioTaskParams.stackSize = RADIO_TASK_STACK_SIZE;
    radioTaskParams.priority = RADIO_TASK_TASK_PRIORITY;
    radioTaskParams.stack = &radioTaskStack;
    Task_construct(&radioTask, radioTaskFunction, &radioTaskParams, NULL);

}

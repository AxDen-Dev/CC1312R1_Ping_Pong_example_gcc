
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

/* TI-RTOS Header files */
#include <ti/drivers/I2C.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/UART2.h>
#include <ti/drivers/NVS.h>
#include <ti/drivers/ADC.h>
#include <ti/drivers/Watchdog.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/aon_batmon.h)

#include <ti/devices/cc13x2_cc26x2/driverlib/aux_adc.h>

/* Board Header files */
#include "ti_drivers_config.h"

/* Application Header files */
#include "SensorTask.h"
#include "Protocol.h"

#include "board_define.h"

#include "uart_usb_in_out.h"

#define SENSOR_TASK_STACK_SIZE 1024
#define SENSOR_TASK_TASK_PRIORITY   3

#define SENSOR_TASK_EVENT_ALL                         0xFFFFFFFF
#define SENSOR_TASK_RADIO_PACKET_SEND    (uint32_t)(1 << 1)
#define SENSOR_TASK_RADIO_PACKET_RECEIVE    (uint32_t)(1 << 2)

#define TIMER_TIMEOUT 100

PIN_Config pinTable[] = {

GPIO_LED_0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,

GPIO_LED_1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL
| PIN_DRVSTR_MAX,

GPIO_LED_2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL
| PIN_DRVSTR_MAX,

GPIO_0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL
| PIN_DRVSTR_MAX,

GPIO_1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL
| PIN_DRVSTR_MAX,

PIN_TERMINATE

};

PIN_Config buttonTable[] = {

GPIO_BUTTON_0 | PIN_INPUT_EN | PIN_PULLDOWN | PIN_IRQ_NEGEDGE,

                             GPIO_BUTTON_1 | PIN_INPUT_EN | PIN_PULLDOWN
                                     | PIN_IRQ_NEGEDGE,

                             PIN_TERMINATE

};

static Task_Params sensorTaskParams;
Task_Struct sensorTask; /* not static so you can see in ROV */
static uint8_t sensorTaskStack[SENSOR_TASK_STACK_SIZE];

Event_Struct sensorEvent; /* not static so you can see in ROV */
static Event_Handle sensorEventHandle;

Clock_Struct sensorTimerClock;

static PacketSendRequestCallback packetSendRequestCallback;

static PIN_State pinState;
static PIN_Handle pinHandle;

static PIN_State buttonState;
static PIN_Handle buttonHandle;

static I2C_Handle i2c;

static char uart2_read_buf = 0x00;
static UART2_Handle uart2;

static uint8_t init_state = 0x00;

#ifdef EXAMPLE_TX

static uint16_t tx_timer_count = 0;

static uint8_t payload_index;
static uint8_t payload_size;
static uint8_t payload[PAYLOAD_SIZE];

#endif

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

void uart2ReadCallback(UART2_Handle handle, void *rxBuf, size_t size,
                       void *userArg, int_fast16_t status);

static void wait_ms(uint32_t wait)
{

    Task_sleep(wait * 1000 / Clock_tickPeriod);

}

static void buttonCallback(PIN_Handle handle, PIN_Id pinId)
{

    if (init_state)
    {

        if (pinId == GPIO_BUTTON_0)
        {

            PIN_setOutputValue(pinHandle, GPIO_LED_0,
                               !PIN_getOutputValue(GPIO_LED_0));

        }
        else if (pinId == GPIO_BUTTON_1)
        {

            PIN_setOutputValue(pinHandle, GPIO_LED_1,
                               !PIN_getOutputValue(GPIO_LED_1));

        }

    }

}

static void sensorTimerClockCallBack(UArg arg0)
{

    if (init_state)
    {

#ifdef EXAMPLE_TX

        tx_timer_count += 1;

        if (tx_timer_count > TX_TIMEOUT_COUNT)
        {

            Event_post(sensorEventHandle, SENSOR_TASK_RADIO_PACKET_SEND);
            tx_timer_count = 0;

        }

#endif

        if (packet_receive)
        {

            Event_post(sensorEventHandle, SENSOR_TASK_RADIO_PACKET_RECEIVE);
            packet_receive = 0x00;

        }

    }

}

static void readVoltage()
{

    ADC_Handle adc;
    ADC_Params ADCParams;
    ADC_Params_init(&ADCParams);

    adc = ADC_open(CONFIG_I2C_0, &ADCParams);

    if (adc != NULL)
    {

        uint16_t adc_value;

        int_fast16_t result = ADC_convert(adc, &adc_value);

        ADC_close(adc);

        if (result == ADC_STATUS_SUCCESS)
        {

            uint32_t microVolt = ADC_convertRawToMicroVolts(adc, adc_value);
            microVolt *= 2;
            microVolt /= 100000;

            int16_t voltage = microVolt / 10;
            set_uart_usb_in_out_voltage(voltage);

        }

    }

}

static void i2cScan()
{

    I2C_Params i2cParams;
    I2C_Transaction i2cTransaction;

    uint8_t txBuffer[2] = { 0x00 };
    uint8_t rxBuffer[2] = { 0x00 };

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(CONFIG_I2C_0, &i2cParams);

    if (i2c == NULL)
    {

        SysCtrlSystemReset();

    }

    set_uart_usb_in_out_write_string("Start I2C Scan");
    wait_ms(10);

    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readCount = 0;

    for (uint16_t i = 0; i < 128; i++)
    {

        i2cTransaction.slaveAddress = i;

        if (I2C_transfer(i2c, &i2cTransaction))
        {

            set_uart_usb_in_out_find_i2c_address(i);
            wait_ms(10);

        }

    }

    I2C_close(i2c);

}

static void initUartUSB()
{

    UART2_Params uart2Params;

    UART2_Params_init(&uart2Params);
    uart2Params.readMode = UART2_Mode_CALLBACK;
    uart2Params.readCallback = uart2ReadCallback;
    uart2Params.baudRate = 9600;

    uart2 = UART2_open(CONFIG_UART2_0, &uart2Params);

    set_uart_usb_in_out_uart2_instance(uart2);

}

static void sensorTaskFunction(UArg arg0, UArg arg1)
{

    ADC_init();

    I2C_init();

    pinHandle = PIN_open(&pinState, pinTable);

    if (!pinHandle)
    {
        SysCtrlSystemReset();
    }

    PIN_setOutputValue(pinHandle, GPIO_LED_0, 0);
    PIN_setOutputValue(pinHandle, GPIO_LED_1, 0);
    PIN_setOutputValue(pinHandle, GPIO_LED_2, 0);
    PIN_setOutputValue(pinHandle, GPIO_0, 0);
    PIN_setOutputValue(pinHandle, GPIO_1, 0);

    buttonHandle = PIN_open(&buttonState, buttonTable);

    if (!buttonHandle)
    {

        SysCtrlSystemReset();

    }

    PIN_registerIntCb(buttonHandle, &buttonCallback);

    initUartUSB();

    set_uart_usb_in_out_write_string("\r\n\r\n");
    wait_ms(10);

#ifdef EXAMPLE_TX

    set_uart_usb_in_out_write_string("Start AxDen TX Example");
    wait_ms(10);

#endif

#ifdef EXAMPLE_RX

    set_uart_usb_in_out_write_string("Start AxDen RX Example");
    wait_ms(10);

#endif

    i2cScan();

    readVoltage();

    PIN_setOutputValue(pinHandle, GPIO_LED_0, 1);
    PIN_setOutputValue(pinHandle, GPIO_LED_1, 1);
    PIN_setOutputValue(pinHandle, GPIO_LED_2, 1);

    while (radio_init == RADIO_INIT_WAIT)
    {

        wait_ms(100);

    }

    set_uart_usb_in_out_mac_address(mac_address);

#ifdef EXAMPLE_RX

    set_uart_usb_in_out_write_string("Start RX");
    wait_ms(10);

#endif

    UART2_read(uart2, &uart2_read_buf, 1, NULL);

    init_state = 0x01;

    while (1)
    {

        uint32_t events = Event_pend(sensorEventHandle, 0,
        SENSOR_TASK_EVENT_ALL,
                                     BIOS_WAIT_FOREVER);

#ifdef EXAMPLE_TX

        if (events == SENSOR_TASK_RADIO_PACKET_SEND)
        {

            set_uart_usb_in_out_write_string("\r\nPacket Send");
            wait_ms(10);

            if (payload_size == 0)
            {

                payload_size = sprintf((char*) payload, "%s", "Ping");

            }

            if (packetSendRequestCallback)
            {

                packetSendRequestCallback(payload, payload_size);

                payload_index = 0;

                payload_size = 0;
                memset(payload, 0x00, PAYLOAD_SIZE);

            }

        }
        else if (events == SENSOR_TASK_RADIO_PACKET_RECEIVE)
        {

            set_uart_usb_in_out_payload(receive_radio_packet, rssi);
            wait_ms(10);

        }

#endif

#ifdef EXAMPLE_RX

        if (events == SENSOR_TASK_RADIO_PACKET_RECEIVE)
        {

            set_uart_usb_in_out_payload(receive_radio_packet, rssi);

        }

#endif

        PIN_setOutputValue(pinHandle, GPIO_LED_2,
                           !PIN_getOutputValue(GPIO_LED_2));

    }

}

void SensorTask_init(void)
{

    Event_Params eventParam;
    Event_Params_init(&eventParam);
    Event_construct(&sensorEvent, &eventParam);
    sensorEventHandle = Event_handle(&sensorEvent);

    Task_Params_init(&sensorTaskParams);
    sensorTaskParams.stackSize = SENSOR_TASK_STACK_SIZE;
    sensorTaskParams.priority = SENSOR_TASK_TASK_PRIORITY;
    sensorTaskParams.stack = &sensorTaskStack;
    Task_construct(&sensorTask, sensorTaskFunction, &sensorTaskParams, NULL);

    Clock_Params clockParams;
    Clock_Params_init(&clockParams);
    clockParams.period = TIMER_TIMEOUT * 1000 / Clock_tickPeriod;
    clockParams.startFlag = TRUE;
    Clock_construct(&sensorTimerClock, sensorTimerClockCallBack,
    TIMER_TIMEOUT * 1000 / Clock_tickPeriod,
                    &clockParams);

}

void SensorTask_registerPacketSendRequestCallback(
        PacketSendRequestCallback callback)
{

    packetSendRequestCallback = callback;

}

void uart2ReadCallback(UART2_Handle handle, void *rxBuf, size_t size,
                       void *userArg, int_fast16_t status)
{
    if (status == UART2_STATUS_SUCCESS)
    {

        UART2_read(handle, &uart2_read_buf, 1, NULL);

        payload[payload_index++] = uart2_read_buf;

        if (payload_size + 1 > PAYLOAD_SIZE)
        {

            payload_size = PAYLOAD_SIZE;

        }
        else
        {

            payload_size += 1;

        }

        if (payload_index >= PAYLOAD_SIZE)
        {

            payload_index = 0;

        }

    }

}


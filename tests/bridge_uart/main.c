
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "board.h"
#include "shell.h"
#include "thread.h"
#include "msg.h"
#include "ringbuffer.h"
#include "periph/uart.h"
#include "periph/gpio.h"
#include "xtimer.h"

#ifdef MODULE_STDIO_UART
#include "stdio_uart.h"
#endif

#ifndef SHELL_BUFSIZE
#define SHELL_BUFSIZE       (128U)
#endif
#ifndef UART_BUFSIZE
#define UART_BUFSIZE        (1U)
#endif

#define PRINTER_PRIO        (THREAD_PRIORITY_MAIN - 1)
#define PRINTER_TYPE        (0xabcd)

#define UART_POWEROFF_DELAY      (250U * US_PER_MS)      /* quarter of a second */

#ifndef STDIO_UART_DEV
#define STDIO_UART_DEV      (UART_UNDEF)
#endif

typedef struct {
    char rx_mem[UART_BUFSIZE];
    ringbuffer_t rx_buf;
} uart_ctx_t;

static uart_ctx_t ctx[UART_NUMOF];

static kernel_pid_t printer_pid;
static char printer_stack[THREAD_STACKSIZE_MAIN];


#ifdef MODULE_PERIPH_UART_MODECFG
static uart_data_bits_t data_bits_lut[] = { UART_DATA_BITS_5, UART_DATA_BITS_6,
                                            UART_DATA_BITS_7, UART_DATA_BITS_8 };
static int data_bits_lut_len = ARRAY_SIZE(data_bits_lut);

static uart_stop_bits_t stop_bits_lut[] = { UART_STOP_BITS_1, UART_STOP_BITS_2 };
static int stop_bits_lut_len = ARRAY_SIZE(stop_bits_lut);
#endif

static void rx_cb(void *arg, uint8_t data)
{
    uart_t dev = (uart_t)arg;

    ringbuffer_add_one(&(ctx[dev].rx_buf), data);
    if (data == '\n') {
        msg_t msg;
        msg.content.value = (uint32_t)dev;
        msg_send(&msg, printer_pid);
    }
}

static void *printer(void *arg)
{
    (void)arg;
    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);

    while (1) {
        msg_receive(&msg);
        uart_t dev = (uart_t)msg.content.value;
        int c;
        do {
            c = ringbuffer_get_one(&(ctx[dev].rx_buf));
            if(c != -1) {
                printf("%c", c);
            }
        } while (c == -1);
    }

    /* this should never be reached */
    return NULL;
}

static void sleep_test(int num, uart_t uart)
{
    printf("UARD_DEV(%i): test uart_poweron() and uart_poweroff()  ->  ", num);
    uart_poweroff(uart);
    xtimer_usleep(UART_POWEROFF_DELAY);
    uart_poweron(uart);
    puts("[OK]");
}

static bool _is__init_uart = false;

static void _init_uart(void) {
	if(_is__init_uart) return;
	_is__init_uart = true;

    /* do sleep test for UART used as STDIO. There is a possibility that the
     * value given in STDIO_UART_DEV is not a numeral (depends on the CPU
     * implementation), so we rather break the output by printing a
     * non-numerical value instead of breaking the UART device descriptor */
    if (STDIO_UART_DEV != UART_UNDEF) {
        sleep_test(STDIO_UART_DEV, STDIO_UART_DEV);
    }

    /* initialize ringbuffers */
    for (unsigned i = 0; i < UART_NUMOF; i++) {
        ringbuffer_init(&(ctx[i].rx_buf), ctx[i].rx_mem, UART_BUFSIZE);
    }

    /* start the printer thread */
    printer_pid = thread_create(printer_stack, sizeof(printer_stack),
                                PRINTER_PRIO, 0, printer, NULL, "l0_uart_printer");
}


static int _dev_init(int dev, uint32_t baud)
{
    int res;

    _init_uart();

    /* initialize UART */
    res = uart_init(UART_DEV(dev), baud, rx_cb, (void *)dev);
    if (res == UART_NOBAUD) {
        printf("Error: Given baudrate (%u) not possible\n", (unsigned int)baud);
        return 1;
    }
    else if (res != UART_OK) {
        puts("Error: Unable to initialize UART device");
        return 1;
    }
    printf("Success: Initialized UART_DEV(%i) at BAUD %"PRIu32"\n", dev, baud);

    return 0;
}

#ifdef MODULE_PERIPH_UART_MODECFG
static int parse_dev(char *arg)
{
    unsigned dev = atoi(arg);
    if (dev >= UART_NUMOF) {
        printf("Error: Invalid UART_DEV device specified (%u).\n", dev);
        return -1;
    }
    else if (UART_DEV(dev) == STDIO_UART_DEV) {
        printf("Error: The selected UART_DEV(%u) is used for the shell!\n", dev);
        return -2;
    }
    return dev;
}


static int _l0_uart_mode_cmd(int argc, char **argv)
{
    int dev, data_bits_arg, stop_bits_arg;
    uart_data_bits_t data_bits;
    uart_parity_t  parity;
    uart_stop_bits_t  stop_bits;

    if (argc < 6) {
        printf("usage: %s %s <dev> <data bits> <parity> <stop bits>\n", argv[0], argv[1]);
        return 1;
    }

    dev = parse_dev(argv[2]);
    if (dev < 0) {
        return 1;
    }

    data_bits_arg = atoi(argv[3]) - 5;
    if (data_bits_arg >= 0 && data_bits_arg < data_bits_lut_len) {
        data_bits = data_bits_lut[data_bits_arg];
    }
    else {
        printf("Error: Invalid number of data_bits (%i).\n", data_bits_arg + 5);
        return 1;
    }

    argv[4][0] &= ~0x20;
    switch (argv[4][0]) {
        case 'N':
            parity = UART_PARITY_NONE;
            break;
        case 'E':
            parity = UART_PARITY_EVEN;
            break;
        case 'O':
            parity = UART_PARITY_ODD;
            break;
        case 'M':
            parity = UART_PARITY_MARK;
            break;
        case 'S':
            parity = UART_PARITY_SPACE;
            break;
        default:
            printf("Error: Invalid parity (%c).\n", argv[4][0]);
            return 1;
    }

    stop_bits_arg = atoi(argv[5]) - 1;
    if (stop_bits_arg >= 0 && stop_bits_arg < stop_bits_lut_len) {
        stop_bits = stop_bits_lut[stop_bits_arg];
    }
    else {
        printf("Error: Invalid number of stop bits (%i).\n", stop_bits_arg + 1);
        return 1;
    }

    if (uart_mode(UART_DEV(dev), data_bits, parity, stop_bits) != UART_OK) {
        puts("Error: Unable to apply UART settings");
        return 1;
    }
    printf("Success: Successfully applied UART_DEV(%i) settings\n", dev);

    return 0;
}
#endif /* MODULE_PERIPH_UART_MODECFG */


static int _info_uart(void) {
    puts("\nUART INFO:");
    printf("Available devices:               %i\n", UART_NUMOF);
    if (STDIO_UART_DEV != UART_UNDEF) {
        printf("UART used for STDIO (the shell): UART_DEV(%i)\n\n", STDIO_UART_DEV);
    }
    return 0;
}


int main(void) {

	_info_uart();

	_dev_init(1, 9600);

	uint8_t c;
    while(1){
    	c = getchar();
        uart_write(UART_DEV(2), &c, 1);
    }

	return 0;
}

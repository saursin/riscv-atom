#include "platform.h"
#include <serial.h>
// #include <stdio.h>
// #include <gpio.h>
#include <stdint.h>

// #define GPIO_PIN_MODESEL 0
// #define GPIO_PIN_LED0 0
// #define GPIO_PIN_LED1 0

// #define AUTOBOOT_WAIT 5

#include "xmodem.h"

// #define SILENT

#ifdef SILENT
#define D(x)
#else
#define D(x) x
#endif

extern int __approm_start;
extern int __approm_size;
// extern int _stack_pointer;
// extern int _global_pointer;

// Function pointer for jumping to user application.
typedef void (*fnc_ptr)(void);

void puts(char *ptr);
void print_hex(unsigned int val, int digits);

void init()
{
    // Init UART
    // UART_Config uart_cfg = {
    //     .baud                = 115200,
    //     .rx_enable           = true,
    //     .tx_enable           = true,
    //     .dual_stop_bits      = false,
    //     .enable_parity_bit   = false,
    //     .even_parity         = false,
    //     .loopback_enable     = false
    // };
    // serial_init(&uart_cfg);

    // Init GPIO
    // gpio_setmode(GPIO_PIN_MODESEL, INPUT);
    // gpio_setmode(GPIO_PIN_LED0, OUTPUT);
    // gpio_setmode(GPIO_PIN_LED1, OUTPUT);
}


void jump_to_application(void)
{
    // Function pointer to the address of the user application
    fnc_ptr app_main = (fnc_ptr)(&__approm_start);

    serial_write('\n');
    // deinit
    // deinit();

    // change stack pointer & global pointer
    // asm volatile("la sp, %0": "=r"(_stack_pointer));
    // print_appl(100);
    // jump to app
    app_main();
}

void puts(char *ptr)
{
    while (*ptr)
    {
        if (*ptr == '\r')
            serial_write('\r');
        serial_write(*ptr++);
    }
}

void print_hex(unsigned int val, int digits)
{
    for (int i = (4 * digits) - 4; i >= 0; i -= 4)
    {
        serial_write("0123456789abcdef"[(val >> i) % 16]);
    }
}

void print_appl()
{
    puts("\n");
    uint32_t *addr = (uint32_t *)&__approm_start;
    uint32_t size = (uint32_t)&__approm_size;
    for (uint32_t i = 0; i < size; i++)
    {
        uint32_t instr = *addr;
        puts("0x");
        print_hex((uint32_t)addr++, 8);
        puts(": ");
        print_hex(instr, 8);
        puts("\n");
    }
}

int main()
{

    // Initialize
    init();

    // Print header
    puts("*** Atom Bootloader ***\n");

    // Check Modesel
    // bool modesel = true; //gpio_read(GPIO_PIN_MODESEL);
    // if(modesel != true)
    // {
    //     // puts("Jumping to application @ "); puthex(__approm_start, 8, false); putchar('\n');
    //     jump_to_application();
    //     /* UNREACHABLE */
    // }

    // Get Executable from UART

    // puts("Send a binary file with xmodem protocol to update firmware\n");
    // gpio_write(GPIO_PIN_LED0, HIGH);    // indicate that we are in recieve mode
    xmod_status s = xmodemReceive((uint8_t *)&__approm_start, (unsigned)&__approm_size);
    if (s == S_EOT) {
        // puts("Jumping to Application\n-----------------------------------------------\n");
        jump_to_application();
    } else {
        return 0;
        // puts("Failed to recieve executable, try again\n");
    }
    // print_appl();
    /* APPLICATION */

    // we only reach here if we have error
    // puts("Failed to recieve executable, try again\n");

    // uint32_t app_base = __approm_start;
    // uint8_t * dptr = (uint8_t*)app_base;

    // W25Q64_init();
    // puts("Copying...   ");
    // W25Q64_read(dptr, __approm_size, FLASH_ADDR);
    // puts("ok\n");

    // puts("Booting...   \n");
    // asm volatile ("jalr ra, %0" : : "r" (app_base));

    while (1)
        ;
}

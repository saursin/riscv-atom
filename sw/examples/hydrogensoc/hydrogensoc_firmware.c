#include "../../lib/gpio.h"

#define UART_D_REG_ADDR 0x08000000
#define UART_S_REG_ADDR 0x08000001
#define UART_CD_REG_ADDR 0x08000004

// =======================================================
#define DELAY_SCALING_FACTOR 1000
void delay(long count)
{
    long i=0;
    while(i++<count*DELAY_SCALING_FACTOR);
    return;
}


// =======================================================
#define CLK_FREQ 12000000

void initUART(int baud)
{
    int cd_reg_val = (CLK_FREQ/baud)-2;
    *((volatile int*) UART_CD_REG_ADDR) = cd_reg_val;
}

int getbaud()
{
    int v = *((volatile int*) UART_CD_REG_ADDR);
    return CLK_FREQ/(v+2);
}

void putChar(const char c)
{
    if (c == '\n')
        putChar('\r');

    while(1)
    {
        // check if Tx is busy    
        if((*((volatile char*) UART_S_REG_ADDR) & 0x02) >> 1)
        {
            delay(1);
            continue;
        }

        *((volatile char*) UART_D_REG_ADDR) = c;
        break;
    }
    return;
}

void print(const char *p)
{
	while (*p)
    {
		putChar(*(p++));
        delay(1);
    }
}

void printInt(long int n, int base)
{
        // If number is smaller than 0, put a - sign
        // and change number to positive
        if (base == 10 && n < 0) {
            putChar('-');
            n = -n;
        }
    
        // Remove the last digit and recur
        switch(base)
        {
            case 2:     if (n/2)
                            printInt(n/2, base);
                        break;

            case 8:     if (n/8)
                            printInt(n/8, base);
                        break;

            case 10:    if (n/10)
                            printInt(n/10, base);
                        break;

            case 16:    if (n/16)
                            printInt(n/16, base);
                        break;
        }
        
        // Print the last digit
        switch(base)
        {
            case 2:     putChar(n%2 + '0');
                        break;

            case 8:     putChar(n%8 + '0');
                        break;

            case 10:    putChar(n%10 + '0');
                        break;

            case 16:    if(n%16 > 9)
                            putChar((n%16)-10 + 'a');
                        else
                            putChar(n%16 + '0');
                        break;
        }
}


char getChar()
{
    char c = (char)-1;
    while(c == (char)-1)
    {
        delay(1);
        c = *((volatile char*) UART_D_REG_ADDR);
    }
    return c;
}

void getString(char * str, int max_size, int enable_echo)
{
    char * ptr = str;
    char c;
    
    int sz = max_size;
    while(sz--)
    {
        c = getChar();

        if (c==0x7f) // backspace
        {
            if(str!=ptr)    // String is not empty
            {   
                // Clear last character in string
                *(--ptr) = 0;

                if(enable_echo)
                {
                    // Send linefeed
                    print("\r> ");

                    // RePrint line with last character deleted
                    char * i = str; 
                    while(i!=ptr+1)
                    {
                        putChar(*(i++));
                    }
                    putChar(' ');   // erase last char
                    
                    // RePrint to move cursor to proper location
                    
                    // Send linefeed
                    print("\r> ");
                    i = str;
                    while(i!=ptr+1)
                    {
                        putChar(*(i++));
                    }
                }
            }
            sz++;
        }

        else if(c=='\n' || c=='\r')
        {
            break;
        }
        else
        {
            *(ptr++) = c;
            if (enable_echo)
                putChar(c);
        }

    }
    if(enable_echo) 
        putChar('\n');
    *(ptr) = '\0';
}

int strcmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strlen(char *s1)
{
    int len;
    
    for(len=0;s1&&*s1++;len++);

    return len;
}

// =======================================================

char * banner = 
"\n"
"                  .';,.           ....;;;.  \n"
"                 .ll,:o,                ':c,. \n"
"                 .dd;co'                  .cl,  \n"
"                .:o:;,.                     'o:  \n"
"                co.                          .oc  \n"
"               ,o'          .coddoc.          'd,  \n"
"               lc         .lXMMMMMMXl.         ll  \n"
"              .o:         ;KMMMMMMMMK,         :o. \n"
"              .o:         'OMMMMMMMMO.         :o. \n"
"               co.         .o0XNNX0o.         .oc  \n"
"               .o:           ..''..           :o.  \n"
"                'o:                          :o'  \n"
"                 .lc.                      .ll.  \n"
"                   ,lc'                  'cl,   \n"
"                     'cc:,..        ..,:c:'   \n"
"                        .;::::;;;;::::;.    \n"
"                              ....        \n"
"     ____  _________ _______    __         __ \n"
"    / __ \\/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___  \n"
"   / /_/ // / \\__ \\/ /    | | / /  / __ `/ __/ __ \\/ __ `__ \\ \n"
"  / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /      \n"
" /_/ |_/___//____/\\____/  |___/   \\__,_/\\__/\\____/_/ /_/ /_/  \n"
"/=========By: Saurabh Singh (saurabh.s99100@gmail.com)====/\n\n";


// ================== COMMANDS ======================
void clear()
{
    int i;
    for(i=0; i<50; i++)
        putChar('\n');
}

void lscpu()
{
    print("Architecture:        RISC-V (RV32I)\n");
    print("CPU Datapath Width:  32-Bit\n");
    print("Byte Ordering:       Little Endian\n");
    print("CPU:                 RISCV-Atom 2.0\n");
    print("Clock Frequency:     12MHz\n");
    print("Cache:               None\n");
}

void lsmem()
{
    print("Mode:                Harvard\n\n");

    print("0x00000000-0x00007fff: ROM   (32KB)\n");
    print("0x04000000-0x04001fff: RAM   (8KB)\n");
    print("0x08000000-0x08000007: UART  (8B)\n");
    print("0x08000100-0x08000107: GPIO  (8B)\n");
}


void gpio_test()
{
    gpio_reset();

    print("Pattern 1 - Rolling\n");
    int i=0;
    int pin = 0;

    for(i=0; i<30; i++)
    {
        gpio_set((pin), !gpio_get(pin));
        
        if(pin>=GPIO_MAX_PINS-1)
            pin = 0;
        else
            pin++;

        delay(50);
    }
    gpio_reset();
    delay(100);

    print("Pattern 2 - Blinking\n");
    for(i=0; i<4; i++)
    {
        for(pin=0; pin<GPIO_MAX_PINS; pin++)
        {
            gpio_set((pin), GPIO_HIGH);
            delay(50);
            gpio_set((pin), GPIO_LOW);
            delay(25);
        }
    }
    gpio_reset();
    delay(100);

    print("Pattern 3 - Alternate\n");
    for(i=0; i<4; i++)
    {
        gpio_set(0, GPIO_HIGH);
        gpio_set(2, GPIO_HIGH);
        gpio_set(4, GPIO_HIGH);
        gpio_set(6, GPIO_HIGH);
        delay(100);
        gpio_set(0, GPIO_LOW);
        gpio_set(2, GPIO_LOW);
        gpio_set(4, GPIO_LOW);
        gpio_set(6, GPIO_LOW);
        delay(100);

        gpio_set(1, GPIO_HIGH);
        gpio_set(3, GPIO_HIGH);
        gpio_set(5, GPIO_HIGH);
        gpio_set(7, GPIO_HIGH);
        delay(100);
        gpio_set(1, GPIO_LOW);
        gpio_set(3, GPIO_LOW);
        gpio_set(5, GPIO_LOW);
        gpio_set(7, GPIO_LOW);
        delay(100);
    }
    gpio_reset();
    delay(100);


    print("Pattern 4 - counting\n");
    for (i=0; i<256; i++)
    {
        gpio_set(0, (i & 0b00000001) >> 0);
        gpio_set(1, (i & 0b00000010) >> 1);
        gpio_set(2, (i & 0b00000100) >> 2);
        gpio_set(3, (i & 0b00001000) >> 3);
        gpio_set(4, (i & 0b00010000) >> 4);
        gpio_set(5, (i & 0b00100000) >> 5);
        gpio_set(6, (i & 0b01000000) >> 6);
        gpio_set(7, (i & 0b10000000) >> 7);
        delay(20);
    }
    gpio_reset();
    delay(100);

    print("Pattern 5 - Blink All\n");
    for(i=0; i<8; i++)
    {
        int j=0; 
        for(j=0; j<GPIO_MAX_PINS; j++)
        {
            gpio_set(j, !gpio_get(j));            
        }
        delay(100);
    }
    gpio_reset();
}

// ================ CONSOLE =====================
void console()
{
    char cmd [50];
    while(1)
    {
        print("> ");
        getString(cmd, 50, 1);

        if(strcmp(cmd, "") == 0)
        {}
        else if(strcmp(cmd, "clear") == 0)
        {
            clear();
        }
        else if(strcmp(cmd, "lscpu") == 0)
        {                        
            lscpu();
        }
        else if(strcmp(cmd, "lsmem") == 0)
        {
            lsmem();
        }
        else if(strcmp(cmd, "gpiotest") == 0)
        {
            print("Testing GPIO Pins...\n");
            gpio_test();
            print("Done!\n");
        }
        else if(strcmp(cmd, "printbanner") == 0)
        {
            print(banner);
        }
        else if(strcmp(cmd, "setuartbaud 9600") == 0 || strcmp(cmd, "setuartbaud default") == 0)
        {
            initUART(9600);
        }
        else if(strcmp(cmd, "setuartbaud 38400") == 0)
        {
            initUART(38400);
        }
        else if(strcmp(cmd, "setuartbaud 115200") == 0)
        {
            initUART(115200);
        }
        else if(strcmp(cmd, "getuartbaud") == 0)
        {
            print("Baud rate: "); printInt(getbaud(), 10); putChar('\n');
        }
        else if(strcmp(cmd, "gpio reset") == 0)
        {
            gpio_reset();
        }
        else if(strcmp(cmd, "gpio0") == 0)
        {
            gpio_set(0, !gpio_get(0));
        }
        else if(strcmp(cmd, "gpio1") == 0)
        {
            gpio_set(1, !gpio_get(1));
        }
        else if(strcmp(cmd, "gpio2") == 0)
        {
            gpio_set(2, !gpio_get(2));
        }
        else if(strcmp(cmd, "gpio3") == 0)
        {
            gpio_set(3, !gpio_get(3));
        }
        else if(strcmp(cmd, "gpio4") == 0)
        {
            gpio_set(4, !gpio_get(4));
        }
        else if(strcmp(cmd, "gpio5") == 0)
        {
            gpio_set(5, !gpio_get(5));
        }
        else if(strcmp(cmd, "gpio6") == 0)
        {
            gpio_set(6, !gpio_get(6));
        }
        else if(strcmp(cmd, "gpio7") == 0)
        {
            gpio_set(7, !gpio_get(7));
        }
        else if(strcmp(cmd, "load") == 0)
        {
            print("Command not yet supported\n");
        }
        else if(strcmp(cmd, "help") == 0)
        {
            print("Supported Commands:\n");
            print(" help                    Display help message\n");
            print(" clear                   Clears screen\n");
            print(" lscpu                   List CPU related information\n");
            print(" lsmem                   List memory related information\n");
            print(" gpiotest                Test gpio pins\n");
            print(" printbanner             Print RISCV-Atom banner\n");
            print(" setuartbaud <baud>      Set UART baud. Suppported values {9600, 38400, 115200, default}\n");
            print(" gpio<x>                 Toggle gpio 'x'\n");
            print(" gpio reset              Reset GPIO pins\n");
            print(" getuartbaud             Print current UART baud\n");
        }
        else
        {
            print("Unknown Command \"");
            print(cmd);
            print("\"\n");
        }
    }
}

int main()
{
    initUART(9600);
    gpio_init();

    delay(100);
    print("Booting");
    delay(100);
    putChar('.');
    delay(100);
    putChar('.');
    delay(100);
    putChar('.');

    print("\nPress ENTER to continue...\n");
    while (getChar(0) != '\r')
    { /* wait */ }
    
    print("\n\n");
    print(banner);
    
    console();
}
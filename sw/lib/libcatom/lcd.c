#include "lcd.h"

#include "defs.h"
#include "gpio.h"
#include "time.h"

// Default values
static int lcd_pin_rs = 0;
static int lcd_pin_rw = 1;
static int lcd_pin_en = 2;

static int lcd_pin_d4 = 4;
static int lcd_pin_d5 = 5;
static int lcd_pin_d6 = 6;
static int lcd_pin_d7 = 7;

//static int lcd_coloumns = 16;
static int lcd_rows = 2;

////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS

/**
 * @brief get specified bit of character
 * 
 * @param dat 8-bit data
 * @param b bit [Convention: LSB=0]
 * @return int state 
 */
int _bitget(char dat, int b)
{
    return (dat & (1 << b))>0;
}


/**
 * @brief Write a nibble to lcd_data bus
 * 
 * @param nibble 
 */
void _lcdbus_write(char nibble)
{
    gpio_write(lcd_pin_d4, _bitget(nibble, 4));
    gpio_write(lcd_pin_d5, _bitget(nibble, 5));
    gpio_write(lcd_pin_d6, _bitget(nibble, 6));
    gpio_write(lcd_pin_d7, _bitget(nibble, 7));
}


/**
 * @brief Pulse enable pin once
 */
void _pulse_en()
{
    gpio_write(lcd_pin_en, HIGH); sleep(1);
    gpio_write(lcd_pin_en, LOW); sleep(1);
}


////////////////////////////////////////////////////////////////////////
// API FUNCTIONS

/**
 * @brief Initialize LCD module with default GPIO pin mapping
 */
void lcd_init()
{
    // Make all pins as output
    gpio_pinmode(lcd_pin_rs, OUTPUT);
    gpio_pinmode(lcd_pin_rw, OUTPUT);
    gpio_pinmode(lcd_pin_en, OUTPUT);

    gpio_pinmode(lcd_pin_d4, OUTPUT);
    gpio_pinmode(lcd_pin_d5, OUTPUT);
    gpio_pinmode(lcd_pin_d6, OUTPUT);
    gpio_pinmode(lcd_pin_d7, OUTPUT);

    // Pull all pins LOW
    gpio_write(lcd_pin_rs, LOW);
    gpio_write(lcd_pin_rw, LOW);
    gpio_write(lcd_pin_en, LOW);

    gpio_write(lcd_pin_d4, LOW);
    gpio_write(lcd_pin_d5, LOW);
    gpio_write(lcd_pin_d6, LOW);
    gpio_write(lcd_pin_d7, LOW);

    // --------- Init sequence ------------
    sleep(50);  // 50ms

    // Put LCD in 4 bit mode
    // lcd_write(0, 0x03); 
    // sleep(5); //wait min 4.1ms

    // lcd_write(0, 0x03); 
    // sleep(5); //wait min 4.1ms

    // lcd_write(0, 0x03); 
    // sleep(1);

    lcd_write(0, 0x02); // Initialize Lcd in 4-bit mode
    //sleep(1);

    lcd_write(0, 0x28); // enable 5x7 mode for chars 
    lcd_write(0, 0x0e); // Display OFF, Cursor ON
    lcd_write(0, 0x01); // Clear Display
    lcd_write(0, 0x80); // Move the cursor to beginning of first line
}


/**
 * @brief Initialize LCD module with custom GPIO pin mapping
 * 
 * @param rs register select pin
 * @param en enable pin
 * @param d4 data[4]
 * @param d5 data[5]
 * @param d6 data[6]
 * @param d7 data[7]
 */
void lcd_init_custom(int rs, int en, int d4, int d5, int d6, int d7)
{
    lcd_pin_rs = rs;
    lcd_pin_en = en;

    lcd_pin_d4 = d4;
    lcd_pin_d5 = d5;
    lcd_pin_d6 = d6;
    lcd_pin_d7 = d7;

    // call init
    lcd_init();
}


/**
 * @brief Write a command or data to LCD
 * 
 * @param isData 1 if data, 0 if command
 * @param byte command/data
 */
void lcd_write(int isData, char byte)
{
    gpio_write(lcd_pin_rs, isData);
    gpio_write(lcd_pin_rw, LOW);
    gpio_write(lcd_pin_en, LOW);
    
    // Write Upper
    _lcdbus_write(byte); _pulse_en();
    sleep(1);

    // Write Lower
    _lcdbus_write(byte << 4); _pulse_en();
    sleep(1);
}


/**
 * @brief Put char on LCD
 * 
 * @param c char
 */
void lcd_putchar(char c)
{
    lcd_write(1, c);
}


/**
 * @brief Put String on LCD
 * 
 * @param str string
 */
void lcd_putstr(char * str)
{
    while(*str)
        lcd_write(1, *str++);
}


/**
 * @brief Clear LCD screen
 */
void lcd_clear()
{
    lcd_write(0, LCD_CLEARDISPLAY);
    sleep(2);
}


/**
 * @brief Bring Cursor to home
 */
void lcd_home()
{
    lcd_write(0, LCD_RETURNHOME);
    sleep(2);
}


/**
 * @brief Set the Cursor position
 * 
 * @param row row
 * @param col coloumn
 */
void lcd_setCursor(int row, int col)
{
  int row_offsets[] = {0x00, 0x40, 0x14, 0x54};
  if (row > lcd_rows) {
    row = lcd_rows - 1; // we count rows starting w/0
  }
  lcd_write(0, LCD_SETDDRAMADDR | (col + row_offsets[row]));
}



// These commands scroll the display without changing the RAM

/**
 * @brief Scroll Display Left
 */
void lcd_scrollLeft(void) 
{
  lcd_write(0, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}


/**
 * @brief Scroll Display Right
 */
void lcd_scrollRight(void) 
{
  lcd_write(0, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}
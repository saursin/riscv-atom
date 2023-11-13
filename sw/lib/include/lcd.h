#pragma once

// commands
#define LCD_CLEARDISPLAY 0x01 //!< Clear display, set cursor position to zero
#define LCD_RETURNHOME 0x02   //!< Set cursor position to zero
#define LCD_ENTRYMODESET 0x04 //!< Sets the entry mode
#define LCD_DISPLAYCONTROL                                                     \
  0x08 //!< Controls the display; does stuff like turning it off and on
#define LCD_CURSORSHIFT 0x10 //!< Lets you move the cursor
#define LCD_FUNCTIONSET                                                        \
  0x20 //!< Used to send the function to set to the display
#define LCD_SETCGRAMADDR                                                       \
  0x40 //!< Used to set the CGRAM (character generator RAM) with characters
#define LCD_SETDDRAMADDR 0x80 //!< Used to set the DDRAM (Display Data RAM)

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00 //!< Used to set text to flow from right to left
#define LCD_ENTRYLEFT 0x02  //!< Uset to set text to flow from left to right
#define LCD_ENTRYSHIFTINCREMENT                                                \
  0x01 //!< Used to 'right justify' text from the cursor
#define LCD_ENTRYSHIFTDECREMENT                                                \
  0x00 //!< Used to 'left justify' text from the cursor

// flags for display on/off control
#define LCD_DISPLAYON 0x04  //!< Turns the display on
#define LCD_DISPLAYOFF 0x00 //!< Turns the display off
#define LCD_CURSORON 0x02   //!< Turns the cursor on
#define LCD_CURSOROFF 0x00  //!< Turns the cursor off
#define LCD_BLINKON 0x01    //!< Turns on the blinking cursor
#define LCD_BLINKOFF 0x00   //!< Turns off the blinking cursor

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08 //!< Flag for moving the display
#define LCD_CURSORMOVE 0x00  //!< Flag for moving the cursor
#define LCD_MOVERIGHT 0x04   //!< Flag for moving right
#define LCD_MOVELEFT 0x00    //!< Flag for moving left

// flags for function set
#define LCD_8BITMODE 0x10 //!< LCD 8 bit mode
#define LCD_4BITMODE 0x00 //!< LCD 4 bit mode
#define LCD_2LINE 0x08    //!< LCD 2 line mode
#define LCD_1LINE 0x00    //!< LCD 1 line mode
#define LCD_5x10DOTS 0x04 //!< 10 pixel high font mode
#define LCD_5x8DOTS 0x00  //!< 8 pixel high font mode


/**
 * @brief Initialize LCD module with default GPIO pin mapping
 */
void lcd_init();


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
void lcd_init_custom(int rs, int en, int d4, int d5, int d6, int d7);


/**
 * @brief Write a command or data to LCD
 * 
 * @param isData 0 if data, 1 if command
 * @param byte command/data
 */
void lcd_write(int isData, char byte);


/**
 * @brief Put char on LCD
 * 
 * @param c char
 */
void lcd_putchar(char c);


/**
 * @brief Put String on LCD
 * 
 * @param str string
 */
void lcd_putstr(char * str);


/**
 * @brief Clear LCD screen
 */
void lcd_clear();


/**
 * @brief Bring Cursor to home
 */
void lcd_home();


/**
 * @brief Set the Cursor position
 * 
 * @param row row
 * @param col coloumn
 */
void lcd_setCursor(int row, int col);


/**
 * @brief Scroll Display Left
 */
void lcd_scrollLeft(void);


/**
 * @brief Scroll Display Right
 */
void lcd_scrollRight(void);

#pragma once

#include "CppLinuxSerial/include/SerialPort.hpp"

#define DEFAULT_BAUDRATE    9600

/**
 * @brief Vuart Class
 * @details Encapsulates the functionality of a virtual uart port with the help of the a modified version of 
 * the CppLinuxSerial Library. The library has been modified to use only a 1 byte recieve buffer in 
 * order to enable character by character send/recieve rather than string by string.
 * @see https://github.com/gbmhunter/CppLinuxSerial
 */
class Vuart
{    
public:
    /**
     * @brief Construct a new Vuart object
     * @param portname name of the port
     * @param baud baud-rate (bps)
     */
    Vuart(std::string portname, int baud=9600);

    /**
     * @brief Destroy the Vuart object
     */
    ~Vuart();

    /**
     * @brief set baud rate
     * @param baud baud-rate
     */
    void setbaud(unsigned int baud=9600);

    /**
     * @brief Get current baudrate
     * @return int baudrate
     */
    unsigned int getbaud();

    /**
     * @brief Returns current state of the port
     * @return true if open 
     * @return false if closed
     */
    bool isOpen();

    /**
     * @brief Send a charactr to uvart
     * @param c character
     */
    void send(char c);

    /**
     * @brief get char in buffer
     * @details Immediately returns the char present in the recieve buffer
     * if no chr present returns 255 i.e. -1
     * @return char character
     */
    char recieve();

    /**
     * @brief Clean any garbage in the recieve buffer
     */
    void clean_recieve_buffer();

private:
    /**
     * @brief pointer to the serial port object
     */
    mn::CppLinuxSerial::SerialPort * port;

    /**
     * @brief Serial port Baudrate
     */
    mn::CppLinuxSerial::BaudRate port_baudrate;

    /**
     * @brief current state of the port
     */
    bool isopen = false;

    // Helper functions

    /**
     * @brief Open serial port
     * @param verbose set true to print acknowledgement message
     */
    inline void _openPort(bool verbose=DEFAULT_BAUDRATE);

    /**
     * @brief Close serial port
     * @param verbose set true to print acknowledgement message
     */
    inline void _closePort(bool verbose=DEFAULT_BAUDRATE);

};
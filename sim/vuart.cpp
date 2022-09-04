#include "vuart.hpp"

#include <string>

// Helper functions
inline void Vuart::_openPort()
{
    port->Open();
    isopen = true;
}


inline void Vuart::_closePort()
{
    port->Close();
    isopen = false;
}


Vuart::Vuart(std::string portname, int baud)
{
    // set baud rate
    setbaud(baud);

    // create a new port object
    port = new mn::CppLinuxSerial::SerialPort(portname, port_baudrate);

    /* Set timeout to 0: i.e. Non blocking Mode
        This (paired with 1 byte recieve buffer of the library) enables the latest pressed key 
        on the keyboard to be returned while calling recieve().
    */
    port->SetTimeout(0);

    // Open port
    _openPort();
}


Vuart::~Vuart()
{
    // close port
    _closePort();

    // destroy port object
    delete port;
}


void Vuart::setbaud(unsigned int baud)
{
    switch(baud)
    {
        case 9600:
            port_baudrate=mn::CppLinuxSerial::BaudRate::B_9600; break;
        case 19200:
            port_baudrate=mn::CppLinuxSerial::BaudRate::B_19200; break;
        case 115200:
            port_baudrate=mn::CppLinuxSerial::BaudRate::B_115200; break;
        
        default:
            throw "Invalid baud rate";
    }

    // if already opened
    if(isopen)
    {
        // Close port, change baudrate, and reopen
        _closePort();

        port->SetBaudRate(port_baudrate);
        
        _openPort();
    }
}



unsigned int Vuart::getbaud()
{
    switch(port_baudrate)
    {
        case mn::CppLinuxSerial::BaudRate::B_9600:
            return 9600;
        case mn::CppLinuxSerial::BaudRate::B_19200:
            return 19200;
        case mn::CppLinuxSerial::BaudRate::B_115200:
            return 115200;
        
        default: 
            return -1;  // unknown baud
    }
}


bool Vuart::isOpen()
{
    return isopen;
}


void Vuart::send(char c)
{
    std::vector<uint8_t> vec;
    vec.push_back((uint8_t) c);
    port->WriteBinary(vec);
}


int Vuart::recieve()
{        
    std::vector<uint8_t> vec;
    port->ReadBinary(vec);

    if(vec.size()==0)
        return (int) -1;
    else
        return (int) vec[0];
}


void Vuart::clean_recieve_buffer()
{
    char c;
    do
    {
        c = recieve();  // dump garbage
    } while(c != -1);
}


#include <iostream>
#include "obj_dir/Vsimpleuart__Dpi.h"
#include "svdpi.h"

#include "SerialPort.hpp"

#ifdef DBG
#define D(x) x
#else
#define D(x)
#endif

///////////////////////////////////////////////////////////////////
// Serial port IO

class SerialIO
{
    public:
    const std::string sim_portname = "simport";
    mn::CppLinuxSerial::SerialPort * port;
    mn::CppLinuxSerial::BaudRate port_baudrate;

    SerialIO()
    {
        // set baud 
        port_baudrate = mn::CppLinuxSerial::BaudRate::B_9600;

        // creat port obj && open port
        try {
            port = new mn::CppLinuxSerial::SerialPort(sim_portname, port_baudrate);
        } catch(const mn::CppLinuxSerial::Exception& e) {
            std::cerr << "ERR_VUART: " << e.what() << std::endl;
        }
        _openport();
    }

    ~SerialIO()
    {
        _closeport();
        delete port;
    }

    void send(char c)
    {
        std::vector<uint8_t> vec;
        vec.push_back((uint8_t) c);
        port->WriteBinary(vec);
    }

    private:
    inline void _openport() {
        port->Open();
        port->Write("\r\n*** Sim Connected ***\r\n");
    }

    inline void _closeport() {
        port->Write("\r\n*** Sim Disonnected ***\r\n");
        port->Close();
    }
};

SerialIO ser_port;

///////////////////////////////////////////////////////////////////

/*  Note: To be changed acc to implementation using formulas
        F_RATIO = (CLK_FREQ/BAUD)    // ratio of clk freq to baud rate
        WAIT_CYC = F_RATIO

    For simpleuart, F_RATIO = 3, therefore WAIT_CYC = 3
*/
const int WAIT_CYC = 3;      /* TODO: Change this acc to implementation */

typedef enum {
    IDLE,
    START_BIT,
    DATA_BITS,
    STOP_BIT
} VIPState;

static uint64_t cyc = 0;

extern "C" void state_tx(int tx) {
    static bool prev_tx = false;            // prev tx pin value
    static VIPState state = IDLE;           // current state
    static int wait_cyc = 0;                // number of cycles to wait
    static int got_bits = 0;                // number of bits received
    static uint8_t byte = 0;                // data byte

    // skip current cycle
    D(printf("----------------\npin = %d\n", tx);)
    if(wait_cyc) {
        wait_cyc--;
        D(printf("wait cyc\n");)
        return;
    }


    // State machine
    switch(state) {
        case IDLE:
            // wait for falling edge on tx
            if (prev_tx==true && tx == false) {
                state = START_BIT;
                byte = 0;
                got_bits = 0;
                D(printf("@%ld: detected negedge on tx\n", cyc);)
                wait_cyc = WAIT_CYC/2-1;
            }
            break;
        
        case START_BIT:
            // check start bit
            if (tx == false) {
                D(printf("start_bit: ok\n");)
                state = DATA_BITS;
                wait_cyc = WAIT_CYC-1;
            }
            else {
                D(printf("err: no start bit\n");)
                state = IDLE;
            }
            break;
        
        case DATA_BITS:
            // fetch data bits & construct byte
            if(got_bits == 8) {
                state = STOP_BIT;
            }
            else {
                byte = (byte >> 1) | (((uint8_t)tx) << 7);
                D(printf("tx=%d; got_bits=%d; byte: %02x\n", tx, got_bits, byte);)
                got_bits++;
                state = DATA_BITS;  // loop
            }
            wait_cyc = WAIT_CYC-1;
            break;
        
        case STOP_BIT:
            // check stop bit
            if(tx==true) {
                D(printf("Recieved Byte! = 0x%02x\n", byte);)
                std::cerr << (char)byte << std::flush;
                ser_port.send(byte);
                // print byte here
                wait_cyc = WAIT_CYC/2-1;
                state = IDLE;
            }
            else {
                D(printf("Err: stop bit not recieved!\n");)
                state = IDLE;
            }

        default:
            state = IDLE;
    }

    prev_tx = tx;
    cyc++;
}
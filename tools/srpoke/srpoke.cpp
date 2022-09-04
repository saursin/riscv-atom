#include <iostream>
#include <string>
#include <thread>
#include "../../sim/vuart.hpp"

bool NO_COLOR_OUTPUT = false;
extern bool NO_COLOR_OUTPUT;

#define CTL_NUL  0x00
#define CTL_SOH  0x01
#define CTL_STX  0x02
#define CTL_ETX  0x03
#define CTL_EOT  0x04
#define CTL_ENQ  0x05
#define CTL_ACK  0x06
#define CTL_BEL  0x07
#define CTL_BS   0x08
#define CTL_HT   0x09
#define CTL_TAB  0x09
#define CTL_LF   0x0A
#define CTL_NL   0x0A
#define CTL_EOL  0x0A
#define CTL_VT   0x0B
#define CTL_FF   0x0C
#define CTL_CR   0x0D
#define CTL_SO   0x0E
#define CTL_SI   0x0F
#define CTL_DLE  0x10
#define CTL_DC1  0x11
#define CTL_DC2  0x12
#define CTL_DC3  0x13
#define CTL_DC4  0x14
#define CTL_NAK  0x15
#define CTL_SYN  0x16
#define CTL_ETB  0x17
#define CTL_CAN  0x18
#define CTL_EOM  0x19
#define CTL_SUB  0x1A
#define CTL_ESC  0x1B
#define CTL_FS   0x1C
#define CTL_GS   0x1D
#define CTL_RS   0x1E
#define CTL_US   0x1F
#define CTL_SP   0x20
#define CTL_DEL  0x7F

const std::string ascii_ctl_names [] = {
    "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS", "HT", 
    "TAB", "LF", "NL", "EOL", "VT", "FF", "CR", "SO", "SI", "DLE", 
    "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB", "CAN", "EOM", "SUB", 
    "ESC", "FS", "GS", "RS", "US", "SP"};

Vuart *vu;

/**
 * @brief Parse integer, hexadecimal or binary number from string
 * @param s string
 * @return int 
 */
long long _parse_num(std::string s)
{
    long long r;
    if(s.substr(0, 2) == "0x")			// Hex Number
        r = std::stoll(s, nullptr, 16);
    else if(s.substr(0, 2) == "0b")		// Binary Number
        r = std::stoll(s, nullptr , 2);
    else								// Decimal Number
        r = std::stoll(s, nullptr, 10);
    return r;
}

bool terminate = false;

void get_rx()
{
    while(!terminate)
    {
        // recieve
        int r = vu->recieve();
        if(r!=(int)-1)
        {
            std::string ctl_name =  r <= 0x20 ? "["+ascii_ctl_names[(int)r]+"]": "";
            printf(">> %c  (0x%02x) %s\n", r, r, ctl_name.c_str());
        }
    }
}

int main(int argc, char** argv)
{
    terminate = false;
    if(argc < 2)
    {
        std::cout << "ERR: expected portname as argument\n";
        return 1;
    }

    std::cout << 
    " *** SrPoke *** \n"
    " - Type a decimal, binary (prefixed with 0b) or hex (prefixed with 0x) \n"
    "   value in the console to send it to the serial port. \n"
    " - enter 'q' to exit.\n";
        
    std::string port(argv[1]);

    Vuart v(port, 9600);
    vu = &v;

    std::thread recv_thr(get_rx);
    
    while (1)
    {
        std::string line;
        std::cin >> line;

        if(line == "q") {
            terminate = true;
            break;
        }

        // send
        int s;
        try {
            s = _parse_num(line);
        } 
        catch(...) {
            std::cerr << "EXCPTION: Invalid input "<< line << std::endl;
        }

        if (s>255)
        {
            std::cout << "ERR: val greater than packet size (255)\n";
            continue;
        }

        v.send((char) s);
    }

    recv_thr.join();

    return 0;
}

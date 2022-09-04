#include <iostream>
#include <string>
#include <thread>
#include "../../sim/vuart.hpp"

bool NO_COLOR_OUTPUT = false;
extern bool NO_COLOR_OUTPUT;

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
        char r = vu->recieve();
        if(r!=(char)-1)
            std::cout << ">> " << r << std::endl;
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
        int s = _parse_num(line);

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

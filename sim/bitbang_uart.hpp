#pragma once

#include <queue>



class BitbangUART
{
    public:
    // publickly accessible fifo queues
    std::queue<char> rx_fifo;
    std::queue<char> tx_fifo;

    // settings

    BitbangUART(bool * rx_pin, bool * tx_pin, unsigned FR);

    // needs to be called every cycle
    void eval() {
        rx_eval();
        tx_eval();
    }

    private:
    enum UART_State {
        IDLE,
        START_BIT,
        DATA_BITS,
        STOP_BIT
    };

    void rx_eval();
    void tx_eval();

    // Pin pointers
    bool * rx_pin = NULL;   
    bool * tx_pin = NULL;


    // UART configs    
    unsigned FR = 3;        // Frequency ratio
};




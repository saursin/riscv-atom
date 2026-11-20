#include "bitbang_uart.hpp"
#include "vuart.hpp"
#include <cstdio>
#include <iostream>

//#define DBG

#ifdef DBG
#define D(x) x
#else
#define D(x)
#endif

BitbangUART::BitbangUART(const uint8_t *rx_pin, uint8_t *tx_pin, const uint32_t *div, std::unique_ptr<Vuart> vuart, bool dump):
    rx_pin_(rx_pin), tx_pin_(tx_pin), div_(div), vuart_(std::move(vuart)), dump_(dump)
{}

BitbangUART::~BitbangUART() {}


void BitbangUART::eval()
{
    unsigned fr = *div_ + 2;    // cycles per bit
    rx_eval(fr);
    tx_eval(fr);
}


void BitbangUART::received(uint8_t byte)
{
    if(vuart_)
        vuart_->send(byte);
    if(dump_)
        std::cout << (char)byte << std::flush;
}


void BitbangUART::rx_eval(unsigned fr)
{
    bool rx = *rx_pin_ & 1;

    if(rx_wait_){
        D(printf("rx: wait %d\n", rx_wait_);)
        rx_wait_--;
    } else {
        switch(rx_state_) {
        case IDLE:
            // wait for falling edge on rx
            if (rx_prev_==true && rx == false) {
                rx_state_ = START_BIT;
                rx_byte_ = 0;
                rx_bits_ = 0;
                D(printf("rx: detected negedge\n");)
                rx_wait_ = fr/2-1;
            }
            break;

        case START_BIT:
            // check start bit
            if (rx == false) {
                D(printf("rx: start_bit: ok\n");)
                rx_state_ = DATA_BITS;
                rx_wait_ = fr-1;
            }
            else {
                D(printf("rx err: no start bit\n");)
                rx_state_ = IDLE;
            }
            break;

        case DATA_BITS:
            rx_byte_ = (rx_byte_ >> 1) | (((uint8_t)rx) << 7);
            D(printf("rx: pin=%d; got_bits=%d; byte: %02x\n", rx, rx_bits_, rx_byte_);)
            rx_bits_++;
            if(rx_bits_ == 8)
                rx_state_ = STOP_BIT;
            rx_wait_ = fr-1;
            break;

        case STOP_BIT:
            // check stop bit
            if(rx==true) {
                D(printf("rx: Recieved Byte! = 0x%02x\n", rx_byte_);)
                received(rx_byte_);
                rx_wait_ = fr/2-1;
            }
            else {
                printf("rx: Framing error, '%c' (0x%02x)\n", rx_byte_, rx_byte_);
            }
            rx_state_ = IDLE;
            break;

        default:
            rx_state_ = IDLE;
        }

        rx_prev_ = rx;
    }
}


void BitbangUART::tx_eval(unsigned fr)
{
    if(tx_wait_){
        D(printf("tx: wait %d\n", tx_wait_);)
        tx_wait_--;
    } else {
        switch(tx_state_) {
        case IDLE:
            tx_val_ = true;
            if(vuart_) {
                int c = vuart_->recieve();
                if(c != -1) {
                    tx_byte_ = c;
                    D(printf("tx: got byte, started transmitting\n");)
                    tx_bits_ = 0;
                    tx_state_ = START_BIT;
                }
            }
            break;

        case START_BIT:
            tx_val_ = false;
            tx_state_ = DATA_BITS;
            tx_wait_ = fr-1;
            break;

        case DATA_BITS:
            tx_val_ = (tx_byte_ & 0x1) != 0;
            tx_byte_ = tx_byte_ >> 1;
            D(printf("tx: pin=%d; sent_bits=%d; byte: %02x\n", tx_val_, tx_bits_, tx_byte_);)
            tx_bits_++;
            if(tx_bits_ == 8)
                tx_state_ = STOP_BIT;
            tx_wait_ = fr-1;
            break;

        case STOP_BIT:
            tx_val_ = true;
            tx_wait_ = fr-1;
            tx_state_ = IDLE;
            break;

        default:
            tx_state_ = IDLE;
        }
    }

    *tx_pin_ = tx_val_;
}

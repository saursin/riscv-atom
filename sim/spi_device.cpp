#include "spi_device.hpp"

SPIDevice::SPIDevice(const uint8_t *sck, const uint8_t *mosi, const uint8_t *cs, unsigned cs_bit, uint8_t *miso):
    sck_(sck), mosi_(mosi), cs_(cs), cs_bit_(cs_bit), miso_(miso)
{}


void SPIDevice::eval()
{
    bool selected = !((*cs_ >> cs_bit_) & 1);
    bool sck = *sck_ & 1;

    if(selected && !selected_) {
        select();
        bit_ = 0;
        out_ = 0xff;
        *miso_ = out_ >> 7;                     // first bit valid before the first rising edge
    } else if(!selected && selected_) {
        deselect();
    }
    selected_ = selected;

    if(!selected) {
        *miso_ = 1;                             // released: pulled up
    } else if(sck && !last_sck_) {              // rising edge: sample MOSI
        in_ = (in_ << 1) | (*mosi_ & 1);
        if(++bit_ == 8) {
            out_ = transfer(in_);
            bit_ = 0;
        }
    } else if(!sck && last_sck_) {              // falling edge: next bit out, MSB first
        *miso_ = (out_ >> (7 - bit_)) & 1;
    }
    last_sck_ = sck;
}

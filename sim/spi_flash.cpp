#include "spi_flash.hpp"
#include "except.hpp"
#include <fstream>
#include <iterator>
#include <algorithm>

enum {
    CMD_WRITE_DISABLE   = 0x04,
    CMD_WRITE_ENABLE    = 0x06,
    CMD_READ_STATUS     = 0x05,
    CMD_READ            = 0x03,
    CMD_FAST_READ       = 0x0b,
    CMD_PAGE_PROGRAM    = 0x02,
    CMD_SECTOR_ERASE    = 0x20,     // 4 KiB
    CMD_BLOCK_ERASE     = 0xd8,     // 64 KiB
    CMD_CHIP_ERASE      = 0xc7,
    CMD_CHIP_ERASE_ALT  = 0x60,
    CMD_JEDEC_ID        = 0x9f,
};

static const uint8_t JEDEC_ID[3] = {0xef, 0x40, 0x17};     // Winbond, W25Q64
static const uint32_t PAGE_SIZE = 256;


SPIFlash::SPIFlash(const uint8_t *sck, const uint8_t *mosi, const uint8_t *cs, unsigned cs_bit, uint8_t *miso,
                   const std::string &image):
    SPIDevice(sck, mosi, cs, cs_bit, miso), mem_(SIZE, 0xff)
{
    if(image.empty())
        return;
    std::ifstream f(image, std::ios::binary);
    if(!f)
        throw Atomsim_exception("can't open flash image " + image);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    if(data.size() > SIZE)
        throw Atomsim_exception("flash image " + image + " is larger than the flash (8 MiB)");
    std::copy(data.begin(), data.end(), mem_.begin());
}


void SPIFlash::select()
{
    n_ = 0;
    addr_ = 0;
}


uint8_t SPIFlash::transfer(uint8_t in)
{
    unsigned n = n_++;
    if(n == 0) {
        cmd_ = in;
        switch(cmd_) {
            case CMD_WRITE_ENABLE:  wel_ = true; break;
            case CMD_WRITE_DISABLE: wel_ = false; break;
            case CMD_JEDEC_ID:      return JEDEC_ID[0];
            case CMD_READ_STATUS:   return wel_ ? 0x02 : 0x00;      // never busy: program and erase are instant
        }
        return 0xff;
    }

    switch(cmd_) {
        case CMD_JEDEC_ID:
            return n < 3 ? JEDEC_ID[n] : 0xff;
        case CMD_READ_STATUS:
            return wel_ ? 0x02 : 0x00;
        case CMD_READ: case CMD_FAST_READ: case CMD_PAGE_PROGRAM:
        case CMD_SECTOR_ERASE: case CMD_BLOCK_ERASE:
            if(n <= 3) {                        // 24-bit address, MSB first
                addr_ = (addr_ << 8) | in;
                if(n == 3 && cmd_ == CMD_READ)
                    return mem_[addr_ % SIZE];  // first data byte goes out next
                return 0xff;
            }
            return data_byte(in);
    }
    return 0xff;
}


uint8_t SPIFlash::data_byte(uint8_t in)
{
    switch(cmd_) {
        case CMD_READ:                          // address auto-increments, wrapping at the end
            addr_ = (addr_ + 1) % SIZE;
            return mem_[addr_];
        case CMD_FAST_READ:                     // one dummy byte, then data
            if(n_ == 5)
                return mem_[addr_ % SIZE];
            addr_ = (addr_ + 1) % SIZE;
            return mem_[addr_];
        case CMD_PAGE_PROGRAM:
            if(wel_) {
                mem_[addr_ % SIZE] &= in;       // programming only clears bits
                addr_ = (addr_ & ~(PAGE_SIZE - 1)) | ((addr_ + 1) & (PAGE_SIZE - 1));  // wrap in the page
            }
            return 0xff;
    }
    return 0xff;
}


void SPIFlash::deselect()
{
    // erase and program take effect when chip select goes high, then clear the write enable latch
    if(!wel_)
        return;
    uint32_t a = addr_ % SIZE;
    switch(cmd_) {
        case CMD_SECTOR_ERASE:
            if(n_ >= 4) std::fill(mem_.begin() + (a & ~0xfffu), mem_.begin() + (a & ~0xfffu) + 0x1000, 0xff);
            break;
        case CMD_BLOCK_ERASE:
            if(n_ >= 4) std::fill(mem_.begin() + (a & ~0xffffu), mem_.begin() + (a & ~0xffffu) + 0x10000, 0xff);
            break;
        case CMD_CHIP_ERASE: case CMD_CHIP_ERASE_ALT:
            std::fill(mem_.begin(), mem_.end(), 0xff);
            break;
        case CMD_PAGE_PROGRAM:
            break;
        default:
            return;     // other commands leave the latch alone
    }
    wel_ = false;
}

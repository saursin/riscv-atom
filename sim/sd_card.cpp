#include "sd_card.hpp"
#include "except.hpp"
#include <fstream>
#include <iterator>

static const uint8_t R1_ILLEGAL_CMD = 0x04;
static const uint8_t R1_ADDR_ERROR  = 0x20;
static const uint8_t START_TOKEN    = 0xfe;
static const uint8_t DATA_ACCEPTED  = 0x05;
static const uint32_t OCR           = 0xc0ff8000;     // powered up, high capacity, 2.7-3.6 V


SDCard::SDCard(const uint8_t *sck, const uint8_t *mosi, const uint8_t *cs, unsigned cs_bit, uint8_t *miso,
               const std::string &image):
    SPIDevice(sck, mosi, cs, cs_bit, miso)
{
    std::ifstream f(image, std::ios::binary);
    if(!f)
        throw Atomsim_exception("can't open sdcard image " + image);
    mem_.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    if(mem_.empty())
        throw Atomsim_exception("sdcard image " + image + " is empty");
    mem_.resize((mem_.size() + BLOCK - 1) / BLOCK * BLOCK, 0);
}


void SDCard::select()
{
    // a frame or block cut short by chip select is dropped
    n_ = 0;
    state_ = COMMAND;
}


uint8_t SDCard::transfer(uint8_t in)
{
    switch(state_) {
        case COMMAND:
            if(n_ > 0 || (in & 0xc0) == 0x40) {     // a frame starts with 01xxxxxx; filler bytes are 0xFF
                frame_[n_++] = in;
                if(n_ == 6) {
                    n_ = 0;
                    command(frame_[0] & 0x3f, frame_[1] << 24 | frame_[2] << 16 | frame_[3] << 8 | frame_[4]);
                }
            }
            break;

        case WRITE_TOKEN:
            if(in == START_TOKEN) {
                state_ = WRITE_DATA;
                wn_ = 0;
            }
            break;

        case WRITE_DATA:
            if(wn_ < BLOCK)
                mem_[waddr_ + wn_] = in;
            if(++wn_ == BLOCK + 2) {                // block and CRC received
                out_ = {DATA_ACCEPTED, 0x00, 0x00}; // accepted, then busy for two bytes
                state_ = COMMAND;
            }
            break;
    }

    if(out_.empty())
        return 0xff;
    uint8_t b = out_.front();
    out_.pop_front();
    return b;
}


void SDCard::command(uint8_t cmd, uint32_t arg)
{
    bool app = app_;
    app_ = false;
    out_ = {0xff};      // one byte of response delay, as on a real card

    switch(cmd) {
        case 0:         // GO_IDLE_STATE
            idle_ = true;
            out_.push_back(r1());
            break;

        case 8:         // SEND_IF_COND: echo voltage and check pattern
            out_.insert(out_.end(), {r1(), 0x00, 0x00, (uint8_t)((arg >> 8) & 0x0f), (uint8_t)arg});
            break;

        case 55:        // APP_CMD
            app_ = true;
            out_.push_back(r1());
            break;

        case 41:        // ACMD41 SD_SEND_OP_COND: initialization finishes at once
            if(!app) {
                out_.push_back(r1(R1_ILLEGAL_CMD));
                break;
            }
            idle_ = false;
            out_.push_back(r1());
            break;

        case 58:        // READ_OCR
            out_.insert(out_.end(), {r1(), (uint8_t)(OCR >> 24), (uint8_t)(OCR >> 16), (uint8_t)(OCR >> 8), (uint8_t)OCR});
            break;

        case 17:        // READ_SINGLE_BLOCK
            if((uint64_t)arg * BLOCK >= mem_.size()) {
                out_.push_back(r1(R1_ADDR_ERROR));
                break;
            }
            out_.insert(out_.end(), {r1(), 0xff, START_TOKEN});
            out_.insert(out_.end(), mem_.begin() + arg * BLOCK, mem_.begin() + (arg + 1) * BLOCK);
            out_.insert(out_.end(), {0xff, 0xff});  // CRC (not checked)
            break;

        case 24:        // WRITE_BLOCK
            if((uint64_t)arg * BLOCK >= mem_.size()) {
                out_.push_back(r1(R1_ADDR_ERROR));
                break;
            }
            waddr_ = arg * BLOCK;
            state_ = WRITE_TOKEN;
            out_.push_back(r1());
            break;

        default:
            out_.push_back(r1(R1_ILLEGAL_CMD));
    }
}

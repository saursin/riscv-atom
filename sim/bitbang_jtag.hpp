# pragma once
#include <queue>
#include <stdint.h>
#include <cstddef>

#define BUF_SIZE 64 * 1024

class BitbangJTAG {
public:
    BitbangJTAG(uint16_t port, bool *tck, bool *trstn, bool *tms, bool *tdi, bool *tdo);
    void tick();
    bool is_connected() {return client_fd > 0;}
    void accept();      // Check for a client connecting, and accept if there is one.
private:
    bool *tck;
    bool *trstn;
    bool *tms;
    bool *tdi;
    bool *tdo;

    void set_pins(bool tck, bool tms, bool tdi, bool trstn=true) {
        printf("JTAG: tck=%d, tms=%d, tdi=%d\n", tck, tms, tdi);
        *this->tck = tck; *this->tms = tms; *this->tdi = tdi; *this->trstn = trstn;}
    bool get_tdo() { return *tdo; }

    // Socket descriptors
    int socket_fd;
    int client_fd;

    bool quit = false;
};

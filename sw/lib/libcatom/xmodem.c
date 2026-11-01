#include <stdio.h>
#include <serial.h>
#include <xmodem.h>

#define SOH 0x01                // 128-byte block
#define STX 0x02                // 1024-byte block (XMODEM-1K)
#define EOT 0x04
#define ACK 0x06
#define NAK 0x15
#define CAN 0x18
#define PAD 0x1A

#define BLOCK           128
#define BLOCK_1K        1024
#define HANDSHAKE_S     120     // receive: how long to wait for a sender
#define C_INTERVAL_MS   1000    // receive: one 'C' per interval while waiting
#define BYTE_MS         50      // timeout within a packet
#define RECEIVER_S      10      // send: how long to wait for a receiver
#define REPLY_MS        1000    // send: wait for ACK/NAK
#define RETRIES         10

__attribute__((weak)) void xmodem_activity(void) {}

// bit by bit: GCC 15+ would otherwise turn the loop into a 512-byte lookup table
#if __GNUC__ >= 15
__attribute__((optimize("no-optimize-crc")))
#endif
static uint16_t crc_byte(uint16_t crc, uint8_t b)
{
    crc ^= (uint16_t)b << 8;
    for(int i = 0; i < 8; i++)
        crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    return crc;
}

uint16_t xmodem_crc16(const uint8_t *buf, int len)
{
    uint16_t crc = 0;
    while(len--)
        crc = crc_byte(crc, *buf++);
    return crc;
}

// Rest of a packet (after SOH/STX) of size bytes into buf: 1 packet pkt_num, 2 an intact copy of
// the previous packet (the sender missed our ACK), 0 corrupted or out of sequence
static int get_packet(uint8_t pkt_num, uint8_t *buf, int size)
{
    uint8_t n = serial_read_timeout(BYTE_MS);
    uint8_t ni = serial_read_timeout(BYTE_MS);
    for(int i = 0; i < size; i++)
        buf[i] = serial_read_timeout(BYTE_MS);
    uint16_t chk = serial_read_timeout(BYTE_MS) << 8;
    chk |= (uint8_t)serial_read_timeout(BYTE_MS);

    if(ni != (uint8_t)(255 - n) || chk != xmodem_crc16(buf, size))
        return 0;
    if(n == pkt_num)
        return 1;
    return n == (uint8_t)(pkt_num - 1) ? 2 : 0;
}

int xmodem_recv(uint8_t *dest, uint32_t maxlen)
{
    puts(XMODEM_RECV_MARKER);
    // clear the receive buffer, finish sending the marker
    while(serial_read_timeout(0) != -1)
        ;
    serial_flush();

    // 'C' until the sender starts; anything other than SOH/STX/EOT/CAN is line noise
    int c = -1;
    for(int t = 0; t < HANDSHAKE_S * 1000 / C_INTERVAL_MS && c != SOH && c != STX; t++) {
        serial_write('C');
        xmodem_activity();
        c = serial_read_timeout(C_INTERVAL_MS);
        if(c == EOT) { serial_write(ACK); return 0; }
        if(c == CAN) return XMODEM_ERR_CANCEL;
    }
    if(c != SOH && c != STX) {
        serial_write(CAN); serial_write(CAN); serial_write(CAN);
        return XMODEM_ERR_TIMEOUT;
    }

    uint32_t received = 0;
    uint8_t seq = 1;
    int start = c;              // the first packet's SOH/STX ended the handshake
    for(;;) {
        if(!start) {
            start = serial_read_timeout(BYTE_MS * 20);
            if(start == EOT) { serial_write(ACK); return received; }
            if(start == CAN) { serial_write(ACK); return XMODEM_ERR_CANCEL; }
            if(start != SOH && start != STX) return XMODEM_ERR_PROTOCOL;
        }
        int size = start == STX ? BLOCK_1K : BLOCK;
        start = 0;
        if(received + size > maxlen) {
            serial_write(CAN); serial_write(CAN); serial_write(CAN);
            return XMODEM_ERR_TOOBIG;
        }
        // only a good packet advances; a bad one is asked for again (NAK) once the line is
        // quiet; a repeat of the previous one is acknowledged and dropped (the next overwrites it)
        int s = get_packet(seq, dest + received, size);
        if(!s) {
            while(serial_read_timeout(BYTE_MS) != -1)
                ;
            serial_write(NAK);
        } else {
            serial_write(ACK);
            if(s == 1) {
                received += size;
                seq++;
            }
        }
        xmodem_activity();
    }
}

static void put_dec(uint32_t v)
{
    if(v >= 10)
        put_dec(v / 10);
    putchar('0' + v % 10);
}

int xmodem_send(const char *name, const void *buf, uint32_t len)
{
    const uint8_t *p = buf;
    puts(XMODEM_SEND_MARKER "(");
    put_dec(len);
    putchar(',');
    for(; *name; name++)
        putchar(*name == ',' || *name == ')' ? '_' : *name);
    putchar(')');
    serial_flush();

    int c = -1;
    for(int t = 0; t < RECEIVER_S * 1000 / REPLY_MS && c != 'C'; t++) {
        c = serial_read_timeout(REPLY_MS);
        if(c == CAN) return XMODEM_ERR_CANCEL;
        xmodem_activity();
    }
    if(c != 'C')
        return XMODEM_ERR_TIMEOUT;

    // 1024-byte blocks, 128-byte blocks for the tail; each block is read twice (CRC, then send)
    // rather than copied, and padded past len
    uint8_t seq = 1;
    for(uint32_t off = 0; off < len; seq++) {
        int size = len - off >= BLOCK_1K ? BLOCK_1K : BLOCK;
        uint16_t crc = 0;
        for(int i = 0; i < size; i++)
            crc = crc_byte(crc, off + i < len ? p[off + i] : PAD);
        for(int r = 0;; r++) {
            if(r == RETRIES) {
                serial_write(CAN); serial_write(CAN); serial_write(CAN);
                return XMODEM_ERR_PROTOCOL;
            }
            serial_write(size == BLOCK_1K ? STX : SOH); serial_write(seq); serial_write(255 - seq);
            for(int i = 0; i < size; i++)
                serial_write(off + i < len ? p[off + i] : PAD);
            serial_write(crc >> 8); serial_write(crc);
            c = serial_read_timeout(REPLY_MS);
            if(c == ACK) break;
            if(c == CAN) return XMODEM_ERR_CANCEL;
        }
        off += size;
        xmodem_activity();
    }
    for(int r = 0; r < RETRIES; r++) {
        serial_write(EOT);
        if(serial_read_timeout(REPLY_MS) == ACK)
            return 0;
    }
    return XMODEM_ERR_PROTOCOL;
}

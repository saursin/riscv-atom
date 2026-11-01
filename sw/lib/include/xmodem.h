#pragma once
#include <stdint.h>

// XMODEM-CRC over the serial port: receives 128- and 1024-byte blocks (XMODEM-1K), sends
// 1024-byte blocks and 128-byte ones for the tail. Both directions announce themselves with a
// marker that scripts/atomterm watches for. Markers are "![NAME]!", optionally followed at once by
// comma-separated parameters "(p1,p2,...)" (values without ',' or ')'):
//   receive: "![XMODREQ]!(1k)" (1024-byte blocks accepted), then the 'C' handshake
//   send:    "![XMODSND]!(<len>,<name>)", then waits for the receiver's 'C'

#define XMODEM_RECV_MARKER  "![XMODREQ]!(1k)"
#define XMODEM_SEND_MARKER  "![XMODSND]!"

#define XMODEM_ERR_TIMEOUT  -1      // no sender / no receiver
#define XMODEM_ERR_CANCEL   -2      // cancelled by the other side
#define XMODEM_ERR_TOOBIG   -3      // more data than maxlen
#define XMODEM_ERR_PROTOCOL -4      // unexpected byte, or retries exhausted

// CRC-16/XMODEM (poly 0x1021) of len bytes
uint16_t xmodem_crc16(const uint8_t *buf, int len);

// Receives into dest (at most maxlen bytes, whole blocks): the number of bytes received, or a
// negative XMODEM_ERR_*. Waits up to 120 s for the sender.
int xmodem_recv(uint8_t *dest, uint32_t maxlen);

// Sends len bytes from buf, announced as <name> (a file name for the receiver; ',' and ')' become
// '_'): 0, or a negative XMODEM_ERR_*. Gives up after ~10 s without a receiver.
int xmodem_send(const char *name, const void *buf, uint32_t len);

// Called while waiting and after every block (e.g. to blink an LED); does nothing by default
void xmodem_activity(void);

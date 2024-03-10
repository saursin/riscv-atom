#! /usr/bin/python3

import serial
import signal
import time
import sys


InterByteDelay_simt = 0.2   # SIM_WITH_TRACE: simulation with trace       
InterByteDelay_sim  = 0.005 # NORMAL SIM: 0.001 works, for safety we chose this
InterByteDelay_fpga = 0     # FPGA

InterByteDelay = 0

#############################################################################
## UTIL

CRC16_XMODEM_TABLE = [
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
        ]


def calc_crc16(data, crc=0):
    """Calculate CRC16 using the given table.
    `data`      - data for calculating CRC, must be bytes
    `crc`       - initial value
    `table`     - table for caclulating CRC (list of 256 integers)
    Return calculated value of CRC
    """
    for byte in data:
        crc = ((crc<<8)&0xff00) ^ CRC16_XMODEM_TABLE[((crc>>8)&0xff)^byte]
    return crc & 0xffff


def ctrl_c_handler(signum, frame): # TODO: at ctrl c, finish current packet and send CAN
    exit(1)


def progressbar(total, progress, prefix=''):
    """
    Displays or updates a console progress bar.
    Original source: https://stackoverflow.com/a/15860757/1391441
    """
    barLength, status = 20, ""
    progr = float(progress) / float(total)
    if progr >= 1.:
        progr, status = 1, "\r\n"
    block = int(round(barLength * progr))
    text = "\r{:s}[{}] {:>2.0f}% ({}/{}) {}".format(prefix, 
        "#" * block + "-" * (barLength - block), 
        round(progr * 100, 0),
        int(progress), int(total), 
        status)
    sys.stdout.write(text)
    sys.stdout.flush()

#############################################################################
## XMODEM

import logging as log
class XMODEM:
    SOH = b'\x01'
    STX = b'\x02'
    EOT = b'\x04'
    ACK = b'\x06'
    DLE = b'\x10'
    NAK = b'\x15'
    CAN = b'\x18'
    CRC = b'C'

    def __init__(self, port, baud, checksum_type='CRC', verbose=False, show_progress=False):
        self.ser_port_name = port
        self.ser_port_baud = baud
        self.ser_port_timeout = 10
        self.inter_out_byte_delay = InterByteDelay
        self.checksumType = checksum_type
        self.binfile = None
        self.ser = None
        self.show_progress = show_progress
        # self.lg = Logger(Logger.ERROR if self.show_progress else verbose)
        
    def pack_file(self, file_name):
        fcontents = []

        try:
            f = open(file_name, "rb")
        except IOError:
            log.error("File can't be opened")
            exit(1)

        while 1:
            packet = f.read(128)
            if len(packet) < 128:
                while len(packet) < 128:
                    packet = bytearray(packet)
                    packet.append(0x1A)
                packet = bytes(packet)
                fcontents.append(packet)
                break
            fcontents.append(packet)
        return fcontents

    def open_serial(self):
        ser = serial.Serial()
        ser.baudrate = self.ser_port_baud
        ser.port = self.ser_port_name
        ser.timeout = self.ser_port_timeout
        ser.parity = serial.PARITY_NONE
        ser.stopbits = serial.STOPBITS_ONE
        ser.bytesize = serial.EIGHTBITS
        ser.open()
        return ser
    
    def close_serial(self):
        self.ser.close()

    def send(self, file, max_failed_packets = 10):
        packed_file = self.pack_file(file)

        log.info("file:    {:s}".format(file))
        log.info("size:    {:d} bytes".format(len(packed_file)*128))
        log.info("packets: {:d}".format(len(packed_file)))            

        # Open Serial Port
        self.ser = self.open_serial()
        log.info("opened serial port '{:s}' at baud {:d}".format(self.ser_port_name, self.ser_port_baud))

        # GET NAK/C :Determine Checksum Type
        inchar = b''
        counter = 0
        countlimit = 1000

        log.info("Waiting for reciever ping...")
        
        tend = 10 + time.time()
        while time.time() < tend:
            # read 10 characters at a time
            inchars = self.ser.read(5)
            log.info(f"got: '{inchars}'")

            # if all 10 characters equal to NAK or CRC, we start
            if all(c == inchars[0] for c in inchars):
                inchar = inchars[:1]
                if inchar == XMODEM.NAK:
                    self.checksumType = "algebraic"
                    break
                elif inchar == XMODEM.CRC:
                    self.checksumType = "CRC"
                    break

        index = 0
        packet_nr = 1
        if inchar != XMODEM.NAK and inchar != XMODEM.CRC:
            log.error('timeout, did not recieve NAK/CRC')
            exit(3)

        log.info("Starting transfer... [checksum type:{:s}]".format(self.checksumType))

        # Send Packets
        failed_packets = 0
        while index != len(packed_file):
            if failed_packets > max_failed_packets:
                log.error('Failed to send too many packets')
                self.close_serial()
                return
            if self.show_progress:
                progressbar(len(packed_file), packet_nr, 'sending: ')
            else:
                log.info(f"Sending packet {packet_nr}/{len(packed_file)}")
            self.send_packet(packet_nr, packed_file[index])
            
            inchar = self.ser.read()
            log.debug("got: 0x{:s}".format(str(inchar)))

            if inchar == XMODEM.ACK:
                log.debug("received ACK")
                index += 1
                packet_nr = 0xff & (packet_nr + 1)
            elif inchar == XMODEM.NAK:
                log.debug("received NAK")
                log.warning(f"Packet {packet_nr} failed to send")
                failed_packets += 1
            elif inchar == XMODEM.CAN:
                log.error("received CAN, exiting")
                exit(4)
            self.ser.read(self.ser.in_waiting)
             
        self.ser.timeout = 2
        log.info("Finished transfer!")
        while True:
            log.debug("sending EOT")
            self.out_byte(XMODEM.EOT)
            rid = self.ser.read()
            if rid == XMODEM.ACK:
                log.debug("received ACK")
                break
            if rid == XMODEM.CAN:
                self.log("received CAN")
                break
            self.ser.read(self.ser.in_waiting)
        
        # Close Serial
        self.close_serial()
        log.info('serial port closed')

    def send_packet(self, packet_nr, packet):
        ctrl_sum = self.checksum(packet)
        packet_to_send = bytearray(XMODEM.SOH)
        packet_to_send += bytearray(packet_nr.to_bytes(1, 'big'))
        packet_to_send += bytearray((255 - packet_nr).to_bytes(1, 'big'))
        packet_to_send += packet
        if self.checksumType == "algebraic":
            ctrl_sum = bytearray(ctrl_sum.to_bytes(1, 'big'))
        else:
            ctrl_sum = bytearray(ctrl_sum.to_bytes(2, 'big'))
        packet_to_send += ctrl_sum
        packet_to_send = bytes(packet_to_send)
        
        # if self.lg.get_level() <= Logger.DEBUG:
        #     self.print_packet(packet_to_send)
        
        for b in packet_to_send:
            self.out_byte(int.to_bytes(b, 1, 'big')) 

    def checksum(self, block):
        if self.checksumType == "algebraic":
            suma = 0
            for i in block:
                suma += i
            suma = suma % 256
            return suma
        elif self.checksumType == "CRC":
            crc = calc_crc16(block)
            return crc
            pass
        return 0

    def print_packet(self, packet):
        print('[ SOH, n={:d}, ~n={:d},'.format(packet[1], packet[2]))

        i = 3
        while i < 128+3:
            print("{:0>2x}".format(packet[i]), end='\n' if (i-3)%16==15 else ' ')
            i+=1
        print('chksum= 0x{:0>2x}{:0>2x}]'.format(packet[131], packet[132]))

    def out_byte(self, byte):
        self.ser.write(byte)
        time.sleep(self.inter_out_byte_delay)



if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Send File to Serial Port using Xmodem Protocol')
    parser.add_argument('-v', '--verbose', dest='verbose', default='info',
                        choices=['debug', 'info', 'warning', 'error', 'critical'],
                        help='Set the verbose level (default: info)')
    parser.add_argument('-p', '--show-progress', action='store_true', help='show progress bar')
    parser.add_argument('port', type=str, help='serial port')
    parser.add_argument('-b', '--baud', type=int, default=115200, help='serial port baud rate (default:115200)')
    parser.add_argument('-s', '--sim', help='if enabled, interbyte delay will be increased', action='store_true')
    parser.add_argument('binfile', type=str, help='binary file to be sent')

    args = parser.parse_args()

    def setup_logging(log_level):
        numeric_level = getattr(log, log_level.upper(), None)
        if not isinstance(numeric_level, int):
            raise ValueError('Invalid log level: %s' % log_level)
        log.basicConfig(level=numeric_level, format="[%(levelname)s]: %(message)s")

    setup_logging(args.verbose)

    InterByteDelay = InterByteDelay_sim if args.sim else InterByteDelay_fpga

    signal.signal(signal.SIGINT, ctrl_c_handler)

    try:
        xm = XMODEM(args.port, args.baud, 'CRC', verbose=args.verbose, show_progress=args.show_progress)
        xm.send(args.binfile)
    except Exception as e:
        print('EXCEPTION', e)
        raise e

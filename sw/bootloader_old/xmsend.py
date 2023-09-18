#! /usr/bin/python3

import serial
import crc16
import signal
import time
import sys

# Ignore deprecation warnings from crc16
import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning) 


# InterByteDelay = 0.2     # SIM_WITH_TRACE: simulation with trace 
InterByteDelay = 0.005   # NORMAL SIM: 0.001 works, for safety we chose this
# InterByteDelay = 0.0001  # FPGA:

#############################################################################
## UTIL


def ctrl_c_handler(signum, frame): # TODO: at ctrl c, finish current packet and send CAN
    exit(1)


class Logger:
    DEBUG=0
    INFO=1
    WARN=2
    ERROR=3
    CRITICAL=4
    def __init__(self, lvl=1, file=None):
        self.lvl = lvl
        self.file=file
        self.fh = None if file==None else open(self.file, 'w')

    def set_level(self, lvl:int):
        self.lvl = lvl

    def get_level(self):
        return self.lvl
    
    def debug(self, msg):
        if self.lvl <= self.DEBUG:
            print("DEBUG: ", msg, file=(sys.stdout if self.file == None else self.fh), flush=True)
    
    def info(self, msg):
        if self.lvl <= self.INFO:
            print("INFO: ", msg, file=(sys.stdout if self.file == None else self.fh), flush=True)
    
    def warn(self, msg):
        if self.lvl <= self.WARN:
            print("WARN: ", msg, file=(sys.stderr if self.file == None else self.fh), flush=True)
    
    def error(self, msg):
        if self.lvl <= self.ERROR:
            print("ERROR: ", msg, file=(sys.stderr if self.file == None else self.fh), flush=True)
    
    def critical(self, msg, abort=True):
        if self.lvl <= self.CRITICAL:
            print("CRITICAL: ", msg, file=(sys.stderr if self.file == None else self.fh), flush=True)
            if abort:
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
    text = "\r{:s}[{}] {:.0f}% ({}/{}) {}".format(prefix, 
        "#" * block + "-" * (barLength - block), 
        round(progr * 100, 0),
        int(progress), int(total), 
        status)
    sys.stdout.write(text)
    sys.stdout.flush()


#############################################################################
## XMODEM


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
        self.lg = Logger(Logger.ERROR if self.show_progress else verbose)
        
    def pack_file(self, file_name):
        fcontents = []

        try:
            f = open(file_name, "rb")
        except IOError:
            self.lg.error("File can't be opened")
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

    def send(self, file):
        packed_file = self.pack_file(file)

        self.lg.info("file:    {:s}".format(file))
        self.lg.info("size:    {:d} bytes".format(len(packed_file)*128))
        self.lg.info("packets: {:d}".format(len(packed_file)))            

        # Open Serial Port
        self.ser = self.open_serial()
        self.lg.info("opened serial port '{:s}' at baud {:d}".format(self.ser_port_name, self.ser_port_baud))

        # GET NAK/C :Determine Checksum Type
        incoming_msg = b''
        counter = 0

        self.lg.info("Waiting for reciever ping...")
        while incoming_msg != XMODEM.NAK and incoming_msg !=XMODEM.CRC and counter < 6:
            incoming_msg = self.ser.read(1)
            if incoming_msg == XMODEM.NAK:
                self.checksumType = "algebraic"
            if incoming_msg == XMODEM.CRC:
                self.checksumType = "CRC"
            counter += 1

        index = 0
        packet_nr = 1
        if incoming_msg != XMODEM.NAK and incoming_msg != XMODEM.CRC:
            exit(3)

        self.lg.info("Starting transfer... [checksum type:{:s}]".format(self.checksumType))
        if self.show_progress:
            progressbar(len(packed_file), 0, 'sending: ')

        # Send Packets
        while index != len(packed_file):

            self.send_packet(packet_nr, packed_file[index])
            self.lg.info("sent packet {:d}/{:d}".format(packet_nr, len(packed_file)))
            
            if self.show_progress:
                progressbar(len(packed_file), packet_nr, 'sending: ')
            
            incoming_msg = self.ser.read()
            self.lg.debug("got: 0x{:s}".format(str(incoming_msg)))

            if incoming_msg == XMODEM.ACK:
                self.lg.debug("received ACK")
                index += 1
                packet_nr += 1
                if packet_nr == 256:
                    packet_nr = 1
                pass
            elif incoming_msg == XMODEM.NAK:
                self.lg.debug("received NAK")
                pass
            elif incoming_msg == XMODEM.CAN:
                self.lg.info("received CAN, exiting")
                exit(4)
                break
            self.ser.read(self.ser.in_waiting)
             
        self.ser.timeout = 2
        self.lg.info("Finished transfer!")
        while True:
            self.lg.debug("sending EOT")
            self.out_byte(XMODEM.EOT)
            rid = self.ser.read()
            if rid == XMODEM.ACK:
                self.lg.debug("received ACK")
                break
            if rid == XMODEM.CAN:
                self.log("received CAN")
                break
            self.ser.read(self.ser.in_waiting)
        
        # Close Serial
        self.close_serial()
        self.lg.info('serial port closed')

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
        
        if self.lg.get_level() <= Logger.DEBUG:
            self.print_packet(packet_to_send)
        
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
            crc = crc16.crc16xmodem(block)
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
    parser.add_argument('-v', '--verbose', type=int, default=Logger.WARN, help='0:debug, 1:info, 2:warn, 2:error, 3:critical (default:2)')
    parser.add_argument('-p', '--show-progress', action='store_true', help='show progress bar')
    parser.add_argument('port', type=str, help='serial port')
    parser.add_argument('-b', '--baud', type=int, default=115200, help='serial port baud rate (default:115200)')
    parser.add_argument('binfile', type=str, help='binary file to be sent')

    args = parser.parse_args()

    signal.signal(signal.SIGINT, ctrl_c_handler)

    xm = XMODEM(args.port, args.baud, 'CRC', verbose=args.verbose, show_progress=args.show_progress)
    xm.send(args.binfile)



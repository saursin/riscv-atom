import logging as l
import serial
import crc16
import sys
import time

l.basicConfig(level=l.INFO)

class XMODEM:
    SOH = b'\x01'
    STX = b'\x02'
    EOT = b'\x04'
    ACK = b'\x06'
    DLE = b'\x10'
    NAK = b'\x15'
    CAN = b'\x18'
    CRC = b'C'

    def __init__(self, port, baud, checksum_type='CRC'):
        self.ser_port_name = port
        self.ser_port_baud = baud
        self.ser_port_timeout = 10
        self.inter_out_byte_delay = 0.2
        self.checksumType = checksum_type
        self.binfile = None
        self.ser = None
       
    def pack_file(self, file_name):
        fcontents = []

        try:
            f = open(file_name, "rb")
        except IOError:
            print("File can't be opened")
            import sys
            sys.exit(1)

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

        # Open Serial Port
        self.ser = self.open_serial()

        # GET NAK/C :Determine Checksum Type
        incoming_msg = b''
        counter = 0
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
        
        # Send Packets
        while index != len(packed_file):        
            self.send_packet(packet_nr, packed_file[index])
            l.debug("sent packet no. ", packet_nr)
            
            incoming_msg = self.ser.read()
            l.debug("got: ", incoming_msg)

            if incoming_msg == XMODEM.ACK:
                l.debug("received input ACK")
                index += 1
                packet_nr += 1
                if packet_nr == 256:
                    packet_nr = 1
                pass
            elif incoming_msg == XMODEM.NAK:
                l.debug("received input NAK")
                pass
            elif incoming_msg == XMODEM.CAN:
                l.debug("received CAN, exiting")
                exit(4)
                break
            self.ser.read(self.ser.in_waiting)
             
        self.ser.timeout = 2
        print("finished transfer")
        while True:
            print("sending EOT")
            self.out_byte(XMODEM.EOT)
            rid = self.ser.read()
            if rid == XMODEM.ACK:
                print("received ACK")
                break
            if rid == XMODEM.CAN:
                print("received CAN")
                break
            self.ser.read(self.ser.in_waiting)
        
        # Close Serial
        self.close_serial()

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
        l.debug('[ SOH, n={:d}, ~n={:d},'.format(packet[1], packet[2]))

        i = 3
        while i < 128+3:
            l.debug("{:0>2x}".format(packet[i]), end='\n' if (i-3)%16==15 else ' ')
            i+=1
        l.debug('chksum= 0x{:0>2x}{:0>2x}]'.format(packet[131], packet[132]))

    def out_byte(self, byte):
        self.ser.write(byte)
        time.sleep(self.inter_out_byte_delay)



if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Send File to Serial Port using Xmodem Protocol')
    parser.add_argument('-v', '--verbose', action='store_true')
    parser.add_argument('port', type=str)
    parser.add_argument('binfile', type=str)

    args = parser.parse_args()

    xm = XMODEM(args.port, 115200, 'CRC')
    xm.send(args.binfile)

////////////////////////////////////////////////////////////////////   
//  File        : IOBUF.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : BiDirectional IO (for GPIO pins)
////////////////////////////////////////////////////////////////////
`default_nettype none

module IOBUF
(
    input   wire    dir_i,      //  Direction control (INPUT=1; OUTPUT=0)
    input   wire    bit_i,      //  Bit value to output to pin
    output  wire    bit_o,      //  Bit value read from pin

    inout   wire    pin_io      //  bi-directional pin
);

    assign pin_io = (dir_i == 1'b1) ? 1'bz : bit_i;
    assign bit_o = pin_io;
endmodule

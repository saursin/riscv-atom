module IOBank #(
    parameter NUM_PINS = 8      // Number of Pins
) (
    input   wire [NUM_PINS-1:0]     dat_i,
    input   wire [NUM_PINS-1:0]     dir_i,
    output  wire [NUM_PINS-1:0]     out_o,
    inout   wire [NUM_PINS-1:0]     pins_io
);
    // IOBuf instances
    genvar i;
    generate
        for(i=0; i<NUM_PINS; i=i+1) begin: bufs
            IOBuf io (
                .dir_i  (dir_i[i]),
                .bit_i  (dat_i[i]),
                .bit_o  (out_o[i]),
                .pin_io (pins_io[i])
            );
        end
    endgenerate
endmodule

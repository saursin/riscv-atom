////////////////////////////////////////////////////////////////////   
//  File        : CSR_Unit.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Control & Status Register (CSR) unit contains logic 
//      to read and write to the CSR registers
////////////////////////////////////////////////////////////////////
`default_nettype none
`include "../common/Utils.vh"
`include  "CSR_defs.vh"

`define isdefined(x) `ifdef x 1'b1 `else 1'b0 `endif 

module CSR_Unit#
(
    parameter [31:0]    VEND_ID     = 32'h0000_0000,
    parameter [31:0]    ARCH_ID     = 32'h0000_0000,
    parameter [31:0]    IMPL_ID     = 32'h0000_0000,
    parameter [31:0]    HART_ID     = 32'h0000_0000
)(
    // Global signals
    input   wire    clk_i,
    input   wire    rst_i,

    // Signals for Reading from / Writing to CSRs
    input   wire [11:0]     addr_i,
    input   wire [31:0]     data_i,
    input   wire [1:0]      op_i,
    input   wire            we_i,

    output  wire [31:0]     data_o

    // input signals from pipeline

    // ouput signals to pipeline
);
    `UNUSED_VAR(we_i)

    // Generate Data to be written
    reg  [31:0] write_value;    // Value to be written onto a CSR register
    `UNUSED_VAR(write_value)

    reg  [31:0] read_value;     // Value of selected CSR register

    always @(*) /* COMBINATIONAL */ begin
        case(op_i[1:0])
            2'b00: write_value = data_i;                   // CSRRW
            2'b01: write_value = data_i & read_value;      // CSRRS
            2'b10: write_value = ~(data_i & read_value);   // CSRRC

            default:
            write_value = read_value;
        endcase
    end

    ////////////////////////////////////////////////////////////
    // CSR Registers

    // CYCLE (Read-Only)
    reg [63:0]  csr_cycle = 64'd0;

    always @(posedge clk_i) begin
        if(rst_i)
            csr_cycle <= 64'd0;
        else
            csr_cycle <= csr_cycle + 1'b1;
    end

    // MISA
    wire [31:0]  csr_misa = {
        2'b01,              // XLEN = 32
        4'd0,               // padding
        1'b0,               // bit-25    Z Reserved
        1'b0,               // bit-24    Y Reserved
        1'b0,               // bit-23    X Non-standard extensions present
        1'b0,               // bit-22    W Reserved
        1'b0,               // bit-21    V Tentatively reserved for Vector extension
        1'b0,               // bit-20    U User mode implemented
        1'b0,               // bit-19    T Reserved
        1'b0,               // bit-18    S Supervisor mode implemented
        1'b0,               // bit-17    R Reserved
        1'b0,               // bit-16    Q Quad-precision floating-point extension
        1'b0,               // bit-15    P Tentatively reserved for Packed-SIMD extension
        1'b0,               // bit-14    O Reserved
        1'b0,               // bit-13    N Tentatively reserved for User-Level Interrupts extension
        `isdefined(RV_M),   // bit-12    M Integer Multiply/Divide extension
        1'b0,               // bit-11    L Reserved
        1'b0,               // bit-10    K Reserved
        1'b0,               // bit-9     J Tentatively reserved for Dynamically Translated Languages extension
        1'b1,               // bit-8     I RV32I/64I/128I base ISA
        1'b0,               // bit-7     H Hypervisor extension
        1'b0,               // bit-6     G Reserved
        `isdefined(RV_F),   // bit-5     F Single-precision floating-point extension
        `isdefined(RV_E),   // bit-4     E RV32E base ISA
        `isdefined(RV_D),   // bit-3     D Double-precision floating-point extension
        `isdefined(RV_C),   // bit-2     C Compressed extension
        1'b0,               // bit-1     B Tentatively reserved for Bit-Manipulation extension
        `isdefined(RV_A)    // bit-0     A Atomic extension
    };

    ////////////////////////////////////////////////////////////
    // CSR Selection

    // assigns the read value function to various CSRs and sets we 
    // pins of CSRs (if not Readonly)

    always @(*) /* COMBINATIONAL */ begin
        // Defaults
        read_value = 0;
        
        case(addr_i)
            `CSR_cycle:     read_value = csr_cycle[31:0];  // cycle
            `CSR_cycleh:    read_value = csr_cycle[63:32]; // cycleh

            `CSR_mvendorid: read_value = VEND_ID;
            `CSR_marchid:   read_value = ARCH_ID;
            `CSR_mimpid:    read_value = IMPL_ID;
            `CSR_mhartid:   read_value = HART_ID;

            `CSR_misa:      read_value = csr_misa;
            default: begin
                // $display("RTL_ERR: invalid read to CSR addr 0x%x", addr_i);
                read_value = 32'hxxxx_xxxx;
            end
                
        endcase
    end

    assign data_o = read_value;

endmodule

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

    // input signals from pipeline
    input   wire    instr_retired_i,

    // ouput signals to pipeline


    // Signals for Reading from / Writing to CSRs
    input   wire [11:0]     addr_i,
    input   wire [31:0]     data_i,
    input   wire [1:0]      op_i,
    input   wire            we_i,

    output  wire [31:0]     data_o
);
    reg  [31:0] write_value;    // Value to be written onto a CSR register
    `UNUSED_VAR(write_value)

    reg  [31:0] read_value;     // Value of selected CSR register

    always @(*) /* COMBINATIONAL */ begin
        case(op_i[1:0])
            2'b01: write_value = data_i;                   // CSRRW
            2'b10: write_value = data_i | read_value;      // CSRRS
            2'b11: write_value = ~(data_i) & read_value;   // CSRRC

            default:
            write_value = read_value;
        endcase
    end

    ////////////////////////////////////////////////////////////
    // CSR Registers

    //===== MCYCLE & MCYCLEH ================================================
    reg [63:0]  csr_cycle = 64'd0;

    always @(posedge clk_i) begin
        if(rst_i)
            csr_cycle <= 64'd0;
        else
            csr_cycle <= csr_cycle + 1'b1;
    end


    //===== INSTRET & INSTRETH ==============================================
    reg [63:0]  csr_instret = 64'd0;
    always @(posedge clk_i) begin
        if(rst_i)
            csr_instret <= 64'd0;
        else if (instr_retired_i)
            csr_instret <= csr_instret + 1'b1;
    end


    //===== MISA ============================================================
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


    `ifdef EN_EXCEPT
    //===== MSTATUS & MSTATUSH ==============================================
    reg         csr_mstatus_mie;    // Machine global interrupt Enable

    wire [31:0] csr_mstatus_readval = {28'd0 , csr_mstatus_mie, 3'd0};
    wire [31:0] csr_mstatush_readval = 32'd0;

    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_mstatus_mie <= 0;
        end
        else if(we_i && (addr_i == `CSR_mstatus)) begin
            csr_mstatus_mie <= write_value[3];
        end
        else if(we_i && (addr_i == `CSR_mstatush)) begin
            // ...
        end
    end



    //===== MTVEC ===========================================================
    reg [31:2]  csr_mtvec_base;     // Base address
    reg [1:0]   csr_mtvec_mode;     // Mode (Direct/Vectored)

    wire [31:0] csr_mtvec_readval = {csr_mtvec_base, csr_mtvec_mode};
    
    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_mtvec_base <= 0;
            csr_mtvec_mode <= 0;
        end
        else if(we_i && (addr_i == `CSR_mtvec)) begin
            csr_mtvec_base <= write_value[31:2];
            csr_mtvec_mode <= write_value[1:0];
        end
    end

    
    //===== MIE =============================================================
    reg         csr_mie_meie;    // Machine external interrupt Enable
    reg         csr_mie_mtie;    // Machine timer interrupt Enable
    reg         csr_mie_msie;    // Machine software interrupt Enable
    
    wire [31:0] csr_mie_readval = {20'd0, csr_mie_meie, 3'd0, csr_mie_mtie, 3'd0, csr_mie_msie, 3'd0};
    
    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_mie_meie <= 0;
            csr_mie_mtie <= 0;
            csr_mie_msie <= 0;
        end
        else if(we_i && (addr_i == `CSR_mie)) begin
            csr_mie_meie <= write_value[11];
            csr_mie_mtie <= write_value[7];
            csr_mie_msie <= write_value[3];
        end
    end


    //===== MIP =============================================================
    reg         csr_mip_meip;    // Machine external interrupt Pending
    reg         csr_mip_mtip;    // Machine timer interrupt Pending
    reg         csr_mip_msip;    // Machine software interrupt Pending
    
    wire [31:0] csr_mip_readval = {20'd0, csr_mip_meip, 3'd0, csr_mip_mtip, 3'd0, csr_mip_msip, 3'd0};
    
    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_mip_meip <= 0;
            csr_mip_mtip <= 0;
            csr_mip_msip <= 0;
        end
        else if(we_i && (addr_i == `CSR_mie)) begin
            csr_mip_meip <= write_value[11];
            csr_mip_mtip <= write_value[7];
            csr_mip_msip <= write_value[3];
        end
    end


    //===== MEPC ============================================================
    reg     [31:1]  csr_mepc;       // Machine exception program counter
    wire    [31:0]  csr_mepc_readval = {csr_mepc, 1'b0};
    
    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_mepc <= 0;
        end
        else if(we_i && (addr_i == `CSR_mepc)) begin
            csr_mepc <= write_value[31:1];
        end
    end

    //===== MCAUSE ==========================================================
    reg             csr_mcause_intr;
    reg     [3:0]   csr_mcause_cause;    // Note: Actual size as per spec is 31 bits

    wire    [31:0]  csr_mcause_readval = {csr_mcause_intr, 27'd0, csr_mcause_cause};
    
    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_mcause_intr <= 0;
            csr_mcause_cause <= 0;
        end
    end
    `endif // EN_EXCEPT

    ////////////////////////////////////////////////////////////
    // CSR Selection

    // assigns the read value function to various CSRs and sets we 
    // pins of CSRs (if not Readonly)

    always @(*) /* COMBINATIONAL */ begin
        // Defaults
        read_value = 0;
        
        case(addr_i)
            `CSR_cycle:     read_value = csr_cycle[31:0];
            `CSR_cycleh:    read_value = csr_cycle[63:32];
            `CSR_instret:   read_value = csr_instret[31:0];
            `CSR_instreth:  read_value = csr_instret[63:32];

            `CSR_mvendorid: read_value = VEND_ID;
            `CSR_marchid:   read_value = ARCH_ID;
            `CSR_mimpid:    read_value = IMPL_ID;
            `CSR_mhartid:   read_value = HART_ID;

            `CSR_misa:      read_value = csr_misa;
            `CSR_mstatus:   read_value = csr_mstatus_readval;
            `CSR_mstatush:  read_value = csr_mstatush_readval;
            
            `ifdef EN_EXCEPT
            `CSR_mtvec:     read_value = csr_mtvec_readval;
            `CSR_mie:       read_value = csr_mie_readval;
            `CSR_mip:       read_value = csr_mip_readval;
            `CSR_mepc:      read_value = csr_mepc_readval;
            `CSR_mcause:    read_value = csr_mcause_readval;
            `endif // EN_EXCEPT

            default: begin
                // $display("RTL_ERR: invalid read to CSR addr 0x%x", addr_i);
                read_value = 32'hxxxx_xxxx;
            end
                
        endcase
    end

    assign data_o = read_value;

endmodule

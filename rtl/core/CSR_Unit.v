////////////////////////////////////////////////////////////////////   
//  File        : CSR_Unit.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Control & Status Register (CSR) unit contains logic 
//      to read and write to the CSR registers
////////////////////////////////////////////////////////////////////
`default_nettype none
`include "Utils.vh"
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
    input   wire    halted_i,

`ifdef EN_EXCEPT
    input   wire            except_instr_addr_misaligned_i,
    input   wire            except_illegal_instr_i,
    input   wire            except_load_addr_misaligned_i,
    input   wire            except_store_addr_misaligned_i,
    input   wire            intrpt_external_i,
    input   wire            intrpt_timer_i, 
    input   wire            intrpt_soft_i,
    input   wire [31:1]     except_pc_i,
`endif // EN_EXCEPT

    // ouput signals to pipeline
`ifdef EN_EXCEPT
    output  wire [31:0]     trap_jump_addr_o,
    output  wire            trap_caught_o,
    output  wire [31:1]     trap_epc_o,
`endif // EN_EXCEPT

    // Signals for Reading from / Writing to CSRs
    input   wire [11:0]     addr_i,
    input   wire [31:0]     data_i,
    input   wire [1:0]      op_i,
    input   wire            we_i,

    output  wire [31:0]     data_o
);
    reg  [31:0] write_value;    // Value to be written onto a CSR register
    `ifndef EN_EXCEPT
    `UNUSED_VAR(write_value)
    `UNUSED_VAR(we_i)
    `endif // EN_EXCEPT

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


    `ifdef EN_EXCEPT
    ////////////////////////////////////////////////////////////
    // TRAP Logic
    /*
        sources of interrupts:
            - synchronous:
                - exceptions:           gated with MSTATUS.MIE
                - sw interrupts         gated with MSTATUS.MIE & MIE.MSIE
            - asynchronous:
                - timer interrupt       gated with MSTATUS.MIE & MIE.MTIE
                - external interrupt    gated with MSTATUS.MIE & MIE.MEIE

        All asynchronous interrupt are passed through posedge edge detector since they cannot be 
        cleared immediately, and level sensitivity will cause PC to jump to trap_jump_addr_o continously.
    */
    wire exception      = (csr_mstatus_mie & (except_illegal_instr_i | except_instr_addr_misaligned_i
                            | except_load_addr_misaligned_i | except_store_addr_misaligned_i));
    
    wire intrpt_ext_async   = csr_mstatus_mie & csr_mie_meie & intrpt_external_i; 
    wire intrpt_timer_async = csr_mstatus_mie & csr_mie_mtie & intrpt_timer_i; 
    wire intrpt_soft        = csr_mstatus_mie & csr_mie_msie & intrpt_soft_i;

    // Posedge Detector
    reg intrpt_ext_async_r, intrpt_timer_async_r;
    always @(posedge clk_i) begin
        intrpt_ext_async_r      <= rst_i ? 0 : intrpt_ext_async;
        intrpt_timer_async_r    <= rst_i ? 0 : intrpt_timer_async;
    end
    wire intrpt_ext     = (intrpt_ext_async & ~intrpt_ext_async_r);
    wire intrpt_timer   = (intrpt_timer_async & ~intrpt_timer_async_r);
   
    assign trap_caught_o = exception | (intrpt_soft | intrpt_ext | intrpt_timer);

    /* 
        Trap Cause Generation (Based on priority) 
        refer previlige_spec section 3.1.15
    */
    reg [3:0]       trap_cause;
    always @(*) /* COMB */ begin
        trap_cause    = 0;
        if (exception) begin
            // if(except_instr_breakpoint_i)            trap_cause = 'd3    // Instr Breakpoint
            // if(except_instr_access_fault_i)          trap_cause = 'd1    // Instr Access fault
            if(except_illegal_instr_i)                  trap_cause = 'd2;   // Illegal Instr
            else if(except_instr_addr_misaligned_i)     trap_cause = 'd0;   // Instr Access misaligned
            // else if(except_instr_ecall_i)            trap_cause = 'd11;
            // else if(except_instr_ebreak_i)           trap_cause = 'd3;
            else if(except_load_addr_misaligned_i)      trap_cause = 'd4;
            else if(except_store_addr_misaligned_i)     trap_cause = 'd6;
            // else if(except_load_access_fault_i)      trap_cause = 'd5;
            // else if(except_store_access_fault_i)     trap_cause = 'd7;
        end else /* interrupt */ begin
            if(intrpt_soft)                             trap_cause = 'd3;   // Machine Sw interrupt
            else if(intrpt_timer)                       trap_cause = 'd7;   // Machine Timer interrupt
            else if(intrpt_ext)                         trap_cause = 'd11;  // Machine Ext interrupt
        end
    end

    // Trap jump address generation
    assign trap_jump_addr_o = (csr_mtvec_mode == 2'b00) ? {csr_mtvec_base, 2'b00}:
                                ((csr_mtvec_mode == 2'b01) ? {csr_mtvec_base, 2'b00} + (4 * trap_cause): 32'hxxxx_xxxx);

    `endif // EN_EXCEPT


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
        `isdefined(EN_RVM), // bit-12    M Integer Multiply/Divide extension
        1'b0,               // bit-11    L Reserved
        1'b0,               // bit-10    K Reserved
        1'b0,               // bit-9     J Tentatively reserved for Dynamically Translated Languages extension
        1'b1,               // bit-8     I RV32I/64I/128I base ISA
        1'b0,               // bit-7     H Hypervisor extension
        1'b0,               // bit-6     G Reserved
        `isdefined(EN_RVF), // bit-5     F Single-precision floating-point extension
        `isdefined(EN_RVE), // bit-4     E RV32E base ISA
        `isdefined(EN_RVD), // bit-3     D Double-precision floating-point extension
        `isdefined(EN_RVC), // bit-2     C Compressed extension
        1'b0,               // bit-1     B Tentatively reserved for Bit-Manipulation extension
        `isdefined(EN_RVA)  // bit-0     A Atomic extension
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
        else begin
            if (intrpt_ext)     csr_mip_meip <= 1'b1;
            if (intrpt_timer)   csr_mip_mtip <= 1'b1;
            if (intrpt_soft)    csr_mip_msip <= 1'b1;
            
            if(we_i && (addr_i == `CSR_mip)) begin
                csr_mip_meip <= write_value[11];
                csr_mip_mtip <= write_value[7];
                csr_mip_msip <= write_value[3];
            end
        end
    end


    //===== MEPC ============================================================
    reg     [31:1]  csr_mepc;       // Machine exception program counter
    wire    [31:0]  csr_mepc_readval = {csr_mepc, 1'b0};
    
    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_mepc <= 0;
        end
        else if(trap_caught_o) begin
            csr_mepc <= halted_i ? except_pc_i + 'b10 : except_pc_i;      // EPC = PC + 4 if core was halted using WFI, else PC
        end
        else if(we_i && (addr_i == `CSR_mepc)) begin
            csr_mepc <= write_value[31:1];
        end
    end
    assign trap_epc_o = csr_mepc;


    //===== MCAUSE ==========================================================
    reg             csr_mcause_intr;
    reg     [3:0]   csr_mcause_cause;    // Note: Actual size as per spec is 31 bits

    wire    [31:0]  csr_mcause_readval = {csr_mcause_intr, 27'd0, csr_mcause_cause};
    
    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_mcause_intr <= 0;
            csr_mcause_cause <= 0;
        end
        else if(trap_caught_o) begin
            csr_mcause_intr <= exception ? 1'b0: 1'b1; 
            csr_mcause_cause <= trap_cause;
        end
    end
    `endif // EN_EXCEPT


`ifdef EN_DEBUG
    //===== DCSR ============================================================
    // Debug Control and Status Register
    wire [3:0]      csr_dcsr_xdebugver = 4'd4;  // Debug spec version 0.13
    reg             csr_dcsr_ebreakm;           // Ebreak behaviour in machine mode (0: normal, 1: enter debug mode)
    wire            csr_dcsr_ebreaks = 1'b0;    // Ebreak behaviour in supervisor mode (0: normal, 1: enter debug mode)
    wire            csr_dcsr_ebreaku = 1'b0;    // Ebreak behaviour in user mode (0: normal, 1: enter debug mode)
    wire            csr_dcsr_setpie = 1'b0;     // Interrupts enabled/disabled during single stepping   (HARDWIRED to 0)
    reg             csr_dcsr_stopcount;         // Stop counter when in debug mode
    reg             csr_dcsr_stoptime;          // Stop timer when in debug mode
    reg  [2:0]      csr_dcsr_cause;             // Cause of entering debug mode (1: ebreak, 2: trigger, 3: haltreq, 4: step, 5: haltresetreq)
    wire            csr_dcsr_mprven = 1'b0;     // 1: ignore mprv in debug mode
    reg             csr_dcsr_nmip;              // Non-maskable interrupt pending                       (HARDWIRED to 0)
    reg             csr_dcsr_step;              // Single step mode
    wire [1:0]      csr_dcsr_prv = 2'b00;       // privelege level before entering debug mode           (HARDWIRED to 0)

    wire [31:0]     csr_dcsr_readval = {csr_dcsr_xdebugver, 12'd0, csr_dcsr_ebreakm, 1'b0, csr_dcsr_ebreaks, 
                                        csr_dcsr_ebreaku, csr_dcsr_setpie, csr_dcsr_stopcount, csr_dcsr_stoptime, 
                                        csr_dcsr_cause, 1'b0, csr_dcsr_mprven, csr_dcsr_nmip, csr_dcsr_step, csr_dcsr_prv};

    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_dcsr_ebreakm    <= 0;
            csr_dcsr_stopcount  <= 0;
            csr_dcsr_stoptime   <= 0;
            csr_dcsr_cause      <= 0;
            csr_dcsr_nmip       <= 0;
            csr_dcsr_step       <= 0;
        end
        else if(we_i && (addr_i == `CSR_dcsr)) begin
            csr_dcsr_ebreakm    <= write_value[15];
            csr_dcsr_stopcount  <= write_value[10];
            csr_dcsr_stoptime   <= write_value[9];
            csr_dcsr_nmip       <= write_value[3];
            csr_dcsr_step       <= write_value[2];
        end
    end

    //===== DPC =============================================================
    // Debug Program Counter
    reg [31:0]      csr_dpc;
    wire [31:0]     csr_dpc_readval = csr_dpc;

    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_dpc <= 0;
        end
        // TODO: implement
    end

    //===== DSCRATCH0 =======================================================
    // Debug Scratch Register 0
    reg [31:0]      csr_dscratch0;
    wire [31:0]     csr_dscratch0_readval = csr_dscratch0;

    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_dscratch0 <= 0;
        end
        else if(we_i && (addr_i == `CSR_dscratch0)) begin
            csr_dscratch0 <= write_value;
        end
    end

    //===== DSCRATCH1 =======================================================
    // Debug Scratch Register 1
    reg [31:0]      csr_dscratch1;
    wire [31:0]     csr_dscratch1_readval = csr_dscratch1;

    always @(posedge clk_i) begin
        if(rst_i) begin
            csr_dscratch1 <= 0;
        end
        else if(we_i && (addr_i == `CSR_dscratch1)) begin
            csr_dscratch1 <= write_value;
        end
    end

`endif // EN_DEBUG

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

            `ifdef EN_EXCEPT
            `CSR_mstatus:   read_value = csr_mstatus_readval;
            `CSR_mstatush:  read_value = csr_mstatush_readval;            
            `CSR_mtvec:     read_value = csr_mtvec_readval;
            `CSR_mie:       read_value = csr_mie_readval;
            `CSR_mip:       read_value = csr_mip_readval;
            `CSR_mepc:      read_value = csr_mepc_readval;
            `CSR_mcause:    read_value = csr_mcause_readval;
            `endif // EN_EXCEPT

            `ifdef EN_DEBUG
            `CSR_dcsr:      read_value = csr_dcsr_readval;
            `CSR_dpc:       read_value = csr_dpc_readval;
            `CSR_dscratch0: read_value = csr_dscratch0_readval;
            `CSR_dscratch1: read_value = csr_dscratch1_readval;
            `endif // EN_DEBUG

            default: begin
                // $display("RTL_ERR: invalid read to CSR addr 0x%x", addr_i);
                read_value = 32'hxxxx_xxxx;
            end
                
        endcase
    end

    assign data_o = read_value;

endmodule

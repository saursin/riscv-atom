///////////////////////////////////////////////////////////////////
//      ____  _________ _______    __         __                  
//     / __ \/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___   
//    / /_/ // / \__ \/ /    | | / /  / __ `/ __/ __ \/ __ `__ \  
//   / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /  
//  /_/ |_/___//____/\____/  |___/   \__,_/\__/\____/_/ /_/ /_/   
//                                                                
//  File        : AtomRV.v                                                        
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Atom is a 2-stage pipelined embedded class 32-bit 
//      RISCV core. It is based on RV32I ISA
///////////////////////////////////////////////////////////////////
`include "Defs.vh"
`include "Utils.vh"

`default_nettype none

`ifdef EN_EXCEPT
`ifndef RV_ZICSR
`error "Exception support requires CSR registers"
`endif
`endif



module AtomRV # (   
    parameter [31:0]    VEND_ID     = 32'h0000_0000,            // Vendor ID
    parameter [31:0]    ARCH_ID     = 32'h0000_0000,            // Architecture ID
    parameter [31:0]    IMPL_ID     = 32'h0000_0000,            // Implementation ID
    parameter [31:0]    HART_ID     = 32'h0000_0000             // Hart ID
)
(
    input   wire            clk_i,          // clock
    input   wire            rst_i,          // reset

    // ===== IPort =====
    output  wire    [31:0]  iport_addr_o,    // IPort Address
    input   wire    [31:0]  iport_data_i,    // IPort data
    output  wire            iport_valid_o,   // IPort Valid signal
    input   wire            iport_ack_i,     // IPort Acknowledge signal

    // ===== DPort =====
    output  wire    [31:0]  dport_addr_o,    // DPort address
    input   wire    [31:0]  dport_data_i,    // DPort data in
    output  reg     [31:0]  dport_data_o,    // DPort data out
    output  reg      [3:0]  dport_sel_o,     // DPort Select
    output  wire            dport_we_o,      // DPort Strobe
    output  wire            dport_valid_o,   // DPort Valid signal
    input   wire            dport_ack_i,     // DPort Ack signal

    input   wire    [31:0]  reset_vector_i

    `ifdef EN_EXCEPT
    // Interrupt Signals
    ,
    input   wire            irq_i,
    input   wire            timer_int_i
    `endif // EN_EXCEPT
);
    /*
        ///////////// Protocol specification //////////////
        CPU has a generic handshaking protocol interface (GHPI). Handshaking is done via means 
        of two signals 'valid' & 'ack'. Valid signal is set by master whenever a transaction 
        begins and slave responds by setting the 'ack' signal. When both signals are set, 
        transaction takes place. GHPI protocol also supports delayed transactions.

        CPU has two GHPI ports namely imem & dmem ports. imem port is used only for reading 
        the memory while dmem pot is used for both eads and writes. The CPU ca ne configured in 
        both Harvard and Von-Neumann fashion. In case of Harvard configuration, separate instruction 
        and data memory ae needed to be connected. In case of Von-Neumann mode, a bus arbiter is
        needed to multiplex between both ports.

        Reads:
        - Master sets the address, the valid signal and clears the strobe signal.
        - Slave responds by providing the data corresponding to that address & setting the ack signal.

        Writes:
        - Master sets the address, the data, the valid signal and the strobe signal (depending on 
            the write width).
        - Slave responds by setting the ack signal.
    */


    /*
        Jump decision:
        final jump decision signal, determines whether the jump will be taken
        sources of jump:
            - software (JAL/JALR)
            - hw (traps)
    */
    wire jump_decision = (d_jump_en & comparison_result) `INLINE_IFDEF(EN_EXCEPT, || csru_trap_caught_o, ); 


    ////////////////////////////////////////////////////////////////////
    // PIPELINE CONTROL
    /*
        Raw handshaking signal: These signals make the handshaking of ibus
        Fetch stage relies on the imem handshake signal to increment PC.

        We dont ever need to ignore the dmem handshake because ignoring dmem handshake only happens if 
        currently executing instruction happens to be a load-store instruction, but since currrent 
        instruction is a jump, there is no memory request made anyways.
    */
    wire raw_imem_handshake = (iport_valid_o && iport_ack_i);

    wire imem_handshake = raw_imem_handshake && !ignore_imem_handshake;
    wire dmem_handshake = (dport_valid_o && dport_ack_i);

    /*
        Definition of stall:
        Stall is a state of a pipeline stage in which the current instuction cannot popogate forward.

        Definition of flush:
        Flush is defined as insertion of nop in the pipeline in order to prevent an unwanted instuction 
        to execute.
    */

    /*
        Stall Stage2 in case it has made a memory request and the result has't arrived yet.
    */
    wire waiting_for_dbus_response = (!dmem_handshake && dport_valid_o);
    wire stall_stage2 = waiting_for_dbus_response;

    /*
        Stall Stage1 in case:
            - Stage1 is waiting for response of a memory request that is has made.
            - Stage2 is stalled, since the instruction in stage1 cant popogate to stage2. Therefore until
            the stage2 is stalled, instruction in stage1 is kept held.
    */
    wire waiting_for_ibus_response = (!imem_handshake && iport_valid_o);
    wire stall_stage1 = waiting_for_ibus_response || stall_stage2;

    /*
        Flush pipeline (insert nop in s2) in case:
            - there is a jump
            - Stage1 is stalled and stage is not, in this case stage can't recieve an new instruction to 
            execute therefore a bubble is introduced. 
            - 
    */
    wire flush_pipeline = jump_decision || (stall_stage2 ? 0 : stall_stage1);


    reg ignore_imem_handshake = 0;
    always @(posedge clk_i) begin
        if(rst_i)
            ignore_imem_handshake <= 0;
        else begin
            case(ignore_imem_handshake)
                0:  if(jump_decision)
                        ignore_imem_handshake <= 1;

                1:  if(raw_imem_handshake)
                        ignore_imem_handshake <= 0;
            endcase
        end
    end

    ////////////////////////////////////////////////////////////////////
    // Excepion Logic
    wire    except_illegal_instr;

    `ifdef EN_EXCEPT
    // Exception signals
    wire    except_instr_addr_misaligned = |ProgramCounter[1:0];
    wire    except_load_addr_misaligned = dport_valid_o & !dport_we_o & |dport_addr_o[1:0];
    wire    except_store_addr_misaligned = dport_valid_o & dport_we_o & |dport_addr_o[1:0];

    wire [31:0]     csru_trap_jump_addr_o;
    wire            csru_trap_caught_o;
    wire [31:1]     csru_trap_epc_o;
    `else
    `UNUSED_VAR(except_illegal_instr)
    `endif // EN_EXCEPT
    ////////////////////////////////////////////////////////////////////
    //  STAGE 1 - FETCH
    ////////////////////////////////////////////////////////////////////
    assign iport_valid_o = !rst_i;  // Always valid (Except on Reset condition)
    /*
        Program Counter
    */
    reg [31:0] ProgramCounter   /*verilator public*/;
    wire [31:0] pc_plus_four = ProgramCounter + 32'd4;

    always @(posedge clk_i) begin 
        if(rst_i)
            ProgramCounter <= reset_vector_i;

        `ifdef EN_EXCEPT
        else if(csru_trap_caught_o) begin
            ProgramCounter <= csru_trap_jump_addr_o;
        end
        `endif // EN_EXCEPT
        
        else if(jump_decision) begin
            `ifdef EN_EXCEPT
            if(csru_trap_caught_o)
                ProgramCounter <= csru_trap_jump_addr_o;
            else if(d_trap_ret)
                ProgramCounter <= {csru_trap_epc_o, 1'b0};
            else
            `endif // EN_EXCEPT
                ProgramCounter <= {alu_out[31:1], 1'b0};    // Only jump to 16 bit aligned addresses, also JALR enforces this
        end

        else if (!stall_stage1) begin
            ProgramCounter <= pc_plus_four;
        end
    end

    // Connect pc to imem address input
    assign iport_addr_o = ProgramCounter;


    `ifdef DPI_LOGGER
        initial begin
            dpi_logger_start();
        end
    `endif

    `ifdef LOG_RVATOM_JUMP
    always @(posedge clk_i) begin
        if(jump_decision)
            dpi_logger("Jump  address=0x%x\n", {alu_out[31:1], 1'b0});
    end
    `endif

    //----------------------------------------------------------
    // PIPELINE REGISTERS
    //----------------------------------------------------------
    /*
        This register is used to store old value of program counter
    */
    reg [31:0]  ProgramCounter_Old /* verilator public */;
    always @(posedge clk_i) begin 
        if(rst_i)
            ProgramCounter_Old <= 32'd0;
        else if(!stall_stage1)
            ProgramCounter_Old <= ProgramCounter;
    end

    /*
        This register is used to store old value of program counter + 4
    */
    reg [31:0]  link_address;
    always @(posedge clk_i) begin 
        if(rst_i)
            link_address <= 32'd0;
        else if(!stall_stage1)
            link_address <= pc_plus_four;
    end

    /*
        This register is used to store current instruction that is being 
        executed
    */
    reg [31:0] InstructionRegister  /*verilator public*/ = `RV_INSTR_NOP;
    always @(posedge clk_i) begin
        if(rst_i)
            InstructionRegister <= `RV_INSTR_NOP;
        else begin
            if(flush_pipeline)
                InstructionRegister <= `RV_INSTR_NOP;
                
            else if(!stall_stage1)
                InstructionRegister <= iport_data_i;
        end
    end



    ////////////////////////////////////////////////////////////////////
    //  STAGE 2 - DECODE & EXECUTE
    ////////////////////////////////////////////////////////////////////
    /*
        ////// Instruction Decode //////
        Instruction decode unit decodes instruction and sets various control 
        signals throughout the pipeline. Is also extracts immediate values 
        from instructions and sign extends them properly.
    */
    wire    [4:0]   d_rd_sel;
    wire    [4:0]   d_rs1_sel;
    wire    [4:0]   d_rs2_sel;
    wire    [31:0]  d_imm;

    wire            d_jump_en;
    wire    [2:0]   d_comparison_type;
    wire            d_rf_we;
    wire    [2:0]   d_rf_din_sel;
    wire            d_a_op_sel;
    wire            d_b_op_sel;
    wire            d_cmp_b_op_sel;
    wire    [2:0]   d_alu_op_sel;
    wire    [2:0]   d_mem_access_width;
    wire            d_mem_load_store;
    wire            d_mem_we;

    `ifdef RV_ZICSR
    wire    [2:0]   d_csru_op_sel;
    wire            d_csru_we;
    `endif

    `ifdef EN_EXCEPT
    wire            d_trap_ret;
    `endif // EN_EXCEPT


    Decode decode
    (
        .instr_i            (InstructionRegister),

        .illegal_instr_o    (except_illegal_instr),
        
        .rd_sel_o           (d_rd_sel),
        .rs1_sel_o          (d_rs1_sel),
        .rs2_sel_o          (d_rs2_sel),
        
        .imm_o              (d_imm),

        .jump_en_o          (d_jump_en),
        .comparison_type_o  (d_comparison_type),
        .rf_we_o            (d_rf_we),
        .rf_din_sel_o       (d_rf_din_sel),
        .a_op_sel_o         (d_a_op_sel),
        .b_op_sel_o         (d_b_op_sel),
        .cmp_b_op_sel_o     (d_cmp_b_op_sel),
        .alu_op_sel_o       (d_alu_op_sel),
        .mem_access_width_o (d_mem_access_width),
        .d_mem_load_store   (d_mem_load_store),
        .mem_we_o           (d_mem_we)
        
        `ifdef RV_ZICSR
        ,
        .csru_op_sel_o      (d_csru_op_sel),
        .csru_we_o          (d_csru_we)
        `endif

        `ifdef EN_EXCEPT
        ,
        .trap_ret_o         (d_trap_ret)
        `endif // EN_EXCEPT
    );



    /*
        ////// Register File //////
        Contains cpu registers (r0-31)
    */

    // RF_Din Multiplexer
    reg    [31:0]  rf_rd_data;
    always @(*) begin
        case(d_rf_din_sel)
            3'd0:   rf_rd_data = d_imm;
            3'd1:   rf_rd_data = link_address;
            3'd2:   rf_rd_data = alu_out;
            3'd3:   rf_rd_data = {31'd0, comparison_result};
            3'd4:   rf_rd_data = memload;
            `ifdef RV_ZICSR
            3'd5:   rf_rd_data = csru_data_o;
            `endif

            default: rf_rd_data = 32'd0;
        endcase
    end


    `ifdef RV_E
    localparam RF_INDX_BITS = 3;
    localparam RF_NREGS = 16;
    `else
    localparam RF_INDX_BITS = 4;
    localparam RF_NREGS = 32;
    `endif

    wire    [31:0]  rf_rs1;
    wire    [31:0]  rf_rs2;

    RegisterFile#(
        .REG_WIDTH(32), 
        .NUM_REGS(RF_NREGS),
        .R0_IS_ZERO(1)
    ) rf (
        .Clk_i      (clk_i),
        .Rst_i      (rst_i),
        .Ra_Sel_i   (d_rs1_sel[RF_INDX_BITS:0]),
        .Ra_o       (rf_rs1),
        .Rb_Sel_i   (d_rs2_sel[RF_INDX_BITS:0]),
        .Rb_o       (rf_rs2),
        .Data_We_i  (d_rf_we & !stall_stage2),
        .Rd_Sel_i   (d_rd_sel[RF_INDX_BITS:0]),
        .Data_i     (rf_rd_data)
    );

    `ifdef RV_E
        // We need these because we are not using MSB 
        // of select lines in RV_E
        `UNUSED_VAR(d_rs1_sel)
        `UNUSED_VAR(d_rs2_sel)
        `UNUSED_VAR(d_rd_sel)
    `endif


    /*
        ////// ALU //////
        Used for arithmetic and logical computations including shifts.
    */
    wire    [31:0]  alu_a_in = (d_a_op_sel) ? ProgramCounter_Old : rf_rs1;
    wire    [31:0]  alu_b_in = (d_b_op_sel) ? d_imm : rf_rs2;
    wire    [31:0]  alu_out;

    Alu alu
    (
        .a_i        (alu_a_in),
        .b_i        (alu_b_in),
        .sel_i      (d_alu_op_sel),
        .result_o   (alu_out)
    );



    /*
        ////// Comparator //////
        Used for all comparative operations
    */
    reg comparison_result;

    wire [31:0] cmp_A = rf_rs1;
    wire [31:0] cmp_B = (d_cmp_b_op_sel) ? d_imm : rf_rs2;
    wire signed  [31:0]  cmp_A_signed = cmp_A;
    wire signed  [31:0]  cmp_B_signed = cmp_B;

    always @(*) /* COMBINATORIAL*/    
    begin
        case(d_comparison_type)
            `CMP_FUNC_UN:   comparison_result = 1'b1;
            `CMP_FUNC_EQ:   comparison_result = (cmp_A == cmp_B);
            `CMP_FUNC_NQ:   comparison_result = (cmp_A != cmp_B);
            `CMP_FUNC_LT:   comparison_result = (cmp_A_signed < cmp_B_signed);
            `CMP_FUNC_GE:   comparison_result = (cmp_A_signed >= cmp_B_signed);
            `CMP_FUNC_LTU:  comparison_result = (cmp_A < cmp_B);
            `CMP_FUNC_GEU:  comparison_result = (cmp_A >= cmp_B);

            default:    comparison_result = 1'b0;
        endcase
    end

    `ifdef RV_ZICSR
    /*
        ////// CSR Unit //////
        Contains all the Control and status registers
    */
    wire    [11:0]  csru_addr_i = d_imm[11:0];
    wire    [31:0]  csru_data_o;

    // check if it is imm type CSR instruction and send data_i accordingly
    wire    [31:0]  csru_data_i = d_csru_op_sel[2] ? {{27{1'b0}}, d_rs1_sel} : rf_rs1;

    CSR_Unit#
    (
        .VEND_ID    (VEND_ID),
        .ARCH_ID    (ARCH_ID),
        .IMPL_ID    (IMPL_ID),
        .HART_ID    (HART_ID)
    ) csr_unit (
        // Global signals
        .clk_i   (clk_i),
        .rst_i   (rst_i),
        
        .instr_retired_i(!stall_stage1),

        `ifdef EN_EXCEPT
        .except_instr_addr_misaligned_i (except_instr_addr_misaligned),
        .except_illegal_instr_i         (except_illegal_instr),
        .except_load_addr_misaligned_i  (except_load_addr_misaligned),
        .except_store_addr_misaligned_i (except_store_addr_misaligned),
        .intrpt_external_i              (irq_i),
        .intrpt_timer_i                 (timer_int_i),            
        .intrpt_soft_i                  (1'b0),     // Triggered by ECALls (Not implemented)
        .except_pc_i                    (ProgramCounter_Old[31:1]),
        .trap_jump_addr_o               (csru_trap_jump_addr_o),
        .trap_caught_o                  (csru_trap_caught_o),
        .trap_epc_o                     (csru_trap_epc_o),  
        `endif // EN_EXCEPT

        // Signals for Reading from / Writing to CSRs
        .addr_i (csru_addr_i),
        .data_i (csru_data_i),
        .op_i   (d_csru_op_sel[1:0]),
        .we_i   (d_csru_we),
        .data_o (csru_data_o)
    );
    `endif


    /*
        DATA MEMORY ACCESS
    */
    wire [31:0] dmem_address = alu_out;
    wire [31:0] dport_data_out = rf_rs2;

    assign dport_addr_o = {dmem_address[31:2], {2{1'b0}}}; // word aligned accesses
    assign dport_valid_o = d_mem_load_store;
    assign dport_we_o = d_mem_we;// & !stall_stage2; IMPORTANT

    /////////////////////////////////
    // READ

    /*
        ////// MEM_LOAD //////
    */
    reg [31:0] memload;

    always @(*) /* COMBINATORIAL */
    begin
        case(d_mem_access_width[1:0])
            2'b00:  begin   // Load Byte
                        case(dmem_address[1:0])
                            2'b00:  memload = {{24{d_mem_access_width[2] ? 1'b0 : dport_data_i[7]}},  dport_data_i[7:0]};
                            2'b01:  memload = {{24{d_mem_access_width[2] ? 1'b0 : dport_data_i[15]}}, dport_data_i[15:8]};
                            2'b10:  memload = {{24{d_mem_access_width[2] ? 1'b0 : dport_data_i[23]}}, dport_data_i[23:16]};
                            2'b11:  memload = {{24{d_mem_access_width[2] ? 1'b0 : dport_data_i[31]}}, dport_data_i[31:24]};
                        endcase
                    end

            2'b01:  begin   // Load Half Word
                        case(dmem_address[1])
                            1'b0:  memload = {{16{d_mem_access_width[2] ? 1'b0 : dport_data_i[15]}}, dport_data_i[15:0]};
                            1'b1:  memload = {{16{d_mem_access_width[2] ? 1'b0 : dport_data_i[31]}}, dport_data_i[31:16]};
                        endcase
                    end
            
            2'b10:  begin   // Load Word
                        memload = dport_data_i;
                    end

            default: memload = 32'h00000000;
        endcase 
    end

    ////////////////////////////////
    // WRITE

    // Setting the sel_o signal
    always @(*) begin /* COMBINATORIAL */
        if (d_mem_we) begin
            case(d_mem_access_width[1:0])
                2'b00:  begin // Store byte
                            case(dmem_address[1:0])
                                2'b00:  dport_sel_o = 4'b0001;
                                2'b01:  dport_sel_o = 4'b0010;
                                2'b10:  dport_sel_o = 4'b0100;
                                2'b11:  dport_sel_o = 4'b1000;
                            endcase
                        end
                
                2'b01:  begin // Store Half Word
                            case(dmem_address[1])
                                1'b0:  dport_sel_o = 4'b0011;
                                1'b1:  dport_sel_o = 4'b1100;
                            endcase
                        end

                2'b10: dport_sel_o = 4'b1111;   // Store Word

                default: dport_sel_o = 4'b1111;
            endcase
        end
        else
            dport_sel_o = 4'b1111;   // Load (Byte/HWord/Word)
    end


    // Setting the data_o signal
    always @(*) begin /* COMBINATORIAL */
        if (d_mem_we) begin
            case(d_mem_access_width[1:0])
                2'b00:  begin // Store byte
                            case(dmem_address[1:0])
                                2'b00:  dport_data_o = { {24{1'b0}}, dport_data_out[7:0] };
                                2'b01:  dport_data_o = { {16{1'b0}}, dport_data_out[7:0], {8{1'b0}} };
                                2'b10:  dport_data_o = { {8{1'b0}} , dport_data_out[7:0], {16{1'b0}} };
                                2'b11:  dport_data_o = { dport_data_out[7:0], {24{1'b0}} };
                            endcase
                        end
                
                2'b01:  begin // Store Half Word
                            case(dmem_address[1])
                                1'b0:  dport_data_o = { {16{1'b0}}, dport_data_out[15:0] };
                                1'b1:  dport_data_o = { dport_data_out[15:0], {16{1'b0}} };
                            endcase
                        end

                2'b10: dport_data_o = dport_data_out;   // Store Word

                default: dport_data_o = dport_data_out;
            endcase
        end
        else
            dport_data_o = 32'h00000000;   // Load (Byte/HWord/Word)
    end

endmodule

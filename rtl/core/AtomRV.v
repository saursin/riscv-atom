///////////////////////////////////////////////////////////////////
//      ____  _________ _______    __         __                  
//     / __ \/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___   
//    / /_/ // / \__ \/ /    | | / /  / __ `/ __/ __ \/ __ `__ \  
//   / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /  
//  /_/ |_/___//____/\____/  |___/   \__,_/\__/\____/_/ /_/ /_/   
//                                                                
//  File       : AtomRV.v                                                        
//  Author     : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description: Nano-RISCV core is a non pipelined embedded class 
//               32 bit RISCV core. It is based on RV32I ISA
//  
///////////////////////////////////////////////////////////////////
`default_nettype none

`define __R0_IS_ZERO__

`include "../Timescale.vh"
`include "../Config.vh"

`include "Decode.v"
`include "RegisterFile.v"
`include "Alu.v"
`include "CSR_Unit.v"

module AtomRV
(
    // ========== General ==========
    input   wire            clk_i,          // clock
    input   wire            rst_i,          // reset


    // ========== IMEM Port ==========
    output  wire    [31:0]  imem_addr_o,    // IMEM Address
    input   wire    [31:0]  imem_data_i,    // IMEM data

    // Imem handshaking signals
    output  wire            imem_valid_o,   // IMEM Valid signal
    input   wire            imem_ack_i,     // IMEM Acknowledge signal


    // ========== DMEM Port ==========
    output  wire    [31:0]  dmem_addr_o,    // DMEM address
    input   wire    [31:0]  dmem_data_i,    // DMEM data in
    output  reg     [31:0]  dmem_data_o,    // DMEM data out
    output  reg      [3:0]  dmem_sel_o,     // DMEM Select
    output  wire            dmem_we_o,      // DMEM Strobe

    // Dmem handshaking signals
    output  wire            dmem_valid_o,   // DMEM Valid signal
    input   wire            dmem_ack_i      // DMEM Ack signal
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

wire jump_decision = d_jump_en & comparison_result; // final jump decision signal

////////////////////////////////////////////////////////////////////
// PIPELINE CONTROL
/*
    Raw handshaking signal: These signals make the handshaking of ibus
    Fetch stage relies on the imem handshake signal to increment PC.

    We dont ever need to ignore the dmem handshake because ignoring dmem handshake only happens if 
    currently executing instruction happens to be a load-store instruction, but since currrent 
    instruction is a jump, there is no memory request made anyways.
*/
wire raw_imem_handshake = (imem_valid_o && imem_ack_i);

wire imem_handshake = raw_imem_handshake && !ignore_imem_handshake;
wire dmem_handshake = (dmem_valid_o && dmem_ack_i);

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
wire waiting_for_dbus_response = (!dmem_handshake && dmem_valid_o);
wire stall_stage2 = waiting_for_dbus_response;

/*
    Stall Stage1 in case:
        - Stage1 is waiting for response of a memory request that is has made.
        - Stage2 is stalled, since the instruction in stage1 cant popogate to stage2. Therefore until
          the stage2 is stalled, instruction in stage1 is kept held.
*/
wire waiting_for_ibus_response = (!imem_handshake && imem_valid_o);
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
//  STAGE 1 - FETCH
////////////////////////////////////////////////////////////////////
assign imem_valid_o = !rst_i;  // Always valid (Except on Reset condition)
/*
    Program Counter
*/
reg [31:0] ProgramCounter   /*verilator public*/;
wire [31:0] pc_plus_four = ProgramCounter + 32'd4;

always @(posedge clk_i) begin 
    if(rst_i)
        ProgramCounter <= `RESET_PC_ADDRESS;

    else if(jump_decision)
        ProgramCounter <= {alu_out[31:1], 1'b0};    // Only jump to 16 bit aligned addresses, also JALR enforces this

    else if (!stall_stage1) begin
        ProgramCounter <= pc_plus_four;
    end
end

// Connect pc to imem address input
assign imem_addr_o = ProgramCounter;


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
reg [31:0] InstructionRegister  /*verilator public*/;
always @(posedge clk_i) begin
    if(rst_i)
        InstructionRegister <= `__NOP_INSTRUCTION__;
    else begin
        if(flush_pipeline)
            InstructionRegister <= `__NOP_INSTRUCTION__;
            
        else if(!stall_stage1)
            InstructionRegister <= imem_data_i;
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
wire    [2:0]   d_csru_op_sel;
wire            d_csru_we;


Decode decode
(
    .instr_i            (InstructionRegister),
    
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
    .mem_we_o           (d_mem_we),
    .csru_op_sel_o      (d_csru_op_sel),
    .csru_we_o          (d_csru_we)
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
        3'd5:   rf_rd_data = csru_data_o;

        default: rf_rd_data = 32'd0;
    endcase
end

wire    [31:0]  rf_rs1;
wire    [31:0]  rf_rs2;

RegisterFile  #(.REG_WIDTH(32), .REG_ADDR_WIDTH(5)) rf
(
    .Ra_Sel_i   (d_rs1_sel),
    .Ra_o       (rf_rs1),

    .Rb_Sel_i   (d_rs2_sel),
    .Rb_o       (rf_rs2),

    .Data_We_i  (d_rf_we & !stall_stage2),
    .Rd_Sel_i   (d_rd_sel),
    .Data_i     (rf_rd_data),

    .Clk_i      (clk_i),
    .Rst_i      (rst_i)
);


/*
    ////// ALU //////
    Used for arithmetic and logical computations including shifts.
*/
wire    [31:0]  alu_a_in = (d_a_op_sel) ? ProgramCounter_Old : rf_rs1;
wire    [31:0]  alu_b_in = (d_b_op_sel) ? d_imm : rf_rs2;
wire    [31:0]  alu_out;

Alu alu
(
    .A      (alu_a_in),
    .B      (alu_b_in),
    .Sel    (d_alu_op_sel),
    .Out    (alu_out)
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
        `__CMP_UN__   :   comparison_result = 1'b1;
        `__CMP_EQ__   :   comparison_result = (cmp_A == cmp_B);
        `__CMP_NQ__   :   comparison_result = (cmp_A != cmp_B);
        `__CMP_LT__   :   comparison_result = (cmp_A_signed < cmp_B_signed);
        `__CMP_GE__   :   comparison_result = (cmp_A_signed >= cmp_B_signed);
        `__CMP_LTU__  :   comparison_result = (cmp_A < cmp_B);
        `__CMP_GEU__  :   comparison_result = (cmp_A >= cmp_B);

        default:    comparison_result = 1'b0;
    endcase
end


/*
    ////// CSR Unit //////
    Contains all the Control and status registers
*/
wire    [11:0]  csru_addr_i = d_imm[11:0];
wire    [31:0]  csru_data_o;

// check if it is imm type CSR instruction and send data_i accordingly
wire    [31:0]  csru_data_i = d_csru_op_sel[2] ? {{27{1'b0}}, d_rs1_sel} : rf_rs1;

CSR_Unit csr_unit
(
    // Global signals
    .clk_i   (clk_i),
    .rst_i   (rst_i),

    // Signals for Reading from / Writing to CSRs
    .addr_i (csru_addr_i),
    .data_i (csru_data_i),
    .op_i   (d_csru_op_sel[1:0]),
    .we_i   (d_csru_we),
    .data_o (csru_data_o)
);


/*
    DATA MEMORY ACCESS
*/
wire [31:0] dmem_address = alu_out;
wire [31:0] dmem_data_out = rf_rs2;

assign dmem_addr_o = {dmem_address[31:2], {2{1'b0}}}; // word aligned accesses
assign dmem_valid_o = d_mem_load_store;
assign dmem_we_o = d_mem_we;// & !stall_stage2; IMPORTANT

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
                        2'b00:  memload = {{24{d_mem_access_width[2] ? 1'b0 : dmem_data_i[7]}},  dmem_data_i[7:0]};
                        2'b01:  memload = {{24{d_mem_access_width[2] ? 1'b0 : dmem_data_i[15]}}, dmem_data_i[15:8]};
                        2'b10:  memload = {{24{d_mem_access_width[2] ? 1'b0 : dmem_data_i[23]}}, dmem_data_i[23:16]};
                        2'b11:  memload = {{24{d_mem_access_width[2] ? 1'b0 : dmem_data_i[31]}}, dmem_data_i[31:24]};
                    endcase
                end

        2'b01:  begin   // Load Half Word
                    case(dmem_address[1])
                        1'b0:  memload = {{16{d_mem_access_width[2] ? 1'b0 : dmem_data_i[15]}}, dmem_data_i[15:0]};
                        1'b1:  memload = {{16{d_mem_access_width[2] ? 1'b0 : dmem_data_i[31]}}, dmem_data_i[31:16]};
                    endcase
                end
        
        2'b10:  begin   // Load Word
                    memload = dmem_data_i;
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
                            2'b00:  dmem_sel_o = 4'b0001;
                            2'b01:  dmem_sel_o = 4'b0010;
                            2'b10:  dmem_sel_o = 4'b0100;
                            2'b11:  dmem_sel_o = 4'b1000;
                        endcase
                    end
            
            2'b01:  begin // Store Half Word
                        case(dmem_address[1])
                            1'b0:  dmem_sel_o = 4'b0011;
                            1'b1:  dmem_sel_o = 4'b1100;
                        endcase
                    end

            2'b10: dmem_sel_o = 4'b1111;   // Store Word

            default: dmem_sel_o = 4'b1111;
        endcase
    end
    else
        dmem_sel_o = 4'b1111;   // Load (Byte/HWord/Word)
end


// Setting the data_o signal
always @(*) begin /* COMBINATORIAL */
    if (d_mem_we) begin
        case(d_mem_access_width[1:0])
            2'b00:  begin // Store byte
                        case(dmem_address[1:0])
                            2'b00:  dmem_data_o = { {24{1'b0}}, dmem_data_out[7:0] };
                            2'b01:  dmem_data_o = { {16{1'b0}}, dmem_data_out[7:0], {8{1'b0}} };
                            2'b10:  dmem_data_o = { {8{1'b0}} , dmem_data_out[7:0], {16{1'b0}} };
                            2'b11:  dmem_data_o = { dmem_data_out[7:0], {24{1'b0}} };
                        endcase
                    end
            
            2'b01:  begin // Store Half Word
                        case(dmem_address[1])
                            1'b0:  dmem_data_o = { {16{1'b0}}, dmem_data_out[15:0] };
                            1'b1:  dmem_data_o = { dmem_data_out[15:0], {16{1'b0}} };
                        endcase
                    end

            2'b10: dmem_data_o = dmem_data_out;   // Store Word

            default: dmem_data_o = dmem_data_out;
        endcase
    end
    else
        dmem_data_o = 32'h00000000;   // Load (Byte/HWord/Word)
end

endmodule
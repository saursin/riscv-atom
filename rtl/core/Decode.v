////////////////////////////////////////////////////////////////////   
//  RISC-V Atom 
//
//  File        : Decode.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Desciption  : Instruction Decoder for RISCV atom core
////////////////////////////////////////////////////////////////////
`default_nettype none

`include "../Timescale.vh"

module Decode
(
    input   wire    [31:0]  instr_i,

    output  wire    [4:0]   rd_sel_o,
    output  wire    [4:0]   rs1_sel_o,
    output  wire    [4:0]   rs2_sel_o,

    output  wire    [31:0]  imm_o,

    output  reg             jump_en_o,
    output  reg     [2:0]   comparison_type_o,
    output  reg             rf_we_o,
    output  reg     [2:0]   rf_din_sel_o,
    output  reg             a_op_sel_o,
    output  reg             b_op_sel_o,
    output  reg             cmp_b_op_sel_o,
    output  reg     [2:0]   alu_op_sel_o,
    output  wire    [2:0]   mem_access_width_o,
    output  reg             d_mem_load_store,
    output  reg             mem_we_o
);

// Decode fields
wire    [6:0]   opcode  = instr_i[6:0];
wire    [2:0]   func3   = instr_i[14:12];
wire    [6:0]   func7   = instr_i[31:25];

assign mem_access_width_o = func3;

assign  rd_sel_o    = instr_i[11:7];
assign  rs1_sel_o   = instr_i[19:15];
assign  rs2_sel_o   = instr_i[24:20];

reg    [2:0] imm_format;

/*
    Decode Immidiate
*/
reg [31:0] getExtImm;

always @(*) /*COMBINATIONAL*/ 
begin
    case(imm_format)
            `__I_IMMIDIATE__    :   getExtImm = {{21{instr_i[31]}}, instr_i[30:25], instr_i[24:21], instr_i[20]};
            `__S_IMMIDIATE__    :   getExtImm = {{21{instr_i[31]}}, instr_i[30:25], instr_i[11:8], instr_i[7]};
            `__B_IMMIDIATE__    :   getExtImm = {{20{instr_i[31]}}, instr_i[7], instr_i[30:25], instr_i[11:8], 1'b0};
            `__U_IMMIDIATE__    :   getExtImm = {instr_i[31], instr_i[30:20], instr_i[19:12], {12{1'b0}}};
            `__J_IMMIDIATE__    :   getExtImm = {{12{instr_i[31]}}, instr_i[19:12], instr_i[20], instr_i[30:25], instr_i[24:21], 1'b0};

            default:
                getExtImm = 32'd0;
    endcase
end

assign imm_o = getExtImm;


always @(*) begin
    // DEFAULT VALUES
    jump_en_o = 1'b0;
    comparison_type_o = `__CMP_UN__;
    rf_we_o = 1'b0;
    rf_din_sel_o = 3'd0;
    a_op_sel_o = 1'b0;
    b_op_sel_o = 1'b0;
    cmp_b_op_sel_o = 1'b0;
    alu_op_sel_o = `__ALU_ADD__;
    mem_we_o = 1'b0;
    d_mem_load_store = 1'b0;
    imm_format = `__U_IMMIDIATE__;


    casez({func7, func3, opcode})
        
        /* LUI   */ 
        17'b???????_???_0110111: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd0;
            imm_format = `__U_IMMIDIATE__;
        end

        /* AUIPC */ 
        17'b???????_???_0010111: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b1;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_ADD__;
            imm_format = `__U_IMMIDIATE__;
        end

        /* JAL   */ 
        17'b???????_???_1101111: 
        begin
            jump_en_o = 1'b1;
            comparison_type_o = `__CMP_UN__;
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd1;
            a_op_sel_o = 1'b1;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_ADD__;
            imm_format = `__J_IMMIDIATE__;
        end

        /* JALR  */ 
        17'b???????_000_1100111: 
        begin
            jump_en_o = 1'b1;
            comparison_type_o = `__CMP_UN__;
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd1;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_ADD__;
            imm_format = `__I_IMMIDIATE__;
        end

        /* BEQ   */ 
        17'b???????_000_1100011: begin
            jump_en_o = 1'b1;
            comparison_type_o = `__CMP_EQ__;
            a_op_sel_o = 1'b1;
            b_op_sel_o = 1'b1;
            cmp_b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_ADD__;
            imm_format = `__B_IMMIDIATE__;
        end

        /* BNE   */ 
        17'b???????_001_1100011: 
        begin
            jump_en_o = 1'b1;
            comparison_type_o = `__CMP_NQ__;
            a_op_sel_o = 1'b1;
            b_op_sel_o = 1'b1;
            cmp_b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_ADD__;
            imm_format = `__B_IMMIDIATE__;
        end

        /* BLT   */ 
        17'b???????_100_1100011:
        begin
            jump_en_o = 1'b1;
            comparison_type_o = `__CMP_LT__;
            a_op_sel_o = 1'b1;
            b_op_sel_o = 1'b1;
            cmp_b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_ADD__;
            imm_format = `__B_IMMIDIATE__;
        end

        /* BGE   */ 
        17'b???????_101_1100011: 
        begin
            jump_en_o = 1'b1;
            comparison_type_o = `__CMP_GE__;
            a_op_sel_o = 1'b1;
            b_op_sel_o = 1'b1;
            cmp_b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_ADD__;
            imm_format = `__B_IMMIDIATE__;
        end

        /* BLTU  */ 
        17'b???????_110_1100011: 
        begin
            jump_en_o = 1'b1;
            comparison_type_o = `__CMP_LTU__;
            a_op_sel_o = 1'b1;
            b_op_sel_o = 1'b1;
            cmp_b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_ADD__;
            imm_format = `__B_IMMIDIATE__;
        end

        /* BGEU  */ 
        17'b???????_111_1100011: 
        begin
            jump_en_o = 1'b1;
            comparison_type_o = `__CMP_GEU__;
            a_op_sel_o = 1'b1;
            b_op_sel_o = 1'b1;
            cmp_b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_ADD__;
            imm_format = `__B_IMMIDIATE__;
        end

        /* LB, LH, LW, LBU, LHU */ 
        17'b???????_???_0000011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd4;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_ADD__;
            mem_we_o = 1'b0;
            d_mem_load_store = 1'b1;
            imm_format = `__I_IMMIDIATE__;
        end

        /* SB, SH, SW */ 
        17'b???????_???_0100011: 
        begin
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_ADD__;
            mem_we_o = 1'b1;
            d_mem_load_store = 1'b1;
            imm_format = `__S_IMMIDIATE__;
        end

        /* ADDI  */
        17'b???????_000_0010011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_ADD__;
            imm_format = `__I_IMMIDIATE__;
        end

        /* SLTI  */
        17'b???????_010_0010011: 
        begin
            comparison_type_o = `__CMP_LT__;
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd3;
            cmp_b_op_sel_o = 1'b1;
            imm_format = `__I_IMMIDIATE__;
        end

        /* SLTIU */ 
        17'b???????_011_0010011: 
        begin
            comparison_type_o = `__CMP_LTU__;
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd3;
            cmp_b_op_sel_o = 1'b1;
            imm_format = `__I_IMMIDIATE__;
        end

        /* XORI  */ 
        17'b???????_100_0010011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_XOR__;
            imm_format = `__I_IMMIDIATE__;
        end

        /* ORI   */
        17'b???????_110_0010011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_OR__;
            imm_format = `__I_IMMIDIATE__;
        end

        /* ANDI  */
        17'b???????_111_0010011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_AND__;
            imm_format = `__I_IMMIDIATE__;
        end

        /* SLLI  */
        17'b0000000_001_0010011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_SLL__;
            imm_format = `__I_IMMIDIATE__;
        end

        /* SRLI  */
        17'b0000000_101_0010011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_SRL__;
            imm_format = `__I_IMMIDIATE__;
        end

        /* SRAI  */
        17'b0100000_101_0010011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b1;
            alu_op_sel_o = `__ALU_SRA__;
            imm_format = `__I_IMMIDIATE__;
        end

        /* ADD   */ 
        17'b0000000_000_0110011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_ADD__;
        end

        /* SUB   */
        17'b0100000_000_0110011:
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_SUB__;
        end

        /* SLL   */ 
        17'b0000000_001_0110011:
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_SLL__;
        end

        /* SLT */ 
        17'b0000000_010_0110011:
        begin
            comparison_type_o = `__CMP_LT__;
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd3;
            cmp_b_op_sel_o = 1'b0;
        end


        /* SLTU  */ 
        17'b0000000_011_0110011:
        begin
            comparison_type_o = `__CMP_LTU__;
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd3;
            cmp_b_op_sel_o = 1'b0;
        end

        /* XOR   */ 
        17'b0000000_100_0110011:
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_XOR__;
        end

        /* SRL   */ 
        17'b0000000_101_0110011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_SRL__;
        end

        /* SRA   */
        17'b0100000_101_0110011:
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_SRA__;
        end

        /* OR    */ 
        17'b0000000_110_0110011:begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_OR__;
        end

        /* AND   */ 
        17'b0000000_111_0110011: 
        begin
            rf_we_o = 1'b1;
            rf_din_sel_o = 3'd2;
            a_op_sel_o = 1'b0;
            b_op_sel_o = 1'b0;
            alu_op_sel_o = `__ALU_AND__;
        end

        default: begin
            jump_en_o = 0;
            comparison_type_o = `__CMP_UN__;
            rf_we_o = 0;
            rf_din_sel_o = 0;
            a_op_sel_o = 0;
            b_op_sel_o = 0;
            cmp_b_op_sel_o = 0;
            alu_op_sel_o = 0;
            mem_we_o = 1'b0;
            imm_format = 0;
        end            

    endcase
end

endmodule
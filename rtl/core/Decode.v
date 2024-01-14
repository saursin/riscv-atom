////////////////////////////////////////////////////////////////////   
//  File        : Decode.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Instruction Decoder for RISCV atom core
////////////////////////////////////////////////////////////////////
`default_nettype none

`include "Utils.vh"
`include "Defs.vh"

module Decode
(
    input   wire    [31:0]  instr_i,

    output  reg             illegal_instr_o,

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
    output  reg     [3:0]   alu_op_sel_o,
    output  wire    [2:0]   mem_access_width_o,
    output  reg             d_mem_load_store,
    output  reg             mem_we_o
    
    `ifdef EN_RVZICSR
    ,
    output  wire    [2:0]   csru_op_sel_o,
    output  reg             csru_we_o
    `endif // EN_RVZICSR

    `ifdef EN_EXCEPT
    ,
    output  reg             trap_ret_o
    `endif // EN_EXCEPT
);

    // Decode fields
    wire    [6:0]   opcode  = instr_i[6:0];
    wire    [2:0]   func3   = instr_i[14:12];
    wire    [6:0]   func7   = instr_i[31:25];

    assign mem_access_width_o = func3;

    `ifdef EN_RVZICSR
    assign csru_op_sel_o = func3;
    `endif // EN_RVZICSR


    assign  rd_sel_o    = instr_i[11:7];
    assign  rs1_sel_o   = instr_i[19:15];
    assign  rs2_sel_o   = instr_i[24:20];

    reg    [2:0] imm_format;

    /*
        Decode Immediate
    */
    reg [31:0] getExtImm;

    always @(*) /*COMBINATORIAL*/ 
    begin
        case(imm_format)
                `RV_IMM_TYPE_I    :   getExtImm = {{21{instr_i[31]}}, instr_i[30:25], instr_i[24:21], instr_i[20]};
                `RV_IMM_TYPE_S    :   getExtImm = {{21{instr_i[31]}}, instr_i[30:25], instr_i[11:8], instr_i[7]};
                `RV_IMM_TYPE_B    :   getExtImm = {{20{instr_i[31]}}, instr_i[7], instr_i[30:25], instr_i[11:8], 1'b0};
                `RV_IMM_TYPE_U    :   getExtImm = {instr_i[31], instr_i[30:20], instr_i[19:12], {12{1'b0}}};
                `RV_IMM_TYPE_J    :   getExtImm = {{12{instr_i[31]}}, instr_i[19:12], instr_i[20], instr_i[30:25], instr_i[24:21], 1'b0};

                default:
                    getExtImm = 32'd0;
        endcase
    end

    assign imm_o = getExtImm;


    reg [63:0] instr_scope; // just for simulation, will be optimised away in synth
    `UNUSED_VAR(instr_scope)


    always @(*) begin
        // DEFAULT VALUES
        instr_scope = "???";
        illegal_instr_o = 1'b0;
        jump_en_o = 1'b0;
        comparison_type_o = `CMP_FUNC_UN;
        rf_we_o = 1'b0;
        rf_din_sel_o = 3'd0;
        a_op_sel_o = 1'b0;
        b_op_sel_o = 1'b0;
        cmp_b_op_sel_o = 1'b0;
        alu_op_sel_o = `ALU_FUNC_ADD;
        mem_we_o = 1'b0;
        d_mem_load_store = 1'b0;
        imm_format = `RV_IMM_TYPE_U;

        `ifdef EN_RVZICSR
        csru_we_o = 0;
        `endif // EN_RVZICSR

        `ifdef EN_EXCEPT
        trap_ret_o = 1'b0;
        `endif // EN_EXCEPT


        casez({func7, func3, opcode})
            
            /* LUI   */ 
            17'b???????_???_0110111: 
            begin
                instr_scope = "LUI";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd0;
                imm_format = `RV_IMM_TYPE_U;
            end

            /* AUIPC */ 
            17'b???????_???_0010111: 
            begin
                instr_scope = "AUIPC";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b1;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_ADD;
                imm_format = `RV_IMM_TYPE_U;
            end

            /* JAL   */ 
            17'b???????_???_1101111: 
            begin
                instr_scope = "JAL";
                jump_en_o = 1'b1;
                comparison_type_o = `CMP_FUNC_UN;
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd1;
                a_op_sel_o = 1'b1;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_ADD;
                imm_format = `RV_IMM_TYPE_J;
            end

            /* JALR  */ 
            17'b???????_000_1100111: 
            begin
                instr_scope = (rd_sel_o == 0 && rs1_sel_o == 1 && imm_o == 0) ? "RET" : "JALR";
                jump_en_o = 1'b1;
                comparison_type_o = `CMP_FUNC_UN;
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd1;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_ADD;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* BEQ   */ 
            17'b???????_000_1100011: begin
                instr_scope = "BEQ";
                jump_en_o = 1'b1;
                comparison_type_o = `CMP_FUNC_EQ;
                a_op_sel_o = 1'b1;
                b_op_sel_o = 1'b1;
                cmp_b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_ADD;
                imm_format = `RV_IMM_TYPE_B;
            end

            /* BNE   */ 
            17'b???????_001_1100011: 
            begin
                instr_scope = "BNE";
                jump_en_o = 1'b1;
                comparison_type_o = `CMP_FUNC_NQ;
                a_op_sel_o = 1'b1;
                b_op_sel_o = 1'b1;
                cmp_b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_ADD;
                imm_format = `RV_IMM_TYPE_B;
            end

            /* BLT   */ 
            17'b???????_100_1100011:
            begin
                instr_scope = "BLT";
                jump_en_o = 1'b1;
                comparison_type_o = `CMP_FUNC_LT;
                a_op_sel_o = 1'b1;
                b_op_sel_o = 1'b1;
                cmp_b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_ADD;
                imm_format = `RV_IMM_TYPE_B;
            end

            /* BGE   */ 
            17'b???????_101_1100011: 
            begin
                instr_scope = "BGE";
                jump_en_o = 1'b1;
                comparison_type_o = `CMP_FUNC_GE;
                a_op_sel_o = 1'b1;
                b_op_sel_o = 1'b1;
                cmp_b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_ADD;
                imm_format = `RV_IMM_TYPE_B;
            end

            /* BLTU  */ 
            17'b???????_110_1100011: 
            begin
                instr_scope = "BLTU";
                jump_en_o = 1'b1;
                comparison_type_o = `CMP_FUNC_LTU;
                a_op_sel_o = 1'b1;
                b_op_sel_o = 1'b1;
                cmp_b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_ADD;
                imm_format = `RV_IMM_TYPE_B;
            end

            /* BGEU  */ 
            17'b???????_111_1100011: 
            begin
                instr_scope = "BGEU";
                jump_en_o = 1'b1;
                comparison_type_o = `CMP_FUNC_GEU;
                a_op_sel_o = 1'b1;
                b_op_sel_o = 1'b1;
                cmp_b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_ADD;
                imm_format = `RV_IMM_TYPE_B;
            end

            /* LB, LH, LW, LBU, LHU */ 
            17'b???????_???_0000011: 
            begin
                case(func3)
                    3'b000: instr_scope = "LB"; 
                    3'b001: instr_scope = "LH"; 
                    3'b010: instr_scope = "LW"; 
                    3'b100: instr_scope = "LBU";
                    3'b101: instr_scope = "LHU";
                    default: begin
                        instr_scope = "LOAD?";
                        illegal_instr_o = 1'b1;
                    end
                endcase
                
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd4;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_ADD;
                mem_we_o = 1'b0;
                d_mem_load_store = 1'b1;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* SB, SH, SW */ 
            17'b???????_???_0100011: 
            begin
                case(func3)
                    3'b000: instr_scope = "SB"; 
                    3'b001: instr_scope = "SH"; 
                    3'b010: instr_scope = "SW"; 
                    default: begin
                        instr_scope = "STORE?";
                        illegal_instr_o = 1'b1;
                    end
                endcase

                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_ADD;
                mem_we_o = 1'b1;
                d_mem_load_store = 1'b1;
                imm_format = `RV_IMM_TYPE_S;
            end

            /* ADDI  */
            17'b???????_000_0010011: 
            begin
                instr_scope = (instr_i == 32'h00000013)? "NOP" : "ADDI";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_ADD;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* SLTI  */
            17'b???????_010_0010011: 
            begin
                instr_scope = "SLTI";
                comparison_type_o = `CMP_FUNC_LT;
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd3;
                cmp_b_op_sel_o = 1'b1;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* SLTIU */ 
            17'b???????_011_0010011: 
            begin
                instr_scope = "SLTIU";
                comparison_type_o = `CMP_FUNC_LTU;
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd3;
                cmp_b_op_sel_o = 1'b1;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* XORI  */ 
            17'b???????_100_0010011: 
            begin
                instr_scope = "XORI";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_XOR;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* ORI   */
            17'b???????_110_0010011: 
            begin
                instr_scope = "ORI";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_OR;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* ANDI  */
            17'b???????_111_0010011: 
            begin
                instr_scope = "ANDI";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_AND;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* SLLI  */
            17'b0000000_001_0010011: 
            begin
                instr_scope = "SLLI";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_SLL;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* SRLI  */
            17'b0000000_101_0010011: 
            begin
                instr_scope = "SRLI";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_SRL;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* SRAI  */
            17'b0100000_101_0010011: 
            begin
                instr_scope = "SRAI";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b1;
                alu_op_sel_o = `ALU_FUNC_SRA;
                imm_format = `RV_IMM_TYPE_I;
            end

            /* ADD   */ 
            17'b0000000_000_0110011: 
            begin
                instr_scope = "ADD";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_ADD;
            end

            /* SUB   */
            17'b0100000_000_0110011:
            begin
                instr_scope = "SUB";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_SUB;
            end

            /* SLL   */ 
            17'b0000000_001_0110011:
            begin
                instr_scope = "SLL";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_SLL;
            end

            /* SLT */ 
            17'b0000000_010_0110011:
            begin
                instr_scope = "SLT";
                comparison_type_o = `CMP_FUNC_LT;
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd3;
                cmp_b_op_sel_o = 1'b0;
            end


            /* SLTU  */ 
            17'b0000000_011_0110011:
            begin
                instr_scope = "SLTU";
                comparison_type_o = `CMP_FUNC_LTU;
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd3;
                cmp_b_op_sel_o = 1'b0;
            end

            /* XOR   */ 
            17'b0000000_100_0110011:
            begin
                instr_scope = "XOR";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_XOR;
            end

            /* SRL   */ 
            17'b0000000_101_0110011: 
            begin
                instr_scope = "SRL";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_SRL;
            end

            /* SRA   */
            17'b0100000_101_0110011:
            begin
                instr_scope = "SRA";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_SRA;
            end

            /* OR    */ 
            17'b0000000_110_0110011:
            begin
                instr_scope = "OR";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_OR;
            end

            /* AND   */ 
            17'b0000000_111_0110011: 
            begin
                instr_scope = "AND";
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_AND;
            end

            /* MUL      */
            17'b0000001_000_0110011:
            begin
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_MUL;
            end

            /* MULH     */
            17'b0000001_001_0110011:
            begin
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_MULH;
            end

            /* MULHSU    */
            17'b0000001_010_0110011:
            begin
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_MULHSU;
            end
            /* MULHU     */
            17'b0000001_011_0110011:
            begin
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_MULHU;
            end

            /* DIV      */
            17'b0000001_100_0110011:
            begin
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_DIV;
            end

            /* DIVU     */
            17'b0000001_101_0110011:
            begin
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_DIVU;
            end

            /* REM      */
            17'b0000001_110_0110011:
            begin
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_REM;
            end

            /* REMU     */
            17'b0000001_111_0110011:
            begin
                rf_we_o = 1'b1;
                rf_din_sel_o = 3'd2;
                a_op_sel_o = 1'b0;
                b_op_sel_o = 1'b0;
                alu_op_sel_o = `ALU_FUNC_REMU;
            end

            /* OPCODE: SYSTEM */
            17'b???????_???_1110011:
            begin
                if(rd_sel_o == 5'b00000 && func3 == 3'b000 && rs1_sel_o == 5'b00000 && instr_i[11:0] == 12'b000000000000) /* ECALL */ begin
                    instr_scope = "ECALL";
                    illegal_instr_o = 1'b1; // Not supported
                end
                else if(rd_sel_o == 5'b00000 && func3 == 3'b000 && rs1_sel_o == 5'b00000 && instr_i[11:0] == 12'b000000000001) /* EBREAK */ begin
                    instr_scope = "EBREAK";
                    illegal_instr_o = 1'b1; // Not supported
                end
                else if(rd_sel_o == 5'b00000 && func3 == 3'b000 && rs1_sel_o == 5'b00000 && rs2_sel_o == 5'b00010 && func7 == 7'b0011000) /* MRET */ begin
                    instr_scope = "MRET";
                    `ifdef EN_EXCEPT
                    jump_en_o = 1'b1;
                    comparison_type_o = `CMP_FUNC_UN;
                    trap_ret_o = 1'b1;
                    `else
                    illegal_instr_o = 1'b1; // Not supported
                    `endif // EN_EXCEPT

                end
                else if(rd_sel_o == 5'b00000 && func3 == 3'b000 && rs1_sel_o == 5'b00000 && rs2_sel_o == 5'b00101 && func7 == 7'b0001000) /* WFI */ begin
                    instr_scope = "WFI";
                    illegal_instr_o = 1'b1; // Not supported
                end
                else begin
                    `ifdef EN_RVZICSR
                        /* CSR Instructions */
                        case(func3)
                            3'b001: instr_scope = "CSRRW";
                            3'b010: instr_scope = "CSRRS";
                            3'b011: instr_scope = "CSRRC";
                            3'b101: instr_scope = "CSRRWI";
                            3'b110: instr_scope = "CSRRSI";
                            3'b111: instr_scope = "CSRRCI";
                            default: begin
                                instr_scope = "CSR?";
                                illegal_instr_o = 1'b1;
                            end
                        endcase
                        rf_we_o = (rd_sel_o!=0);   // CSR Reads should not take place if rs1 == x0
                        rf_din_sel_o = 3'd5;
                        csru_we_o = 1;
                        imm_format = `RV_IMM_TYPE_I;
                    `else
                        illegal_instr_o = 1'b1;
                    `endif // EN_RVZICSR
                end
            end

            default: begin
                instr_scope = "???";
                jump_en_o = 0;
                comparison_type_o = `CMP_FUNC_UN;
                rf_we_o = 0;
                rf_din_sel_o = 0;
                a_op_sel_o = 0;
                b_op_sel_o = 0;
                cmp_b_op_sel_o = 0;
                alu_op_sel_o = 0;
                mem_we_o = 1'b0;
                imm_format = 0;

                `ifdef EN_RVZICSR
                csru_we_o = 0;
                `endif // EN_RVZICSR

                `ifdef EN_EXCEPT
                trap_ret_o = 0;
                `endif // EN_EXCEPT

                illegal_instr_o = 1'b1;
                `debug($display("RTL-ERROR: Illegal Instruction (insn: %x, Opcode: %b)", instr_i, opcode);)
            end

        endcase
    end

endmodule

`default_nettype none

module RVC_Decoder(
    /* verilator lint_off UNUSEDSIGNAL */
    input   wire            clk_i,
    /* verilator lint_on UNUSEDSIGNAL */
    input   wire    [31:0]  instr_i,
    input   wire            ack_i,
    output  wire    [31:0]  instr_o,
    output  wire            is_compressed
);
    localparam OPCODE_LOAD     = 7'h03;
    localparam OPCODE_OP_IMM   = 7'h13;
    localparam OPCODE_STORE    = 7'h23;
    localparam OPCODE_OP       = 7'h33;
    localparam OPCODE_LUI      = 7'h37;
    localparam OPCODE_BRANCH   = 7'h63;
    localparam OPCODE_JALR     = 7'h67;
    localparam OPCODE_JAL      = 7'h6f;

    wire [1:0]  op = instr_i[1:0];
    // wire [3:0]  func4 = instr_i[15:12];
    wire [2:0]  func3 = instr_i[15:13];
    // wire [5:0]  func6 = instr_i[15:10];
    // wire [4:0]  rdrs1 = instr_i[11:7];
    // wire [4:0]  rs2 = instr_i[6:2];
    // wire [2:0]  rd_c = instr_i[4:2];
    // wire [2:0]  rs2_c = instr_i[4:2];
    // wire [2:0]  rs1_c = instr_i[4:2];

    reg [127:0] instr_scope;         // just for simulation, will be optimised away in synth
    `UNUSED_VAR(instr_scope)

    reg [31:0] decompr_instr;       // Decompressed instruction
    reg is_illegal;

    assign instr_o = is_illegal ? instr_i : decompr_instr;
    assign is_compressed = ack_i & !is_illegal;

    // always @(posedge clk_i) begin
    //     if(ack_i) begin
    //         is_compressed <= !is_illegal; 
    //     end
    // end
    
    always @(*) /* COMBINATORIAL */ begin
        decompr_instr = instr_i;
        is_illegal = 1'b0;
        instr_scope = "C.???";

        case({op, func3})
            // ========== Quadrant-0 ==========
            5'b00_000: begin
                // c.addi4spn -> addi rd', x2, imm
                decompr_instr = {2'b0, instr_i[10:7], instr_i[12:11], instr_i[5],
                            instr_i[6], 2'b00, 5'h02, 3'b000, 2'b01, instr_i[4:2], {OPCODE_OP_IMM}};
                instr_scope = "C.ADDI4SPN";
            end
            
            5'b00_010: begin
                // c.lw -> lw rd', imm(rs1')
                decompr_instr = {5'b0, instr_i[5], instr_i[12:10], instr_i[6],
                            2'b00, 2'b01, instr_i[9:7], 3'b010, 2'b01, instr_i[4:2], {OPCODE_LOAD}};
                instr_scope = "C.LW";
            end

            5'b00_110: begin
                // c.sw -> sw rs2', imm(rs1')
                decompr_instr = {5'b0, instr_i[5], instr_i[12], 2'b01, instr_i[4:2],
                            2'b01, instr_i[9:7], 3'b010, instr_i[11:10], instr_i[6],
                            2'b00, {OPCODE_STORE}};
                instr_scope = "C.SW";
            end


            // ========== Quadrant-1 ==========
            5'b01_000: begin
                // c.addi -> addi rd, rd, nzimm
                // c.nop -> addi x0, x0, nzimm
                decompr_instr = {{6 {instr_i[12]}}, instr_i[12], instr_i[6:2],
                            instr_i[11:7], 3'b0, instr_i[11:7], {OPCODE_OP_IMM}};
                instr_scope = (instr_i[11:7] == 5'b0) ? "C.NOP": "C.ADDI";
            end

            5'b01_001, /* c.jal -> jal x1, imm */
            5'b01_101: /* c.j   -> jal x0, imm */ begin
                decompr_instr = {instr_i[12], instr_i[8], instr_i[10:9], instr_i[6],
                            instr_i[7], instr_i[2], instr_i[11], instr_i[5:3],
                            {9 {instr_i[12]}}, 4'b0, ~instr_i[15], {OPCODE_JAL}};
                instr_scope = (func3 == 3'b001) ? "C.JAL" : "C.J";
            end

            5'b01_010: begin
                // c.li -> addi rd, x0, nzimm
                decompr_instr = {{6 {instr_i[12]}}, instr_i[12], instr_i[6:2], 5'b0,
                      3'b0, instr_i[11:7], {OPCODE_OP_IMM}};
                instr_scope = "C.LI";
            end

            5'b01_011: begin
                if (instr_i[11:7] == 5'h02) begin
                    // c.addi16sp -> addi x2, x2, nzimm
                    decompr_instr = {{3 {instr_i[12]}}, instr_i[4:3], instr_i[5], instr_i[2],
                        instr_i[6], 4'b0, 5'h02, 3'b000, 5'h02, {OPCODE_OP_IMM}};
                    instr_scope = "C.ADDI16SP";
                end else begin
                    // c.lui -> lui rd, imm
                    decompr_instr = {{15 {instr_i[12]}}, instr_i[6:2], instr_i[11:7], {OPCODE_LUI}};
                    instr_scope = "C.LUI";
                end
            end

            5'b01_100: begin
                case (instr_i[11:10])
                    2'b00,
                    2'b01: begin
                        // 00: c.srli -> srli rd, rd, shamt
                        // 01: c.srai -> srai rd, rd, shamt
                        decompr_instr = {1'b0, instr_i[10], 5'b0, instr_i[6:2], 2'b01, instr_i[9:7],
                                3'b101, 2'b01, instr_i[9:7], {OPCODE_OP_IMM}};
                        instr_scope = (instr_i[11:10]==2'b00) ? "C.SRLI" : "C.SRAI";
                    end

                    2'b10: begin
                        // c.andi -> andi rd, rd, imm
                        decompr_instr = {{6 {instr_i[12]}}, instr_i[12], instr_i[6:2], 2'b01, instr_i[9:7],
                                3'b111, 2'b01, instr_i[9:7], {OPCODE_OP_IMM}};
                        instr_scope = "C.ANDI";
                    end

                    2'b11: begin
                        case (instr_i[6:5])
                            2'b00: begin
                                // c.sub -> sub rd', rd', rs2'
                                decompr_instr = {2'b01, 5'b0, 2'b01, instr_i[4:2], 2'b01, instr_i[9:7],
                                            3'b000, 2'b01, instr_i[9:7], {OPCODE_OP}};
                                instr_scope = "C.SUB";
                            end

                            2'b01: begin
                                // c.xor -> xor rd', rd', rs2'
                                decompr_instr = {7'b0, 2'b01, instr_i[4:2], 2'b01, instr_i[9:7], 3'b100,
                                        2'b01, instr_i[9:7], {OPCODE_OP}};
                                instr_scope = "C.XOR";
                            end

                            2'b10: begin
                                // c.or  -> or  rd', rd', rs2'
                                decompr_instr = {7'b0, 2'b01, instr_i[4:2], 2'b01, instr_i[9:7], 3'b110,
                                        2'b01, instr_i[9:7], {OPCODE_OP}};
                                instr_scope = "C.OR";
                            end

                            2'b11: begin
                                // c.and -> and rd', rd', rs2'
                                decompr_instr = {7'b0, 2'b01, instr_i[4:2], 2'b01, instr_i[9:7], 3'b111,
                                        2'b01, instr_i[9:7], {OPCODE_OP}};
                                instr_scope = "C.AND";
                            end
                        endcase
                    end
                endcase
            end

            5'b01_110, 
            5'b01_111: begin
                // 0: c.beqz -> beq rs1', x0, imm
                // 1: c.bnez -> bne rs1', x0, imm
                decompr_instr = {{4 {instr_i[12]}}, instr_i[6:5], instr_i[2], 5'b0, 2'b01,
                        instr_i[9:7], 2'b00, instr_i[13], instr_i[11:10], instr_i[4:3],
                        instr_i[12], {OPCODE_BRANCH}};
                instr_scope = (func3 == 3'b110) ? "C.EQEZ": "C.BNEZ";
            end

            // ========== Quadrant-2 ==========
            5'b10_000: begin
                // c.slli -> slli rd, rd, shamt
                // (c.ssli hints are translated into a slli hint)
                decompr_instr = {7'b0, instr_i[6:2], instr_i[11:7], 3'b001, instr_i[11:7], {OPCODE_OP_IMM}};
                instr_scope = "C.SLLI";
            end

            5'b10_010: begin
                // c.lwsp -> lw rd, imm(x2)
                decompr_instr = {4'b0, instr_i[3:2], instr_i[12], instr_i[6:4], 2'b00, 5'h02,
                            3'b010, instr_i[11:7], OPCODE_LOAD};
                instr_scope = "C.LWSP";
            end

            5'b10_100: begin
                if (instr_i[12] == 1'b0) begin
                    if (instr_i[6:2] != 5'b0) begin
                        // c.mv -> add rd/rs1, x0, rs2
                        // (c.mv hints are translated into an add hint)
                        decompr_instr = {7'b0, instr_i[6:2], 5'b0, 3'b0, instr_i[11:7], {OPCODE_OP}};
                        instr_scope = "C.MV";
                    end else begin
                        // c.jr -> jalr x0, rd/rs1, 0
                        decompr_instr = {12'b0, instr_i[11:7], 3'b0, 5'b0, {OPCODE_JALR}};
                        instr_scope = "C.JR";
                    end
                end else begin
                    if (instr_i[6:2] != 5'b0) begin
                        // c.add -> add rd, rd, rs2
                        // (c.add hints are translated into an add hint)
                        decompr_instr = {7'b0, instr_i[6:2], instr_i[11:7], 3'b0, instr_i[11:7], {OPCODE_OP}};
                        instr_scope = "C.ADD";
                    end else begin
                        if (instr_i[11:7] == 5'b0) begin
                            // c.ebreak -> ebreak
                            decompr_instr = {32'h00_10_00_73};
                            instr_scope = "C.EBREAK";
                        end else begin
                            // c.jalr -> jalr x1, rs1, 0
                            decompr_instr = {12'b0, instr_i[11:7], 3'b000, 5'b00001, {OPCODE_JALR}};
                            instr_scope = "C.JALR";
                        end
                    end
                end
            end

            5'b10_110: begin
                // c.swsp -> sw rs2, imm(x2)
                decompr_instr = {4'b0, instr_i[8:7], instr_i[12], instr_i[6:2], 5'h02, 3'b010,
                        instr_i[11:9], 2'b00, {OPCODE_STORE}};
                instr_scope = "C.SWSP";
            end

            default:
                is_illegal = 1'b1;
        endcase
    end
endmodule

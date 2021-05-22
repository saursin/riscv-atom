// Config file
`define RESET_PC_ADDRESS 32'h00000000

`define __NOP_INSTRUCTION__ 32'd0

// DECODER
`define __I_IMMIDIATE__ 3'b000
`define __S_IMMIDIATE__ 3'b001
`define __B_IMMIDIATE__ 3'b010
`define __U_IMMIDIATE__ 3'b011
`define __J_IMMIDIATE__ 3'b100

// ALU
`define __ALU_ADD__ 4'd0
`define __ALU_SUB__ 4'd1
`define __ALU_XOR__ 4'd2
`define __ALU_OR__  4'd3
`define __ALU_AND__ 4'd4
`define __ALU_EQ__  4'd5
`define __ALU_NQ__  4'd6
`define __ALU_LT__  4'd7
`define __ALU_GE__  4'd8
`define __ALU_LTU__ 4'd9
`define __ALU_GEU__ 4'd10

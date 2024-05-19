////////////////////////////////////////////////////////////////////   
//  File        : GPIO.v
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : GPIO IP with Wishbone B-4 Interface.
//
// MIT License
//
// Copyright (c) 2021 Saurabh Singh
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////

`default_nettype none
`include "Utils.vh"

module Timer_wb(
    // Wishbone Interface
    input   wire            wb_clk_i,
    input   wire            wb_rst_i,

    input   wire    [3:2]   wb_adr_i,
    output  reg     [31:0]  wb_dat_o,
    input   wire    [31:0]  wb_dat_i,
    input   wire            wb_we_i,
    input   wire    [3:0]   wb_sel_i,
    input   wire            wb_stb_i,
    output  reg             wb_ack_o,

    output  wire            int_o
);

// Set Ack_o
always @(posedge wb_clk_i) begin
  if (wb_rst_i)
    wb_ack_o <= 1'b0;
  else
    wb_ack_o <= wb_stb_i & !wb_ack_o;
end

wire    [3:0]   we  = {4{wb_we_i & wb_stb_i}} & wb_sel_i;


reg     [63:0]  mtime;
reg     [63:0]  mtimecmp;

assign int_o = mtime >= mtimecmp;

// Writes
always @(posedge wb_clk_i) begin
    if(wb_rst_i) begin
        mtime       <= 'd0;
        mtimecmp    <= 64'hffffffff_ffffffff;   // Initialize to large value
    end
    else begin
        case(wb_adr_i)
            2'b10: begin  // mtimecmp
                    if (we[0])  mtimecmp[7:0]       <= wb_dat_i[7:0];
                    if (we[1])  mtimecmp[15:8]      <= wb_dat_i[15:8];
                    if (we[2])  mtimecmp[23:16]     <= wb_dat_i[23:16];
                    if (we[3])  mtimecmp[31:24]     <= wb_dat_i[31:24];
                end
            2'b11: begin   // mtimecmp_h
                    if (we[0])  mtimecmp[39:32]     <= wb_dat_i[7:0];
                    if (we[1])  mtimecmp[47:40]     <= wb_dat_i[15:8];
                    if (we[2])  mtimecmp[55:48]     <= wb_dat_i[23:16];
                    if (we[3])  mtimecmp[63:56]     <= wb_dat_i[31:24];
                end
            default: begin   // INC  (NOT IMPLEMENTED)
            end
        endcase
        mtime <= mtime + 'd1;
    end
end


// READS
always @(*) /* COMBINATORIAL */ begin
    case(wb_adr_i)
        2'b00:  /* mtime */         wb_dat_o = mtime[31:0];
        2'b01:  /* mtime_h */       wb_dat_o = mtime[63:32];
        2'b10:  /* mtimecmp */      wb_dat_o = mtimecmp[31:0];
        2'b11:  /* mtimecmp_h */    wb_dat_o = mtimecmp[63:32];
        default:
            wb_dat_o = 'dx;
    endcase
end

endmodule

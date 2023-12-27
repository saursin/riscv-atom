

`define bus_select(bus, index, field_width) bus[(field_width*(index+1))-1 : (field_width*index)]

module Crossbar_wb #(
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 32,
    parameter SELECT_WIDTH = (DATA_WIDTH/8),
    parameter NSLAVES = 5,
    parameter [NSLAVES*ADDR_WIDTH-1:0] DEVICE_ADDR = {NSLAVES{32'h0a0a0a0a}},
    parameter [NSLAVES*ADDR_WIDTH-1:0] DEVICE_MASK = {NSLAVES{32'h0f0f0f0f}}
)(
    input  wire                             wb_clk_i,

    // Wishbone slave port
    input  wire [ADDR_WIDTH-1:0]            wbm_adr_i,
    input  wire [DATA_WIDTH-1:0]            wbm_dat_i,
    output wire [DATA_WIDTH-1:0]            wbm_dat_o,
    input  wire                             wbm_we_i,
    input  wire [SELECT_WIDTH-1:0]          wbm_sel_i,
    input  wire                             wbm_stb_i,
    input  wire                             wbm_cyc_i,
    output wire                             wbm_ack_o,
    output wire                             wbm_err_o,

    // Wishbone master port
    output wire [ADDR_WIDTH*NSLAVES-1:0]    wbs_adr_o,
    input  wire [DATA_WIDTH*NSLAVES-1:0]    wbs_dat_i,
    output wire [DATA_WIDTH*NSLAVES-1:0]    wbs_dat_o,
    output wire [NSLAVES-1:0]               wbs_we_o,
    output wire [SELECT_WIDTH*NSLAVES-1:0]  wbs_sel_o,
    output wire [NSLAVES-1:0]               wbs_cyc_o,
    output wire [NSLAVES-1:0]               wbs_stb_o,
    input  wire [NSLAVES-1:0]               wbs_ack_i,
    input  wire [NSLAVES-1:0]               wbs_err_i
);
    // Selection Logic
    wire [NSLAVES-1:0]  selected_device;
    genvar i;
    generate
        for(i=0; i<NSLAVES ; i=i+1) begin : addr_match
            assign selected_device[i] = (wbm_adr_i & DEVICE_MASK[i*ADDR_WIDTH+:ADDR_WIDTH]) == DEVICE_ADDR[i*ADDR_WIDTH+:ADDR_WIDTH];
        end
    endgenerate

    reg [$clog2(NSLAVES)-1:0]   selected_device_enc;
    integer k;
    always @(*) begin
        selected_device_enc = 0;
        for (k = NSLAVES-1; k >= 0; k=k-1) begin
            if(selected_device[k])  selected_device_enc = k[$clog2(NSLAVES)-1:0];
        end
    end

    // Error Logic
    reg wbm_err;
    always @(posedge wb_clk_i) begin
        wbm_err <= wbm_cyc_i & !(|selected_device);
    end

    // Slave port signals
    assign wbs_adr_o = {NSLAVES{wbm_adr_i}};
    assign wbs_dat_o = {NSLAVES{wbm_dat_i}};
    assign wbs_sel_o = {NSLAVES{wbm_sel_i}};
    assign wbs_we_o  = {NSLAVES{wbm_we_i}};
    assign wbs_cyc_o = selected_device & ({NSLAVES{wbm_cyc_i}} << selected_device_enc);
    assign wbs_stb_o = {NSLAVES{wbm_stb_i}};

    assign wbm_dat_o = wbs_dat_i[selected_device_enc*DATA_WIDTH+:DATA_WIDTH];
    assign wbm_ack_o = wbs_ack_i[selected_device_enc];
    assign wbm_err_o = wbs_err_i[selected_device_enc] | wbm_err;
endmodule

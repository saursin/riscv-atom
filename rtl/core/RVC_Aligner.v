`default_nettype none

module RVC_Aligner(
    input                   clk_i,
    input                   rst_i,
    // From Core (Slave)
    input   wire    [31:0]  s_adr_i,
    output  wire    [31:0]  s_dat_o,
    input   wire            s_valid_i,
    output  wire            s_ack_o,

    // To Memory (Master)
    output  wire    [31:0]  m_adr_o,
    input   wire    [31:0]  m_dat_i,
    output  wire            m_valid_o,
    input   wire            m_ack_i
);

/*
    Aligns the fetches to 4 byte boundary
    if addr is 4 byte aligned -> e.g. 0x0 or 0x4 ->  pass through
    if addr is 2 byte aligned and not 4 byte aligned -> e.g. 0x2:
        - Step 1:
            - fetch the previous 4 byte aligned address (0x0)
            - store upper half word
            - block ack signal to core     
        - Step 2:
            - fetch next 4 byte aligned address (0x4)
            - concatenate lower half word with upper half word
            - return ack to core
*/
wire is_misaligned = s_adr_i[1];
reg was_misaligned;
always @(posedge clk_i) begin
    if(rst_i)
        was_misaligned <= 1'b0;
    else if(m_ack_i & is_misaligned)
        was_misaligned <= !was_misaligned;
end

assign m_adr_o = is_misaligned ? (was_misaligned ? s_adr_i + 32'd2 : s_adr_i - 32'd2) : s_adr_i;
assign m_valid_o = s_valid_i;

reg [15:0] lower_half_word;
always @(posedge clk_i) begin
    if(rst_i)
        lower_half_word <= 0;
    else if(m_ack_i)
        lower_half_word <= m_dat_i[31:16];
end

assign s_dat_o = was_misaligned ? {m_dat_i[15:0], lower_half_word}: m_dat_i;
assign s_ack_o = m_ack_i & !(is_misaligned & !was_misaligned);

endmodule

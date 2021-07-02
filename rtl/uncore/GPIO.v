`default_nettype none

module GPIO
(
    input wire wb_clk_i,
    input wire wb_dat_i,
    input wire wb_we_i,
    input wire wb_cyc_i,
    output reg wb_rdt_o,
    output reg gpio_o
);

always @(posedge wb_clk_i) begin
    wb_rdt_o <= gpio_o;
    if (wb_cyc_i & wb_we_i)
        gpio_o <= wb_dat_i;
end

endmodule
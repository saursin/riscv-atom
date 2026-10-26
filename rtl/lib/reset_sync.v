`default_nettype none

module reset_sync #(
    parameter NUM_STAGES = 2        // Number of synchronization stages (>=2)
)(
    input   wire    clk_i,
    input   wire    rst_i,
    output  wire    rst_o
);
    reg [NUM_STAGES-1:0] sync_reg;
    always @(posedge clk_i or posedge rst_i) begin
        if(rst_i) begin
            // Asynchronous reset sets all stages to '1'
            sync_reg <= {NUM_STAGES{1'b1}};
        end else begin
            // Shift in '0' to synchronize reset de-assertion
            sync_reg <= {sync_reg[NUM_STAGES-2:0], 1'b0};
        end
    end

    assign rst_o = sync_reg[NUM_STAGES-1];
endmodule

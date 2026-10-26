module FIFO_sync #(
    parameter DEPTH = 32,
    parameter DATAW = 8
)(
    input  wire             clk_i,
    input  wire             rst_i,
    input  wire             we_i,
    input  wire             re_i,
    input  wire [DATAW-1:0] dat_i,
    output wire [DATAW-1:0] dat_o,
    output wire             full_o,
    output wire             empty_o
);
    parameter ADDRW = $clog2(DEPTH);

    reg [DATAW-1:0] mem [0: DEPTH-1];
        reg [ADDRW:0]   count;
    reg [ADDRW-1:0] w_ptr;
    reg [ADDRW-1:0] r_ptr;

    wire enq_cond = we_i & !full_o;
    wire deq_cond = re_i & !empty_o;

    always@(posedge clk_i) begin
        if(rst_i) begin
            w_ptr <= 0;
            r_ptr <= 0;
            count <= 0;
        end else begin
            if(enq_cond)begin
                mem[w_ptr] <= dat_i;
                w_ptr <= w_ptr + 1;
                count <= count +1;
            end

            if(deq_cond) begin
                r_ptr <= r_ptr + 1;
                count <= count - 1;
            end

            if(enq_cond && deq_cond) begin
                count <= count;
            end
        end
    end
        assign dat_o = mem[r_ptr];

    assign empty_o = (count == 0);
    assign full_o = (count == DEPTH);
endmodule

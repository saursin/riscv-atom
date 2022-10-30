`default_nettype none

module uart_vip #(
    parameter F_RATIO = 3,                  // ratio of clock frequency to baudrate. F_RATIO = (CLK_FREQ/BAUD)  (for simpleuart with div=, F_RATIO is 3)
    parameter DUMPFILE = "uart_tx.dump",    // tx dumpfile
    parameter ECHO_ON_STDOUT = 1,           // if 1: uart packets are ALSO sent to stdout
    parameter LOOPBACK = 0                  // if 1: uart_vip.rx is looped back to uart_vip.tx
) (
    input   wire    clk, 
    input   wire    rst,
    input   wire    rx, 
    output  wire    tx,
    output  reg     dbg = 0
);

// loopback
//if(LOOPBACK)
assign tx = rx;
//else
//assign tx = 1'b1;

// file handle
integer fd = 0;

// Enum
localparam STATE_IDLE = 0;
localparam STATE_START_BIT = 1;
localparam STATE_DATA_BITS = 2;
localparam STATE_STOP_BIT = 3;

// VIP State machine Variables
integer prev_rx = 0;
integer state = STATE_IDLE;
integer wait_cyc = 0;
integer got_bits = 0;
integer i_byte = 0;

task rx_state (input rx);
begin
    dbg = 0;
    if(wait_cyc) begin
        wait_cyc = wait_cyc - 1;
    end else begin
        case(state)
            STATE_IDLE: begin
                if(prev_rx == 1 && rx == 0) begin
                    state = STATE_START_BIT;
                    i_byte = 0;
                    got_bits = 0;
                    wait_cyc = (F_RATIO/2)-1;
                    dbg = 1;
                end
            end

            STATE_START_BIT: begin
                if(rx == 0) begin
                    state = STATE_DATA_BITS;
                    wait_cyc = F_RATIO - 1;
                    dbg = 1;
                end else begin
                    state = STATE_IDLE;
                end
            end

            STATE_DATA_BITS: begin
                i_byte = (i_byte >> 1) | {rx, 7'b0000000};
                got_bits = got_bits + 1;

                if(got_bits == 8) begin
                    state = STATE_STOP_BIT;
                    dbg = 1;
                end else begin
                    state = STATE_DATA_BITS;
                    dbg = 1;
                end
                wait_cyc = F_RATIO - 1;
            end

            STATE_STOP_BIT: begin
                if(rx == 1) begin
                    // packet received successfully
                    if(ECHO_ON_STDOUT)
                        $write("%c", i_byte);
                    $fwrite(fd, "%c", i_byte);
                    wait_cyc = (F_RATIO/16)-1;       // wait for 1/16th of baud period
                    state = STATE_IDLE;
                    dbg = 1;
                end else begin
                    $display("UART-VIP: Framing error for <%x>", i_byte);
                    state = STATE_IDLE;
                    dbg = 0;
                end
            end

            default: begin
                state = STATE_IDLE;
                dbg = 0;
            end
        endcase
        prev_rx = rx;
    end
end
endtask


initial begin
    fd = $fopen(DUMPFILE, "w");
end

always @(posedge clk) begin
    rx_state(rx);
end

endmodule

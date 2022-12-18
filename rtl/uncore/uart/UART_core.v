`default_nettype none

module UART_core
(
    input	wire	        clk,
    input 	wire	        rst,

    // Serial Interface
    output 	wire	        ser_tx,
    input  	wire	        ser_rx,

    // Register interfaces
    input	wire 	[31:0] 	divisor,

    input   wire     		reg_dat_we,
    input   wire      		reg_dat_re,
    input	wire 	[7:0]	reg_dat_di,
    output	wire 	[7:0]	reg_dat_do,

    // Transreciever Settings
    input   wire            rx_en,
    input   wire            tx_en,

    // Parity settings
    input   wire            enable_parity,
    input   wire            even_parity,

    // Stop Bit Settings
    input   wire            enable_dual_stop_bit,

    // Status Bits
    output  wire            tx_buf_empty,
    output  wire            rx_buf_valid,
    output  wire            err_framing,
    output  wire            err_parity,
    output  wire            rx_parity
);

    /////////////////////////////////////////////////////
    // Glue Logic
    assign  tx_buf_empty    = (send_state == 0);
    assign  rx_buf_valid    = recv_buf_valid;
    assign  err_framing     = recv_framing_error;
    assign  err_parity      = recv_parity_error;
    assign  rx_parity       = recv_parity_bit;

    assign reg_dat_do = recv_pattern;

    /////////////////////////////////////////////////////
    // RX Logic

    reg [4:0]   recv_state;
    reg [31:0]  recv_divcnt;
    reg [7:0]   recv_pattern;
    reg [7:0]   recv_buf_data;
    reg         recv_buf_valid;

    reg         recv_parity_bit;
    reg         recv_stopbit;

    reg         recv_framing_error = 0;
    reg         recv_parity_error = 0;

    always @(posedge clk) begin
        if (rst) begin
            recv_state      <= 0;
            recv_divcnt     <= 0;
            recv_pattern    <= 0;
            recv_buf_data   <= 0;
            recv_parity_bit <= 0;
            recv_buf_valid  <= 0;
            recv_stopbit    <= 0;

            recv_framing_error <= 0;
            recv_parity_error <= 0;
        end else begin
            recv_divcnt <= recv_divcnt + 1;

            if (reg_dat_re) begin
                // Invalidate recieve data after being read once
                recv_buf_valid <= 0;
                recv_parity_bit <= 0;
            end
                
            case (recv_state)
                0: /* IDLE */ begin
                    if (rx_en && !ser_rx)
                        recv_state <= 1;
                    recv_divcnt <= 0;
                end
                1: /* START BIT */ begin
                    if (2*recv_divcnt > divisor) begin
                        // Initialize for Packet
                        recv_framing_error  <= 0;
                        recv_parity_error   <= 0;
                        recv_stopbit        <= 0;

                        recv_divcnt <= 0;
                        recv_state  <= 2;                       
                    end
                end
                10: /* PARITY BIT */ begin
                    if (recv_divcnt > divisor) begin
                        recv_parity_bit <= ser_rx;

                        // check parity
                        // if even parity disabled invert get_even_parity output
                        recv_parity_error <= ~( (~even_parity) ^ (get_even_parity(recv_buf_data)) & ser_rx);
                        
                        recv_divcnt <= 0;
                        recv_state <= recv_state + 1;   // go to stop bit 0 state (11)
                    end
                end               
                11: /* STOP BIT */ begin
                    if (recv_divcnt > divisor) begin
                        if(!ser_rx) begin
                            recv_framing_error <= 1;   // Framing error
                        end else if(~enable_dual_stop_bit || (enable_dual_stop_bit & !recv_stopbit)) begin
                            // Store only in the last stop bit
                            recv_buf_data <= recv_pattern;
                            recv_buf_valid <= 1;
                        end

                        if(enable_dual_stop_bit & !recv_stopbit) begin
                            recv_divcnt <= 0;
                            recv_state <= 11;
                            recv_stopbit <= 1;
                        end else
                            recv_state <= 0;
                    end
                end
                default: /* DATA BITS */ begin
                    if (recv_divcnt > divisor) begin
                        recv_pattern <= {ser_rx, recv_pattern[7:1]};
                        recv_divcnt <= 0;

                        if(recv_state == 9) begin // If in last data bit state
                            if(enable_parity)
                                recv_state <= recv_state + 1;   // go to parity state
                            else
                                recv_state <= 11; // go to stop bit 0 state
                        end else
                            recv_state <= recv_state + 1;
                    end
                end
            endcase
        end
    end

    /////////////////////////////////////////////////////
    // TX Logic

    function get_even_parity;
        input [7:0] data;
        begin
            get_even_parity =   (data[0] ^ data[1]) ^ (data[2] ^ data[3]) ^ 
                                (data[4] ^ data[5]) ^ (data[6] ^ data[7]);
        end 
    endfunction


    reg [8:0] send_pattern; // data + parity
    reg [3:0] send_state;
    reg [31:0] send_divcnt;
    reg send_stopbit;
   
    reg send_tx = 1;
    assign ser_tx = send_tx;
    

    always @(posedge clk) begin
        if (rst) begin
            send_state <= 0;
            send_pattern <= ~0;
            send_divcnt <= 0;
            send_tx <= 1;
            send_stopbit <= 0;
        end else begin
            send_divcnt <= send_divcnt + 1;

            case(send_state)
                0: /* IDLE */ begin
                    if(tx_en && reg_dat_we) begin
                        send_pattern <= {(~even_parity) ^ get_even_parity(reg_dat_di[7:0]), reg_dat_di[7:0]};
                        send_state <= 1;
                    end
                    send_divcnt <= 0;
                end
                1: /* START BIT */ begin
                    send_tx <= 0;

                    send_stopbit <= 0;
                    
                    send_state <= 2;
                    send_divcnt <= 0;
                end
                10: /* PARITY BIT */ begin
                    if (send_divcnt > divisor) begin
                        send_tx <= send_pattern[0];  // parity bit
                        send_divcnt <= 0;
                        send_state <= send_state + 1;   // go to stop bit 0 state (11)
                    end
                end               
                11: /* STOP BIT */ begin
                    if (send_divcnt > divisor) begin
                        send_tx <= 1;

                        if(enable_dual_stop_bit & !send_stopbit) begin
                            send_state <= 11;
                            send_stopbit <= 1;
                        end else
                            send_state <= 12;
                        
                        send_divcnt <= 0;
                    end
                end
                12: /* POST STOP HOLD */ begin
                    /*
                        This is a dummy state that holds the 
                        line 1 after stop bit state to prevent 
                        framing error
                    */
                    if (send_divcnt > divisor) begin
                        send_tx <= 1;
                        send_state <= 0;    // back to IDLE
                    end
                end
                default: /* DATA BITS */ begin
                    if (send_divcnt > divisor) begin
                        send_tx <= send_pattern[0];
                        send_pattern <= send_pattern >> 1;
                        send_divcnt <= 0;

                        if(send_state == 9) begin // If in last data bit state
                            if(enable_parity)
                                send_state <= send_state +1;    // go to parity bit state
                            else
                                send_state <= 11;   // go to stop bit state
                        end else
                            send_state <= send_state + 1;
                    end
                end
            endcase
        end
    end

endmodule

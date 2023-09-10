`default_nettype none
module SPI_core (
    input   wire    clk_i,
    input   wire    rst_i,

    input   wire            spi_en_i,
    input   wire            lsb_first_i,
    input   wire            pol_i,
    input   wire            pha_i,
    input   wire    [7:0]   post_csl_delay,
    input   wire    [7:0]   pre_csh_delay,
    input   wire    [7:0]   txdata_i,
    input   wire            tx_we_i,
    output  wire    [7:0]   rxdata_o,
    input   wire    [31:0]  sckdiv_i,

    output  reg             sck_o,
    input   wire            miso_i,
    output  wire            mosi_o,
    output  reg             cs_o
);

reg [31:0]  divcnt;
reg [3:0]   bits_remaining;     // number of bits remaining to be transferred
reg [7:0]   tx_shiftreg;        // TX shift register
reg [7:0]   rx_shiftreg;        // RX shift register

assign rxdata_o = rx_shiftreg;

assign mosi_o = lsb_first_i ? tx_shiftreg[0] : tx_shiftreg[7];

wire [7:0]  tx_shiftreg_updateval = lsb_first_i ? tx_shiftreg >> 1 : tx_shiftreg << 1;
wire [7:0]  rx_shiftreg_updateval = lsb_first_i ? {miso_i, rx_shiftreg[7:1]} : {rx_shiftreg[6:0], miso_i};

// ** State Machine **
localparam IDLE = 0;
localparam POST_CSL_DELAY = 1;
localparam XFER = 2;
localparam PRE_CSH_DELAY = 3;
reg [1:0]   state;              // State
reg [7:0]   delay_ctr;          // delay counter

always @(posedge clk_i) begin
    if(rst_i) begin
        divcnt <= 0;
        state <= 0;
        tx_shiftreg <= 0;
        rx_shiftreg <= 0;
        bits_remaining <= 0;
        cs_o <= 1;
        delay_ctr <= 0;
        sck_o <= 0;
    end else begin
        divcnt <= divcnt + 1;

        case(state)
            IDLE: begin
                sck_o <= pol_i;
                if(spi_en_i && tx_we_i) begin
                    tx_shiftreg <= txdata_i;
                    bits_remaining <= 7;
                    cs_o <= 0;
                    divcnt <= 0;
                    if (post_csl_delay == 0) begin
                        state <= XFER;
                        sck_o <= pha_i ? ~sck_o: sck_o;
                    end else begin
                        state <= POST_CSL_DELAY;
                        delay_ctr <= post_csl_delay-1;
                    end
                end
            end

            POST_CSL_DELAY: begin
                if(divcnt == sckdiv_i) begin
                    delay_ctr <= delay_ctr - 1;
                    if (delay_ctr == 0) begin
                        state <= XFER;
                        sck_o <= pha_i ? ~sck_o: sck_o;
                    end
                    divcnt <= 0;
                end
            end

            XFER: begin
                if(divcnt == sckdiv_i/2) begin
                    sck_o <= ~sck_o;
                end if(divcnt == sckdiv_i) begin
                    tx_shiftreg <= tx_shiftreg_updateval;
                    rx_shiftreg <= rx_shiftreg_updateval;
                    if (bits_remaining == 0) begin
                        if(pre_csh_delay == 0) begin
                            state <= IDLE;
                            cs_o <= 1;
                        end else begin
                            state <= PRE_CSH_DELAY;
                            delay_ctr <= pre_csh_delay-1;
                        end
                        sck_o <= pol_i;
                    end else begin
                        bits_remaining <= bits_remaining - 1;   
                        sck_o <= ~sck_o;
                    end
                    divcnt <= 0;
                end
            end

            PRE_CSH_DELAY: begin
                if(divcnt == sckdiv_i) begin
                    delay_ctr <= delay_ctr - 1;
                    if (delay_ctr == 0) begin
                        state <= IDLE;
                        cs_o <= 1;
                    end
                    divcnt <= 0;
                end
            end

        endcase
    end
end

endmodule


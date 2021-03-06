`include "sha256.svh"

module sha256_w(
    input logic clk,
    input logic resetn,
    input logic wr_en,
    input logic [31:0] data,
    input logic [5:0] t,
    output logic [31:0] W
);
    logic [31:0] W_reg [1:16];
    logic [31:0] W_gen;
    logic [31:0] s0;
    logic [31:0] s1;

    assign s0 = `rotr(W_reg[15],7) ^ `rotr(W_reg[15],18) ^ `shr(W_reg[15],3);
    assign s1 = `rotr(W_reg[2],17) ^ `rotr(W_reg[2],19) ^ `shr(W_reg[2],10);
    assign W_gen = s1 + W_reg[7] + s0 + W_reg[16];
    assign wr_ready = (t[5:4]==2'b00); // t < 16

    genvar i;
    generate
        for (i=2; i<17; i++) 
            always_ff @(posedge clk)
                 if (!resetn)           W_reg[i] <= 0; 
            else if (wr_ready && wr_en) W_reg[i] <= W_reg[i-1];
            else if (!wr_ready)         W_reg[i] <= W_reg[i-1];
            else                        W_reg[i] <= W_reg[i];
    endgenerate

    always_ff @(posedge clk)
            if (!resetn)            W_reg[1] <= 0; 
    else    if (wr_ready && wr_en)  W_reg[1] <= data;
    else    if (!wr_ready)          W_reg[1] <= W_gen;
    else                            W_reg[1] <= W_reg[1];

    assign W = (wr_ready)? data: W_gen;

endmodule
    
`include "sha256.svh"

module sha256_iter(
        input logic [31:0] a,
        input logic [31:0] b,
        input logic [31:0] c,
        input logic [31:0] d,
        input logic [31:0] e,
        input logic [31:0] f,
        input logic [31:0] g,
        input logic [31:0] h,
        input logic [31:0] W,
        input logic [31:0] K,
        output logic [31:0] a_out,
        output logic [31:0] b_out,
        output logic [31:0] c_out,
        output logic [31:0] d_out,
        output logic [31:0] e_out,
        output logic [31:0] f_out,
        output logic [31:0] g_out,
        output logic [31:0] h_out
    );
    logic [31:0] ch;
    logic [31:0] maj;
    logic [31:0] S0;
    logic [31:0] S1;
    logic [31:0] T1;
    logic [31:0] T2;

    assign ch = (e & f) ^ ((~e) & g);
    assign maj = (a & b) ^ (a & c) ^ (b & c);
    assign S0 = `rotr(a,2) ^ `rotr(a,13) ^ `rotr(a,22);
    assign S1 = `rotr(e,6) ^ `rotr(e,11) ^ `rotr(e,25);
    assign T1 = h + S1 + ch + K + W;
    assign T2 = S0 + maj;
    
    assign h_out = g;
    assign g_out = f;
    assign f_out = e;
    assign e_out = d + T1;
    assign d_out = c;
    assign c_out = b;
    assign b_out = a;
    assign a_out = T1 + T2;

endmodule

module sha256_core_tb_vec_gen(
    input integer n,
    output logic [31:0] data
);
// using test vector from website below
// https://qvault.io/cryptography/how-sha-2-works-step-by-step-sha-256/
always_comb
case (n)
     0: data <= 32'h68656C6C;
     1: data <= 32'h6F20776F;
     2: data <= 32'h726C6480;
     15: data <= 32'd88;    // message length
    default: data <= 32'h00000000;
endcase
endmodule

module sha256_core_tb;

    localparam T = 10ns;

    logic clk;
    logic resetn;
    logic [31:0] data;
    logic wr_en;
    logic wr_ready;
    logic blk_ready;
    logic [255:0] hash;

    integer n;

    sha256_core uut(.*);
    sha256_core_tb_vec_gen vec(.*);

    // clock gen
    always
    begin
        clk = 0;
        #(T/2);
        clk = 1;
        #(T/2);
    end

    // init reset
    initial
    begin
        resetn = 0;
        #T;
        resetn = 1;
    end

    // main test
    initial
    begin
        wr_en = 0;
        #(10*T);
        n = 0;
        while (n < 16)
        begin
            if (wr_ready)
            begin
                wr_en = 1;
                #T;
                wr_en = 0;
                #T;
                n += 1;
            end
        end
    end

endmodule

module sha256_core(
    input logic clk,
    input logic resetn,
    input logic [31:0] data,
    input logic wr_en,
    output logic wr_ready,
    output logic blk_ready,
    output logic [255:0] hash
);

    typedef enum {idle, hashing} state_type;

    localparam H0_0 = 32'h6a09e667;
    localparam H0_1 = 32'hbb67ae85;
    localparam H0_2 = 32'h3c6ef372;
    localparam H0_3 = 32'ha54ff53a;
    localparam H0_4 = 32'h510e527f;
    localparam H0_5 = 32'h9b05688c;
    localparam H0_6 = 32'h1f83d9ab;
    localparam H0_7 = 32'h5be0cd19;

    state_type state_reg, state_next;
    logic [5:0] t_reg, t_next, t;

    logic [31:0] a,b,c,d,e,f,g,h,W,K;
    logic [31:0] a_next,b_next,c_next,d_next,e_next,f_next,g_next,h_next;
    logic [31:0] H [0:7];
    logic [31:0] H_next [0:7];
    logic [31:0] a_out,b_out,c_out,d_out,e_out,f_out,g_out,h_out;

    assign t = t_reg;
    assign wr_ready = (t[5:4]==2'b00); // t < 16

    // comb logic module instantiation
    sha256_iter iter(.*);
    sha256_w wgen(.*);
    sha256_k kgen(.*);

    // state and data reset and progression
    always_ff @(posedge clk)
    if (!resetn) begin // reset to hash new message
       state_reg <= idle;
       t_reg <= 6'b000000;
       H[0] <= H0_0;
       H[1] <= H0_1;
       H[2] <= H0_2;
       H[3] <= H0_3;
       H[4] <= H0_4;
       H[5] <= H0_5;
       H[6] <= H0_6;
       H[7] <= H0_7;
       a <= H0_0;
       b <= H0_1;
       c <= H0_2;
       d <= H0_3;
       e <= H0_4;
       f <= H0_5;
       g <= H0_6;
       h <= H0_7;
    end   
    else begin
       state_reg <= state_next;
       t_reg <= t_next;
       a <= a_next;
       b <= b_next;
       c <= c_next;
       d <= d_next;
       e <= e_next;
       f <= f_next;
       g <= g_next;
       h <= h_next;
       H[0] <= H_next[0];
       H[1] <= H_next[1];
       H[2] <= H_next[2];
       H[3] <= H_next[3];
       H[4] <= H_next[4];
       H[5] <= H_next[5];
       H[6] <= H_next[6];
       H[7] <= H_next[7];
    end

    // next state logic
    always_comb
    begin
        state_next = state_reg;
        t_next = t_reg;
        a_next = a;
        b_next = b;
        c_next = c;
        d_next = d;
        e_next = e;
        f_next = f;
        g_next = g;
        h_next = h;
        H_next[0] = H[0];
        H_next[1] = H[1];
        H_next[2] = H[2];
        H_next[3] = H[3];
        H_next[4] = H[4];
        H_next[5] = H[5];
        H_next[6] = H[6];
        H_next[7] = H[7];
        case (state_reg)
            idle: begin // ready for new block
                if (wr_en)
                begin
                    // do one iteration
                    a_next = a_out;
                    b_next = b_out;
                    c_next = c_out;
                    d_next = d_out;
                    e_next = e_out;
                    f_next = f_out;
                    g_next = g_out;
                    h_next = h_out;
                    t_next = 6'b000001;
                    state_next = hashing; 
                end
                else
                begin
                    // init working register
                    a_next = H[0];
                    b_next = H[1];
                    c_next = H[2];
                    d_next = H[3];
                    e_next = H[4];
                    f_next = H[5];
                    g_next = H[6];
                    h_next = H[7];
                    t_next = 6'b000000;
                    state_next = idle;
                end
            end
            hashing: begin // hashing a block
                if (t == 6'b111111)
                begin // last iteration
                    a_next = H[0] + a_out;
                    b_next = H[1] + b_out;
                    c_next = H[2] + c_out;
                    d_next = H[3] + d_out;
                    e_next = H[4] + e_out;
                    f_next = H[5] + f_out;
                    g_next = H[6] + g_out;
                    h_next = H[7] + h_out;
                    H_next[0] += a_out;
                    H_next[1] += b_out;
                    H_next[2] += c_out;
                    H_next[3] += d_out;
                    H_next[4] += e_out;
                    H_next[5] += f_out;
                    H_next[6] += g_out;
                    H_next[7] += h_out;
                    state_next = idle;
                    t_next = 0'b000000;
                end
                else if (!wr_ready || (wr_ready && wr_en))
                begin // t>=16 or (t<16 and write enabled i.e. new data)
                    a_next = a_out;
                    b_next = b_out;
                    c_next = c_out;
                    d_next = d_out;
                    e_next = e_out;
                    f_next = f_out;
                    g_next = g_out;
                    h_next = h_out;
                    state_next = state_reg;
                    t_next += 0'b000001;
                end
                else
                begin // t<16 and write disabled i.e. no new data
                    // do nothing
                    state_next = state_reg;
                    t_next = t_reg;
                    a_next = a;
                    b_next = b;
                    c_next = c;
                    d_next = d;
                    e_next = e;
                    f_next = f;
                    g_next = g;
                    h_next = h;
                    H_next[0] = H[0];
                    H_next[1] = H[1];
                    H_next[2] = H[2];
                    H_next[3] = H[3];
                    H_next[4] = H[4];
                    H_next[5] = H[5];
                    H_next[6] = H[6];
                    H_next[7] = H[7];
                end
            end
        endcase
    end

    // moore output
    assign blk_ready = (state_reg == idle);
    assign hash = {H[0],H[1],H[2],H[3],H[4],H[5],H[6],H[7]};

endmodule

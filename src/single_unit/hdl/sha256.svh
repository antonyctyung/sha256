`ifndef SHA256_SVH
`define SHA256_SVH

`define rotr(x,n) {x[0+:n],x[31:n]}
`define rotl(x,n) {x[0+:(32-n)],x[31:(32-n)]}
`define shr(x,n) {n'b0,x[31:n]}
`define shl(x,n) {n'b0,x[31:(32-n)]}

`define H0_0 = 32'h6a09e667
`define H0_1 = 32'hbb67ae85
`define H0_2 = 32'h3c6ef372
`define H0_3 = 32'ha54ff53a
`define H0_4 = 32'h510e527f
`define H0_5 = 32'h9b05688c
`define H0_6 = 32'h1f83d9ab
`define H0_7 = 32'h5be0cd19

`endif
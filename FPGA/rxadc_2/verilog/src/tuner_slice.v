// tuner_slice.v - one leg of real->complex tuner
// 07-17-16 E. Brombaugh

module tuner_slice #(
    parameter dsz = 10,
              psz = 10
)
(
    input clk, reset, shf_90,
    input signed [dsz-1:0] in,
    input [psz-1:0] phs,
    output reg signed [dsz-1:0] out
);
    // split acc into quadrant and address and delay sign bit
    wire [1:0] p_quad = phs[psz-1:psz-2] + {1'b0,shf_90};
    reg [1:0] quad;
    reg [psz-3:0] addr;
    reg sincos_sign;
    always @(posedge clk)
    begin
        if(reset == 1'b1)
        begin
            quad <= 2'b0;
            addr <= 8'b0;
            sincos_sign <= 1'b0;
        end
        else
        begin
            quad <= p_quad;
            addr <= phs[psz-3:0] ^ {8{p_quad[0]}};
            sincos_sign <= quad[1];
        end
    end
    
    // look up 8-bit 1/4 cycle sine
    reg signed [15:0] sine_lut[0:255];
    reg signed [15:0] sincos_raw;
    initial
    begin
        $readmemh("../src/sine_table.memh", sine_lut);
    end
    always @(posedge clk)
    begin
        sincos_raw <= sine_lut[addr];
    end
    
    // invert sign of lut output and delay to align
    reg signed [15:0] sincos_p, sincos;
    always @(posedge clk)
    begin
        if(reset == 1'b1)
        begin
            sincos_p <= 16'h0000;
            sincos <= 16'h0000;
        end
        else
        begin
            sincos_p <= sincos_sign ? -sincos_raw : sincos_raw;
            sincos <= sincos_p;
        end
    end
    
    // multiply, round, saturate
    reg signed [dsz+16-1:0] mult;
    wire signed [dsz+1:0] out_rnd = mult[dsz+16-1:14] + 12'd1;
    wire signed [dsz-1:0] out_sat;
    
    sat #(.isz(dsz+1),.osz(dsz))
        u_sat(.in(out_rnd[dsz+1:1]),.out(out_sat));
    
    always @(posedge clk)
    begin
        if(reset == 1'b1)
        begin
            mult <= {dsz+16{1'b0}};
            out <= {dsz{1'b0}};
        end
        else
        begin
            mult <= in * sincos;
            out <= out_sat;
        end
    end
endmodule


//
// i2s_out: I2S serializer
//
module i2s_out(clk, reset,
				l_data, r_data,
				mclk, sdout, sclk, lrclk,
				load);
	
	input clk;									// System clock
	input reset;								// System POR
	input signed [15:0] l_data, r_data;			// inputs
	output mclk;								// I2S master clock (256x)
	output sdout;								// I2S serial data
	output sclk;								// I2S serial clock
	output lrclk;								// I2S Left/Right clock
	output load;								// Sample rate enable output
	
	// Sample rate generation
	wire mclk_ena;
	clkgen
		uclk(.clk(clk), .reset(reset),
			.mclk(mclk), .mclk_ena(mclk_ena), .rate(load));
	
	// Serial Clock divider (/8 for 48MHz -> 48kHz)
	reg [2:0] scnt;		// serial clock divide register
	always @(posedge clk)
		if(reset)
			scnt <= 0;
		else if(load)
			scnt <= 0;
		else if(mclk_ena)
			scnt <= scnt + 1;
	
	// generate serial clock pulse
	reg p_sclk;			// 1 cycle wide copy of serial clock
	always @(posedge clk)
		if (mclk_ena)
			p_sclk <= (scnt==3'b000);
	
	// Shift register advances on serial clock
	reg [31:0] sreg;
	always @(posedge clk)
		if(load)
			sreg <= {l_data,r_data};
		else if(p_sclk & mclk_ena)
			sreg <= {sreg[30:0],1'b0};
	
	// 1 serial clock cycle delay on data relative to LRCLK
	reg sdout;
	always @(posedge clk)
		if(p_sclk & mclk_ena)
			sdout <= sreg[31];
	
	// Generate LR clock
	reg [3:0] lrcnt;
	reg lrclk;
	always @(posedge clk)
		if(reset | load)
		begin
			lrcnt <= 0;
			lrclk <= 0;
		end
		else if(p_sclk & mclk_ena)
		begin
			if(lrcnt == 4'd15)
			begin
				lrcnt <= 0;
				lrclk <= ~lrclk;
			end
			else
				lrcnt <= lrcnt + 1;
		end
	
	// align everything
	reg sclk_p0, sclk;
	always @(posedge clk)
	begin
		sclk_p0 <= scnt[2];
		sclk <= sclk_p0;
	end
endmodule

// cic_dec_3.v: CIC Decimator - single with internal truncation before combs
// 2016-07-17 E. Brombaugh

module cic_dec_3 #(
	parameter NUM_STAGES = 4,						// Stages of int / comb
	          STG_GSZ = 8,							// Bit growth per stage
	          ISZ = 10,								// Input word size
	          ASZ = (ISZ + (NUM_STAGES * STG_GSZ)),	// Integrator/Adder word size
	          OSZ = ASZ	                            // Output word size
)
(
	input clk,						// System clock
	input reset,					// System POR
	input ena_out,					// Decimated output rate (2 clks wide)
	input signed [ISZ-1:0] x,	    // Input data
	output signed [OSZ-1:0] y,	    // Output data
	output valid					// Output Valid
);	
	// sign-extend input
	wire signed [ASZ-1:0] x_sx = {{ASZ-ISZ{x[ISZ-1]}},x};

	// Integrators
	reg signed [ASZ-1:0] integrator[0:NUM_STAGES-1];
	always @(posedge clk)
	begin
		if(reset == 1'b1)
		begin
			integrator[0] <= {ASZ{1'b0}};
		end
		else
		begin
			integrator[0] <= integrator[0] + x_sx;
		end
	end
	generate
		genvar i;
		for(i=1;i<NUM_STAGES;i=i+1)
		begin
			always @(posedge clk)
			begin
				if(reset == 1'b1)
                begin
					integrator[i] <= {ASZ{1'b0}};
                end
				else
                begin
					integrator[i] <= integrator[i] + integrator[i-1];
                end
			end
		end
	endgenerate
		
	// Combs
	reg [NUM_STAGES:0] comb_ena;
	reg signed [OSZ-1:0] comb_diff[0:NUM_STAGES];
	reg signed [OSZ-1:0] comb_dly[0:NUM_STAGES];
	always @(posedge clk)
	begin
		if(reset == 1'b1)
		begin
			comb_ena <= {NUM_STAGES+2{1'b0}};
			comb_diff[0] <= {OSZ{1'b0}};
			comb_dly[0] <= {OSZ{1'b0}};
		end
		else
        begin
            if(ena_out == 1'b1)
            begin
                comb_diff[0] <= integrator[NUM_STAGES-1]>>>(ASZ-OSZ);
                comb_dly[0] <= comb_diff[0];
            end
            comb_ena <= {comb_ena[NUM_STAGES:0],ena_out};
        end
	end
	generate
		genvar j;
		for(j=1;j<=NUM_STAGES;j=j+1)
		begin
			always @(posedge clk)
			begin
				if(reset == 1'b1)
				begin
					comb_diff[j] <= {OSZ{1'b0}};
					comb_dly[j] <= {OSZ{1'b0}};
				end
				else if(comb_ena[j-1] == 1'b1)
				begin
					comb_diff[j] <= comb_diff[j-1] - comb_dly[j-1];
					comb_dly[j] <= comb_diff[j];
				end
			end
		end
	endgenerate
	
	// assign output
	assign y = comb_diff[NUM_STAGES];
	assign valid = comb_ena[NUM_STAGES];
endmodule

// ddc_2.v - decimating downconverter V2 - runs at full 40MSPS
// 07-17-16 E. Brombaugh

module ddc_2 #(
    parameter isz = 10,
              fsz = 26,
              osz = 16
)
(
    input clk, reset,
    input [isz-1:0] in,
    input [fsz-1:0] frq,
    input ns_ena,
    output reg valid,
    output reg signed [osz-1:0] i_out, q_out
);
	//------------------------------
    // clock divider
	//------------------------------
    reg [7:0] dcnt;
    reg ena_cic;
    always @(posedge clk)
    begin
        if(reset == 1'b1)
        begin
            dcnt <= 8'b0;
            ena_cic <= 1'b0;
        end
        else
        begin
            dcnt <= dcnt + 1;
            ena_cic <= &dcnt;
       end
    end
    
	//------------------------------
    // convert to signed
	//------------------------------
    reg signed [isz-1:0] ddat;
    always @(posedge clk)
    begin
        if(reset)
        begin
            ddat <= {isz{1'b0}};
        end
        else
        begin
            ddat <= in ^ {1'b1,{isz-1{1'b0}}};
        end
    end
    
	//------------------------------
    // tuner instance
	//------------------------------
    wire signed [isz-1:0] tuner_i, tuner_q;
    tuner_2 #(
        .dsz(isz),
        .fsz(fsz)
    )
    u_tuner(
        .clk(clk),
        .reset(reset),
        .in(ddat),
        .frq(frq),
        .ns_ena(ns_ena),
        .i_out(tuner_i),
        .q_out(tuner_q)
    );
    
    // testing - zero out one of the TDM samples on the tuner output
    //wire signed [isz-1:0] tuner_iq_tst = (tuner_v==1'b1) ? tuner_iq : 10'h000;

//`define FULL_CIC
`ifdef FULL_CIC
    //------------------------------
    // Full precision CICs 
    //------------------------------

    //------------------------------
    // I cic decimator instance
	//------------------------------
    wire cic_v;
    wire signed [32+isz-1:0] cic_i;
    cic_dec_2 #(
        .NUM_STAGES(4),		// Stages of int / comb
	    .STG_GSZ(8),		// Bit growth per stage
	    .ISZ(isz)           // input word size
    )
    u_cic_i(
        .clk(clk),				// System clock
        .reset(reset),			// System POR
        .ena_out(ena_cic),		// Decimated output rate (2 clks wide)
        .x(tuner_i),	        // Input data
        .y(cic_i),	            // Output data
        .valid(cic_v)			// Output Valid
    );	

	//------------------------------
    // Q cic decimator instance
	//------------------------------
    wire signed [32+isz-1:0] cic_q;
    cic_dec_2 #(
        .NUM_STAGES(4),		// Stages of int / comb
	    .STG_GSZ(8),		// Bit growth per stage
	    .ISZ(isz)           // input word size
    )
    u_cic_q(
        .clk(clk),				// System clock
        .reset(reset),			// System POR
        .ena_out(ena_cic),		// Decimated output rate (2 clks wide)
        .x(tuner_q),	        // Input data
        .y(cic_q),	            // Output data
        .valid()			    // Output Valid (unused)
    );	

    // trim cic output to 16 bits
    wire signed [osz-1:0] cic_i_trim = cic_i[32+isz-1:32+isz-osz];
    wire signed [osz-1:0] cic_q_trim = cic_q[32+isz-1:32+isz-osz];
    
`else
    //------------------------------
    // Full precision CICs 
    //------------------------------
    parameter cicsz = 21;
    
    //------------------------------
    // I cic decimator instance
	//------------------------------
    wire cic_v;
    wire signed [cicsz-1:0] cic_i;
    cic_dec_3 #(
        .NUM_STAGES(4),		// Stages of int / comb
	    .STG_GSZ(8),		// Bit growth per stage
	    .ISZ(isz),          // input word size
        .OSZ(cicsz)         // output word size
    )
    u_cic_i(
        .clk(clk),				// System clock
        .reset(reset),			// System POR
        .ena_out(ena_cic),		// Decimated output rate (2 clks wide)
        .x(tuner_i),	        // Input data
        .y(cic_i),	            // Output data
        .valid(cic_v)			// Output Valid
    );	

	//------------------------------
    // Q cic decimator instance
	//------------------------------
    wire signed [cicsz-1:0] cic_q;
    cic_dec_3 #(
        .NUM_STAGES(4),		// Stages of int / comb
	    .STG_GSZ(8),		// Bit growth per stage
	    .ISZ(isz),          // input word size
        .OSZ(cicsz)         // output word size
    )
    u_cic_q(
        .clk(clk),				// System clock
        .reset(reset),			// System POR
        .ena_out(ena_cic),		// Decimated output rate (2 clks wide)
        .x(tuner_q),	        // Input data
        .y(cic_q),	            // Output data
        .valid()			    // Output Valid (unused)
    );	

    // trim cic output to 16 bits
    wire signed [osz-1:0] cic_i_trim = cic_i[cicsz-1:cicsz-osz];
    wire signed [osz-1:0] cic_q_trim = cic_q[cicsz-1:cicsz-osz];    
`endif

    //------------------------------
    // Mux CIC outputs into single stream
 	//------------------------------
    reg cic_v_d, cic_iq_v;
    reg signed [osz-1:0] cic_iq_mux;
    always @(posedge clk)
    begin
        if(reset == 1'b1)
        begin
            cic_v_d <= 1'b0;
            cic_iq_v <= 1'b0;
            cic_iq_mux <= {osz{1'b0}};
        end
        else
        begin
            cic_v_d <= cic_v;
            if((cic_v_d == 1'b0) & (cic_v == 1'b1))
            begin
                cic_iq_v <= 1'b1;
                cic_iq_mux <= cic_i_trim;
            end
            else if((cic_v_d == 1'b1) & (cic_v == 1'b0))
            begin
                cic_iq_v <= 1'b1;
                cic_iq_mux <= cic_q_trim;
            end
            else
            begin
                cic_iq_v <= 1'b0;
            end
        end
    end
    
 	//------------------------------
    // 8x FIR decimator instance
	//------------------------------
    wire fir_v;
    wire signed [osz-1:0] fir_qi;
    fir8dec #(
        .isz(osz),          // input data size
	    .osz(osz)           // output data size
    )
    u_fir(
        .clk(clk),              // System clock
        .reset(reset),          // System POR
        .ena(cic_iq_v),         // New sample available on input
        .iq_in(cic_iq_mux),     // Input data
        .valid(fir_v),          // New output sample ready
        .qi_out(fir_qi)         // Decimated Output data
    );	
	
	//------------------------------
    // demux outputs (fir output order is reversed)
	//------------------------------
    reg p_valid;
    always @(posedge clk)
    begin
        if(reset == 1'b1)
        begin
            p_valid <= 1'b0;
            valid <= 1'b0;
            i_out <= {osz{1'b0}};
            q_out <= {osz{1'b0}};
        end
        else
        begin
            p_valid <= fir_v;
            if((p_valid == 1'b1) & (fir_v == 1'b1))
                i_out <= fir_qi;
            if((p_valid == 1'b0) & (fir_v == 1'b1))
            begin
                q_out <= fir_qi;
                valid <= 1'b1;
            end
            else
                valid <= 1'b0;
        end
    end
    
endmodule


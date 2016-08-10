// fir8dec.v: 8x FIR decimator with TDM I/Q I/O
// 07-17-16 E. Brombaugh

module fir8dec #(
	parameter isz = 16,					// input data size
	          osz = 16,					// output data size
	          psz = 8,					// pointer size
              csz = 16,                 // coeff data size
              clen = 246,               // coeff data length
              agrw = 3                  // accumulator growth
)
(
	input clk,							// System clock
	input reset,						// System POR
	input ena,							// New sample available on input
	input signed [isz-1:0] iq_in,       // Input data
	output reg valid,                   // New output sample ready
	output reg signed [osz-1:0] qi_out  // Output data - reverse order
);	
	
	//------------------------------
    // write address generator
	//------------------------------
    reg [psz:0] w_addr;
    always @(posedge clk)
    begin
        if(reset == 1'b1)
        begin
            w_addr <= {psz+1{1'd0}};
        end
        else
        begin
            if(ena == 1'b1)
            begin
                w_addr <= w_addr + 1;
            end
        end
    end
    
	//------------------------------
    // MAC control state machine
	//------------------------------
    `define sm_wait 3'b000
    `define sm_macq 3'b001
    `define sm_maci 3'b010
    `define sm_dmpq 3'b011
    `define sm_dmpi 3'b100
    
    reg [2:0] state;
    reg mac_ena, dump;
    reg [psz:0] r_addr;
    reg [psz-1:0] c_addr;
    always @(posedge clk)
    begin
        if(reset == 1'b1)
        begin
            state <= `sm_wait;
            mac_ena <= 1'b0;
            dump <= 1'b0;
            r_addr <= {psz+1{1'd0}};
            c_addr <= {psz{1'd0}};
        end
        else
        begin
            case(state)
                `sm_wait :
                begin
                    // halt and hold
                    if(w_addr[3:0] == 4'b1111)
                    begin
                        // start a MAC sequence every 16 entries (8 samples)
                        state <= `sm_macq;
                        mac_ena <= 1'b1;
                        r_addr <= w_addr;
                        c_addr <= {psz{1'd0}};
                    end
                end
                
                `sm_macq :
                begin
                    // Accumulate Q and advance to I
                    state <= `sm_maci;
                    r_addr <= r_addr - 1;
                end
                
                `sm_maci :
                begin
                    // Accumulate I
                    if(c_addr != clen)
                    begin
                        // advance to next coeff
                        state <= `sm_macq;
                        r_addr <= r_addr - 1;
                        c_addr <= c_addr + 1;
                    end
                    else
                    begin
                        // finish mac and advance to dump Q
                        state <= `sm_dmpq;
                        mac_ena <= 1'b0;
                        dump <= 1'b1;
                    end
                end
                
                `sm_dmpq :
                begin
                    // advance to dump 1
                    state <= `sm_dmpi;
                end
                
                `sm_dmpi :
                begin
                    // finish dump and return to wait
                    state <= `sm_wait;
                    dump <= 1'b0;
                end
                
                default :
                begin
                    state <= `sm_wait;
                    mac_ena <= 1'b0;
                    dump <= 1'b0;
                end
            endcase
        end
    end
    
	//------------------------------
    // input buffer memory
	//------------------------------
	reg signed [isz-1:0] buf_mem [511:0];
	reg signed [isz-1:0] r_data;
	always @(posedge clk) // Write memory.
	begin
        if(ena == 1'b1)
        begin
            buf_mem[w_addr] <= iq_in;
        end
	end
    
	always @(posedge clk) // Read memory.
	begin
		r_data <= buf_mem[r_addr];
	end
    
	//------------------------------
    // coeff ROM
	//------------------------------
    reg signed [csz-1:0] coeff_rom[0:255];
    reg signed [csz-1:0] c_data;
    initial
    begin
        $readmemh("../src/fir8dec_coeff.memh", coeff_rom);
    end
    always @(posedge clk)
    begin
        c_data <= coeff_rom[c_addr];
    end
    
	//------------------------------
    // MAC
	//------------------------------
    reg [2:0] mac_ena_pipe;
    reg [2:0] dump_pipe;
    reg signed [csz+isz-1:0] mult;
    reg signed [csz+isz+agrw-1:0] acc_a, acc_b;
    wire signed [csz+isz+agrw-1:0] rnd_const = 1<<(csz+1);
    wire signed [osz-1:0] qi_sat;
    // Saturate accum output
    sat #(.isz(agrw+osz-2), .osz(osz))
        u_sat(.in(acc_b[csz+isz+agrw-1:csz+2]), .out(qi_sat));
    
    always @(posedge clk)
    begin
        if(reset == 1'b1)
        begin
            mac_ena_pipe <= 3'b000;
            dump_pipe <= 3'b000;
            mult <= {csz+isz{1'b0}};
            acc_a <= rnd_const;
            acc_b <= rnd_const;
            valid <= 1'b0;
            qi_out <= {osz{1'b0}};
        end
        else
        begin
            // shift pipes
            mac_ena_pipe <= {mac_ena_pipe[1:0],mac_ena};
            dump_pipe <= {dump_pipe[1:0],dump};

            // multiplier always runs
            mult <= r_data * c_data;
            
            // accumulator
            if(mac_ena_pipe[1] == 1'b1)
            begin
                // two-term accumulate
                acc_a <= acc_b + {{agrw{mult[csz+isz-1]}},mult};
                acc_b <= acc_a;
            end
            else
            begin
                // clear to round constant
                acc_a <= rnd_const;
                acc_b <= acc_a;
            end
            
            // output
            if(dump_pipe[1] == 1'b1)
            begin
                qi_out <= qi_sat;
            end
            
            // valid
            valid <= dump_pipe[1];
        end
    end    
    
endmodule
	

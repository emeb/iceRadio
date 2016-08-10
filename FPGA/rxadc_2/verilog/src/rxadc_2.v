// lattice ice5lp4k rxadc v2
// 07-17-16 E. Brombaugh

module rxadc_2 #(
    parameter isz = 10,
              fsz = 26,
              dsz = 16
)
(
	// SPI slave port
	input SPI_CSL,
	input SPI_MOSI,
	output SPI_MISO,
	input SPI_SCLK,
    
	// rxadc board interface
	input rxadc_clk,
	output rxadc_dfs,
	input rxadc_otr,
	input [9:0] rxadc_dat,
    
	// I2S DAC output
	output dac_mclk,
	output dac_sdout,
	output dac_sclk,
	output dac_lrck,
    
    // I2S MCU interface
    input mcu_sdin,
    output mcu_sdout,
    output mcu_sclk,
    output mcu_lrck,

	// RGB output
	output wire o_red,
	output wire o_green,
	output wire o_blue
);
    
	// This should be unique so firmware knows who it's talking to
	parameter DESIGN_ID = 32'h2ADC0003;

	//------------------------------
	// Instantiate ADC clock buffer
	//------------------------------
	wire adc_clk;
	SB_GB clk_gbuf(
		.USER_SIGNAL_TO_GLOBAL_BUFFER(!rxadc_clk),
		.GLOBAL_BUFFER_OUTPUT(adc_clk)
	);

    //------------------------------
	// Instantiate HF Osc with div 1
	//------------------------------
	wire clk;
	SB_HFOSC #(.CLKHF_DIV("0b00")) OSCInst0 (
		.CLKHFEN(1'b1),
		.CLKHFPU(1'b1),
		.CLKHF(clk)
	) /* synthesis ROUTE_THROUGH_FABRIC= 0 */;
	
	//------------------------------
	// reset generators
	//------------------------------
	reg [3:0] reset_pipe = 4'hf;
	reg reset = 1'b1;
	always @(posedge clk)
	begin
		reset <= |reset_pipe;
		reset_pipe <= {reset_pipe[2:0],1'b0};
	end
	
	reg [3:0] adc_reset_pipe = 4'hf;
	reg adc_reset = 1'b1;
	always @(posedge adc_clk)
	begin
		adc_reset <= |adc_reset_pipe;
		adc_reset_pipe <= {adc_reset_pipe[2:0],1'b0};
	end
	
	//------------------------------
	// Internal SPI slave port
	//------------------------------
	wire [31:0] wdat;
	reg [31:0] rdat;
	wire [6:0] addr;
	wire re, we, spi_slave_miso;
	spi_slave
		uspi(.clk(clk), .reset(reset),
			.spiclk(SPI_SCLK), .spimosi(SPI_MOSI),
			.spimiso(SPI_MISO), .spicsl(SPI_CSL),
			.we(we), .re(re), .wdat(wdat), .addr(addr), .rdat(rdat));
	
	//------------------------------
	// Writeable registers
	//------------------------------
	reg [13:0] cnt_limit_reg;
	reg [9:0] dpram_raddr;
	reg dpram_trig;
	reg [25:0] ddc_frq;
    reg dac_mux_sel;
    reg ddc_ns_ena;
	always @(posedge clk)
		if(reset)
		begin
			cnt_limit_reg <= 14'd2499;	// 1/4 sec blink rate
			dpram_raddr <= 10'h000;		// read address
			dpram_trig <= 1'b0;
            ddc_frq <= 26'h0;
			dac_mux_sel <= 1'b0;
			ddc_ns_ena <= 1'b0;
		end
		else if(we)
			case(addr)
				7'h01: cnt_limit_reg <= wdat;
				7'h02: dpram_raddr <= wdat;
				7'h03: dpram_trig <= wdat;
				7'h10: ddc_frq <= wdat;
                7'h11: dac_mux_sel <= wdat;
                7'h12: ddc_ns_ena <= wdat;
			endcase
	
	//------------------------------
	// readable registers
	//------------------------------
	reg dpram_wen;
	reg [10:0] dpram_rdat;
	always @(*)
		case(addr)
			7'h00: rdat = DESIGN_ID;
			7'h01: rdat = cnt_limit_reg;
			7'h02: rdat = dpram_raddr;
			7'h03: rdat = {dpram_raddr,dpram_rdat};
			7'h04: rdat = dpram_wen;
			7'h10: rdat = ddc_frq;
			7'h11: rdat = dac_mux_sel;
			7'h12: rdat = ddc_ns_ena;
			default: rdat = 32'd0;
		endcase
	
	//------------------------------
	// register ADC input data
	//------------------------------
	reg [9:0] dpram_waddr;
	reg [10:0] dpram_wdat;
	always @(posedge adc_clk)
		dpram_wdat <= {rxadc_otr,rxadc_dat};
	
	//------------------------------
	// 1k x 11 dual-port RAM
	//------------------------------
	reg [10:0] mem [1023:0];
	always @(posedge adc_clk) // Write memory.
	begin
		if(dpram_wen)
            mem[dpram_waddr] <= dpram_wdat;
	end
    
	always @(posedge clk) // Read memory.
		dpram_rdat <= mem[dpram_raddr];

	//------------------------------
	// write state machine - runs from 40MHz clock
	//------------------------------
	reg [2:0] dpram_trig_sync;
	always @(posedge adc_clk)
		if(adc_reset)
		begin
			dpram_waddr <= 10'h000;
			dpram_trig_sync <= 3'b000;
			dpram_wen <= 1'b0;
		end
		else
		begin
			dpram_trig_sync <= {dpram_trig_sync[1:0],dpram_trig};
			
			if(~dpram_wen)
			begin
				if(dpram_trig_sync[2])
					dpram_wen <= 1'b1;
				
				dpram_waddr <= 10'h000;
			end
			else
			begin
				if(dpram_waddr == 10'h3ff)
					dpram_wen <= 1'b0;
				
				dpram_waddr <= dpram_waddr + 1;
			end
		end
        
	//------------------------------
	// DDC instance
	//------------------------------
    wire signed [dsz-1:0] ddc_i, ddc_q;
    wire ddc_v;
    ddc_2 #(
        .isz(isz),
        .fsz(fsz),
        .osz(dsz)
    )
    u_ddc(
        .clk(adc_clk), .reset(adc_reset),
        .in(dpram_wdat[isz-1:0]),
        .frq(ddc_frq),
        .ns_ena(ddc_ns_ena),
        .valid(ddc_v),
        .i_out(ddc_i), .q_out(ddc_q)
    );
        
	//------------------------------
	// Strap ADC Data Format for 2's comp
	//------------------------------
	assign rxadc_dfs = 1'b0;
    
	//------------------------------
    // handoff between ddc and i2s
	//------------------------------
	reg signed [dsz-1:0] ddc_i_l, ddc_q_l;
    wire audio_ena;
	always @(posedge adc_clk)
		if(adc_reset)
		begin
			ddc_i_l <= 16'h0000;
			ddc_q_l <= 16'h0000;
		end
		else
		begin
			if(audio_ena)
			begin
                ddc_i_l <= ddc_i;
                ddc_q_l <= ddc_q;
			end
		end
        
	//------------------------------
	// I2S serializer
	//------------------------------
    wire sdout;
	i2s_out
		ui2s(.clk(adc_clk), .reset(adc_reset),
			.l_data(ddc_i_l), .r_data(ddc_q_l),
			.mclk(dac_mclk), .sdout(sdout), .sclk(dac_sclk), .lrclk(dac_lrck),
			.load(audio_ena));
	
	// Hook up I2S to DAC and MCU I2S pins
	assign mcu_lrck = dac_lrck;
	assign mcu_sclk = dac_sclk;
	assign dac_sdout = (dac_mux_sel == 1'b1) ? mcu_sdin : sdout;
	assign mcu_sdout = sdout;
	
	//------------------------------
	// Instantiate LF Osc
	//------------------------------
	wire CLKLF;
	SB_LFOSC OSCInst1 (
		.CLKLFEN(1'b1),
		.CLKLFPU(1'b1),
		.CLKLF(CLKLF)
	) /* synthesis ROUTE_THROUGH_FABRIC= 0 */;
	
	//------------------------------
	// Divide the clock
	//------------------------------
	reg [13:0] clkdiv;
	reg onepps;
	always @(posedge CLKLF)
	begin		
		if(clkdiv == 14'd0)
		begin
			onepps <= 1'b1;
			clkdiv <= cnt_limit_reg;
		end
		else
		begin
			onepps <= 1'b0;
			clkdiv <= clkdiv - 14'd1;
		end
	end
	
	//------------------------------
	// LED signals
	//------------------------------
	reg [2:0] state;
	always @(posedge CLKLF)
	begin
		if(onepps)
			state <= state + 3'd1;
	end
	
	//------------------------------
	// Instantiate RGB DRV 
	//------------------------------
	wire red_pwm_i = state[0];
	wire grn_pwm_i = state[1];
	wire blu_pwm_i = state[2];
	SB_RGB_DRV RGB_DRIVER (
	   .RGBLEDEN  (1'b1), // Enable current for all 3 RGB LED pins
	   .RGB0PWM   (red_pwm_i), // Input to drive RGB0 - from LEDD HardIP
	   .RGB1PWM   (grn_pwm_i), // Input to drive RGB1 - from LEDD HardIP
	   .RGB2PWM   (blu_pwm_i), // Input to drive RGB2 - from LEDD HardIP
	   .RGBPU     (led_power_up_i), //Connects to LED_DRV_CUR primitive
	   .RGB0      (o_red), 
	   .RGB1      (o_green),
	   .RGB2      (o_blue)
	);
	defparam RGB_DRIVER.RGB0_CURRENT = "0b000111";
	defparam RGB_DRIVER.RGB1_CURRENT = "0b000111";
	defparam RGB_DRIVER.RGB2_CURRENT = "0b000111";

	//------------------------------
	// Instantiate LED CUR DRV 
	//------------------------------
	SB_LED_DRV_CUR LED_CUR_inst (
		.EN    (1'b1), //Enable to supply reference current to the LED drivers
		.LEDPU (led_power_up_i) //Connects to SB_RGB_DRV primitive
	);

endmodule

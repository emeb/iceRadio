// tb_top.v: top-level testbench for FM generator in Lattice iCE40-Ultra
// 2016-06-02 E. Brombaugh

module tb_rxadc;
	// SPI slave port
	reg SPI_CSL;
	reg SPI_MOSI;
	wire SPI_MISO;
	reg SPI_SCLK;
	
    // rxadc board interface
    reg rxadc_clk;
    wire rxadc_dfs;
    reg rxadc_otr;
    reg [9:0] rxadc_dat;
    
	// DAC I2S output
	wire dac_mclk;
	wire dac_sdout;
	wire dac_sclk;
	wire dac_lrck;

	// MCU I2S output
	reg mcu_sdin;
	wire mcu_sdout;
	wire mcu_sclk;
	wire mcu_lrck;

	// RGB output
	wire o_red;
	wire o_green;
	wire o_blue;
    
    // ADC NCO source
    real rxadc_phs;
    
	// spi shift
	reg [39:0] sr;
	reg [31:0] read_data;
	
	// spi transaction task
	task spi_rxtx
		(
			input rw,
			input [6:0] addr,
			input [31:0] wdata
		);
		begin: spi_task
			
			sr = {rw,addr,wdata};
			SPI_CSL = 1'b0;
			SPI_SCLK = 1'b0;
			SPI_MOSI = sr[39];
			
			repeat(40)
			begin
				#100
				SPI_SCLK = 1'b1;
				#100
				SPI_SCLK = 1'b0;
				sr = {sr[38:0],SPI_MISO};
				SPI_MOSI = sr[39];
			end
			
			#100
			SPI_CSL = 1'b1;
			#100
			read_data = sr[31:0];
		end
	endtask
		
	rxadc
		uut(
			// SPI slave port
			.SPI_CSL(SPI_CSL),
			.SPI_MOSI(SPI_MOSI),
			.SPI_MISO(SPI_MISO),
			.SPI_SCLK(SPI_SCLK),
			
            // rxadc board interface
            .rxadc_clk(rxadc_clk),
            .rxadc_dfs(rxadc_dfs),
            .rxadc_otr(rxadc_otr),
            .rxadc_dat(rxadc_dat),
    
            // I2S DAC output
            .dac_mclk(dac_mclk),
            .dac_sdout(dac_sdout),
            .dac_sclk(dac_sclk),
            .dac_lrck(dac_lrck),

            // I2S MCU interface
            .mcu_sdin(mcu_sdin),
            .mcu_sdout(mcu_sdout),
            .mcu_sclk(mcu_sclk),
            .mcu_lrck(mcu_lrck),

			// RGB output
			.o_red(o_red),
			.o_green(o_green),
			.o_blue(o_blue)
		);

	// test setup
	initial
    begin
		// initial SPI setting
		SPI_CSL = 1'b1;
		SPI_MOSI = 1'b0;
		SPI_SCLK = 1'b0;
        
        // initialize RXADC
        rxadc_clk = 1'b0;
        rxadc_otr = 1'b0;
        
        // initialize MCU SDIN
        mcu_sdin = 1'b0;
        
		// wait for chip to init
		#4000
		
		// read ID
		spi_rxtx(1'b1, 7'd00, 32'd0);
		
		// write params - assume the defaults are good
		spi_rxtx(1'b0, 7'd1, 32'd1000); // cnt_reg
		spi_rxtx(1'b0, 7'h10, 32'h00001000); // freq
		spi_rxtx(1'b0, 7'h11, 32'd0); // DAC mux
		spi_rxtx(1'b0, 7'd3, 32'd1); // trig
		spi_rxtx(1'b0, 7'd3, 32'd0); // untrig
	end
    
    // ADC clock source
    always
        #12.5 rxadc_clk <= ~rxadc_clk;
    
    // ADC data source
    always @(posedge rxadc_clk)
    begin
        rxadc_phs <= rxadc_phs + 3*3.14159/64;
        //rxadc_dat <= 10'd512 + 511*$sin(rxadc_phs);
        rxadc_dat <= 10'd950;
    end
    
endmodule
	

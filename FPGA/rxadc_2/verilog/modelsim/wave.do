# wave.do: Modelsim do file to load waves up for fm debug

add wave -noupdate -divider tb_rxadc_2
add wave -noupdate /tb_rxadc_2/SPI_CSL
add wave -noupdate /tb_rxadc_2/SPI_MOSI
add wave -noupdate /tb_rxadc_2/SPI_MISO
add wave -noupdate /tb_rxadc_2/SPI_SCLK
add wave -noupdate /tb_rxadc_2/rxadc_clk
add wave -noupdate /tb_rxadc_2/rxadc_dfs
add wave -noupdate /tb_rxadc_2/rxadc_otr
add wave -noupdate /tb_rxadc_2/rxadc_phs
add wave -noupdate /tb_rxadc_2/dac_mclk
add wave -noupdate /tb_rxadc_2/dac_sdout
add wave -noupdate /tb_rxadc_2/dac_sclk
add wave -noupdate /tb_rxadc_2/dac_lrck
add wave -noupdate /tb_rxadc_2/mcu_sdin
add wave -noupdate /tb_rxadc_2/mcu_sdout
add wave -noupdate /tb_rxadc_2/mcu_sclk
add wave -noupdate /tb_rxadc_2/mcu_lrck
add wave -noupdate -radix unsigned /tb_rxadc_2/rxadc_dat
#add wave -noupdate /tb_rxadc_2/o_red
#add wave -noupdate /tb_rxadc_2/o_green
#add wave -noupdate /tb_rxadc_2/o_blue
add wave -noupdate /tb_rxadc_2/sr
add wave -noupdate -radix hexadecimal /tb_rxadc_2/read_data

add wave -noupdate -divider rxadc
add wave -noupdate /tb_rxadc_2/uut/SPI_CSL
add wave -noupdate /tb_rxadc_2/uut/SPI_MOSI
add wave -noupdate /tb_rxadc_2/uut/SPI_MISO
add wave -noupdate /tb_rxadc_2/uut/SPI_SCLK
add wave -noupdate /tb_rxadc_2/uut/rxadc_clk
add wave -noupdate /tb_rxadc_2/uut/rxadc_dfs
add wave -noupdate /tb_rxadc_2/uut/rxadc_otr
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/rxadc_dat
add wave -noupdate /tb_rxadc_2/uut/dac_mclk
add wave -noupdate /tb_rxadc_2/uut/dac_sdout
add wave -noupdate /tb_rxadc_2/uut/dac_sclk
add wave -noupdate /tb_rxadc_2/uut/dac_lrck
add wave -noupdate /tb_rxadc_2/uut/mcu_sdin
add wave -noupdate /tb_rxadc_2/uut/mcu_sdout
add wave -noupdate /tb_rxadc_2/uut/mcu_sclk
add wave -noupdate /tb_rxadc_2/uut/mcu_lrck
#add wave -noupdate /tb_rxadc_2/uut/o_red
#add wave -noupdate /tb_rxadc_2/uut/o_green
#add wave -noupdate /tb_rxadc_2/uut/o_blue
add wave -noupdate /tb_rxadc_2/uut/adc_clk
add wave -noupdate /tb_rxadc_2/uut/clk
add wave -noupdate /tb_rxadc_2/uut/reset_pipe
add wave -noupdate /tb_rxadc_2/uut/reset
add wave -noupdate /tb_rxadc_2/uut/adc_reset_pipe
add wave -noupdate /tb_rxadc_2/uut/adc_reset
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/wdat
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/rdat
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/addr
add wave -noupdate /tb_rxadc_2/uut/re
add wave -noupdate /tb_rxadc_2/uut/we
add wave -noupdate /tb_rxadc_2/uut/spi_slave_miso
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/cnt_limit_reg
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/dpram_raddr
add wave -noupdate /tb_rxadc_2/uut/dpram_trig
add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/ddc_frq
#add wave -noupdate /tb_rxadc_2/uut/dac_mux_sel
#add wave -noupdate /tb_rxadc_2/uut/ddc_ns_ena
#add wave -noupdate /tb_rxadc_2/uut/dpram_wen
#add wave -noupdate -radix unsigned /tb_rxadc_2/uut/dpram_rdat
#add wave -noupdate -radix unsigned /tb_rxadc_2/uut/dpram_waddr
#add wave -noupdate -radix unsigned /tb_rxadc_2/uut/dpram_wdat
#add wave -noupdate -radix unsigned /tb_rxadc_2/uut/mem
#add wave -noupdate /tb_rxadc_2/uut/dpram_trig_sync
add wave -noupdate /tb_rxadc_2/uut/ddc_v
add wave -noupdate -radix decimal /tb_rxadc_2/uut/ddc_i
add wave -noupdate -radix decimal /tb_rxadc_2/uut/ddc_q
add wave -noupdate /tb_rxadc_2/uut/audio_ena
add wave -noupdate -radix decimal /tb_rxadc_2/uut/ddc_i_l
add wave -noupdate -radix decimal /tb_rxadc_2/uut/ddc_q_l
#add wave -noupdate /tb_rxadc_2/uut/CLKLF
#add wave -noupdate /tb_rxadc_2/uut/clkdiv
#add wave -noupdate /tb_rxadc_2/uut/onepps
#add wave -noupdate /tb_rxadc_2/uut/state
#add wave -noupdate /tb_rxadc_2/uut/red_pwm_i
#add wave -noupdate /tb_rxadc_2/uut/grn_pwm_i
#add wave -noupdate /tb_rxadc_2/uut/blu_pwm_i
#add wave -noupdate /tb_rxadc_2/uut/led_power_up_i

add wave -noupdate -divider ddc
add wave -noupdate /tb_rxadc_2/uut/u_ddc/clk
add wave -noupdate /tb_rxadc_2/uut/u_ddc/reset
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/u_ddc/in
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/u_ddc/frq
add wave -noupdate /tb_rxadc_2/uut/u_ddc/valid
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/i_out
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/q_out
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/u_ddc/dcnt
add wave -noupdate /tb_rxadc_2/uut/u_ddc/ena_cic
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/ddat
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/tuner_i
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/tuner_q
add wave -noupdate /tb_rxadc_2/uut/u_ddc/cic_v
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/cic_i
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/cic_q
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/cic_i_trim
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/cic_q_trim
add wave -noupdate /tb_rxadc_2/uut/u_ddc/cic_iq_v
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/cic_iq_mux
add wave -noupdate /tb_rxadc_2/uut/u_ddc/fir_v
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/fir_qi
add wave -noupdate /tb_rxadc_2/uut/u_ddc/p_valid

add wave -noupdate -divider tuner
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_tuner/clk
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_tuner/reset
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_tuner/in
add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/u_ddc/u_tuner/frq
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_tuner/ns_ena
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_tuner/i_out
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_tuner/q_out
add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/u_ddc/u_tuner/acc
add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/u_ddc/u_tuner/ns_acc
add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/u_ddc/u_tuner/phs
add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/u_ddc/u_tuner/res

#add wave -noupdate -divider tuner_slice
#add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_tuner/clk
#add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_tuner/reset
#add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_tuner/ena
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_tuner/in
#add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/u_ddc/u_tuner/frq
#add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_tuner/valid
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_tuner/out
#add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/u_ddc/u_tuner/acc
#add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/u_ddc/u_tuner/p_quad
#add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/u_ddc/u_tuner/quad
#add wave -noupdate -radix hexadecimal /tb_rxadc_2/uut/u_ddc/u_tuner/addr
#add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_tuner/sincos_sign
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_tuner/sincos_raw
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_tuner/sincos_p
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_tuner/sincos
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_tuner/mult

#add wave -noupdate -divider cic_dec
#add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_cic/clk
#add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_cic/reset
#add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_cic/ena_out
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_cic/iq_in
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_cic/iq_out
#add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_cic/valid
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_cic/iq_in_sx
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_cic/integrator_a
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_cic/integrator_b
#add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_cic/comb_ena
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_cic/comb_diff
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_cic/comb_dly_a
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_cic/comb_dly_b

add wave -noupdate -divider fir4dec
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_fir/clk
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_fir/reset
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_fir/ena
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_fir/iq_in
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_fir/valid
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_fir/qi_out
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/u_ddc/u_fir/w_addr
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_fir/state
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_fir/mac_ena
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_fir/dump
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/u_ddc/u_fir/r_addr
add wave -noupdate -radix unsigned /tb_rxadc_2/uut/u_ddc/u_fir/c_addr
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_fir/buf_mem
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_fir/r_data
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_fir/coeff_rom
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_fir/c_data
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_fir/mac_ena_pipe
add wave -noupdate /tb_rxadc_2/uut/u_ddc/u_fir/dump_pipe
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_fir/mult
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_fir/acc_a
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_fir/acc_b
add wave -noupdate -radix decimal /tb_rxadc_2/uut/u_ddc/u_fir/qi_sat

#add wave -noupdate -divider i2s_out
#add wave -noupdate /tb_rxadc_2/uut/ui2s/clk
#add wave -noupdate /tb_rxadc_2/uut/ui2s/reset
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/ui2s/l_data
#add wave -noupdate -radix decimal /tb_rxadc_2/uut/ui2s/r_data
#add wave -noupdate /tb_rxadc_2/uut/ui2s/mclk
#add wave -noupdate /tb_rxadc_2/uut/ui2s/sdout
#add wave -noupdate /tb_rxadc_2/uut/ui2s/sclk
#add wave -noupdate /tb_rxadc_2/uut/ui2s/lrclk
#add wave -noupdate /tb_rxadc_2/uut/ui2s/load
#add wave -noupdate /tb_rxadc_2/uut/ui2s/mclk_ena
#add wave -noupdate /tb_rxadc_2/uut/ui2s/scnt
#add wave -noupdate /tb_rxadc_2/uut/ui2s/p_sclk
#add wave -noupdate /tb_rxadc_2/uut/ui2s/sreg
#add wave -noupdate /tb_rxadc_2/uut/ui2s/lrcnt
#add wave -noupdate /tb_rxadc_2/uut/ui2s/sclk_p0

#add wave -noupdate -divider clkgen
#add wave -noupdate /tb_rxadc_2/uut/ui2s/uclk/clk
#add wave -noupdate /tb_rxadc_2/uut/ui2s/uclk/reset
#add wave -noupdate /tb_rxadc_2/uut/ui2s/uclk/mclk
#add wave -noupdate /tb_rxadc_2/uut/ui2s/uclk/mclk_ena
#add wave -noupdate /tb_rxadc_2/uut/ui2s/uclk/rate
#add wave -noupdate /tb_rxadc_2/uut/ui2s/uclk/cnt_mask
#add wave -noupdate /tb_rxadc_2/uut/ui2s/uclk/mclk_cnt
#add wave -noupdate /tb_rxadc_2/uut/ui2s/uclk/rate_cnt


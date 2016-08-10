# create the work directory
vlib work

# compile the design
vlog -work work -f comp.f -timescale "1 ns / 1 ps"
	

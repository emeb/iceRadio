# run.do: Modelsim do file to execute the ddc testbench

# compile the design
do comp.do

# simulate the design
do load.do

# add some waves
do wave.do

# run it
run 100us
#run 1ms
#run 100ms

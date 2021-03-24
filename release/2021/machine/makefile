all:
	iverilog -g2005-sv -DICARUS=1 tb.v core/cpu.v core/alu.v -o tb.qqq
	vvp tb.qqq >> /dev/null

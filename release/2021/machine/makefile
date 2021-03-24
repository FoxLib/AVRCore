all:
	iverilog -g2005-sv -DICARUS=1 tb.v avrcpu.v -o tb.qqq
	vvp tb.qqq >> /dev/null

OSCILLATOR = va
PROJECT = mo2_$(OSCILLATOR)

UCXXSRC = macro-oscillator2.cc \
	eurorack/plaits/dsp/engine/virtual_analog_engine.cc \
	eurorack/stmlib/dsp/units.cc

include makefile.inc

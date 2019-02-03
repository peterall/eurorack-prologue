OSCILLATOR = wtd
PROJECT = mo2_$(OSCILLATOR)

UCXXSRC = macro-oscillator2.cc \
	eurorack/plaits/dsp/engine/wavetable_engine.cc \
	eurorack/plaits/resources.cc \
	eurorack/stmlib/dsp/units.cc

include makefile.inc

OSCILLATOR = string
PROJECT = mo2_$(OSCILLATOR)

UCXXSRC = macro-oscillator2.cc \
	eurorack/plaits/dsp/engine/string_engine.cc \
	eurorack/plaits/dsp/physical_modelling/string_voice.cc \
	eurorack/plaits/dsp/physical_modelling/string.cc \
	eurorack/plaits/resources.cc \
	eurorack/stmlib/dsp/units.cc \
	eurorack/stmlib/utils/random.cc

include makefile.inc

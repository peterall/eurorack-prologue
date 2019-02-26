OSCILLATOR = modal
PROJECT = mo2_$(OSCILLATOR)

UCXXSRC = macro-oscillator2.cc \
	eurorack/plaits/dsp/engine/modal_engine.cc \
	eurorack/plaits/dsp/physical_modelling/modal_voice.cc \
	eurorack/plaits/dsp/physical_modelling/resonator.cc \
	eurorack/plaits/resources.cc \
	eurorack/stmlib/dsp/units.cc \
	eurorack/stmlib/utils/random.cc 

UCSRC = logue-sdk/platform/ext/CMSIS/CMSIS/DSP_Lib/Source/SupportFunctions/arm_fill_f32.c

include makefile.inc

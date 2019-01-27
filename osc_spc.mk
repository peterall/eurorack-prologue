OSCILLATOR = spc

UCXXSRC = macro-oscillator2.cc \
	eurorack/plaits/dsp/engine/speech_engine.cc \
	eurorack/plaits/dsp/speech/lpc_speech_synth.cc \
	eurorack/plaits/dsp/speech/lpc_speech_synth_controller.cc \
	eurorack/plaits/dsp/speech/lpc_speech_synth_phonemes.cc \
	eurorack/plaits/dsp/speech/lpc_speech_synth_words.cc \
	eurorack/plaits/dsp/speech/naive_speech_synth.cc \
	eurorack/plaits/dsp/speech/sam_speech_synth.cc \
	eurorack/plaits/resources.cc \
	eurorack/stmlib/dsp/units.cc \
	eurorack/stmlib/utils/random.cc

include makefile.inc

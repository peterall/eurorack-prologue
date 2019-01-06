TOPTARGETS := all clean

OSCILLATORS := $(wildcard *mk)

$(TOPTARGETS): $(OSCILLATORS)
$(OSCILLATORS):
	@rm -fR .dep ./build
	@$(MAKE) -f $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(OSCILLATORS)

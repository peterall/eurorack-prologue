TOPTARGETS := all clean

OSCILLATORS := $(wildcard *mk)

$(TOPTARGETS): $(OSCILLATORS) package
$(OSCILLATORS):
	@rm -fR .dep ./build
	@$(MAKE) -f $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(OSCILLATORS)

package:
	@echo Packaging to ./mo2_prologue.zip
	@rm -f mo2_prologue.zip
	@mkdir mo2_prologue
	@cp -a *.prlgunit mo2_prologue/
	@cp -a credits.txt mo2_prologue/
	@zip -rq9m mo2_prologue.zip mo2_prologue/

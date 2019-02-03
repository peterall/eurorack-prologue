TOPTARGETS := all clean

OSCILLATORS := $(wildcard *mk)

$(TOPTARGETS): $(OSCILLATORS) package
$(OSCILLATORS):
	@rm -fR .dep ./build
	@$(MAKE) -f $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(OSCILLATORS)

PACKAGE=eurorack_prologue

package:
	@echo Packaging to ./${PACKAGE}.zip
	@rm -f ${PACKAGE}.zip
	@mkdir ${PACKAGE}
	@cp -a *.prlgunit ${PACKAGE}/
	@cp -a credits.txt ${PACKAGE}/
	@zip -rq9m ${PACKAGE}.zip ${PACKAGE}/

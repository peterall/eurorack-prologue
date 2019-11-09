TOPTARGETS := all clean

OSCILLATORS := $(wildcard *mk)

VERSION=1.5-0

$(TOPTARGETS): $(OSCILLATORS) package_prologue package_minilogue-xd package_nutekt-digital
$(OSCILLATORS):
	@rm -fR .dep ./build
	@PLATFORM=prologue VERSION=$(VERSION) $(MAKE) -f $@ $(MAKECMDGOALS)
	@rm -fR .dep ./build
	@PLATFORM=minilogue-xd VERSION=$(VERSION) $(MAKE) -f $@ $(MAKECMDGOALS)
	@rm -fR .dep ./build
	@PLATFORM=nutekt-digital VERSION=$(VERSION) $(MAKE) -f $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(OSCILLATORS)

PROLOGUE_PACKAGE=eurorack_prologue
MINILOGUE_XD_PACKAGE=eurorack_minilogue-xd
NUTEKT_DIGITAL_PACKAGE=eurorack_nutekt-digital

package_prologue:
	@echo Packaging to ./${PROLOGUE_PACKAGE}.zip
	@rm -f ${PROLOGUE_PACKAGE}.zip
	@rm -rf ${PROLOGUE_PACKAGE}
	@mkdir ${PROLOGUE_PACKAGE}
	@cp -a *.prlgunit ${PROLOGUE_PACKAGE}/
	@cp -a credits.txt ${PROLOGUE_PACKAGE}/
	@zip -rq9m ${PROLOGUE_PACKAGE}.zip ${PROLOGUE_PACKAGE}/

package_minilogue-xd:
	@echo Packaging to ./${MINILOGUE_XD_PACKAGE}.zip
	@rm -f ${MINILOGUE_XD_PACKAGE}.zip
	@rm -rf ${MINILOGUE_XD_PACKAGE}
	@mkdir ${MINILOGUE_XD_PACKAGE}
	@cp -a *.mnlgxdunit ${MINILOGUE_XD_PACKAGE}/
	@cp -a credits.txt ${MINILOGUE_XD_PACKAGE}/
	@zip -rq9m ${MINILOGUE_XD_PACKAGE}.zip ${MINILOGUE_XD_PACKAGE}/

package_nutekt-digital:
	@echo Packaging to ./${NUTEKT_DIGITAL_PACKAGE}.zip
	@rm -f ${NUTEKT_DIGITAL_PACKAGE}.zip
	@rm -rf ${NUTEKT_DIGITAL_PACKAGE}
	@mkdir ${NUTEKT_DIGITAL_PACKAGE}
	@cp -a *.ntkdigunit ${NUTEKT_DIGITAL_PACKAGE}/
	@cp -a credits.txt ${NUTEKT_DIGITAL_PACKAGE}/
	@zip -rq9m ${NUTEKT_DIGITAL_PACKAGE}.zip ${NUTEKT_DIGITAL_PACKAGE}/

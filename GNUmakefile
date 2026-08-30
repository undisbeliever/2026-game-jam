
.DELETE_ON_ERROR:
.SUFFIXES:

SFC_BASENAME  := game


# Format: <name>.<format>:<palette>
IMAGES := \
  title-screen-fg.4bpp:title-screen

PALETTES :=


SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h) src/gen-enums.h

LLVM_CA65_OBJ := $(patsubst src/llvm-mos/%.ca65.s,out/llvm-mos/%.o, $(wildcard src/llvm-mos/*.s))

VBCC_SOURCES := $(wildcard src/vbcc/*.s src/vbcc/*.c)


.PHONY: all directories resources jcc816 llvm-mos vbcc


all: resources llvm-mos vbcc jcc816


ALL_RESOURCES :=

define IMAGE_template =
  ALL_RESOURCES += out/resources/images/$(1) out/resources/images/$(basename $(1)).map out/resources/palettes/$(2).bin
  out/resources/images/$(1): IMG_PALETTE_SRC=resources/palettes/$(2).png
endef
$(foreach i,$(IMAGES), $(eval $(call IMAGE_template,$(firstword $(subst :, ,$i)),$(lastword $(subst :, ,$i)))))

ALL_RESOURCES += $(patsubst %,out/resources/palettes/%.bin,$(PALETTES))


resources: directories $(ALL_RESOURCES)

out/resources/images/%.2bpp out/resources/images/%.map &: resources/images/%.png $(IMG_PALETTE_SRC) tools/image2snes.py tools/_snes.py
	python3 tools/image2snes.py -f 2bpp --tileset-output out/resources/images/$*.4bpp --tilemap-output out/resources/images/$*.map resources/images/$*.png $(IMG_PALETTE_SRC)

out/resources/images/%.4bpp out/resources/images/%.map &: resources/images/%.png $(IMG_PALETTE_SRC) tools/image2snes.py tools/_snes.py
	python3 tools/image2snes.py -f 4bpp --tileset-output out/resources/images/$*.4bpp --tilemap-output out/resources/images/$*.map resources/images/$*.png $(IMG_PALETTE_SRC)

out/resources/images/%.8bpp out/resources/images/%.map &: resources/images/%.png $(IMG_PALETTE_SRC) tools/image2snes.py tools/_snes.py
	python3 tools/image2snes.py -f 8bpp --tileset-output out/resources/images/$*.4bpp --tilemap-output out/resources/images/$*.map resources/images/$*.png $(IMG_PALETTE_SRC)


out/resources/palettes/%.bin: resources/palettes/%.png tools/png2palette.py tools/_snes.py
	python3 tools/png2palette.py -o out/resources/palettes/$*.bin resources/palettes/$*.png


# jcc816 does not support relative includes.
# I cannot get it to read a `out/enums.h` file.
# For now, I'll create the file in the src directory
# ::TODO switch to `out/enums.h` after figuring out how to load it in jcc816::
src/gen-enums.h: resources/resources.txt tools/gen-enum-headers.py
	python3 tools/gen-enum-headers.py -o "$@" resources/resources.txt


llvm-mos: directories out/$(SFC_BASENAME)-llvm-mos.sfc
vbcc: directories out/$(SFC_BASENAME)-vbcc.sfc
jcc816: directories out/$(SFC_BASENAME)-jcc816.sfc



out/llvm-mos/%.o: src/llvm-mos/%.ca65.s
	ca65 -o '$@' '$<'

out/llvm-mos/$(SFC_BASENAME).rom out/$(SFC_BASENAME)-llvm-mos.sym &: $(SOURCES) $(HEADERS) $(LLVM_CA65_OBJ) cfg/llvm-mos-lorom.ld
	mos-common-clang -mcpu=mosw65816 -g -Os -T cfg/llvm-mos-lorom.ld -flto -fnonreentrant -ffast-math -funroll-loops -finline-functions -fomit-frame-pointer -fno-stack-protector -fdata-sections -ffunction-sections -lexit-loop -o '$@' $(LLVM_CA65_OBJ) $(SOURCES)
	# Create a symbol file from the elf debugging information
	nm $@.elf | awk '$$2 != "t" { print $$1, $$3 }' >| out/$(SFC_BASENAME)-llvm-mos.sym

out/$(SFC_BASENAME)-llvm-mos.sfc: out/llvm-mos/$(SFC_BASENAME).rom out/$(SFC_BASENAME)-llvm-mos.sym $(ALL_RESOURCES) tools/insert-resources.py
	python3 tools/insert-resources.py --lorom -o '$@' out/llvm-mos/$(SFC_BASENAME).rom out/$(SFC_BASENAME)-llvm-mos.sym resources/resources.txt out/resources



out/vbcc/$(SFC_BASENAME).rom out/$(SFC_BASENAME)-vbcc.sym &: $(VBCC_SOURCES) $(SOURCES) $(HEADERS)
	vc +snes-hi -O4 -maxoptpasses=300 -inline-depth=1000 -unroll-all -force-statics -range-opt '--symfmt %06x\ %s' '--symfile out/$(SFC_BASENAME)-vbcc.sym' -o '$@' $(VBCC_SOURCES) $(SOURCES)

out/$(SFC_BASENAME)-vbcc.sfc: out/vbcc/$(SFC_BASENAME).rom out/$(SFC_BASENAME)-vbcc.sym $(ALL_RESOURCES) tools/insert-resources.py
	python3 tools/insert-resources.py --hirom -o '$@' out/vbcc/$(SFC_BASENAME).rom out/$(SFC_BASENAME)-vbcc.sym resources/resources.txt out/resources



out/jcc816/$(SFC_BASENAME).rom out/$(SFC_BASENAME)-jcc816.dbg &: $(SOURCES) $(HEADERS) cfg/jcc816.xml
	java -jar "$(JCC816_DIR)/JCC816.jar" -V1 -O2 -D2 -r=src -l='out/jcc816/$(SFC_BASENAME)'=../cfg/jcc816.xml $(patsubst src/%,%,$(SOURCES))
	mv out/jcc816/$(SFC_BASENAME).sfc out/jcc816/$(SFC_BASENAME).rom
	mv out/jcc816/$(SFC_BASENAME).dbg out/$(SFC_BASENAME)-jcc816.dbg

out/$(SFC_BASENAME)-jcc816.sfc: out/jcc816/$(SFC_BASENAME).rom out/$(SFC_BASENAME)-jcc816.dbg $(ALL_RESOURCES) tools/insert-resources.py
	python3 tools/insert-resources.py --lorom -o '$@' out/jcc816/$(SFC_BASENAME).rom out/$(SFC_BASENAME)-jcc816.dbg resources/resources.txt out/resources



DIRECTORIES := out/jcc816/ out/llvm-mos/ out/vbcc/ out/resources/ out/resources/images/ out/resources/palettes/
directories: $(DIRECTORIES)

$(DIRECTORIES):
  ifeq ($(OS),Windows_NT)
	mkdir $(subst /,\,$@)
  else
	mkdir -p $(DIRECTORIES)
  endif


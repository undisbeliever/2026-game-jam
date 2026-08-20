
.DELETE_ON_ERROR:
.SUFFIXES:

SFC_FILENAME  := game.sfc

SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)

LLVM_CA65_OBJ := $(patsubst src/llvm-mos/%.ca65.s,out/llvm-mos/%.o, $(wildcard src/llvm-mos/*.s))

VBCC_SOURCES := $(wildcard src/vbcc/*.s src/vbcc/*.c)

.PHONY: all directories jcc816 llvm-mos vbcc


all: llvm-mos vbcc jcc816

llvm-mos: directories out/llvm-mos/$(SFC_FILENAME)
vbcc: directories out/vbcc/$(SFC_FILENAME)
jcc816: directories out/jcc816/$(SFC_FILENAME)


out/llvm-mos/$(SFC_FILENAME): $(SOURCES) $(HEADERS) $(LLVM_CA65_OBJ) cfg/llvm-mos-lorom.ld
	mos-common-clang -mcpu=mosw65816 -g -Os -T cfg/llvm-mos-lorom.ld -flto -fnonreentrant -ffast-math -funroll-loops -finline-functions -fomit-frame-pointer -fno-stack-protector -fdata-sections -ffunction-sections -lexit-loop -o '$@' $(LLVM_CA65_OBJ) $(SOURCES)
	python3 tools/write-sfc-checksum.py --lorom "$@"

out/llvm-mos/%.o: src/llvm-mos/%.ca65.s
	ca65 -o '$@' '$<'


out/vbcc/$(SFC_FILENAME): $(VBCC_SOURCES) $(SOURCES) $(HEADERS)
	vc +snes-hi -O4 -maxoptpasses=300 -inline-depth=1000 -unroll-all -force-statics -range-opt '--symfmt %06x\ %s' '--symfile $(patsubst %.sfc,%.sym,$@)' -o '$@' $(VBCC_SOURCES) $(SOURCES)
	python3 tools/write-sfc-checksum.py --hirom "$@"


out/jcc816/$(SFC_FILENAME): $(SOURCES) $(HEADERS) cfg/jcc816.xml
	java -jar "$(JCC816_DIR)/JCC816.jar" -V1 -O2 -r=src -l='$@'=../cfg/jcc816.xml $(patsubst src/%,%,$(SOURCES))
	python3 tools/write-sfc-checksum.py --lorom "$@"



DIRECTORIES := out/jcc816/ out/llvm-mos/ out/vbcc/
directories: $(DIRECTORIES)

$(DIRECTORIES):
  ifeq ($(OS),Windows_NT)
	mkdir $(subst /,\,$@)
  else
	mkdir -p $(DIRECTORIES)
  endif




# makefile


OUTPUT      := build/z86

CSRC        += $(shell find source -iname "*.c")
CXXSRC      += $(shell find source -iname "*.cpp")

COBJ        = $(CSRC:.c=.c.o)
CDEPS       = $(COBJ:.o=.d)

CXXOBJ      = $(CXXSRC:.cpp=.cpp.o)
CXXDEPS     = $(CXXOBJ:.o=.d)

CFLAGS      := -std=c11
CXXFLAGS    := -std=c++17 -fno-exceptions

OPTS        = -O0 -g
DEFINES     =
WARNINGS    = -Wall -Wextra -Werror=return-type -Wno-unused-parameter -Wno-unused-private-field

PRECOMP_HDR = include/precompile.h
PRECOMP_GCH = $(PRECOMP_HDR:.h=.h.gch)

INCLUDES    = -Iinclude
SANITISERS  = # -fsanitize=undefined


.DEFAULT_GOAL = all


.PHONY: all run
.PRECIOUS: $(PRECOMP_GCH)


build: all

all: $(OUTPUT)
	@mkdir -p build

run: all rom/rom.bin
	@nasm -f bin -o build/test.bin build/test.asm
	@build/z86 --rom rom/rom.bin --program build/test.bin

rom/rom.bin: rom/rom.asm
	@nasm -f bin -o rom/rom.bin rom/rom.asm


$(OUTPUT): $(COBJ) $(CXXOBJ)
	@$(CXX) $(CXXFLAGS) $(SANITISERS) -o $@ $(COBJ) $(CXXOBJ)

%.c.o: %.c makefile
	@echo "  $(notdir $<)"
	@$(CC) $(CFLAGS) $(SANITISERS) $(WARNINGS) $(DEFINES) $(INCLUDES) $(OPTS) -c -MMD -MP -o $@ $<

%.cpp.o: %.cpp makefile $(PRECOMP_GCH)
	@echo "  $(notdir $<)"
	@$(CXX) $(CXXFLAGS) $(SANITISERS) $(WARNINGS) $(DEFINES) $(INCLUDES) $(OPTS) -c -include $(PRECOMP_HDR) -MMD -MP -o $@ $<

%.h.gch: %.h makefile
	@echo "  $(notdir $<)"
	@$(CXX) $(CXXFLAGS) $(SANITISERS) $(WARNINGS) $(DEFINES) $(INCLUDES) $(OPTS) -MMD -MP -x c++-header -o $@ $<

-include $(PRECOMP_HDR:.h=.h.d)
-include $(CDEPS)
-include $(CXXDEPS)

clean:
	@find . -name "*.o" -delete
	@find . -name "*.h.d" -delete
	@find . -name "*.cpp.d" -delete
	@find . -name "*.h.gch" -delete
	@rm -f $(OUTPUT)









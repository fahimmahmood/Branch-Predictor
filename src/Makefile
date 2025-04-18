# Compiler and flags
CC = g++
OPT = -O0
WARN = -Wall
CFLAGS = $(OPT) $(WARN) -std=c++20 \
    -I../LLBP \
    -I../LLBP/bpmodels \
    -I../LLBP/bpmodels/llbp \
    -I../LLBP/bpmodels/tage \
    -I../LLBP/bpmodels/components \
    -I../LLBP/btb \
    -I../LLBP/utils

# LLBP source files
LLBP_SRC = \
    ../LLBP/bpmodels/base_predictor.cc \
    ../LLBP/bpmodels/llbp/llbp.cc \
    ../LLBP/bpmodels/tage/tage.cc \
    ../LLBP/bpmodels/tage/tage_scl.cc \
    ../LLBP/btb/btb.cc \
    ../LLBP/utils/common.cc \
    ../LLBP/utils/fileutils.cc

LLBP_OBJ = $(LLBP_SRC:.cc=.o)

# Attack source files and corresponding binaries
SIMS = \
    custom_spectre_attack \
    llbp_spectre_attack \
    improved_spectre_attack

BINARIES = $(SIMS:%=%_sim)

# Default target
all: $(BINARIES)
	@echo "---------------------------------------------------------------------------------------------------------------"
	@echo "Build complete! Run binaries like:"
	@echo "  ./llbp_spectre_attack      <cache hit threshold> <# attempts per byte> <# training iterations> <# executions>"
	@echo "  ./custom_spectre_attack    <cache hit threshold> <# attempts per byte> <# training iterations> <# executions>"
	@echo "  ./improved_spectre_attack  <cache hit threshold> <# attempts per byte> <# training iterations> <# executions>"
	@echo "     Default sim values:     <cache hit threshold = 40 ticks> <# attempts per byte = 2500> <# training iterations = 16> <# executions = 1>"
	@echo "---------------------------------------------------------------------------------------------------------------"

# Pattern rule to build each binary
%_sim: %.o $(LLBP_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lz

# Compile rules
.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cc.o:
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rules
clean:
	rm -f *.o $(BINARIES) $(LLBP_OBJ)

clobber:
	rm -f *.o
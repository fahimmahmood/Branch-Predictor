# Compiler and flags
CC = g++
OPT = -O0
WARN = -w # The LLBP repo throws lots of warnings with -Wall

#		This will hold if LLBP repo installed at specified location v
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
#		This will hold if LLBP repo installed at specified location ^

LLBP_OBJ = $(LLBP_SRC:.cc=.o)

# Source file for main simulator
MAIN_SRC = spectre_simulator.cpp
MAIN_OBJ = $(MAIN_SRC:.cpp=.o)

# Output binary name
TARGET = spectre_sim

# Default target
all: $(TARGET)
	@echo "---------------------------------------------------------------------------------------------------------------"
	@echo "Build complete! Run the binary like:"
	@echo "  ./$(TARGET) <predictor> <cache hit threshold> <# attempts per byte> <# training iterations> <improved training (y/n)> <# runs>"
	@echo "     Default Usage: ./$(TARGET) <smith> <40> <2500> <16> <1>"
	@echo "     List of Predictors:\n- (s)mith\n- (b)imodal\n- (g)share\n- (h)ybrid\n- (t)age\n- (l)lbp\n"
	@echo "---------------------------------------------------------------------------------------------------------------"

# Link everything into final binary
$(TARGET): $(MAIN_OBJ) $(LLBP_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lz

# Compile rules
.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

.cc.o:
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rules
clean:
	rm -f *.o $(LLBP_OBJ) $(TARGET)

clobber:
	rm -f *.o
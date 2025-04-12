# Compiler and flags
CC = g++
OPT = -O3
WARN = -Wall
CFLAGS = $(OPT) $(WARN)

# Source files
SRC = spectre_attack.cpp bimodal_predictor.cpp gshare.cpp
OBJ = $(SRC:.cpp=.o)

# Output binary
BIN = sim

# Default target
all: $(BIN)
	@echo "Build complete. \
	Run with ./sim bimodal <m> <trace_file> <spectre_attack_to_initiate_attack> or \
	./sim gshare <n> <m> <trace_file> <spectre_attack_to_initiate_attack>"

# Build rule
$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

# Generic rule to compile .cpp to .o
.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and binary
clean:
	rm -f *.o $(BIN)

# Remove only .o files
clobber:
	rm -f *.o

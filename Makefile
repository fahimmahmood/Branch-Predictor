CC = g++
OPT = -O0
WARN = -Wall
STD = -std=c++11
CFLAGS = $(OPT) $(WARN) $(STD)

# List all your .cpp files here (source files, excluding header files)
SIM_SRC = spectre_attack.cpp

# List corresponding compiled object files here (.o files)
SIM_OBJ = spectre_attack.o
#################################

# default rule

all: sim
	@echo "Nothing to do."


# rule for making sim

sim: $(SIM_OBJ)
	$(CC) -o spectre_sim $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH SIM-----------"


# generic rule for converting any .cpp file to any .o file
 
.cpp.o:
	$(CC) $(CFLAGS)  -c $*.cpp


# type "make clean" to remove all .o files plus the sim binary

clean:
	rm -f *.o sim


# type "make clobber" to remove all .o files (leaves sim binary)

clobber:
	rm -f *.o




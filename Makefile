CC = g++
OPT = -O3
WARN = -Wall
CFLAGS = $(OPT) $(WARN)

# List all your .cpp files here (source files, excluding header files)
SIM_SRC = bimodal.cpp

# List corresponding compiled object files here (.o files)
SIM_OBJ = bimodal.o
#################################

# default rule

all: sim
	@echo "my work is done here..."


# rule for making sim

sim: $(SIM_OBJ)
	$(CC) -o sim $(CFLAGS) $(SIM_OBJ) -lm
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




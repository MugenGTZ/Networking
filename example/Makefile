# Compiler
CC = g++

# Compiler flags
CFLAGS = -g -Wall

# Linker flags
LDFLAGS = -lm ../network.a ../src/libnetcrypt.a -lpthread

# source files
NETSRC = main.cpp

TARGET = protocolTest

# Compile everything.
all: $(TARGET)

# Define object files.
NETOBJ = $(NETSRC:.cpp=.o)

# Link program
$(TARGET): $(NETOBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compile:
%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

# rule for cleaning files
clean:
	rm -f $(TARGET) $(NETOBJ)

rebuild:	clean all

.PHONY: all rebuild clean


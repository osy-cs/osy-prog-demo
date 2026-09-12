##############################################################################
#
# Simple Makefile example
#
# Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2026
#
# This Makefile will initiate compilation of all *.cpp files
# in current directory into individual executables.
#
##############################################################################

# Source files *.cpp can be changed to list of individual files
SOURCES = $(wildcard *.cpp)
OBJS = $(SOURCES:%.cpp=%.o)
TARGETS = $(SOURCES:%.cpp=%)

CPPFLAGS += -g -pthread -Wall -Werror
LDFLAGS += -g -pthread
LDLIBS += -lrt

.PHONY: all clean

all: $(TARGETS)

$(TARGETS): %: %.o
	g++ $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -rf *.o $(TARGETS)


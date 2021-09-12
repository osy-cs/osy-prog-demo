#############################################################################
#
# Program example for subject Operating Systems
#
# Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2021
#
# Example of using a static library
#
#############################################################################

TARGET = lib_test
LIBNAME = modules
LIBFILE = lib$(LIBNAME).a

CPPFLAGS += -g
LDFLAGS += -L. 
LDLIBS += -l$(LIBNAME)

all: $(TARGET)

# clean binary files
clean:
	rm -rf $(TARGET) $(LIBFILE) *.o

# creating static library 
$(LIBFILE): module_1.o module_2.o
	@echo "\nCreating static library...\n"
	#rm -f $@
	ar r $@ $^
	@echo "\nStatic library created: '$(LIBFILE)'.\n"

# creating application
$(TARGET): $(TARGET).cpp $(LIBFILE)
	@echo "\nCreating application...\n"
	g++ $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@
	@echo "\nApplication '$(TARGET)' is ready.\n"


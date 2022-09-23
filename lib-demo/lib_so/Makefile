#############################################################################
#
# Program example for labs in subject Operating Systems
#
# Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2017
#
# Example of using a dynamic library
#
#############################################################################

TARGET  = lib_test
LIBNAME = modules
LIBFILE = lib$(LIBNAME).so

CPPFLAGS  += -g 
LDFLAGS += -L. 
LDLIBS += -l$(LIBNAME)

ifeq ($(shell uname -m),x86_64) 
 CPPFLAGS += -fPIC
endif

all: $(TARGET)

# clean binary files
clean:
	rm -rf $(TARGET) $(LIBFILE) *.o

# creating dynamic library 
$(LIBFILE): module_1.o module_2.o
	@echo "\nCreating dynamic library...\n"
	g++ $(CPPFLAGS) $^ -shared $(LDFLAGS) -o $@
	@echo "\nDynamic library created: '$(LIBFILE)'.\n"

# creating application
$(TARGET): $(TARGET).cpp $(LIBFILE)
	@echo "\nCreating application...\n"
	g++ $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@
	@echo "\nApplication '$(TARGET)' is ready.\n"
	@echo "Set path to library: 'export LD_LIBRARY_PATH=.' !\n"



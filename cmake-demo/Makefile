##############################################################################
#
# Simple Makefile example for usage with cmake. 
#
# This Makefile will initiate cmake in ./build directory and then call make.
#
##############################################################################

BUILD_DIR=build
COMPDB=compile_commands.json

all: cmake $(COMPDB)

cmake:
	cmake . -B$(BUILD_DIR)
	make -C $(BUILD_DIR)

$(COMPDB): 
	ln -s $(BUILD_DIR)/$(COMPDB) $(COMPDB)

clean:
	make -C $(BUILD_DIR) clean
	rm -rf $(BUILD_DIR) $(COMPDB)


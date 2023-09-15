.PHONY: all release debug clean run console

BUILD_DIR ?= build
ROOT_DIR = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

all: release

release:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo $(ROOT_DIR)
	$(MAKE) -C $(BUILD_DIR)

debug:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug $(ROOT_DIR)
	$(MAKE) -C $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

run: debug
	arm-none-eabi-gdb -x openocd.gdb build/firmware.elf

console:
	minicom -D /dev/ttyACM0 -b 115200

openocd:
	openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -s tcl

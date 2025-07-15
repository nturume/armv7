
ifeq ($(V), 1)
V_AT =
else
V_AT = @
endif


ifeq ($(T), 1)
CFLAGS = -Wall -Wextra -lc -std=c++20 -DTESTING -fno-strict-aliasing
else
CFLAGS = -Wall -Wextra -lc -std=c++20 -fno-strict-aliasing -Wpacked -Wpadded
endif



TARGET_EXEC := build/bin
BUILD_DIR := ./build
SRC_DIRS := ./src
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
HEADERS := $(shell find $(SRC_DIRS) -name '*.hpp')
CPPCODE := $(shell find $(SRC_DIRS) -name '*.*pp')
ASM := $(shell find $(SRC_DIRS) -name '*.s')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
CPPFLAGS := $(INC_FLAGS) -MMD -MP
CXX := g++

TOOLCHAIN := /home/m/Desktop/opt/arm/bin/arm-linux-gnueabi-

$(TARGET_EXEC): $(OBJS)
	@echo "Linking $@"
	$(V_AT)$(CXX) -g $(OBJS) -o $@ $(LDFLAGS) $(CFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp $(HEADERS) 
	@echo "Compiling $<"
	$(V_AT)mkdir -p $(dir $@)
	$(V_AT)$(CXX) -g -O3 $(CPPFLAGS) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/elf: $(ASM) $(SRC_DIRS)/l.ld
	$(V_AT)$(TOOLCHAIN)as $(SRC_DIRS)/prog.s -o $(BUILD_DIR)/a.out
	$(V_AT)$(TOOLCHAIN)ld $(BUILD_DIR)/a.out -o $(BUILD_DIR)/elf -T$(SRC_DIRS)/l.ld
	$(V_AT)rm $(BUILD_DIR)/a.out
	
$(BUILD_DIR)/c.elf: $(SRC_DIRS)/prog.c $(SRC_DIRS)/l.ld
	$(V_AT)$(TOOLCHAIN)gcc -g -mcpu=cortex-a7 $(SRC_DIRS)/prog.c -o $(BUILD_DIR)/c.elf -lc -static

clean:
	$(V_AT)rm -rf build/* *.iso
	$(V_AT)rm -rf doc/doxygen

run: $(TARGET_EXEC) $(BUILD_DIR)/c.elf
	$(V_AT)$(TARGET_EXEC)

fmt:
	clang-format -i $(CPPCODE)
	

.PHONY: all doc clean

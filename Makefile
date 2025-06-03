
ifeq ($(V), 1)
V_AT =
else
V_AT = @
endif

# Compiler settings
CFLAGS = -Wall -Wextra -lc

TARGET_EXEC := build/bin
BUILD_DIR := ./build
SRC_DIRS := ./src
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
CPPFLAGS := $(INC_FLAGS) -MMD -MP
CC := g++

$(TARGET_EXEC): $(OBJS)
	@echo "Linking $@"
	$(V_AT)$(CXX) -g $(OBJS) -o $@ $(LDFLAGS) $(CFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	@echo "Compiling $<"
	$(V_AT)mkdir -p $(dir $@)
	$(V_AT)$(CC) -g $(CPPFLAGS) -c $< -o $@ $(CFLAGS)

assemble:
	arm-none-eabi-as $(SRC_DIRS)/prog.s -o $(BUILD_DIR)/a.out
	arm-none-eabi-ld $(BUILD_DIR)/a.out -o $(BUILD_DIR)/elf
	rm $(BUILD_DIR)/a.out

clean:
	$(V_AT)rm -rf build/* *.iso
	$(V_AT)rm -rf doc/doxygen

run: $(TARGET_EXEC)
	$(V_AT)$(TARGET_EXEC)

.PHONY: all doc clean

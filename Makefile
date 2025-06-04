
ifeq ($(V), 1)
V_AT =
else
V_AT = @
endif


ifeq ($(T), 1)
CFLAGS = -Wall -Wextra -lc -DTESTING
else
CFLAGS = -Wall -Wextra -lc
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
CC := g++

$(TARGET_EXEC): $(OBJS)
	@echo "Linking $@"
	$(V_AT)$(CXX) -g $(OBJS) -o $@ $(LDFLAGS) $(CFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp $(HEADERS) 
	@echo "Compiling $<"
	$(V_AT)mkdir -p $(dir $@)
	$(V_AT)$(CC) -g $(CPPFLAGS) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/elf: $(ASM) $(SRC_DIRS)/l.ld
	$(V_AT)arm-none-eabi-as $(SRC_DIRS)/prog.s -o $(BUILD_DIR)/a.out
	$(V_AT)arm-none-eabi-ld $(BUILD_DIR)/a.out -o $(BUILD_DIR)/elf -T$(SRC_DIRS)/l.ld
	$(V_AT)rm $(BUILD_DIR)/a.out

clean:
	$(V_AT)rm -rf build/* *.iso
	$(V_AT)rm -rf doc/doxygen

run: $(TARGET_EXEC) $(BUILD_DIR)/elf
	$(V_AT)$(TARGET_EXEC)

fmt:
	clang-format -i $(CPPCODE)
	

.PHONY: all doc clean

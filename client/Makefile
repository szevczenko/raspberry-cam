TARGET_EXEC ?= a.out

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)\
			include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?=  $(INC_FLAGS) -MMD -MP -g

OPENCV = `pkg-config opencv --cflags --libs`

LIBS ?= -lpthread  

LIB_PATH += -L/usr/local/lib

LIBS_CPP = $(LIBS) $(OPENCV)



$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) $(LIBS) $(LIBS_CPP)  -o $@ $(LDFLAGS) 

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< $(LIBS) -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< $(LIBS) -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< $(LIBS_CPP) $(LIB_PATH) -o $@


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

echo: 
	echo $(LIBS_CPP)

-include $(DEPS)

MKDIR_P ?= mkdir -p
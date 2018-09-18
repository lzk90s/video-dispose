TOP_DIR=../../../../../

MODULE_NAME=libseemmo_wrapper
MODULE_SUFFIX=.so

OUT_PKG_DIR=$(TOP_DIR)/pkg
OUT_OBJ_DIR = $(TOP_DIR)/obj
TARGET=$(OUT_PKG_DIR)/$(MODULE_NAME)$(MODULE_SUFFIX)

CXX = g++
AR = ar

VENDOR_DIR=$(TOP_DIR)/algo/gosun/vendor

INC = -I$(VENDOR_DIR) -I$(TOP_DIR) -I$(TOP_DIR)/thirdparty -I$(SEEMMO_SDK_HOME)/include
LIB = -L$(TOP_DIR) -L$(SEEMMO_SDK_HOME)/lib -L$(OPENCV_HOME)/lib -lexport_sdk -lpthread


CXX_FLAGS = $(INC) -std=c++11 -fPIC
LD_FLAGS = $(LIB)

ifdef DEBUG
CXX_FLAGS += -g
endif

SRC_SUFFIX=.cc
SRC_PATH=$(shell find . -type f -name "*$(SRC_SUFFIX)")
OBJS = $(addprefix $(OUT_OBJ_DIR)/,$(subst $(SRC_SUFFIX),.o,$(SRC_PATH)))


MAKE_PKG_DIR := $(shell mkdir -p $(OUT_PKG_DIR))
MAKE_OBJECT_DIR := $(foreach f, $(OBJS), $(shell mkdir -p $(dir $f)))

default: $(TARGET)

$(OUT_PKG_DIR)/$(MODULE_NAME): $(OBJS)
	$(CXX) -o $@ $^ $(CXX_FLAGS) $(LD_FLAGS) 

$(OUT_PKG_DIR)/$(MODULE_NAME).a: $(OBJS)
	$(AR) rcs $@ $^

$(OUT_PKG_DIR)/$(MODULE_NAME).so: $(OBJS)
	$(CXX) -shared -fPIC -o $@ $^ $(CXX_FLAGS) $(LD_FLAGS) 

$(OUT_OBJ_DIR)/%.o : %$(SRC_SUFFIX) $(OUT_OBJ_DIR)/%$(SRC_SUFFIX).d
	$(CXX) -o $@ -c $< $(CXX_FLAGS) 

DEPS = $(OBJS:.o=$(SRC_SUFFIX).d)
$(DEPS) : $(OUT_OBJ_DIR)/%$(SRC_SUFFIX).d : %$(SRC_SUFFIX)
	$(CXX) $(CXX_FLAGS) $< -MM > $(OUT_OBJ_DIR)/$<.d

#install: $(TARGET)
#	@mkdir -p $(TOP_DIR)/pkg
#	@cp $(TARGET) $(OUT_PKG_DIR) -rf
#	@cp resource/* $(OUT_PKG_DIR) -rf

clean:
	@rm -rf $(OUT_OBJ_DIR)
	@rm -rf $(TARGET)

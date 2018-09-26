TOP_DIR=../

MODULE_NAME=libnotify
MODULE_SUFFIX=.so

INSTALL_PREFIX=/usr/local
OUT_PKG_DIR=$(TOP_DIR)/pkg
OUT_OBJ_DIR = $(TOP_DIR)/obj
TARGET=$(OUT_PKG_DIR)/$(MODULE_NAME)$(MODULE_SUFFIX)

CXX = g++
AR = ar

INC =-I$(TOP_DIR) -I$(TOP_DIR)/thirdparty -I$(TOP_DIR)/common 
LIB = -L$(TOP_DIR)/pkg -lpthread  -lstdc++ 

CXX_FLAGS = $(INC) -std=c++11 -fPIC
LD_FLAGS = $(LIB)

#ifdef DEBUG
CXX_FLAGS += -g -O0
#endif

SRC_SUFFIX=.cc
SRC_PATH=$(shell find . -type f -name "*$(SRC_SUFFIX)")
OBJS = $(addprefix $(OUT_OBJ_DIR)/,$(subst $(SRC_SUFFIX),.o,$(SRC_PATH)))



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

install: $(TARGET)
	@mkdir -p $(TOP_DIR)/pkg
	cp $(TARGET) $(INSTALL_PREFIX)/lib -rf
	cp vfilter.h $(INSTALL_PREFIX)/include	-f

clean:
	@rm -rf $(OUT_OBJ_DIR)
	@rm -rf $(TARGET)

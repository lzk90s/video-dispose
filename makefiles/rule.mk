ifeq (x$(TOP_DIR), x)
$(error TOP_DIR must be set at first)
endif

OUT_PKG_DIR=$(TOP_DIR)/pkg
OUT_OBJ_DIR = $(TOP_DIR)/obj
TARGET=$(OUT_PKG_DIR)/$(MODULE_NAME)$(MODULE_SUFFIX)
INSTALL_PREFIX=/usr/local

CXX = g++
AR = ar

#CXXFLAGS+=-fvisibility=hidden

ifdef DEBUG
CXXFLAGS += -g -O0
else
CXXFLAGS += -O3
endif

ifeq (x$(SRC_DIRS), x)
#$(error SRC_DIRS must be set)
SRC_DIRS=.
endif

SRC_SUFFIX=.cc
SRC_PATH=$(shell find $(SRC_DIRS) -type f -name "*$(SRC_SUFFIX)")
OBJS = $(addprefix $(OUT_OBJ_DIR)/,$(subst $(SRC_SUFFIX),.o,$(SRC_PATH)))


MAKE_PKG_DIR := $(shell mkdir -p $(OUT_PKG_DIR))
MAKE_OBJECT_DIR := $(foreach f, $(OBJS), $(shell mkdir -p $(dir $f)))

default: $(TARGET)

$(OUT_PKG_DIR)/$(MODULE_NAME): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) 

$(OUT_PKG_DIR)/$(MODULE_NAME).a: $(OBJS)
	$(AR) rcs $@ $^

$(OUT_PKG_DIR)/$(MODULE_NAME).so: $(OBJS)
	$(CXX) -shared -fPIC -o $@ $^ $(CXXFLAGS) $(LDFLAGS) 

$(OUT_OBJ_DIR)/%.o : %$(SRC_SUFFIX) $(OUT_OBJ_DIR)/%$(SRC_SUFFIX).d
	$(CXX) -o $@ -c $< $(CXXFLAGS) 

DEPS = $(OBJS:.o=$(SRC_SUFFIX).d)
$(DEPS) : $(OUT_OBJ_DIR)/%$(SRC_SUFFIX).d : %$(SRC_SUFFIX)
	$(CXX) $(CXXFLAGS) $< -MM > $(OUT_OBJ_DIR)/$<.d


ifeq (x$(MODULE_SUFFIX), x)
INSTALL_DIR=$(INSTALL_PREFIX)/bin
else
INSTALL_DIR=$(INSTALL_PREFIX)/lib
endif
install: $(TARGET)
	@mkdir -p $(TOP_DIR)/pkg
	\cp -rf $(TARGET) $(INSTALL_DIR)

clean:
	@rm -rf $(OBJS)
	@rm -rf $(TARGET)


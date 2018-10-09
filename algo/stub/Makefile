TOP_DIR=../../

MODULE_NAME=libalgo_stub
MODULE_SUFFIX=.so

INC = -I$(TOP_DIR) -I$(TOP_DIR)/thirdparty 
LIB = -L$(TOP_DIR) -L$(TOP_DIR)/pkg -lseemmo_stub_impl -lgosun_stub_impl

CXXFLAGS = $(INC) -std=c++11 -fPIC
LDFLAGS = $(LIB)

SRC_DIRS=.

include $(TOP_DIR)/makefiles/rule.mk

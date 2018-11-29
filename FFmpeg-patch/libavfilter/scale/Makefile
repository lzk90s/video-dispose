TOP_DIR=../../../

MODULE_NAME=libyuv_scale
MODULE_SUFFIX=.so

INC =-I$(TOP_DIR) -I$(TOP_DIR)/thirdparty -I$(TOP_DIR)/common
LIB = -L$(TOP_DIR)/pkg -lyuv 

CXXFLAGS = $(INC) -std=c++11 -fPIC -Wall
LDFLAGS = $(LIB)

include $(TOP_DIR)/makefiles/rule.mk

TOP_DIR=../

MODULE_NAME=libvideo_filter
MODULE_SUFFIX=.so

INC =-I$(TOP_DIR) -I$(TOP_DIR)/thirdparty -I$(TOP_DIR)/common -I$(TOP_DIR)/thirdparty/libyuv/include `pkg-config --cflags opencv freetype2`
#LIB = -L$(TOP_DIR)/pkg `pkg-config --libs freetype2 opencv` -lpthread -ldl -lstdc++ -lopencv_core -lopencv_imgproc -lippicv -lalgo_stub -lturbojpeg
LIB = -L$(TOP_DIR)/pkg `pkg-config --libs freetype2 opencv` -lpthread -ldl -lstdc++ -littnotify -lippicv -lalgo_stub -lturbojpeg -lyuv

CXXFLAGS = $(INC) -std=c++11 -fPIC -Wall
LDFLAGS = $(LIB)

include $(TOP_DIR)/makefiles/rule.mk

install_font:
	cp -rf fonts/* /usr/share/fonts/truetype/

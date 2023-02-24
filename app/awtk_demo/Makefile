include $(NVR_ROOT_PATH)/Makefile.param

BINNAME = awtk_demo

OBJDIR = ./obj
INSTALL_DIR = $(NVR_ROOT_PATH)/opt/bin

#CC_OPTS += -Werror
CC_OPTS += -Wall -fPIC -std=c++11
ifeq ($(ENABLE_DEBUG),yes)
	CC_OPTS += -g
endif

CC_OPTS += $(GLOBAL_CC_OPTS)

#3rd
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/glog/include
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/tcli/include
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/onvif
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/base64
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/log
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/ffmpeg/include
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/md5

#AWTK
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/awtk/include/src
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/awtk/include/src/ext_widgets

#MW
INC_DIR += -I$(SDK_PATH)/middleware/include

#HDMI
INC_DIR += -I$(NVR_ROOT_PATH)/opt/lt9611/include

#AI
ifeq ($(ENABLE_CVIAI),yes)
	INC_DIR += -I$(NVR_ROOT_PATH)/ai/include
	INC_DIR += -I$(NVR_ROOT_PATH)/ai/include/cviai
endif

#FRAMEWORK
INC_DIR += -I$(NVR_ROOT_PATH)/common
INC_DIR += -I$(NVR_ROOT_PATH)/basesvc
INC_DIR += -I$(NVR_ROOT_PATH)/media/include

INC_DIR += -I./res
INC_DIR += -I./src/base
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/awtk/include/src/base
INC_DIR += -I./src/ui
INC_DIR += -I./src/awtk-port

CC_OPTS += $(INC_DIR) -DWITH_WIDGET_TYPE_CHECK=1

LD_OPTS += -L$(SDK_PATH)/middleware/lib
LD_OPTS += -L$(SDK_PATH)/middleware/lib/3rd
LD_OPTS += -L$(NVR_ROOT_PATH)/opt/lib
ifeq ($(ENABLE_CVIAI),yes)
	LD_OPTS += -L$(NVR_ROOT_PATH)/opt/ai_lib
endif

LD_OPTS += -lz -lglog -lts

LD_OPTS += -lawtk -ltkc -lawtk_port
LD_OPTS += -lpthread
LD_OPTS += -lts -lzflog
LD_OPTS += -lnvr_lt9611

#FRAMEWORK
LD_OPTS += -lnvr_common -lnvr_basesvc -lnvr_media

#ffmpeg
LD_OPTS += -lavcodec -lavfilter -lavformat -lavutil -lswscale -lswresample

LD_OPTS += -lglog -lzflog

#MW LIB

#if base sdk 2.2.0
#LD_OPTS += -lvpu -lini -lisp -lae -lvenc -lcvi_jpeg -lvdec -lcvitracer -lcvi_vcodec -lawb -lm -ldl -lsys -lcvi_audio -lcvi_RES1 -lcvi_vqe -ltinyalsa -lcvi_VoiceEngine -lmipi_tx
#if base sdk 1.4.0
LD_OPTS += -lvpu -lini -lisp -lae -lvenc -lcvi_jpeg -lvdec -lcvitracer -lcvi_vcodec -lawb -lm -ldl -lsys -lcvi_audio -lcvi_RES1 -lcvi_vqe -ltinyalsa -lcvi_VoiceEngine -lmipi_tx
#if enable asan
#LD_OPTS += -fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address -lasan
ifeq ($(ENABLE_CVIAI),yes)
	LD_OPTS +=  -lnvr_ai -lcviai -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lcnpy -lcvikernel -lcvimath -lcviruntime -lsns_full -lnanomsg -lcvi_ive_tpu -laf
endif
SRC_DIR += ./src/base
SRC_DIR += ./src/ui

VPATH = $(SRC_DIR)

SRC_CPP = $(shell find $(SRC_DIR) -maxdepth 1 -name '*.cpp')

OBJ_CPP_0 = $(notdir $(patsubst %.cpp, %.o, $(SRC_CPP)))

OBJ_CPP    = $(foreach file, $(OBJ_CPP_0), $(OBJDIR)/$(file))

DEPS_CPP   = $(OBJ_CPP:%.o=%.d)

.PHONY: all setup awtk_port target install strip clean

all: setup target install strip

setup:
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(OBJDIR)

awtk_port:
	@cd src/awtk-port;make

target: setup awtk_port $(OBJ_CPP)
	$(CXX) $(OBJ_CPP) $(LD_OPTS) -o $(BINNAME)

$(OBJ_CPP): $(OBJDIR)/%.o : %.cpp
	$(CXX) $(CC_OPTS) -o $@ -c $<

install: target
#	cp $(BINNAME) $(INSTALL_DIR)

strip: install
ifneq ($(ENABLE_DEBUG),yes)
	$(STRIP) $(INSTALL_DIR)/$(BINNAME)
endif


clean:
	@cd src/awtk-port;make clean
	rm -rf $(OBJDIR)
	rm -rf $(BINNAME)

$(DEPS_CPP): $(OBJDIR)/%.d : %.cpp
	@mkdir -p $(OBJDIR); \
	set -e; rm -f $@; \
	$(CXX) -MM $(CC_OPTS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJDIR)\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEPS_CPP)

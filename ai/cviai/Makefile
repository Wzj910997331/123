include $(NVR_ROOT_PATH)/Makefile.param

LIBNAME = libnvr_ai
SHARED_LIB = $(LIBNAME).so
STATIC_LIB = $(LIBNAME).a

OBJDIR = ./obj
INSTALL_DIR = $(NVR_ROOT_PATH)/opt/lib

#CC_OPTS += -Werror
CC_OPTS += -Wall -fPIC -std=c++11
ifeq ($(ENABLE_DEBUG),yes)
	CC_OPTS += -g
endif

CC_OPTS += $(GLOBAL_CC_OPTS)

INC_DIR += -I./include
INC_DIR += -I$(NVR_ROOT_PATH)/ai/include
INC_DIR += -I$(NVR_ROOT_PATH)/ai/include/cviai
INC_DIR += -I$(NVR_ROOT_PATH)/ai/include/cvimath

INC_DIR += -I$(NVR_ROOT_PATH)/common
INC_DIR += -I$(NVR_ROOT_PATH)/basesvc
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/glog/include
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/tcli/include
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/onvif
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/base64
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/log
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/ffmpeg/include
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/md5
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/db_sqlite3/sqlite3/include
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/cvi_fileop/include
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/cvi_fileop/utest
INC_DIR += -I$(NVR_ROOT_PATH)/opt/framework_include
INC_DIR += -I$(NVR_ROOT_PATH)/media
INC_DIR += -I$(NVR_ROOT_PATH)/media/include
INC_DIR += -I$(NVR_ROOT_PATH)/ai_app/include

INC_DIR += -I$(NVR_ROOT_PATH)/3rd/awtk/include/src
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/awtk/include/src/ext_widgets

INC_DIR += -I$(NVR_ROOT_PATH)/app/yuanwei/src/cvi
INC_DIR += -I$(NVR_ROOT_PATH)/app/yuanwei/src/sys
INC_DIR += -I$(NVR_ROOT_PATH)/app/yuanwei/src/ui

INC_DIR += -I$(NVR_ROOT_PATH)/app/awtk_demo/src/base
INC_DIR += -I$(NVR_ROOT_PATH)/app/awtk_demo/src/utest

INC_DIR += -I$(SDK_PATH)/middleware/include

INC_DIR += -I$(NVR_ROOT_PATH)/3rd/db_sqlite3/sqlite3/include
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/db_sqlite3/include/

CC_OPTS += $(INC_DIR)

LD_OPTS += -rdynamic -shared

SRC_DIR += .

VPATH = $(SRC_DIR)

SRC_CPP = $(shell find $(SRC_DIR) -maxdepth 1 -name '*.cpp')

OBJ_CPP_0 = $(notdir $(patsubst %.cpp, %.o, $(SRC_CPP)))

OBJ_CPP    = $(foreach file, $(OBJ_CPP_0), $(OBJDIR)/$(file))

DEPS_CPP   = $(OBJ_CPP:%.o=%.d)

ifeq ($(APP_CUSTOM_TYPE),yuanwei)
	GLOBAL_CC_OPTS += -DAPP_YUANWEI
endif
ifeq ($(APP_CUSTOM_TYPE),cvi_nvr)
	GLOBAL_CC_OPTS += -DAPP_CVINVR
endif

.PHONY: all setup target install strip clean

all: setup target install strip

setup:
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(OBJDIR)

target: setup $(OBJ_CPP)
	$(CXX) $(OBJ_CPP) $(LD_OPTS) -o $(SHARED_LIB)

$(OBJ_CPP): $(OBJDIR)/%.o : %.cpp
	$(CXX) $(CC_OPTS) -o $@ -c $<

install: target
	cp $(SHARED_LIB) $(INSTALL_DIR)

strip: install
ifneq ($(ENABLE_DEBUG),yes)
	$(STRIP) $(INSTALL_DIR)/$(SHARED_LIB)
endif

clean:
	rm -rf $(OBJDIR)
	rm -rf *.so *.a

$(DEPS_CPP): $(OBJDIR)/%.d : %.cpp
	@mkdir -p $(OBJDIR); \
	set -e; rm -f $@; \
	$(CXX) -MM $(CC_OPTS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJDIR)\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEPS_CPP)


include $(NVR_ROOT_PATH)/Makefile.param

LIBNAME = libawtk_port
SHARED_LIB = $(LIBNAME).so
STATIC_LIB = $(LIBNAME).a

OBJDIR = ./obj
INSTALL_DIR = $(NVR_ROOT_PATH)/opt/lib

CC_OPTS += -Wall -fPIC
ifeq ($(ENABLE_DEBUG),yes)
	CC_OPTS += -g
endif

CC_OPTS += $(GLOBAL_CC_OPTS)

INC_DIR += -I$(NVR_ROOT_PATH)/3rd/log

INC_DIR += -I$(NVR_ROOT_PATH)/3rd/awtk/include/src
INC_DIR += -I$(NVR_ROOT_PATH)/3rd/tslib/include

CC_OPTS += $(INC_DIR)

LD_OPTS += -rdynamic -shared

SRC_DIR += .

VPATH = $(SRC_DIR)

SRC_C = $(shell find $(SRC_DIR) -maxdepth 1 -name '*.c')

OBJ_C_0 = $(notdir $(patsubst %.c, %.o, $(SRC_C)))

OBJ_C    = $(foreach file, $(OBJ_C_0), $(OBJDIR)/$(file))

DEPS_C   = $(OBJ_C:%.o=%.d)

.PHONY: all setup target install strip clean

all: setup target install strip

setup:
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(OBJDIR)

target: setup $(OBJ_C)
	$(CC) $(OBJ_C) $(LD_OPTS) -o $(SHARED_LIB)
#	$(AR) rcs $(STATIC_LIB) $^

$(OBJ_C): $(OBJDIR)/%.o : %.c
	$(CC) $(CC_OPTS) -o $@ -c $<

install: target
	cp $(SHARED_LIB) $(INSTALL_DIR)

strip: install
ifneq ($(ENABLE_DEBUG),yes)
	$(STRIP) $(INSTALL_DIR)/$(SHARED_LIB)
endif

clean:
	rm -rf $(OBJDIR)
	rm -rf *.so *.a

$(DEPS_C): $(OBJDIR)/%.d : %.c
	@mkdir -p $(OBJDIR); \
	set -e; rm -f $@; \
	$(CC) -MM $(CC_OPTS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJDIR)\/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(DEPS_C)

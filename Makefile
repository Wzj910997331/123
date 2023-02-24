
include ./Makefile.param

.PHONY: all setup 3rd app clean

all: setup 3rd app

setup:
	@mkdir -p ./opt/lib
	@cp ./3rd/awtk/lib/* ./opt/lib
	@cp ./basesvc/*.so ./opt/lib
	@cp ./common/*.so ./opt/lib
	@cp ./media/*.so ./opt/lib

3rd: setup
	@cd ./3rd;make

app: 3rd
ifeq ($(ENABLE_HDMI_OUTPUT),yes)
	@cd ./opt/lt9611;make ;
endif
ifeq ($(ENABLE_CVIAI),yes)
	@cd $(NVR_ROOT_PATH)/ai;make;
endif
	@cd ./app/awtk_demo;make

clean:
ifeq ($(ENABLE_CVIAI),yes)
	@cd $(NVR_ROOT_PATH)/ai;make clean;
endif
ifeq ($(ENABLE_HDMI_OUTPUT),yes)
	@cd ./opt/lt9611;make clean;
endif
	@cd ./3rd;make clean
	@cd ./app/awtk_demo;make clean


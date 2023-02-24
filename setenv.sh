#!/bin/bash

PWD=`pwd -P`

export NVR_ROOT_PATH=$PWD
#export SDK_PATH=/home/xxx/cv183x_x.x.x_sdk
export SDK_PATH="$TOP_DIR"

export APP_CVINVR_ENABLE=y

if [ -z $SDK_PATH ] ; then
  echo "warning: set SDK_PATH first, eg: export SDK_PATH=/home/xxx/cv183x_x.x.x_sdk"
fi

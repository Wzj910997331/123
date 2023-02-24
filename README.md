## 目录文件说明

	3rd            存放使用到的第三方库，大部分是提前编译好的
	app            app 存放路径
	basesvc        中间层
	common         中间层依赖
	mediaPlayer    媒体播放相关
	opt            存放其他文件与编译输出

## 如何编译

1、修改 setenv.sh 内容，填写正确的 SDK_PATH，如下：

	export SDK_PATH=/home/zoulm/work/src/cv183x

2、执行如下命令设置编译需要用到的环境变量：

	source ./setenv.sh

3、然后执行 make 即可全编译或进行单独编译app：

	make -C ./app/awtk_demo

# bazel 交叉编译方法
bazel build --crosstool_top=//tools/arm_compiler:toolchain --cpu=armeabi-v7a
#需要编译的库文件
opencv, tensorflow_cc, tensorflow_framework


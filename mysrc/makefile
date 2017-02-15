#目标名称
TARGET=haoserver

#目标存放目录
TARGET_DIR=./

#源文件目录
SOURCE_DIR=../src/

#中间文件目录
TEMP=./objs/

#编译器
CC=g++

#编译选项
COMPLIE_FLAGS=-Wall -O2 -std=c++11

#连接选项
LINK_FLAGS=-Wl,-rpath,'$$ORIGIN'

#库头文件目录
HEAD_DIR=-I../boost_1_60_0/ -I../libuv-1.x/include/ -I../libprotobuf/include/ -I../libvirt/include/

#库文件目录
LIB_DIR= -L../libuv-1.x/libs/ -L../libprotobuf/libs/ -L../libvirt/libs/

#库文件
LIB= -luv -lprotobuf -lvirt

#依赖其它工程库文件
PROJECT_LIB=

PROJECT_LIB_DIR=$(dir $(PROJECT_LIB))
L_PROJECT_LIB_DIR=$(addprefix -L ,$(PROJECT_LIB_DIR))
PROJECT_LIB_NAME=$(notdir $(PROJECT_LIB))
PROJECT_LIB_BASENAME=$(basename $(PROJECT_LIB_NAME))
PROJECT_LIB_BASENAME_E_LIB=$(patsubst lib%,-l%,$(PROJECT_LIB_BASENAME))
PATHSOURCE=$(wildcard $(SOURCE_DIR)*.cpp)
SOURCE=$(notdir $(PATHSOURCE))
OBJS=$(patsubst %.cpp,$(TEMP)%.o,$(SOURCE))
DEPS=$(patsubst %.cpp,$(TEMP)%.d,$(SOURCE))

$(TARGET_DIR)$(TARGET):$(OBJS) $(PROJECT_LIB)
	$(CC) $(LINK_FLAGS) $(OBJS) $(LIB_DIR) $(LIB) $(L_PROJECT_LIB_DIR) $(PROJECT_LIB_BASENAME_E_LIB) -o $@
-include $(DEPS)
%.d:
	rm -f $@;\
	$(CC) -MM $(SOURCE_DIR)$(notdir $(patsubst %.d,%.cpp,$@)) > $@.$$$$;\
	sed 's,[^:]*:,$@ $(patsubst %.d,%.o,$@):,g' < $@.$$$$ >$@;\
	rm -f $@.$$$$
%.o:
	$(CC) $(COMPLIE_FLAGS) -c $(HEAD_DIR) $(SOURCE_DIR)$(notdir $(patsubst %.o,%.cpp,$@)) -o $@
%.so:
	@echo	
.PHONY:clean
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_DIR)$(TARGET)


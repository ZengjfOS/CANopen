# Makefile for CANopend.


# socketCAN协议栈驱动源代码目录
STACKDRV_SRC =  ../CANopenNode/stack/socketCAN
# canopen协议栈源代码源代码目录
STACK_SRC =     ../CANopenNode/stack
# CANopenNode目录
CANOPEN_SRC =   ../CANopenNode
# canopend源代码目录
CANOPEND_SRC =  ./src
# 对象字典目录
OBJ_DICT_SRC =  ./objDict
# 应用程序目录
APP_SRC =       ./app


LINK_TARGET  =  $(APP_SRC)/canopend							# 输出目标程序名


# 引入各部分头文件
INCLUDE_DIRS = -I$(STACKDRV_SRC) \
               -I$(STACK_SRC)    \
               -I$(CANOPEN_SRC)  \
               -I$(CANOPEND_SRC) \
               -I$(OBJ_DICT_SRC) \
               -I$(APP_SRC)


# 指定编译是的源代码
SOURCES =       $(STACKDRV_SRC)/CO_driver.c         \
                $(STACKDRV_SRC)/CO_OD_storage.c     \
                $(STACKDRV_SRC)/CO_Linux_tasks.c    \
                $(STACK_SRC)/crc16-ccitt.c          \
                $(STACK_SRC)/CO_SDO.c               \
                $(STACK_SRC)/CO_Emergency.c         \
                $(STACK_SRC)/CO_NMT_Heartbeat.c     \
                $(STACK_SRC)/CO_SYNC.c              \
                $(STACK_SRC)/CO_PDO.c               \
                $(STACK_SRC)/CO_HBconsumer.c        \
                $(STACK_SRC)/CO_SDOmaster.c         \
                $(STACK_SRC)/CO_trace.c             \
                $(CANOPEN_SRC)/CANopen.c            \
                $(CANOPEND_SRC)/CO_command.c        \
                $(CANOPEND_SRC)/CO_comm_helpers.c   \
                $(CANOPEND_SRC)/CO_master.c         \
                $(CANOPEND_SRC)/CO_time.c           \
                $(CANOPEND_SRC)/main.c              \
                $(OBJ_DICT_SRC)/CO_OD.c             \
                $(APP_SRC)/application.c


# canopend is three-threaded application: nonblocking mainline, nonblocking
# rt-thread and blocking command interface thread.
# If there is a need to compile canopend as a single-threaded application, then:
#   - Command interface is not possible, so remove CO_command.c,
#     CO_comm_helpers.c, CO_master.c (and CO_SDOmaster.c) from SOURCES.
#   - Add flag -DCO_SINGLE_THREAD to the CFLAGS.
#   - Remove flag -pthread from LDFLAGS.
# Flag -lrt in LDFLAGS is necessary only with older kernels.
#
# canopend是3线程应用程序：
#   1. 非阻塞主线程；
#   2. 非阻塞实时时钟线程；
#   3. 阻塞命令行接口线程；
# 如果需要将canopend作为单线程应用程序，需要做以下事情：
#   1. 命令行接口是不能存在的，所以请移除SOURCES变量中的CO_command.c、CO_comm_helpers.c、
#     CO_master.c、CO_SDOmaster.c文件。
#   2. 在CFLAGS中添加-DCO_SINGLE_THREAD标志。
#   3. 移除LDFLAGS中的-pthread标志。
# LDFLAGS中的-lrt标志针对老版本的内核添加的。


OBJS = $(SOURCES:%.c=%.o)								# 编译目标依赖
CC = arm-none-linux-gnueabi-gcc							# 指定交叉编译器
# CC = gcc							# 指定交叉编译器
CFLAGS = -Wall -g $(INCLUDE_DIRS)						# 编译时的标志
LDFLAGS = -lrt -pthread									# 编译依赖库标志


.PHONY: all clean

all: clean $(LINK_TARGET)

clean:
	rm -f $(OBJS) $(LINK_TARGET)						# 删除相关文件

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LINK_TARGET): $(OBJS)									# 编译输出
	$(CC) $(LDFLAGS) $^ -o $@ -static

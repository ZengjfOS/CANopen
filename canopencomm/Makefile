# Makefile for CANopenCommand.


APPL_SRC =      .									# 相关头文件所在位置


LINK_TARGET  =  canopencomm							# 输出文件名


INCLUDE_DIRS = -I$(APPL_SRC)						# 加载头文件


SOURCES =       $(APPL_SRC)/CANopenCommand.c		# 程序源代码


OBJS = $(SOURCES:%.c=%.o)
CC = arm-none-linux-gnueabi-gcc						# 编译器
CFLAGS = -Wall -g $(INCLUDE_DIRS)					# 编译标志
LDFLAGS =


.PHONY: all clean

all: clean $(LINK_TARGET)

clean:
	rm -f $(OBJS) $(LINK_TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LINK_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

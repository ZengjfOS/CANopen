# Makefile for CANopenCGI.


APPL_SRC =      .								# 头文件所在位置


LINK_TARGET  =  canopen.cgi						# 目标文件名


INCLUDE_DIRS = -I$(APPL_SRC)					# 引入头文件


SOURCES =       $(APPL_SRC)/CANopenCGI.c		# 源代码


OBJS = $(SOURCES:%.c=%.o)
CC = arm-linux-gnueabihf-gcc					# 编译器
CFLAGS = -Wall -g $(INCLUDE_DIRS)				# 编译标志
LDFLAGS =


.PHONY: all clean

all: clean $(LINK_TARGET)

clean:
	rm -f $(OBJS) $(LINK_TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LINK_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

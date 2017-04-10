/*****************************************************************************
 *                      CANopenSocket CANopenCommand hacking
 * 说明：
 *     分析一下CANopenSocket中的CANopenCommand部分是怎么工作的。
 *
 *                                          2017-3-23 深圳 南山平山村 曾剑锋
 ****************************************************************************/
/*
 * Client socket command interface for CANopenSocket.
 *
 * @file        CANopenCommand.c
 * @author      Janez Paternoster
 * @copyright   2015 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * CANopenNode is free and open source software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>


#ifndef BUF_SIZE
#define BUF_SIZE            100000
#endif

/**
 * 错误退出函数
 */
/* Helper functions */
void errExit(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}


/**
 * 是否打印错误描述相关信息标志
 */
static int printErrorDescription = 0;

/**
 * 发送命令函数，fd是socket描述符
 */
static void sendCommand(int fd, char* command, size_t commandLength);


/**
 * 参数使用方法说明
 */
static void printUsage(char *progName) {
fprintf(stderr,
"Usage: %s [options] <command string>\n", progName);
fprintf(stderr,
"\n"
"Program reads arguments or standard input or file. It sends commands to\n"
"canopend via socket, line after line. Result is printed to standard output.\n"
"For more information see http://www.can-cia.org/, CiA 309 standard.\n"
"\n"
"Options:\n"
"  -f <input file path>  Path to the input file.\n"
"  -s <socket path>      Path to the socket (default '/tmp/CO_command_socket').\n"
"  -h                    Display description of error codes in case of error.\n"
"                        (Default, if command is passed by program arguments.)\n"
"  --help                Display this help.\n"
"  --helpall             Display this help, internal and SDO error codes.\n"
"\n"
"Command strings must start with \"[\"<sequence>\"]\" (except if from arguments):\n"
"  - SDO upload:   [[<net>] <node>] r[ead]  <index> <subindex> [<datatype>]\n"
"  - SDO download: [[<net>] <node>] w[rite] <index> <subindex>  <datatype> <value>\n"
"  - Configure SDO time-out: [<net>] set sdo_timeout <value>\n"
"  - Enable SDO block transfer: [<net>] set sdo_block <value>\n"
"  - Set default node: [<net>] set node <value>\n"
"\n"
"  - Start node:                  [[<net>] <node>] start\n"
"  - Stop node:                   [[<net>] <node>] stop\n"
"  - Set node to pre-operational: [[<net>] <node>] preop[erational]\n"
"  - Reset node:                  [[<net>] <node>] reset node\n"
"  - Reset communication:         [[<net>] <node>] reset comm[unication]\n"
"\n"
"Comments started with '#' are ignored. They may be on the beginning of the line\n"
"or after the command string. 'sdo_timeout' is in milliseconds, 500 by default.\n"
"If <node> is not specified within commands, then value defined by 'set node'\n"
"command is used.\n"
"\n"
"\n"
"Datatypes:\n"
"  - b                 - Boolean.\n"
"  - u8, u16, u32, u64 - Unsigned integers.\n"
"  - i8, i16, i32, i64 - Signed integers.\n"
"  - r32, r64          - Real numbers.\n"
"  - t, td             - Time of day, time difference.\n"
"  - vs                - Visible string (between double quotes).\n"
"  - os, us, d         - Octet string, unicode string, domain\n"
"                        (mime-base64 (RFC2045) should be used).\n"
"\n"
"\n"
"Response: \"[\"<sequence>\"]\" \\\n"
"    OK | <value> | ERROR: <SDO-abort-code> | ERROR: <internal-error-code>\n"
"\n"
"\n"
"LICENSE\n"
"    This program is part of CANopenSocket and can be downloaded from:\n"
"    https://github.com/CANopenNode/CANopenSocket\n"
"    Permission is granted to copy, distribute and/or modify this document\n"
"    under the terms of the GNU General Public License, Version 2.\n"
"\n"
);
}


/**
 * 错误描述对应的结构体
 */
/* Extract error description */
typedef struct {
    int code;
    char* desc;
} errorDescs_t;

/**
 * 这里列出所有的canopend返回的错误编号对应的显示的英文
 */
static const errorDescs_t errorDescs[] = {
        {100, "Request not supported."},
        {101, "Syntax error."},
        {102, "Request not processed due to internal state."},
        {103, "Time-out (where applicable)."},
        {104, "No default net set."},
        {105, "No default node set."},
        {106, "Unsupported net."},
        {107, "Unsupported node."},
        {200, "Lost guarding message."},
        {201, "Lost connection."},
        {202, "Heartbeat started."},
        {203, "Heartbeat lost."},
        {204, "Wrong NMT state."},
        {205, "Boot-up."},
        {300, "Error passive."},
        {301, "Bus off."},
        {303, "CAN buffer overflow."},
        {304, "CAN init."},
        {305, "CAN active (at init or start-up)."},
        {400, "PDO already used."},
        {401, "PDO length exceeded."},
        {501, "LSS implementation- / manufacturer-specific error."},
        {502, "LSS node-ID not supported."},
        {503, "LSS bit-rate not supported."},
        {504, "LSS parameter storing failed."},
        {505, "LSS command failed because of media error."},
        {600, "Running out of memory."},
        {0x00000000, "No abort."},
        {0x05030000, "Toggle bit not altered."},
        {0x05040000, "SDO protocol timed out."},
        {0x05040001, "Command specifier not valid or unknown."},
        {0x05040002, "Invalid block size in block mode."},
        {0x05040003, "Invalid sequence number in block mode."},
        {0x05040004, "CRC error (block mode only)."},
        {0x05040005, "Out of memory."},
        {0x06010000, "Unsupported access to an object."},
        {0x06010001, "Attempt to read a write only object."},
        {0x06010002, "Attempt to write a read only object."},
        {0x06020000, "Object does not exist."},
        {0x06040041, "Object cannot be mapped to the PDO."},
        {0x06040042, "Number and length of object to be mapped exceeds PDO length."},
        {0x06040043, "General parameter incompatibility reasons."},
        {0x06040047, "General internal incompatibility in device."},
        {0x06060000, "Access failed due to hardware error."},
        {0x06070010, "Data type does not match, length of service parameter does not match."},
        {0x06070012, "Data type does not match, length of service parameter too high."},
        {0x06070013, "Data type does not match, length of service parameter too short."},
        {0x06090011, "Sub index does not exist."},
        {0x06090030, "Invalid value for parameter (download only)."},
        {0x06090031, "Value range of parameter written too high."},
        {0x06090032, "Value range of parameter written too low."},
        {0x06090036, "Maximum value is less than minimum value."},
        {0x060A0023, "Resource not available: SDO connection."},
        {0x08000000, "General error."},
        {0x08000020, "Data cannot be transferred or stored to application."},
        {0x08000021, "Data cannot be transferred or stored to application because of local control."},
        {0x08000022, "Data cannot be transferred or stored to application because of present device state."},
        {0x08000023, "Object dictionary not present or dynamic generation fails."},
        {0x08000024, "No data available."}
};

/**
 * 打印所有的错误描述文档内容，有一部分是十进制的，有一部分是二进制的，注意for循环中的if判断和上面结构体数组中的数值就能体现出来了
 */
static void printErrorDescs(void) {
    int i, len;

    len = sizeof(errorDescs) / sizeof(errorDescs_t);

    fprintf(stderr, "Internal error codes:\n");

    for(i=0; i<len; i++) {
        const errorDescs_t *ed = &errorDescs[i];

        if(ed->code == 0) break;
        fprintf(stderr, "  - %d - %s\n", ed->code, ed->desc);
    }

    fprintf(stderr, "\n");
    fprintf(stderr, "SDO abort codes:\n");

    for(; i<len; i++) {
        const errorDescs_t *ed = &errorDescs[i];

        fprintf(stderr, "  - 0x%08X - %s\n", ed->code, ed->desc);
    }

    fprintf(stderr, "\n");
}


/******************************************************************************/
int main (int argc, char *argv[]) {
    char *socketPath = "/tmp/CO_command_socket";  /* Name of the local domain socket, configurable by arguments. */
    char *inputFilePath = NULL;

    char buf[BUF_SIZE];
    int fd;
    struct sockaddr_un addr;
    int opt;
    int i;

    /**
     * 输出基本帮助信息
     */
    if(argc >= 2 && strcmp(argv[1], "--help") == 0) {
        printUsage(argv[0]);
        exit(EXIT_SUCCESS);
    }
    /**
     * 输出基本帮助信息和返回的错误码信息
     */
    if(argc >= 2 && strcmp(argv[1], "--helpall") == 0) {
        printUsage(argv[0]);
        printErrorDescs();
        exit(EXIT_SUCCESS);
    }

    /* Get program options */
    /**
     * 获取程序运行时命令行传递过来的参数，并解析
     */
    while((opt = getopt(argc, argv, "s:f:h")) != -1) {
        switch (opt) {
            case 'f':       // 将命令放置于文件中，通过读取文件并发送给canopend处理
                inputFilePath = optarg;
                break;
            case 's':       // 指定canopend的socket的path在哪里
                socketPath = optarg;
                break;
            case 'h':       // 输出错误描述
                printErrorDescription = 1;
                break;
            default:
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /* Create and connect client socket */
    /**
     * 创建本地客户端socket
     */
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd == -1) {
        errExit("Socket creation failed");
    }

    /**
     * 设置本地socket
     */
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);

    /**
     * 链接本地socket
     */
    if(connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        errExit("Socket connection failed");
    }


    /* get commands from input file, line after line */
    /**
     * 从文件中获取命令，这里不进行任何处理，直接全部传送，仅仅是将文件中的内容完整转给canopend处理
     */
    if(inputFilePath != NULL) {
        FILE *fp = fopen(inputFilePath, "r");
        if(fp == NULL) {
            errExit("Can't open input file");
        }

        while(fgets(buf, BUF_SIZE, fp) != NULL) {
            sendCommand(fd, buf, strlen(buf));
        }

        fclose(fp);
    }

    /* get command from arguments */
    /**
     * 从命令行的参数中获取命令
     */
    else if(optind < argc) {
        buf[0] = 0;
        size_t buflen = 0;

        /* Add sequence number if not present on command line arguments */
        /**
         * 如果CAN设备的序号没有提供，那么就是用默认的1号设备(can1)设备
         */
        if(argv[optind][0] != '[') {
            strcat(buf, "[1] ");
        }

        /**
         * 将命令行参数合成命令
         */
        for(i=optind; i<argc; i++) {
            strncat(buf, argv[i], (BUF_SIZE - 2) - buflen);
            strcat(buf, " ");
            buflen = strlen(buf);
            if(buflen >= (BUF_SIZE - 1)) {
                fprintf(stderr, "String too long!\n");
                exit(EXIT_FAILURE);
            }
        }
        /**
         * 每一条命令都是使用'\n'结束，表示一行
         */
        buf[buflen - 1] = '\n'; /* replace last space with newline */

        /**
         * 将合成的命令发送给canopend
         */
        printErrorDescription = 1;
        sendCommand(fd, buf, buflen);
    }

    /* get commands from stdin, line after line */
    /**
     * 从终端的标准输入中一行一行的获取命令
     */
    else {
        while(fgets(buf, BUF_SIZE, stdin) != NULL) {
            sendCommand(fd, buf, strlen(buf));
        }
    }

    close(fd);

    exit(EXIT_SUCCESS);
}


static void sendCommand(int fd, char* command, size_t commandLength) {
    size_t n;
    char buf[BUF_SIZE];

    /**
     * 发送命令
     */
    if (write(fd, command, commandLength) != commandLength) {
        errExit("Socket write failed");
    }

    /**
     * 接收命令返回信息
     */
    n = read(fd, buf, sizeof(buf));

    if(n == -1) {
        errExit("Socket read failed");
    }

    /**
     * 是否处理返回的信息
     */
    if(printErrorDescription == 1) {
        char *errLoc = strstr(buf, "ERROR:");
        char *endLoc = strstr(buf, "\r\n");

        if(errLoc != NULL && endLoc != NULL) {
            int num;
            char *sRet = NULL;

            errLoc += 6;

            num = strtol(errLoc, &sRet, 0);
            if(strlen(errLoc) != 0 && sRet == strchr(errLoc, '\r')) {
                int i, len;

                len = sizeof(errorDescs) / sizeof(errorDescs_t);

                /**
                 * 查找并输出信息
                 */
                for(i=0; i<len; i++) {
                    const errorDescs_t *ed = &errorDescs[i];
                    if(ed->code == num) {
                        sprintf(endLoc, " - %s\r\n", ed->desc);
                        break;
                    }
                }
            }
        }
    }

    /**
     * 输出返回原始信息
     */
    printf("%s", buf);
}


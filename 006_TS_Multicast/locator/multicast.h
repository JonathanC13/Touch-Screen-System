/*
 * multicast.h
 *
 *  Created on: Sep 16, 2016
 *      Author: hwa
 */

#ifndef MULTICAST_H_
#define MULTICAST_H_

const int default_port = 16789;
const char DEFAULT_MULTICAST_ADDR[] = "224.0.0.1";

#define MULTICAST_QUEUE_NAME        "/projmcast"

//it is octal number because it is started with 0
//permission --> [0][special:uid,gid,stick][user:read,write,exec][gr:read,write,exec][other:read,write,exec]
#define QUEUE_PERMISSIONS 0666

#define MAX_QU_MESSAGES         100          //raspberry has been set to have 300. su -c "echo 300 > /proc/sys/fs/mqueue/msg_max"
#define MAX_QU_MSGSIZE          1024
#define MAX_UDP_BODY_SIZE       512

/*
 multicast message structure. Little Endian
 ----------------------------------
 bodyIndex  type        description
 ----------------------------------
 0 to 3     int         header or general info.
 4 to 7     time_t      at message created. (in 64-bit machine, sizeof(time_t) is padded into 8-byte)
 8 to 11    float       x value at intercept
 12 to 15   float       y value at intercept

 16 to 19   int         sensor state while IR LED #0 is ON
 20 to 23   int         sensor state while IR LED #1 is ON
 24 to 27   int         sensor state while IR LED #2 is ON
 28 to 31   int         sensor state while IR LED #3 is ON
 32 to 35   int         sensor state while IR LED #4 is ON
 36 to 39   int         sensor state while IR LED #5 is ON
 40 to 43   int         sensor state while IR LED #6 is ON
 44 to 47   int         sensor state while IR LED #7 is ON
 48 to 127  char        80-byte, any text.
 */
#define MCIDX_HEADER                0
#define MCIDX_TIME_T                4

#define MCIDX_X_INTERCEPT           8
#define MCIDX_Y_INTERCEPT           12

#define MCIDX_IR0_SENSOR_ST         16
#define MCIDX_IR1_SENSOR_ST         20
#define MCIDX_IR2_SENSOR_ST         24
#define MCIDX_IR3_SENSOR_ST         28
#define MCIDX_IR4_SENSOR_ST         32
#define MCIDX_IR5_SENSOR_ST         36
#define MCIDX_IR6_SENSOR_ST         40
#define MCIDX_IR7_SENSOR_ST         44

#define MCIDX_GEN_TEXT              48

#define MCMSG_LEN                   128

struct __attribute__((__packed__)) mc_msg {
    int         byte_count;                     //message body byte count.
    char        body[MAX_UDP_BODY_SIZE + 1];
};

#endif /* MULTICAST_H_ */

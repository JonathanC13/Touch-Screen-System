/*
 * multicast_send.cpp
 *
 *  Created on: Dec 1, 2017
 *      Author: hchan
 */
/*
 * multicast_send.cpp
 *
 *  Created on: Sep 14, 2016
 *      Author: hwa
 */

#include <iostream>
using namespace std;

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>      //INET_ADDRSTRLEN
#include <string.h>     //needed for memset
#include <stdio.h>

#include <mqueue.h>
#include <errno.h>
#include <time.h>
#include <string.h>     //needed for memset
#include <unistd.h>     //needed for close, sleep
#include <pthread.h>

#include "locator.h"
#include "multicast.h"

int multicast_sequence = 0;
unsigned int    multicast_signature = 0;
pthread_mutex_t mcast_seq_mutex = PTHREAD_MUTEX_INITIALIZER;

extern char* get_cur_time_string(char* buf);
extern token_cfg tok_cfg_store[NUM_TOKS_IN_FILE];

static thread_info *tinf;       //static means that it is local copy, visible for all functions in this file

#define MQ_BUFFER_SIZ   8192        //Make sure it is greater than mq_msgsize

void *mcast_send_thread(void *arg)
{
    char    time_string[64];
    tinf = (thread_info*)arg;

    tinf->state = THREAD_ENTERING_THREAD;

    cout << "mcast_send_thread[" << get_cur_time_string(time_string) << "]: Entered. thread ID=" << tinf->thread_id << endl;


    int socket_descriptor;
    struct sockaddr_in address;


    //-----------
    //init socket
    //-----------
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);     // For broadcasting, this socket can be treated like a UDP socket:
    if (socket_descriptor == -1) {
        cout << "mcast_send_thread[" << get_cur_time_string(time_string) << "]: " << "Socket() failed. thread ID=" << tinf->thread_id
                << ", error=" << strerror(errno) << endl;

        tinf->ret_value = -300;
        return NULL;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    //address.sin_addr.s_addr = inet_addr(DEFAULT_MULTICAST_ADDR);
    //address.sin_port = htons(default_port);
    printf("mcast_send_thread[%s]: Multicast address=%s, port=%d.\n",
            get_cur_time_string(time_string), tok_cfg_store[IDX_MCAST_IP_ADDR].text, tok_cfg_store[IDX_MCAST_IP_PORT].value);
    address.sin_addr.s_addr = inet_addr( tok_cfg_store[IDX_MCAST_IP_ADDR].text );
    address.sin_port = htons( tok_cfg_store[IDX_MCAST_IP_PORT].value );


    //---------------------------------------------------------------------------------------------------
    //Queue
    //  Any function wants to multicast its message will write into the (Server) queue.
    //  'Multicast send' will dequeue the message and multicast it by sending it to the multicast address
    //  'Multicast send' acts like a server for those functions.
    //---------------------------------------------------------------------------------------------------
    mqd_t qd_server;    // queue descriptors
    struct mq_attr attr;
    char in_buffer [MQ_BUFFER_SIZ + 1];

    //attr.mq_flags = 0;
    //attr.mq_maxmsg = MAX_QU_MESSAGES;
    //attr.mq_msgsize = MAX_QU_MSGSIZE;
    //attr.mq_curmsgs = 0;

    //"mq_open() failed. error=Invalid argument" if the last argument is not NULL
    //if((qd_server = mq_open (MULTICAST_QUEUE_NAME, O_RDONLY  | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {

    if((qd_server = mq_open (MULTICAST_QUEUE_NAME, O_RDONLY  | O_CREAT, QUEUE_PERMISSIONS, NULL)) == -1) {
        cout << "mcast_send_thread[" << get_cur_time_string(time_string) << "]: " << "mq_open() failed. error=" << strerror(errno) << endl;
        tinf->ret_value = -301;
        return NULL;
    }

    mq_getattr(qd_server, &attr);
    cout << "mcast_send_thread[" << get_cur_time_string(time_string) << "]: " << "mq_getattr. mq_flags=" << attr.mq_flags
            << ", mq_maxmsg=" << attr.mq_maxmsg << ", mq_msgsize=" << attr.mq_msgsize << ", mq_curmsgs=" << attr.mq_curmsgs << endl;

    tinf->state = THREAD_ENTERING_LOOP;
    cout << "mcast_send_thread[" << get_cur_time_string(time_string) << "]: Entered loop." << endl;

    while(tinf->conitnueRun) {  //run until the main routine set the flag to false

        //wait for message to be dequeue
        bzero(in_buffer, sizeof(mc_msg));
        int byte_received = mq_receive(qd_server, in_buffer, MQ_BUFFER_SIZ + 1, NULL);  //Must MAX_MC_MSG_BUFFER_SIZE(msg_len) > MAX_MC_MSG_SIZE(mq_msgsize)

        if(byte_received != -1) {   //no error

            int byte_sent = sendto(socket_descriptor, in_buffer, byte_received, 0, (struct sockaddr *)&address, sizeof(address));

            if (byte_sent != -1) {  //no error
                 cout << "mcast_send_thread[" << get_cur_time_string(time_string) << "]: sendto() OK. byte received from qu="
                         << byte_received << ", byte sent by sendto()=" << byte_sent << endl;

            } else {    //error
                cout << "mcast_send_thread[" << get_cur_time_string(time_string) << "]: sendto() failed. error=" << strerror(errno) << endl;
            }

        } else {    //error
            cout << "mcast_send_thread[" << get_cur_time_string(time_string) << "]: mq_receive() failed. error=" << strerror(errno) << endl;
        }
    }


    if(socket_descriptor >= 0) {
        close(socket_descriptor);
    }
    if(mq_close(qd_server) == -1) {
        cout << "mcast_send_thread[" << get_cur_time_string(time_string) << "]: mq_close() failed. " << strerror(errno) << endl;
    }

    cout << "mcast_send_thread[" << get_cur_time_string(time_string) << "]: exited. thread id=" << tinf->thread_id << endl;

    tinf->state = THREAD_RETURNING;
    tinf->ret_value = 0;

    return NULL;
}



//--------------------------------------------------
// util to be used for sending multicast message
//
//to delete the queue from command line
//---------------------------------------------------
//pi@rpi_back:~/fourth $ ls -l /dev/mqueue
//total 0
//-rw-r--r-- 1 pi pi 80 Nov 27 16:06 projmcast
//pi@rpi_back:~/fourth $ sudo rm /dev/mqueue/projmcast
//pi@rpi_back:~/fourth $ ls -l /dev/mqueue
//total 0
//pi@rpi_back:~/fourth $
//
//check mqueue default attributes
//----------------------------------
//pi@rpi_back:~ $ cat /proc/sys/fs/mqueue/msg_max
//10
//pi@rpi_back:~ $ cat /proc/sys/fs/mqueue/msgsize_max   ****RPi: not allow to set this number below 8192. can be set higher.
//8192
//pi@rpi_back:~ $ cat /proc/sys/fs/mqueue/queues_max
//256
//
//
// (DON'T USE sizeof(). sizeof(time_t) in Linux desktop is 8, and in Raspberry is 4. It is PROBLEM.
// queue message structure:
//      message body byte count:    4 bytes     (not include the length of the 'byte_count' variable)
//      message body:
//          time:   8 bytes                     (instead of 4. By using 8 to match desktop size)
//          float:  4 bytes                     (x value)
//          float:  4 bytes                     (y value)
//
//----------------------------------------------------------------------------------------------------
int to_mc_qu(mc_msg *udp_msg)
{
    mqd_t   qd;    // queue descriptors
    struct  mq_attr attr;
    char    time_string[64];

    //attr.mq_flags = 0;
    //attr.mq_maxmsg = MAX_QU_MESSAGES;
    //attr.mq_msgsize = MAX_QU_MSGSIZE;
    //attr.mq_curmsgs = 0;          don't clear out the queue. may have previous data

    if((qd = mq_open (MULTICAST_QUEUE_NAME, O_WRONLY)) == -1) {
        //it failed because it may be that the queue does not exist. So, create it

        cout << "to_mc_qu[" << get_cur_time_string(time_string) << "]: mq_open(O_WRONLY) failed. error=" << strerror(errno)
                << ", Reopen it with CREATE." << endl;

        //"mq_open() failed. error=Invalid argument" if the last argument is not NULL. man mq_open is incorrect
        if((qd = mq_open (MULTICAST_QUEUE_NAME, O_WRONLY | O_CREAT, QUEUE_PERMISSIONS, NULL)) == -1) {
            cout << "to_mc_qu[" << get_cur_time_string(time_string) << "]: mq_open(O_WRONLY | O_CREAT) failed. error=" << strerror(errno) << endl;
            return -302;
        }
    }

    mq_getattr(qd, &attr);
    cout << "to_mc_qu[" << get_cur_time_string(time_string) << "]: mq_getattr. mq_flags=" << attr.mq_flags << ", mq_maxmsg=" << attr.mq_maxmsg << ", mq_msgsize=" << attr.mq_msgsize << ", mq_curmsgs=" << attr.mq_curmsgs << endl;

    char ch[MAX_QU_MSGSIZE + 1];
    bzero(ch, MAX_QU_MSGSIZE + 1);

    memcpy(ch, (char*)&(udp_msg->byte_count), 4);
    memcpy(ch + 4, udp_msg->body, udp_msg->byte_count);

    if( (udp_msg->byte_count > 0) && ( (udp_msg->byte_count + 4) < MAX_UDP_BODY_SIZE) ) {

        if( mq_send(qd, ch, udp_msg->byte_count + 4, 0) == -1) {
            cout << "to_mc_qu[" << get_cur_time_string(time_string) << "]: mq_send() failed. error" << strerror(errno) << endl;
            mq_close(qd);
            return -303;
        }

    } else {
        cout << "to_mc_qu[" << get_cur_time_string(time_string) << "]: wrong byte count=" << udp_msg->byte_count << endl;
        mq_close(qd);
        return -304;
    }

    mq_close(qd);
    cout << "to_mc_qu[" << get_cur_time_string(time_string) << "]: succeeded." << endl;

    return 0;
}


//----------------------------------------------------------------------------------------------
// Send test MC message.
// the message can contain a byte that its value can be 0. This zero will look like the terminating
// NULL byte. strlen() will fail.
// SO, in order to send the binary message, it needs the msg_len
//----------------------------------------------------------------------------------------------

int mcast_test_message()
{
    char    time_string[64];

    time_t  current_time = time(0);
    float   x_float = 0.0;
    float   y_float = 0.0;

    mc_msg  mc_udp_msg;
    bzero(&mc_udp_msg, sizeof(mc_udp_msg));

    mc_udp_msg.byte_count = MCMSG_LEN;  //make sure body_len is less than MAX_MC_BODY_SIZE

    //mc_udp_msg.body[MCIDX_HEADER] is zeroed
    memcpy(&mc_udp_msg.body[MCIDX_TIME_T], &current_time, 4 );     //even reserve 8 bytes, copy only the valid 4 LSBytes
    memcpy(&mc_udp_msg.body[MCIDX_X_INTERCEPT], &x_float, 4 );
    memcpy(&mc_udp_msg.body[MCIDX_Y_INTERCEPT], &y_float, 4 );

    memcpy(&mc_udp_msg.body[MCIDX_GEN_TEXT], "from mcast_test_message", 23);

    //Debug: print the Raspberry sizes for comparing the Linux desktop sizes
    char* ptime = ctime_r((time_t*)&current_time, time_string);
    printf("\t\tInfo:\n\t\traspberry: current_time=0x%x, %s", (unsigned int)current_time, ptime);
    printf("\t\traspberry type size: sizeof(int)=%d, sizeof(time_t)=%d, sizeof(int) + sizeof(time_t)=%d\n",
            sizeof(int), sizeof(time_t), (sizeof(int) + sizeof(time_t)) );



    to_mc_qu(&mc_udp_msg);
    cout << "mcast_test_message[" << get_cur_time_string(time_string) << "]: msg queued. OK." << endl;

    return 0;
}





//============================================================================
// Name        : 006_TS_Multicast.cpp
// Author      : JC
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style

// chan@chan-IdeaPad-U410:~$ ssh pi@192.168.2.239
// chan@chan-IdeaPad-U410:/ProjC/workspaces-neon/raspberryPi3/proj_term/004_TS_EmitAndSense_Thread/Debug$ scp 004_TS_EmitAndSense_Thread pi@192.168.2.239:/home/pi/fourth
// chan@chan-IdeaPad-U410:/ProjC/workspaces-neon/raspberryPi3/proj_term/004_TS_EmitAndSense_Thread/data$ scp cfg.txt pi@192.168.2.239:/home/pi/fourth/data
// chan@chan-IdeaPad-U410:~$ ssh pi@192.168.2.239
//



//
//============================================================================

#include <iostream>
#include <fstream>

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>		//for sleep()
#include <string.h>		//for strlen()
#include <errno.h>
#include <mqueue.h>


#include "../pifacedigital2/pifacedigital.h"
#include "../signal_exit/SignalHandler.hpp"
#include "locator.h"
#include "multicast.h"

int test_pifacedigital2(int addr);
int run_emit();
int run_sensor_thread();
void ns_wait(int nsec);
int stop_sensor();


//one scanning cycle result, hard coded to 32 IR LED because one raspberry pi3 can have only four PFD2
one_led_result scanned_result[32];

//line_paras[a][b]: the parameters of the straight line that connect from LED 'a' to Sensor 'b'
//This table must be filled/initialized with data from a text file during application starting up.
ln_para line_paras[32][32];

//global variable. automatic initialized to null (0)
blocked_line blocked;

//Set pointer as constant, and its contents as constant too. It is read only
char const* const tok_cfg_names[NUM_TOKS_IN_FILE] =
{
		"test_pf2",				//set its value to 1 for running the test function only
		"num_pf2",				//total number of pifacedigital2 connected to the RPi
		"num_sensor_used",		//Total number of sensor/input of pifacedigital2s are used. Starts from pf2 #0, ip#0 as 1 input
		"num_emitter_used",
		"emitter_on_msec",
		"blink_period_nsec",
		"blink_led_flag",
		"emit_sensor_distance_mm",
		"dev_dev_distance_mm",
		"mcast_ip_addr",
		"mcast_ip_port"
};
token_cfg tok_cfg_store[NUM_TOKS_IN_FILE];


SignalHandler signalHandler;

thread_info sensor_thread_info; 		//global, initialized to 0 by default
thread_info mcast_send_thread_info;

extern void * mcast_send_thread(void *arg);
extern void * sensor_thread(void *arg);
extern char * get_cur_time_string(char* buf);
extern void fill_line_parameters();
extern int run_mcast_send_thread();
extern int stop_multicast_threads();
extern int mcast_test_message();

sem_t sync_emit_and_sensor;

////////////////////////////// Iteration 4 //////////////////////////////////////////
//
// Emitter and detector are run in separate threads.
//
/////////////////////////////////////////////////////////////////////////////////////

char const* proj_name = "main[";

int main( int argc, char *argv[])
{
    //int hw_addr = 0;        /**< PiFaceDigital hardware address  */
    char const* fileName = "./data/cfg.txt";	//address/pointer is constant, but content can be changed
    char time_string[64];

    FILE* file = NULL;
    char line[512];

	if(argc != 2) {
		cout << proj_name << get_cur_time_string(time_string) << "]: default config file '" << fileName << "' is used." << endl;
	} else {
		cout << proj_name << get_cur_time_string(time_string) << "]: user specified config file '" << argv[1] << "' is used." << endl;
		fileName = argv[1];
	}
	cout << endl;


    file = fopen(fileName, "r");
    if(file == NULL) {
    	cout << proj_name << get_cur_time_string(time_string) << "]: open file named '" << fileName << "' failed. exit." << endl << endl;
    	return -1;
    }


	while (fgets(line, sizeof(line), file)) {

		char *token_name, *token_vaule_text;
		char *rest = line;
		token_name = strtok_r(rest, "=", &rest);		//strtok_r is re-entrance. Don't use strtok
		if(token_name == NULL) {
			continue;
		}
		token_vaule_text = strtok_r(rest, " \r\n\t/", &rest);
		if(token_vaule_text == NULL) {
			continue;
		}
		//printf("value=%s.....", token_vaule);


		for(int j = 0; j < NUM_TOKS_IN_FILE; j++)
		{
			if(strcmp(token_name, tok_cfg_names[j])==0)
			{
				int bytes = strlen(token_vaule_text);
				bytes = (bytes < 256) ? bytes : 255;
				memcpy(tok_cfg_store[j].text, token_vaule_text, bytes);

				tok_cfg_store[j].value = atoi(token_vaule_text);

				printf("%s%s]: name=%s.\t", proj_name, get_cur_time_string(time_string), token_name);
				printf("tok_cfg_store[%d].text = %s,\t\ttok_cfg_store[%d].value = %d.\n",
				        j, tok_cfg_store[j].text, j, tok_cfg_store[j].value);
				break;
			}
		}
	}
	fclose(file);
	cout << endl;

	// Register signal handler to handle kill signal
    signalHandler.setupSignalHandlers();				// signalHandler.gotExitSignal() returns true is Ctrl-C was presssed.
    cout << proj_name << get_cur_time_string(time_string) << "]: setup Signal to handle Ctrl-C." << endl;


	if(tok_cfg_store[IDX_TEST_PF2].value != 0)
	{
		for(int j = 0; j < tok_cfg_store[IDX_NUM_PF2].value; j++)
		{
			printf("%s%s]: Test pifacedigital2 with address %d.\n\n", proj_name, get_cur_time_string(time_string), j);
			test_pifacedigital2(j);
		}

	} else {

	    //check error before doing anything
	    if(tok_cfg_store[IDX_NUM_PF2].value > 4) {
	        return -100;
	    }


	    //fill up lines parameters
	    fill_line_parameters();

	    //start multicast thread
	    run_mcast_send_thread();

		// last argument 0, means taken so that sensor will not start scanning until emit releases it
		sem_init(&sync_emit_and_sensor, 0, 0);
		run_sensor_thread();

		run_emit();	//this is an infinite loop of the main thread. It must be called after all thread created.

		//shut down orderly
		//stop other threads here before exit
		stop_sensor();

		//stop multicast thread
		stop_multicast_threads();

		sem_destroy(&sync_emit_and_sensor);
		cout << proj_name << get_cur_time_string(time_string) << "]: semaphore destroyed." << endl;
		cout << "---------end------------" << endl;
	}

}


//-----------------------------------------------------------------------------
// wait for 'nsec' nanoseconds
// re-start the timer with the remaining time if it is interrupted by signal
//-----------------------------------------------------------------------------
void ns_wait(int nsec)
{
    timespec tm_sleep, tm_remain;

    tm_sleep.tv_sec = nsec / 999999999;
    tm_sleep.tv_nsec = nsec % 999999999;

    while(1) {
        if(nanosleep(&tm_sleep, &tm_remain) == -1) {
            if(errno == EINTR) {
                tm_sleep = tm_remain;
            }
        } else {
            break;
        }
    }
}


int run_mcast_send_thread()
{
    int             result;
    char            time_string[64];
    pthread_attr_t  attr;

    result = pthread_attr_init(&attr);
    if (result != 0) {
        cout << proj_name << get_cur_time_string(time_string) << "]: create_mcast_send_thread. pthread_attr_init() failed. result code="
                << result << ", " << strerror(result) << endl;
        return -1;
    }

    mcast_send_thread_info.conitnueRun = true;
    result = pthread_create(&mcast_send_thread_info.thread_id, &attr, &mcast_send_thread, &mcast_send_thread_info);
    if (result != 0) {
        cout << proj_name << get_cur_time_string(time_string) << "]: create_mcast_send_thread. pthread_create() failed. result code="
                << result << ", " << strerror(result) << endl;
        return -2;
    }

    //Destroy the thread attributes object, since it is no longer needed
    result = pthread_attr_destroy(&attr);
    if (result != 0) {
        cout << proj_name << get_cur_time_string(time_string) << "]: create_mcast_send_thread. pthread_attr_destroy() failed. result="
                << result << ", " << strerror(result) << endl;
        return -3;
    }

    cout << proj_name << get_cur_time_string(time_string) << "]: create_mcast_send_thread. exited. thread_id="
            << mcast_send_thread_info.thread_id << endl;

    return 0;
}

int stop_multicast_threads()
{
    int     result = 0;
    char    time_string[64];

    mcast_send_thread_info.conitnueRun = false;
    mcast_test_message();   //cause the qu_receive unblocked so that it can quit.

    if(mcast_send_thread_info.thread_id != 0) {  //let listening thread waits for the Sending thread to join back. Then listener exits.

        int result = pthread_join(mcast_send_thread_info.thread_id, NULL);
        if (result != 0) {
            cout << proj_name << get_cur_time_string(time_string) << "]: mcast_send_thread pthread_join() failed. result="
                    << result << ", " << strerror(result) << endl;
            result = -102;
        }
        else
        {
            cout << proj_name << get_cur_time_string(time_string) << "]: mcast_send_thread joined. result="
                    << result << ", " << strerror(result) << endl;
            result = 0;
        }
    }

    return result;
}

int open_mcast_resources(char *caller)
{
    mqd_t qd_server;    // queue descriptors
    struct mq_attr attr;
    char    time_string[64];

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_QU_MESSAGES;
    attr.mq_msgsize = MAX_QU_MSGSIZE;
    attr.mq_curmsgs = 0;

    if((qd_server = mq_open (MULTICAST_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        cout << proj_name << get_cur_time_string(time_string) << "]: open_multicast_resources. mq_open() failed. error=" << strerror(errno) << endl;
        return -10;
    }

    mq_getattr(qd_server, &attr);
    printf("%s%s]: open_mcast_resources. mq_getattr(). mq_flags=%d, mq_maxmsg=%d, mq_msgsize=%d, mq_curmsgs=%d.\n",
            proj_name, get_cur_time_string(time_string), (int)attr.mq_flags, (int)attr.mq_maxmsg, (int)attr.mq_msgsize, (int)attr.mq_curmsgs);


    if(mq_close(qd_server) == -1) {
        printf("%s%s]: open_mcast_resources. mq_close() failed. %s\n", proj_name, get_cur_time_string(time_string), strerror(errno));
        return -11;
    }

    printf("%s%s]: open_mcast_resources. mq_close(). queue created and closed. %s\n", proj_name, get_cur_time_string(time_string), strerror(errno));

    return 0;
}


int close_mcast_resources(char *caller)
{
    char    time_string[64];
    if(mq_unlink(MULTICAST_QUEUE_NAME) == -1) { //creator destroy/unlink it. -1 means error
        printf("%s%s]: close_mcast_resources. mq_unlink() failed. %s\n", proj_name, get_cur_time_string(time_string), strerror(errno));
        return -20;
    }

    printf("%s%s]: close_mcast_resources. mq_unlink() OK. %s\n", proj_name, get_cur_time_string(time_string), strerror(errno));

    return 0;
}



int run_sensor_thread()
{
	pthread_attr_t  attr;
	int result = 0;
	char time_string[64];

	sensor_thread_info.state = THREAD_CREATING;

	result = pthread_attr_init(&attr);
	if (result != 0) {
		result = -1;

	} else {
		sensor_thread_info.conitnueRun = true;

	    result = pthread_create(&sensor_thread_info.thread_id, &attr, &sensor_thread, &sensor_thread_info);
	    if (result != 0) {
	    	result =  -2;
	    } else {

	    }
	}

	cout << proj_name << get_cur_time_string(time_string) << "]: run_sensor_thread() returns " << result << endl;

	return result;	//OK
}

int stop_sensor()
{
	int ret_value = 0;
	char time_string[64];

	sensor_thread_info.conitnueRun = false;

	if(sensor_thread_info.thread_id != 0) {

		int result = 0;
        result = pthread_join(sensor_thread_info.thread_id, NULL);
        if (result != 0) {
            cout << proj_name << get_cur_time_string(time_string) << "]: sensor thread did not exit!" << endl;
            ret_value = -1;
        }
        else
        {
        	cout << proj_name << get_cur_time_string(time_string) << "]: sensor thread joined back OK." << endl;
        }
	}

	return ret_value;
}

char* get_cur_time_string(char* buf)
{
	time_t current_time = time(0);		//get current time
	char* p = ctime_r(&current_time, buf);	//convert to text. Don't use c_time which is not thread safe

	//remove carry return char
	if(p != NULL) {
		p[strcspn(p, "\r\n")] = 0;
	}

	return p;
}

//
//Assumption:
//  1. Space between two IRLEDs or two Sensors are all equal.
//  2. They forms a rectangle box shape.
//  3. IRLED #n and Sensor #n are on the same vertical line
//  4. IRLED #0 is reference as the origin point 0,0
//ln_para line_paras[led][sensor];
//
void fill_line_parameters()
{
    char    buf[128];
    float y_distance = tok_cfg_store[IDX_EMIT_SENSOR_DISTANCE].value;
    float device_spacing = tok_cfg_store[IDX_DEV_DEV_DISTANCE].value;



    FILE *ptr_file;
    ptr_file =fopen("./data/line.txt", "w");

    bool a, b;
    float c, d, e;


    if (ptr_file) {
        fprintf(ptr_file,"%s\n", get_cur_time_string(buf));
        fprintf(ptr_file,"y_distance=%5.5f, device_spacing=%5.5f\n\n", y_distance, device_spacing);
        fprintf(ptr_file,"valid_line_paras, vertical_line, slope_m, intercept_at_y, intercept_at_x\n");
        fprintf(ptr_file,"-------------------------------------------------------------------------------------------\n");
    }

    for(int led = 0; led < tok_cfg_store[IDX_NUM_EMITTER_USED].value; led++) {
        for(int sensor = 0; sensor < tok_cfg_store[IDX_NUM_SENSOR_USED].value; sensor++) {
            if(led == sensor) { //vertical line. slope is infinity.
                line_paras[led][sensor].valid_line_paras = a = true;
                line_paras[led][sensor].vertical_line = b = true;
                line_paras[led][sensor].slope_m = c = 0;        //don't care
                line_paras[led][sensor].intercept_at_y = d = 0; //don't care
                line_paras[led][sensor].intercept_at_x = e = (device_spacing * led);

            } else {
                line_paras[led][sensor].valid_line_paras = a = true;
                line_paras[led][sensor].vertical_line = b = false;
                line_paras[led][sensor].slope_m = c = ( y_distance / ( (float)(sensor - led) * device_spacing ) );

                line_paras[led][sensor].intercept_at_x = e = (device_spacing * led);

                // Y = mX + b at point[X=e, Y=0]
                // 0 = ce + b
                // b = -ce
                line_paras[led][sensor].intercept_at_y = d = (c * e) * -1.0;
            }

            if (ptr_file) {
                fprintf(ptr_file,"%3d,\t%3d,\t%5.5f,\t%5.5f,\t%5.5f.\t%s line.\t[led=%d,sensor=%d].\n", a, b, c, d, e,
                        (led == sensor) ? "vertical" : "slanted", led, sensor);
            }
        }

        fprintf(ptr_file,"\n");
    }

    fclose(ptr_file);
}


/*
 * emit.cpp
 *
 *  Created on: Oct 25, 2017
 */

#include <iostream>
using namespace std;

#include <unistd.h>		//for sleep()
#include <stdio.h>		//for printf()
#include <semaphore.h>

#include "../pifacedigital2/pifacedigital.h"
#include "../pifacedigital2/mcp23s17.h"

#include "../signal_exit/SignalHandler.hpp"
#include "locator.h"

extern SignalHandler signalHandler;
extern token_cfg tok_cfg_store[];
extern void ns_wait(int nsec);
extern thread_info sensor_thread_info;
extern sem_t sync_emit_and_sensor;
extern char* get_cur_time_string(char* buf);
//extern int mcast_test_message();


int run_emit()
{
	int num_pfd2 = tok_cfg_store[IDX_NUM_PF2].value;
	int num_emitter = tok_cfg_store[IDX_NUM_EMITTER_USED].value;
	int num_sensors = tok_cfg_store[IDX_NUM_SENSOR_USED].value;
	int led_on_duration_nsec = tok_cfg_store[IDX_EMITTER_ON_MSEC].value * 1000 * 1000;
	int blink_period_nsec = tok_cfg_store[IDX_BLINK_PERIOD_NSEC].value;
	int blink_led_flag = tok_cfg_store[IDX_BLINK_LED_FLAG].value;

	int blink_on_time_nsec = blink_period_nsec / 2;
	int blink_off_time_nsec = blink_period_nsec /2;

	char time_string[64];

	cout << endl << "run_emit[" << get_cur_time_string(time_string) << "]: Starts emitter." << endl;
	cout << "run_emit[" << get_cur_time_string(time_string) << "]: num_pfd2:" << num_pfd2 << ", num_emitter:" << num_emitter << ", num_sensors:" << num_sensors << endl;


    //open all the pifacedigital2 boards that will be used.
	for(int pfd2 = 0; pfd2 < num_pfd2; pfd2++)
	{
		cout << "run_emit[" << get_cur_time_string(time_string) << "]: pifacedigital_open #" << pfd2 << endl;
		pifacedigital_open(pfd2);
	}

	sleep(1);

	int counter = 1;
	int working_nsec;

    while(!signalHandler.gotExitSignal())	//run until Ctrl-C is pressed
    {

    	for(int emitter = 0; emitter < num_emitter; emitter++)
    	{
    		working_nsec = led_on_duration_nsec;

    		int max_blink = working_nsec / blink_period_nsec;
    		int num_blink = 0;
    		printf("run_emit[%s]: led_on_duration_nsec=%d, working_nsec=%d, max_blink=%d, blink_on_time_nsec=%d.\n", get_cur_time_string(time_string),
    		        led_on_duration_nsec, working_nsec, max_blink, blink_on_time_nsec);

    		//these will be shared between emitter and sensor
    		//changing is guarded by sema, sync_emit_and_sensor.
    		sensor_thread_info.pfd2_num = emitter / 8;
    		sensor_thread_info.pin_num = emitter % 8;

    		cout << "run_emit[" << get_cur_time_string(time_string) << "]: counter=" << counter++ << ", pfd2_num=" << sensor_thread_info.pfd2_num << ", pin_num=" << sensor_thread_info.pin_num << endl;
			pifacedigital_write_bit(1, sensor_thread_info.pin_num, OUTPUT, sensor_thread_info.pfd2_num);	//turn on IR LED

			//release the semaphore so that sensor will have a chance to start scanning the detector input
			sem_post(&sync_emit_and_sensor);
			printf("run_emit[%s]: released semaphore sync_emit_and_sensor for sensor to scan.\n", get_cur_time_string(time_string));

			//-------------------------------------------------------------
			//Based on the flag, IR LED can be blinked for the duration or..
			//turned on solidly for the duration.
			//-------------------------------------------------------------
			if(blink_led_flag) {

				ns_wait(blink_on_time_nsec);
				working_nsec -= blink_on_time_nsec;

	            printf("run_emit[%s]: Before blinking. working_nsec=%d, max_blink=%d, blink_on_time_nsec=%d.\n", get_cur_time_string(time_string),
	                    working_nsec, max_blink, blink_on_time_nsec);

				for(num_blink = 0; num_blink < max_blink; num_blink++ ) {
					pifacedigital_write_bit(1, sensor_thread_info.pin_num, OUTPUT, sensor_thread_info.pfd2_num);	//turn on IR LED
					ns_wait(blink_on_time_nsec);
					working_nsec -= blink_on_time_nsec;

					pifacedigital_write_bit(0, sensor_thread_info.pin_num, OUTPUT, sensor_thread_info.pfd2_num);	//turn off IR LED
					ns_wait(blink_off_time_nsec);
					working_nsec -= blink_off_time_nsec;

					if(working_nsec < 0) {
						printf("run_emit[%s]: IR LED on time expired. goto wait for semaphore.\n", get_cur_time_string(time_string));
						break;
					}
				}

                printf("run_emit[%s]: After blinking. working_nsec=%d, max_blink=%d, blink_on_time_nsec=%d.\n", get_cur_time_string(time_string),
                        working_nsec, max_blink, blink_on_time_nsec);

				//time's up to switch to next IR LED
				//to grab back the semaphore from detector
				for(num_blink = 0; num_blink < 1000000000; num_blink++ ) {
					pifacedigital_write_bit(1, sensor_thread_info.pin_num, OUTPUT, sensor_thread_info.pfd2_num);	//turn on IR LED
					ns_wait(blink_on_time_nsec);

					//get semaphore. if succeeds, set success flag and break
					if(sem_trywait(&sync_emit_and_sensor) == 0) {
						printf("run_emit[%s]: got semaphore sync_emit_and_sensor at the end of blinking ON cycle.\n", get_cur_time_string(time_string));
						break;
					}

					pifacedigital_write_bit(0, sensor_thread_info.pin_num, OUTPUT, sensor_thread_info.pfd2_num);	//turn off IR LED
					ns_wait(blink_off_time_nsec);
				}

				if(num_blink == 1000000000) {
					printf("run_emit[%s]: failed in acquiring the semaphore. Fatal error, quit run_emit.\n", get_cur_time_string(time_string));
					break;	//fatal failure. quit
				}

				pifacedigital_write_bit(0, sensor_thread_info.pin_num, OUTPUT, sensor_thread_info.pfd2_num);		//turn off IR LED


			} else {	//configured to have NO blinking IR LED

				ns_wait(working_nsec);
				sem_wait(&sync_emit_and_sensor);
				printf("run_emit[%s]: got semaphore sync_emit_and_sensor at the end of solidly ON cycle.\n", get_cur_time_string(time_string));

				pifacedigital_write_bit(0, sensor_thread_info.pin_num, OUTPUT, sensor_thread_info.pfd2_num);		//turn off IR LED
			}

			//pause here just for testing ONLY
			//this sleep also blocks the sensor detection function because this emitter has already acquired the semaphore in above.
    		//sleep(2);

			//mcast_test_message();
    	}
    }

    //release the semaphore so that sensor will have a chance to start scanning the detector input then exit the while loop
    sem_post(&sync_emit_and_sensor);
    printf("run_emit[%s]: before returning, released semaphore sync_emit_and_sensor\n", get_cur_time_string(time_string));

	//close all the opened pifacedigital2 boards
	for(int pfd2 = 0; pfd2 < num_pfd2; pfd2++)
	{
		cout << "run_emit[" << get_cur_time_string(time_string) << "]: pifacedigital_close #" << pfd2 << endl;
		pifacedigital_close(pfd2);
	}


    return 0;
}

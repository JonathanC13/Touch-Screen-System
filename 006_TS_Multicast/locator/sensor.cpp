/*
 * sensor_thread.cpp
 *
 *  Created on: Oct 31, 2017
 */


#include <iostream>
#include "locator.h"
#include <string.h>		//strcspn
#include <stdio.h>		//for printf()
#include <unistd.h>		//for sleep()
#include <semaphore.h>
#include <string.h>

#include "../pifacedigital2/pifacedigital.h"
#include "../pifacedigital2/mcp23s17.h"
#include "locator.h"
#include "multicast.h"

extern void ns_wait(int nanosec);	//upto 2 seconds
extern token_cfg tok_cfg_store[NUM_TOKS_IN_FILE];
extern sem_t sync_emit_and_sensor;
extern char* get_cur_time_string(char* buf);
extern int calculate_intercept(x_y_point * xy);
extern void init_scanned_result();
extern int mcast_sensor_states(x_y_point * xy);
extern int to_mc_qu(mc_msg *udp_msg);


static thread_info *tinf;

//one scanning cycle result, hard coded to 32 sensors because one raspberry pi3 can have only four
extern one_led_result scanned_result[32];   //indexed by LED #

//line_paras[a][b]: the parameters of the straight line that connect from LED 'a' to Sensor 'b'
extern ln_para line_paras[32][32];
extern blocked_line blocked;

using namespace std;


void * sensor_thread(void *arg)
{
	uint8_t inputs[4];
	char time_string[64];

	int num_pfd2 = tok_cfg_store[IDX_NUM_PF2].value;
	bool start_scanning_cycle = false;
	int num_led_shone = 0;
	int current_shining_led;


	tinf = (thread_info*)arg;

	tinf->state = THREAD_ENTERING_LOOP;		//just for checking where is the thread is executing
	cout << "sensor_thread[" << get_cur_time_string(time_string) << "]: entered" << endl;


	//open all the pifacedigital2 boards that will be used.
	for(int pfd2 = 0; pfd2 < num_pfd2; pfd2++)
	{
		cout << "sensor_thread[" << get_cur_time_string(time_string) << "]: pifacedigital_open #" << pfd2 << endl;
		pifacedigital_open(pfd2);
		inputs[pfd2] = 0;      //print the state if all 8 bits are 0 for debugging
	}

	start_scanning_cycle = false;   //don't assume detection cycle is started
    num_led_shone = 0;              //counter of how many LED switched on then off since the start_scanning_cycle
    init_scanned_result();          //set all bits to 1 (ON) so that it can AND the 0 (broken beam) into.
                                    //detected_sensor_bits = detected_sensor_bits & current_read_sensor bit
                                    //so, 0 (OFF) will be recorded without overwritten. 0 value bit, in det_sensor_bits, stay as 0.

	current_shining_led = -1;

	while(tinf->conitnueRun)
	{

		//when the application starts, semaphore is set to be 'taken', so always wait the first time
		//acquire the semaphore. Make sure it is released if there is any BREAK
		sem_wait(&sync_emit_and_sensor);
		//printf("sensor_thread[%s]: got semaphore sync_emit_and_sensor. LED=%d\n", get_cur_time_string(time_string), tinf->pin_num);


		//---Is it time to calculate intercepts?----------------------------------------------
		//
		//check led # changing (mean emitting cycle restarted.) if yes, check any sensor has detected broken IR before.
		//if yes, count up one more led has shone. When all leds have shone, then calculate the intercept. passes the intercept to
		//its processor. then, clear 1) close_detected, 2) clear led counter, 3) clear scanned_result[] storage.
		//
		//the scanned_result[] storage structure will have the result of one cycle. At the application beginning, scanned_result[]
		//storage is initialized to all zero because it is in global variable. (Local variable will NOT be automatically zero'ed.)
		if(current_shining_led != tinf->pin_num) {  //At shinning LED # changed ONLY. It can be multiple scanning during 1 LED shinning.
		    current_shining_led = tinf->pin_num;

		    if(start_scanning_cycle) {    //the first IRLED

		        //debugging starts
		        if(num_led_shone == 0) {
		            cout << "sensor_thread[" << get_cur_time_string(time_string) << "]: started the scanning cycle." << endl;
		        }
		        cout << "sensor_thread[" << get_cur_time_string(time_string) << "]: led=" << current_shining_led << ", num_led_shone=" << num_led_shone << endl;
		        //debugging ends

		        num_led_shone++;    //now, if it is 1, 1 led has shone.

		        if(num_led_shone >= tok_cfg_store[IDX_NUM_EMITTER_USED].value) {

                    //calculate intercept based on scanned_result[]. It is thread safe because no code writing to it at this time
		            x_y_point xy;
		            calculate_intercept(&xy);  //based on the data in scanned_result[].
		            cout << "sensor_thread[" << get_cur_time_string(time_string) << "]: calculate_intercept returned. x=" << xy.x << ", y=" << xy.y << endl;
		            printf("sensor_thread[%s]: calculate_intercept returned. x=%.5f, y=%.5f\n", get_cur_time_string(time_string), xy.x, xy.y);

		            //send the result out
		            mcast_sensor_states(&xy);

                    //initialize the scanned_result[] storage structure back to default for a new cycle
		            start_scanning_cycle = false;
		            num_led_shone = 0;
		            init_scanned_result();

		            cout << "sensor_thread[" << get_cur_time_string(time_string) << "]: initialized control variables for calculation." << endl;

		        }
		    }
		}
		//---Done checking '?time to calculate intercepts'----------------------------------------------



		//do scanning. It may scan multiple time for one led if the led ON time is long.
		unsigned int states, mask;

		for(int pfd2_addr = 0; pfd2_addr < num_pfd2; pfd2_addr++)
		{
			uint8_t inp = pifacedigital_read_reg(INPUT, pfd2_addr);	//Bulk read all 8 inputs at once from one pfd2_addr


			if(inp != 0xff) {
                //the first PFD2 8-bit data goes to LSByte, .. . the fourth PFD2 8-bit data goes to MSbyte
                //since there may be multiple scanning for one led, this 'states' should be AND'ed into a master 'states'
                //in scanned_result[] for processing.
                //Straight writing 'states' into scanned_result[] will be resulting the cancellation of the detection if
                //received states are changing.
                states = ( (unsigned int)inp << (pfd2_addr * 8) );          //move the working byte into proper byte position
                mask = ~( 0xff << (pfd2_addr * 8) );                        //the working byte's bits to 0, the rest to 1
                scanned_result[tinf->pin_num].det_sensor_bits &= mask;      //clear the working byte in the result to 0
                scanned_result[tinf->pin_num].det_sensor_bits |= states;    //merge the current result in

                //triggers the counting up to the #of LED. When it is reached, starts calculate the intercept.
                //after calculation, it is set back to false to indicate another new cycle is started.
                //this is done before the scanning all configured pfd2 boards.
                start_scanning_cycle = true;
			}



            //--for debug printing starts---don't need this in release app----
			if( (inp != inputs[pfd2_addr]) || (inputs[pfd2_addr] == 0) ) {	//check current is different from previous values
				inputs[pfd2_addr] = inp;					//changed. save it for next compare.

				for(int sensor = 0; sensor < 8; sensor++)
				{
					cout << "sensor_thread[" << get_cur_time_string(time_string) << "]: Emitter pin#" << tinf->pin_num << " of pifacedigital2 #" << tinf->pfd2_num << " casuses ";
					cout << "sensor #" << sensor << " of pifacedigital2 #" << pfd2_addr << " to ";
					if( inputs[pfd2_addr] & (1 << sensor) ) {
						cout << "OPEN." << endl;
					} else {
						cout << "CLOSED." << endl;
					}
				}
			}
			//--for debug printing ends-------
		}

		//release the semaphore so that emitter will have a chance to change index and led if it needs to do.
		sem_post(&sync_emit_and_sensor);
		//printf("sensor_thread[%s]: released semaphore sync_emit_and_sensor after one scan cycle.\n", get_cur_time_string(time_string));

		ns_wait(1000);  //gives other thread a chance to grab the sema
	}


	//done. release the semaphore so that emitter will have a chance to exit.
	sem_post(&sync_emit_and_sensor);
	printf("sensor_thread[%s]: done. released semaphore sync_emit_and_sensor\n", get_cur_time_string(time_string));


	//close all the opened pifacedigital2 boards
	for(int pfd2 = 0; pfd2 < num_pfd2; pfd2++)
	{
		cout << "sensor_thread[" << get_cur_time_string(time_string) << "]: pifacedigital_close #" << pfd2 << endl;
		pifacedigital_close(pfd2);
	}


	cout << "sensor_thread[" << get_cur_time_string(time_string) << "]: left while loop." << endl;

	tinf->state = THREAD_EXITED_LOOP;

	//do clean up
	tinf->state = THREAD_RETURNING;
	tinf->ret_value = 0;

	return NULL;
}

void init_scanned_result()
{
    //initialize the scanned_result[] storage structure back to default for a new cycle
    int emitters_used = tok_cfg_store[IDX_NUM_EMITTER_USED].value;

    for(int led = 0; led < emitters_used; led++) {
        scanned_result[led].valid_scanned_data = false;
        scanned_result[led].det_sensor_bits = 0xffffffff;   //assumed all IR beams not broken
    }
}



void intercept(int i, int j, x_y_point *xy)
{
    bool i_vertical = false;
    bool j_vertical = false;

    if( blocked.line[i].led == blocked.line[i].sensor )
    {
        i_vertical = true;
    }
    if( blocked.line[j].led == blocked.line[j].sensor )
    {
        j_vertical = true;
    }

    if( i_vertical && j_vertical )      //both are vertical, no solution
    {
        xy->x = 0.0;
        xy->y = 0.0;

        printf("Both lines, %d and %d, are vertical. No solution.\n", i, j);
    }
    else
    {
        if( i_vertical )
        {
            //first line: verticaL at line #i
            int i_led = blocked.line[i].led;
            int i_sensor = blocked.line[i].sensor;

            xy->x = line_paras[i_led][i_sensor].intercept_at_x;   //in mm

            //second line: y = mx + b
            int j_led = blocked.line[j].led;
            int j_sensor = blocked.line[j].sensor;
            xy->y = ( line_paras[j_led][j_sensor].slope_m * xy->x ) + line_paras[j_led][j_sensor].intercept_at_y;

            printf("line %d is vertical. xy->x=%.02f, xy->y=%.02f \n", i, xy->x, xy->y);
        }
        else
        {
            if( j_vertical )
            {
                //first line: verticaL at line #i
                int j_led = blocked.line[j].led;
                int j_sensor = blocked.line[j].sensor;

                xy->x = line_paras[j_led][j_sensor].intercept_at_x;   //in mm

                //second line: y = mx + b
                int i_led = blocked.line[i].led;
                int i_sensor = blocked.line[i].sensor;
                xy->y = ( line_paras[i_led][i_sensor].slope_m * xy->x ) + line_paras[i_led][i_sensor].intercept_at_y;
                printf("line %d is vertical. xy->x=%.02f, xy->y=%.02f \n", j, xy->x, xy->y);
            }
            else
            {
                //neither are vertical
                int i_led = blocked.line[i].led;
                int i_sensor = blocked.line[i].sensor;
                float i_m = line_paras[ i_led ][ i_sensor ].slope_m;
                float i_b = line_paras[ i_led ][ i_sensor ].intercept_at_y;


                int j_led = blocked.line[j].led;
                int j_sensor = blocked.line[j].sensor;
                float j_m = line_paras[ j_led ][ j_sensor ].slope_m;
                float j_b = line_paras[ j_led ][ j_sensor ].intercept_at_y;

                // i_m * X + i_b = j_m * X + j_b
                // i_m * X - j_m * X = j_b - i_b
                // X(i_m - j_m) = (j_b - i_b)
                // X = (j_b - i_b) / (i_m - j_m)
                xy->x = (j_b - i_b) / (i_m - j_m);

                // Y = i_m * X + i_b
                xy->y = (i_m * xy->x) + i_b;
            }
        }
    }
}

int calculate_intercept(x_y_point * xy)
{
    //test code
    int emitters_used = tok_cfg_store[IDX_NUM_EMITTER_USED].value;
    int sensors_used = tok_cfg_store[IDX_NUM_SENSOR_USED].value;

    //ln_para lines[32];
    //for(int i = 0; i < emitters_used; i++) {
    //    lines[i] = line_paras[0][i];
    //}

    xy->x = 0.0;
    xy->y = 0.0;


    blocked_line working_blocked;
    working_blocked.count = 0;

    for(int led = 0; led < emitters_used; led++)
    {
        unsigned int bits = scanned_result[led].det_sensor_bits;

        for(int sensor=0; sensor<sensors_used; sensor++)
        {
            int mask = (1 << sensor);
            if( (bits & mask) == 0 )
            {
                //blocked
                working_blocked.line[blocked.count].led = led;
                working_blocked.line[blocked.count].sensor = sensor;
                working_blocked.count++;

                printf("working: count=%d, led=%d, sensor=%d\n",
                        working_blocked.count, working_blocked.line[working_blocked.count].led, working_blocked.line[working_blocked.count].sensor);
            }
        }
    }

    if(working_blocked.count > 1) { //2 or more

        //reduce lines per LED. Suppose if three or more adjacent lines of the same LED are blocked,
        //take the middle line as blocked line, and discards the rest. Then update the counter.
        //if there are more than one finger, now, only the leftmost finger location will be considered. Ignore others
        int emitter = working_blocked.line[0].led;
        int receiver = working_blocked.line[0].sensor;
        int blocked_adjacent_lines = 1;

        blocked.count = 0;

        for(int ln = 1; ln < working_blocked.count; ln++)
        {
            if( working_blocked.line[ln].led != emitter)
            {
                blocked.line[blocked.count].led = emitter;
                blocked.line[blocked.count].sensor = receiver + (blocked_adjacent_lines / 2); //integer divide. e.g. result of (3 / 2) is 1
                blocked.count++;
                printf("final: count=%d, led=%d, sensor=%d, blocked_adjacent_lines=%d\n",
                        blocked.count, blocked.line[blocked.count].led, blocked.line[blocked.count].sensor, blocked_adjacent_lines);

                emitter = working_blocked.line[ln].led;
                receiver = working_blocked.line[ln].sensor;
                blocked_adjacent_lines = 0;
            }
            else {
                //same led.
                if( working_blocked.line[ln].sensor == ( receiver + blocked_adjacent_lines ) )
                {
                    blocked_adjacent_lines++;
                } else {
                    //ignore others. assume that only one blocking
                }
            }
        }
        blocked.line[blocked.count].led = emitter;
        blocked.line[blocked.count].sensor = receiver + (blocked_adjacent_lines / 2); //integer divide. e.g. result of (3 / 2) is 1
        blocked.count++;
        printf("final: count=%d, led=%d, sensor=%d, blocked_adjacent_lines=%d\n",
                blocked.count, blocked.line[blocked.count].led, blocked.line[blocked.count].sensor, blocked_adjacent_lines);




        if(blocked.count > 1) {
            //pick two lines to calculate the intercept.
            //it can also use all combination of blocked lines to calculate the intercept points, then average them

            //for now use only two lines. there are two possibilities:
            //1) involved a vertical line (led# == sensor#, assume led and sensor are lined up). slope is infinity
            //2) involved two slanted lines. (led# != sensor#, assume led and sensor are lined up)

            x_y_point target = { 0.0, 0.0 };
            int avg_count = 0;

            for(int i = 0; i < blocked.count; i++)
            {
                for(int j = 0; j < blocked.count; j++)
                {
                    x_y_point xy;
                    intercept(i, j, &xy);

                    target.x += xy.x;
                    target.y += xy.y;
                    avg_count++;

                    printf("i=%d, j=%d, avg_count=%d, target.x=%.2f, target.y=%.2f\n",i, j, avg_count, target.x, target.y );
                }

            }

            xy->x = target.x / avg_count;
            xy->y = target.y / avg_count;
        }

    }



    return 0;
}

int mcast_sensor_states(x_y_point *xy)
{
    char    time_string[64];
    time_t  current_time = time(0);
    mc_msg  mc_udp_msg;

    bzero(&mc_udp_msg, sizeof(mc_udp_msg));
    mc_udp_msg.byte_count = MCMSG_LEN;  //make sure body_len is less than MAX_MC_BODY_SIZE

    //mc_udp_msg.body[MCIDX_HEADER] is zeroed
    memcpy(&mc_udp_msg.body[MCIDX_TIME_T], &current_time, 4 );     //even reserve 8 bytes, copy only the valid 4 LSBytes
    memcpy(&mc_udp_msg.body[MCIDX_X_INTERCEPT], &(xy->x), 4 );
    memcpy(&mc_udp_msg.body[MCIDX_Y_INTERCEPT], &(xy->y), 4 );

    //for now, upload sensor results for ONLY first 8 IR LED
    for(int led=0; led<8; led++)
    {
        //this type casting cause 'dereferencing type-punned pointer will break strict-aliasing rules c++' error.
        //rewrite it with memcpy
        //*(unsigned int *)&mc_udp_msg.body[MCIDX_IR0_SENSOR_ST + (led*4)] = scanned_result[led].det_sensor_bits;

        memcpy(&mc_udp_msg.body[MCIDX_IR0_SENSOR_ST + (led*4)], &scanned_result[led].det_sensor_bits, 4);
    }

    to_mc_qu(&mc_udp_msg);
    cout << "mc_intercepts[" << get_cur_time_string(time_string) << "]: msg queued. OK." << endl;

    return 0;
}


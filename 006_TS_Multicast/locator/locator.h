#ifndef LOCATOR_H_
#define LOCATOR_H_


#include <pthread.h>

//MAKE SURE THEY MATCH THE NUMBER OF ENTRIES of tok_cfg_names BECAUSE THEY ARE USED AS CROSS INDEX
struct token_cfg {
    int     value;
    char    text[256];
};

#define	IDX_TEST_PF2			0
#define IDX_NUM_PF2				1
#define IDX_NUM_SENSOR_USED		2
#define IDX_NUM_EMITTER_USED	3
#define IDX_EMITTER_ON_MSEC		4
#define IDX_BLINK_PERIOD_NSEC	5
#define IDX_BLINK_LED_FLAG		6
#define IDX_EMIT_SENSOR_DISTANCE    7
#define IDX_DEV_DEV_DISTANCE        8
#define IDX_MCAST_IP_ADDR           9
#define IDX_MCAST_IP_PORT           10
#define NUM_TOKS_IN_FILE		    11	//this number is the number of elements in the structure



//thread
struct thread_info {                //Used as argument to thread_start()
   pthread_t    thread_id;          //ID returned by pthread_create()
   int          thread_num;         //Application-defined thread #
   int          conitnueRun;        //
   int          state;          	//
   int			pfd2_num;			//
   int			pin_num;			//
   int          ret_value;
};

const int THREAD_CREATING        = 1;
const int THREAD_ENTERING_THREAD = 2;
const int THREAD_INITIALIZING    = 3;
const int THREAD_ENTERING_LOOP   = 4;
const int THREAD_EXITED_LOOP     = 5;
const int THREAD_RETURNING       = 6;


//structure to store the scanning data of one scanning cycle.
struct one_led_result {
	bool			valid_scanned_data;	//set it to false if it is nt used
	time_t			det_tm;				//time when detection was done. use time(0) to get current time.
	unsigned int	det_sensor_bits;	//the LSB is the state of the first sensor.
										//Raspberry is 32-bit system. good enough to hold all 32 sensor states.
										//In 64-bit system, top 32 bits will not be used.
};
										//pifacedigital2, each pifacedigital2 has only 8 sensors.

struct ln_para {
	bool			valid_line_paras;	//set it to false if this line is not used.
	bool			vertical_line;
	float			intercept_at_x;		//must have valid data if vertical_line is true.
	float			slope_m;			//meaningless if vertical_line is true
	float			intercept_at_y;		//meaningless if vertical_line is true. It is b of the y=mx+b
};

struct x_y_point {
    float   x;
    float   y;
};

//for defining the line by led# and sensor#
struct line_def {
    int led;        //count 0 to 7. project has 8 led
    int sensor;     //count 0 to 7. project has 8 sender
};

const int TOTAL_LINES = 64;

struct blocked_line {
    int count;
    line_def line[TOTAL_LINES];  //#led times #sensor that project has
};

#endif /* LOCATOR_H_ */

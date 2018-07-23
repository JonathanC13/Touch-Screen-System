/*
 * pfd2_test.cpp
 *
 *  Created on: Oct 25, 2017
 */

/**hw_addr: PiFaceDigital hardware address set by J1 and J2
 *
 *
chan@chan-IdeaPad-U410:~$ ssh pi@192.168.2.239
pi@192.168.2.239's password:

The programs included with the Debian GNU/Linux system are free software;
the exact distribution terms for each program are described in the
individual files in /usr/share/doc//copyright.

Debian GNU/Linux comes with ABSOLUTELY NO WARRANTY, to the extent
permitted by applicable law.
Last login: Thu Oct 19 20:17:09 2017
pi@rpi_back:~ $
pi@rpi_back:~ $ ls
Desktop    Downloads  modules  oldconffiles  opencv-3.2.0  projects  python_games  Videos
Documents  fourth     Music    opencv32      Pictures      Public    Templates     wiringPi
pi@rpi_back:~ $ cd fourth
pi@rpi_back:~/fourth $ ls
002_TS_PiFace2  pifacedigital_irq
pi@rpi_back:~/fourth $
 * */

#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../pifacedigital2/pifacedigital.h"
#include "../pifacedigital2/logger.h"
#include "../pifacedigital2/mcp23s17.h"

extern char* get_cur_time_string(char* buf);
char const* pfd2_name = "pfd2_test";

int test_pifacedigital2(int addr)
{
    uint8_t i = 0;          /**< Loop iterator */
    uint8_t inputs;         /**< Input bits (pins 0-7) */

    int interrupts_enabled; /**< Whether or not interrupts are enabled  */
    char time_string[64];

    /**
     * Open piface digital SPI connection(s)
     */
    printf("\n%s[%s]: --- Opening Piface Digital 2 Connection. ---\n", pfd2_name, get_cur_time_string(time_string) );
    printf("%s[%s]: IOHW #%d. Opening piface digital connection.\n", pfd2_name, get_cur_time_string(time_string), addr);
    pifacedigital_open(addr);


    /**
     * Enable interrupt processing (only required for all blocking/interrupt methods).
     * Reverse the return value of pifacedigital_enable_interrupts() to be consistent
     * with the variable name "interrupts_enabled". (the function returns 0 on success)
     *
     ***********************************************************************************************************************
     * Could not enable interrupt in the first run of this program. After that, all successive runs can enable the interrupt.
     * So to fix the problem, try again... .
     ***********************************************************************************************************************
     */
    interrupts_enabled = 0;
    int num_try = 1;
    int enable_interrupt_result = pifacedigital_enable_interrupts();

    printf("\n%s[%s]: --- Enable Piface Digital 2 Interrupt. ---\n", pfd2_name, get_cur_time_string(time_string) );
    while((enable_interrupt_result != 0) && (num_try < 5)) {
        printf("%s[%s]: IOHW #%d. Could not enable interrupts. num_try=%d\n", pfd2_name, get_cur_time_string(time_string), addr, num_try++);
    	sleep(1);

    	enable_interrupt_result = pifacedigital_enable_interrupts();
    }

    if(enable_interrupt_result == 0) {
		interrupts_enabled = 1;
		printf("%s[%s]: IOHW #%d. Interrupts enabled. Total_try=%d\n", pfd2_name, get_cur_time_string(time_string), addr, num_try++);

    } else {
        printf("%s[%s]: IOHW #%d. Failed to enable interrupt after %d try.\n", pfd2_name, get_cur_time_string(time_string), addr, num_try++);
    }



    //interrupts_enabled = !pifacedigital_enable_interrupts();
    //if (interrupts_enabled) {
    //	LOG_DEBUG_MSG(log_attribs[LOG_INFO_INDEX].priority, log_attribs[LOG_INFO_INDEX].ident, (char*)"IOHW #%d. Interrupts enabled.\n", hw_addr);
    //}
    //else {
    //	LOG_MSG(log_attribs[LOG_ERROR_INDEX].priority, log_attribs[LOG_ERROR_INDEX].ident, (char*)"IOHW #%d. Could not enable interrupts.\n", hw_addr);
    //}


    /**
     * Bulk set all 8 outputs at once using a hexidecimal
     * representation of the inputs as an 8-bit binary
     * number, where each bit represents an output from
     * 0-7
     */
    /* Set all outputs off (00000000) */
    printf("\n%s[%s]: --- Output in Alternate Pattern. ---\n", pfd2_name, get_cur_time_string(time_string));
    printf("%s[%s]: IOHW #%d. Setting all outputs off.\n", pfd2_name, get_cur_time_string(time_string), addr);
    pifacedigital_write_reg(0x00, OUTPUT, addr);
    sleep(1);

    /* Set output states to alternating on/off (10101010) */
    printf("%s[%s]: IOHW #%d. Setting outputs to 10101010\n", pfd2_name, get_cur_time_string(time_string), addr);
    pifacedigital_write_reg(0xaa, OUTPUT, addr);
    sleep(1);

    /* Set output states to alternating off/on (01010101) */
    printf("%s[%s]: IOHW #%d. Setting outputs to 01010101\n", pfd2_name, get_cur_time_string(time_string), addr);
    pifacedigital_write_reg(0x55, OUTPUT, addr);
    sleep(1);


    /* ***************************************************************
     * Short to High (Cross Connect) Testing - Walking '1' on OUTPUT Reg
     * ***************************************************************
     * Set all outputs off (000000) */
    printf("%s[%s]: IOHW #%d. Setting all outputs off\n", pfd2_name, get_cur_time_string(time_string), addr);
    pifacedigital_write_reg(0x00, OUTPUT, addr);


    /**
     * Read/write single input bits
     * It can be any one of these registers: INPUT, OUTPUT, GPPUB
     */

    uint8_t bit_value;
    printf("\n%s[%s]: --- Short to High (Cross Connect) Testing - Walking '1' on OUTPUT Reg. ---\n", pfd2_name, get_cur_time_string(time_string));

    for(int bit_num=0; bit_num<8; bit_num++) {

    	//turn on the output
		bit_value = 1;
		printf("%s[%s]: IOHW #%d. Writing bit value %d to bit number %d of OUTPUT Reg (%d).\n", pfd2_name, get_cur_time_string(time_string),
		        addr, bit_value, bit_num, OUTPUT);
		pifacedigital_write_bit(bit_value, bit_num, OUTPUT, addr);
		//sleep(1);

        printf("%s[%s]: IOHW #%d. Reading OUTPUT Reg (%d) Bits.\n", pfd2_name, get_cur_time_string(time_string), addr, OUTPUT);
        for(int k=0; k<8; k++) {
            bit_value = pifacedigital_read_bit(k, OUTPUT, addr);
            printf("%s[%s]: ...IOHW #%d. Reading bit number %d of OUTPUT Reg (%d). Bit value is %d\n", pfd2_name, get_cur_time_string(time_string),
                    addr, k, OUTPUT, bit_value);
        }
		sleep(1);

		//turn off the output
		bit_value = 0;
		printf("%s[%s]: IOHW #%d. Writing bit value %d to bit number %d of OUTPUT Reg (%d).\n", pfd2_name, get_cur_time_string(time_string),
		        addr, bit_value, bit_num, OUTPUT);
		pifacedigital_write_bit(bit_value, bit_num, OUTPUT, addr);
		//sleep(1);

        printf("%s[%s]: IOHW #%d. Reading OUTPUT Reg (%d) Bits.\n", pfd2_name, get_cur_time_string(time_string), addr, OUTPUT);
        for(int k=0; k<8; k++) {
            bit_value = pifacedigital_read_bit(k, OUTPUT, addr);
            printf("%s[%s]: ...IOHW #%d. Reading bit number %d of OUTPUT Reg (%d). Bit value is %d\n", pfd2_name, get_cur_time_string(time_string),
                    addr, k, OUTPUT, bit_value);
        }
        sleep(1);

        printf("\n");
    }




    /* ***************************************************************
     * Short to Low (Cross Connect) Testing - Walking '0' on OUTPUT Reg
     * ***************************************************************
     * Set all outputs off (000000) */
    printf("%s[%s]: IOHW #%d. Setting all outputs ON\n", pfd2_name, get_cur_time_string(time_string), addr);
    pifacedigital_write_reg(0xff, OUTPUT, addr);


    /**
     * Read/write single input bits
     * It can be any one of these registers: INPUT, OUTPUT, GPPUB
     */

    printf("\n%s[%s]: --- Short to Low (Cross Connect) Testing - Walking '1' on OUTPUT Reg. ---\n", pfd2_name, get_cur_time_string(time_string));

    for(int bit_num=0; bit_num<8; bit_num++) {

        //turn off the output
        bit_value = 0;
        printf("%s[%s]: IOHW #%d. Writing bit value %d to bit number %d of OUTPUT Reg (%d).\n", pfd2_name, get_cur_time_string(time_string),
                addr, bit_value, bit_num, OUTPUT);
        pifacedigital_write_bit(bit_value, bit_num, OUTPUT, addr);

        printf("%s[%s]: IOHW #%d. Reading OUTPUT Reg (%d) Bits.\n", pfd2_name, get_cur_time_string(time_string), addr, OUTPUT);
        for(int k=0; k<8; k++) {
            bit_value = pifacedigital_read_bit(k, OUTPUT, addr);
            printf("%s[%s]: ...IOHW #%d. Reading bit number %d of OUTPUT Reg (%d). Bit value is %d\n", pfd2_name, get_cur_time_string(time_string),
                    addr, k, OUTPUT, bit_value);
        }
        sleep(1);

        //turn on the output
        bit_value = 1;
        printf("%s[%s]: IOHW #%d. Writing bit value %d to bit number %d of OUTPUT Reg (%d).\n", pfd2_name, get_cur_time_string(time_string),
                addr, bit_value, bit_num, OUTPUT);
        pifacedigital_write_bit(bit_value, bit_num, OUTPUT, addr);
        //sleep(1);

        printf("%s[%s]: IOHW #%d. Reading OUTPUT Reg (%d) Bits.\n", pfd2_name, get_cur_time_string(time_string), addr, OUTPUT);
        for(int k=0; k<8; k++) {
            bit_value = pifacedigital_read_bit(k, OUTPUT, addr);
            printf("%s[%s]: ...IOHW #%d. Reading bit number %d of OUTPUT Reg (%d). Bit value is %d\n", pfd2_name, get_cur_time_string(time_string),
                    addr, k, OUTPUT, bit_value);
        }
        sleep(1);

        printf("\n");
    }






    /**
     * Set input pullups (must #include "mcp23s17.h")
     */
    pifacedigital_write_reg(0xff, GPPUB, addr);


    /**
     * Bulk read all inputs at once
     */
    printf("\n%s[%s]: --- Read INPUT Register. ---\n", pfd2_name, get_cur_time_string(time_string) );
    inputs = pifacedigital_read_reg(INPUT, addr);
    printf("%s[%s]: IOHW #%d. Inputs: 0x%x\n", pfd2_name, get_cur_time_string(time_string), addr, inputs);


    /**
     * Write each output pin individually
     */
    printf("\n%s[%s]: --- Write OUTPUT Register. ---\n", pfd2_name, get_cur_time_string(time_string) );
    for (i = 0; i < 8; i++) {
        const char *desc;
        if (i <= 1) {
        	desc = "pin with attached relay";
        }
        else {
        	desc = "pin";
        }

        /* Turn output pin i high */
        printf("%s[%s]: IOHW #%d. Setting output %s %d HIGH\n", pfd2_name, get_cur_time_string(time_string), addr, desc, (int)i);
        pifacedigital_digital_write(i, 1);
        sleep(1);

        /* Turn output pin i low */
        printf("%s[%s]: IOHW #%d. Setting output %s %d LOW\n\n", pfd2_name, get_cur_time_string(time_string), addr, desc, (int)i);
        pifacedigital_digital_write(i, 0);
        sleep(1);
    }


    /**
     * Read each input pin individually
     * A return value of 0 is pressed.
     */
    printf("\n%s[%s]: --- Read INPUT Pin. ---\n", pfd2_name, get_cur_time_string(time_string) );
    for (int i = 0; i < 8; i++) {
        uint8_t pinState = pifacedigital_digital_read(i);
        printf("%s[%s]: IOHW #%d. Input %d value: %d\n", pfd2_name, get_cur_time_string(time_string), addr, (int)i, (int)pinState);
    }


    /**
     * Wait for input change interrupt.
     * pifacedigital_wait_for_input returns a value <= 0 on failure.
     */
    printf("\n%s[%s]: --- Enabled interrupt and test switch activity 8 times. ---\n", pfd2_name, get_cur_time_string(time_string) );

    if (interrupts_enabled) {

    	for(int i=0; i<8; i++) {
    		printf("%s[%s]: IOHW #%d. iteration:%d - Waiting for input (press any button on the PiFaceDigital)\n", pfd2_name, get_cur_time_string(time_string), addr, i);
			if (pifacedigital_wait_for_input(&inputs, -1, addr) > 0) {
				printf("%s[%s]: 8-Input Status Bits: 0x%x\n", pfd2_name, get_cur_time_string(time_string), inputs);
			}
			else {
				printf("%s[%s]: IOHW #%d. iteration:%d. Can't wait for input. Something went wrong!\n", pfd2_name, get_cur_time_string(time_string), addr, i);
			}
    	}

    }
    else {
        printf("%s[%s]: IOHW #%d. Interrupts disabled, skipping interrupt tests.\n", pfd2_name, get_cur_time_string(time_string), addr);
    }

    /**
     * Close the connection to the PiFace Digital
     */
    printf("\n%s[%s]: --- IOHW #%d. Closing piface digital connection. ---\n", pfd2_name, get_cur_time_string(time_string), addr);
    pifacedigital_close(addr);

    return 0;
}




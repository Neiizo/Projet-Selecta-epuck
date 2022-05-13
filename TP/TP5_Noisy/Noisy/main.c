#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <motors.h>
#include <audio/microphone.h>
#include <camera/po8030.h>
#include <chprintf.h>

#include <pi_regulator.h>
#include <process_image.h>
#include <audio_processing.h>
#include <fft.h>
#include <communications.h>
#include <arm_math.h>
#include <spi_comm.h>

#define GOAL 			10.0f
#define DEMITOUR 		1
#define QUARTDROITE 	2
#define QUARTGAUCHE 	3
#define PI 				3.1415926536f
#define WHEELDISTANCE	5.35f
#define PERIMETRE		(PI*WHEELDISTANCE)
#define DEFAULT_SPEED 	1000 //1000
#define WHEEL_PERIMETER 13.0f
#define CMTOSTEP		1000/13.0f

#define PATH1			1
#define PATH2 			2
#define PATH3			3
#define NO_PATH 		0
#define ERROR_TRAVEL	1.0f //cm

static float distance_step = 0;
static float tof = 0;
static uint8_t rotation =0;

#define SEND_FROM_MIC

void SendUint8ToComputer(uint8_t* data, uint16_t size)
{
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)"START", 5);
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)&size, sizeof(uint16_t));
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)data, size);
}

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}
//utilité du timer ?
static void timer12_start(void){
    static const GPTConfig gpt12cfg = {
        1000000,        /* 1MHz timer clock in order to measure uS.*/
        NULL,           /* Timer callback.*/
        0,
        0
    };
    gptStart(&GPTD12, &gpt12cfg);
    //let the timer count to max value
    gptStartContinuous(&GPTD12, 0xFFFF);
}

int main(void)
{
	// initialisations :
    halInit();
    chSysInit();
    mpu_init();
    //starts the serial communication
    serial_start();
    //start the USB communication
    usb_start();
    //starts timer 12
    timer12_start();
    //starts the camera
    dcmi_start();
	po8030_start();
	//inits the motors
	motors_init();
    spi_comm_start();

	chThdSleepSeconds(3);
    mic_start(&processAudioData);

    uint32_t count = 0;
    bool starter = 0 ;

	process_image_start();

    while (1) {

    	if(get_ready_signal())
    	{
    		if(!starter)
    		{
    		mic_wait();
    			// pi_regulator_start(); // changer le fonctionnement ici
    			starter = 1;
    			re_enable_pi_regulator();
    		}

    		count ++;
    		if(!get_pi_status() || count == 10)
    		{
    			chThdSleepSeconds(3);

				if(get_code_bar() == get_code_audio())
				{
					chprintf((BaseSequentialStream *)&SDU1, "weeeeeeeeeeeee\n");
				}
    			else{
					chprintf((BaseSequentialStream *)&SDU1, "shieet\n");
				}
    			chThdSleepMilliseconds(100);
    			disable_pi_regulator();
    			mic_standby();
				count = 0;
    			starter = 0;
    		}
    	}
		else
		{
    		chprintf((BaseSequentialStream *)&SDU1, "RESET\n");
			count = 0;
			starter = 0;
			disable_pi_regulator();
			mic_standby();
		}

        chThdSleepMilliseconds(1000);
    }
}

void motor_distance(void)
{
    //chprintf((BaseSequentialStream *)&SDU1, "distance =%f, tof =  %f, distance_step = %f\n", GOAL, tof, distance_step);
    
    distance_step = GOAL *CMTOSTEP ; // conversion cm to step
    tof = distance_step*1000/DEFAULT_SPEED; // computation of the duration
    right_motor_set_speed(DEFAULT_SPEED);
    left_motor_set_speed(DEFAULT_SPEED);
    
    chThdSleepMilliseconds(tof);
    right_motor_set_speed(0);
    left_motor_set_speed(0);

}
void motor_rotation(void)
{
	chprintf((BaseSequentialStream *)&SDU1, "motor rotation\n");
	static bool flag =0;
	static float speed =0;
	static float rotation_cm =0;
	switch(rotation){
		case DEMITOUR :
			rotation_cm = (float)PERIMETRE/2;//2
			chprintf((BaseSequentialStream *)&SDU1, "1111\n");
			break;
		case QUARTDROITE :
			rotation_cm = PERIMETRE/4;
			chprintf((BaseSequentialStream *)&SDU1, "2222\n");
			if(flag)
			{
				rotation_cm = 2* rotation_cm;
				flag = 0;
			}
			else
			{
				flag = 1; // rendre plus opti
			}
			break;
		case QUARTGAUCHE :
			rotation_cm = (float)-PERIMETRE/4;
			chprintf((BaseSequentialStream *)&SDU1, "3333\n");
			if(flag)
			{
				rotation_cm = 2* rotation_cm;
						flag = 0;
			}
			else
			{
				flag = 1; // rendre plus opti
			}
			break;
	}
	static float rotation_step;
	rotation_step = rotation_cm*1000/WHEEL_PERIMETER ; // conversion cm to step
	speed = DEFAULT_SPEED*rotation_step/fabs(rotation_step);
	tof = rotation_step*4000/DEFAULT_SPEED;
	right_motor_set_speed(speed);
	left_motor_set_speed(-speed);

	chThdSleepMilliseconds(tof);
	right_motor_set_speed(0);
	left_motor_set_speed(0);
	chprintf((BaseSequentialStream *)&SDU1, "TEST = %f, tof= %d\n", speed, (uint32_t)tof);
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
   chSysHalt("Stack smashing detected");
}

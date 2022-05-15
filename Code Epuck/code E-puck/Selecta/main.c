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
#include <leds.h>

#include <pi_regulator.h>
#include <process_image.h>
#include <audio_processing.h>
#include <fft.h>
#include <communications.h>
#include <arm_math.h>
#include <spi_comm.h>

//#define DEBUG_NO_PI

#define SEND_FROM_MIC

void blink_led_error(void);
void blink_led_found(void);
void blink_led_lock(void);

void SendUint8ToComputer(uint8_t* data, uint16_t size){
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)"START", 5);
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)&size, sizeof(uint16_t));
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)data, size);
}

static void serial_start(void){
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};
	sdStart(&SD3, &ser_cfg); // UART3.
}

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

int main(void){
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

    mic_start(&processAudioData);

    uint32_t count = 0;
    bool starter = 0 ;

	process_image_start();

#ifndef DEBUG_NO_PI
	{
		pi_regulator_start();
	}
#endif

    while (1){
    	if(get_ready_signal() && !starter){
    		mic_wait();
    		starter = 1;
    		re_enable_pi_regulator();
    	}
#ifdef DEBUG_NO_PI
    	{
    		count++;
    	}
#else
    	{
    		count = -1;
    	}
#endif
    	if(!get_ready_signal()){
			disable_pi_regulator();
			mic_standby();
			count = 0;
			starter = 0;
    	}
    	if((get_found_status() && !get_lock_status()) || count == 10){
    		if(get_code_bar() == get_code_audio()){
    			blink_led_found();
    		}
    		else{
    			blink_led_error();
    		}
			disable_pi_regulator();
			mic_standby();
			count = 0;
			starter = 0;
    	}else if(get_lock_status()){
    		blink_led_lock();
    	}
    	chThdSleepMilliseconds(1000);
    }
}


void blink_led_error(void){
	set_rgb_led(0,255,0,0);
	set_rgb_led(1,255,0,0);
	set_rgb_led(2,255,0,0);
	set_rgb_led(3,255,0,0);
	chThdSleepSeconds(3);
	set_rgb_led(0,0,0,0);
	set_rgb_led(1,0,0,0);
	set_rgb_led(2,0,0,0);
	set_rgb_led(3,0,0,0);
}

void blink_led_lock(void){
	set_rgb_led(0,0,0,255);
	set_rgb_led(1,0,0,255);
	set_rgb_led(2,0,0,255);
	set_rgb_led(3,0,0,255);
	chThdSleepSeconds(3);
	set_rgb_led(0,0,0,0);
	set_rgb_led(1,0,0,0);
	set_rgb_led(2,0,0,0);
	set_rgb_led(3,0,0,0);
}

void blink_led_found(void){
	set_body_led(1);
	chThdSleepSeconds(3);
	set_body_led(0);
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void){
   chSysHalt("Stack smashing detected");
}

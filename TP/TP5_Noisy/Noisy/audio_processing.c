#include "ch.h"
#include "hal.h"
#include <main.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <motors.h>
#include <audio/microphone.h>
#include <audio_processing.h>
#include <communications.h>
#include <fft.h>
#include <arm_math.h>
#include <leds.h>

//semaphore
//static BSEMAPHORE_DECL(sendToComputer_sem, TRUE);

//2 times FFT_SIZE because these arrays contain complex numbers (real + imaginary)
static float micLeft_cmplx_input[2 * FFT_SIZE];
static float micRight_cmplx_input[2 * FFT_SIZE];
static float micFront_cmplx_input[2 * FFT_SIZE];
static float micBack_cmplx_input[2 * FFT_SIZE];
//Arrays containing the computed magnitude of the complex numbers
static float micLeft_output[FFT_SIZE];
static float micRight_output[FFT_SIZE];
static float micFront_output[FFT_SIZE];
static float micBack_output[FFT_SIZE];
//static float mic_used[2*FFT_SIZE];

static int16_t max_norm_index = -1; //index

#define MIN_VALUE_THRESHOLD	10000

#define MIN_POS 		0	//we don't analyze before this index to not use resources for nothing
#define MIN_FREQ		15000
#define BUENO			400 	//Hz
#define MARS 			600 	//Hz
#define SNICKERS		500 	//Hz
#define STOP_SEARCH		700 	//Hz
#define UNLOCK_SEARCH	900		//Hz  AJUSTER FREQ
#define LOCK_SEARCH		1100	//Hz
#define ERROR_FREQ		30  	//Hz
#define NO_CODE 		0
#define BUENO_CODE		2
#define SNICKERS_CODE	5
#define MARS_CODE		6
#define STOP_CODE 		7
#define LOCK_CODE		8

//#define DEBUG_AUDIO


#define FREQ_FORWARD_L		(FREQ_FORWARD-1)
#define FREQ_FORWARD_H		(FREQ_FORWARD+1)
#define FREQ_LEFT_L			(FREQ_LEFT-1)
#define FREQ_LEFT_H			(FREQ_LEFT+1)
#define FREQ_RIGHT_L		(FREQ_RIGHT-1)
#define FREQ_RIGHT_H		(FREQ_RIGHT+1)
#define FREQ_BACKWARD_L		(FREQ_BACKWARD-1)
#define FREQ_BACKWARD_H		(FREQ_BACKWARD+1)


static uint8_t search = NO_CODE;
static bool waiting = 0;
static bool locked = 0;
static unsigned int size = 0;
static float frequency = 0;
static bool is_searching = 0;

unsigned int detectPeak1(float *data)
{
	float tmp_max = MIN_FREQ; 
	unsigned int tmp_index = MIN_POS;
	for(unsigned int i = MIN_POS; i < FFT_SIZE/4; i++)
	{
		if(data[i] > tmp_max)
		{
			tmp_max = data[i];
			tmp_index= i;
		}
	}
	return tmp_index;
}

void processAudioData(int16_t *data){

	unsigned int pos_tmp = 10;

	for(unsigned int i=0; i < 160; i++)
	{
		micRight_cmplx_input[2*size] = data[4*i];
		micLeft_cmplx_input[2*size] = data[4*i+1];
		micBack_cmplx_input[2*size] = data[4*i+2];
		micFront_cmplx_input[2*size] = data[4*i+3];

		micRight_cmplx_input[2*size+1] = 0;
		micLeft_cmplx_input[2*size+1] = 0;
		micBack_cmplx_input[2*size+1] = 0;
		micFront_cmplx_input[2*size+1] = 0;
		size++;

		if(size >= FFT_SIZE)
		{
			break;
		}
	}

	if(size >= FFT_SIZE)
	{
		doFFT_optimized(FFT_SIZE, micLeft_cmplx_input);
		arm_cmplx_mag_f32(micLeft_cmplx_input, micLeft_output, FFT_SIZE);
		size = 0;
		pos_tmp = detectPeak1(micLeft_output);
			
		frequency = pos_tmp*15.1925; 

		if(!waiting && frequency !=0 && !locked) // FAIRE UN LIEN ENTRE LOCK ET GO SEARCHING
		{
			if(frequency > SNICKERS - ERROR_FREQ && frequency < SNICKERS + ERROR_FREQ)
			{
				search = SNICKERS_CODE;
			}
			else if(frequency > MARS - ERROR_FREQ && frequency < MARS + ERROR_FREQ)
			{
				search = MARS_CODE;
			}
			else if(frequency > BUENO - ERROR_FREQ && frequency < BUENO + ERROR_FREQ)
			{
				search = BUENO_CODE;
			}
			else if(frequency > LOCK_SEARCH - ERROR_FREQ && frequency < LOCK_SEARCH + ERROR_FREQ)
			{
				search = LOCK_CODE;
			}
			else
			{
				search = NO_CODE;
			}
		}
		
		if(frequency > STOP_SEARCH - ERROR_FREQ && frequency < STOP_SEARCH + ERROR_FREQ && !locked)
		{
			search = STOP_CODE;
			is_searching = 0; //utilité?
		}
		if(frequency > UNLOCK_SEARCH - ERROR_FREQ && frequency < UNLOCK_SEARCH + ERROR_FREQ)
		{
			search = NO_CODE;
			is_searching = 0; //utilité?
			locked = FALSE;
		}
		if(search == LOCK_CODE)
		{
			locked = TRUE;
		}
		else if(search != NO_CODE && search != STOP_CODE)
		{
			is_searching = TRUE;
		}
#ifdef DEBUG_AUDIO
		{
			chprintf((BaseSequentialStream *)&SDU1, "frequency = %f\n", frequency);
			chprintf((BaseSequentialStream *)&SDU1, "CODE %d\n", search);
		}
#endif
	}
}

void mic_wait(void){
	waiting = TRUE;
}

void mic_standby(void){
	if(search != LOCK_CODE)
	{
		search = NO_CODE;
	}
	is_searching = FALSE;
	waiting = FALSE;
}

bool get_ready_signal(void)
{
	return is_searching;
}

uint8_t get_code_audio(void)
{
	return search;
}

float* get_audio_buffer_ptr(BUFFER_NAME_t name){
	if(name == LEFT_CMPLX_INPUT){
		return micLeft_cmplx_input;
	}
	else if (name == RIGHT_CMPLX_INPUT){
		return micRight_cmplx_input;
	}
	else if (name == FRONT_CMPLX_INPUT){
		return micFront_cmplx_input;
	}
	else if (name == BACK_CMPLX_INPUT){
		return micBack_cmplx_input;
	}
	else if (name == LEFT_OUTPUT){
		return micLeft_output;
	}
	else if (name == RIGHT_OUTPUT){
		return micRight_output;
	}
	else if (name == FRONT_OUTPUT){
		return micFront_output;
	}
	else if (name == BACK_OUTPUT){
		return micBack_output;
	}
	else{
		return NULL;
	}
}

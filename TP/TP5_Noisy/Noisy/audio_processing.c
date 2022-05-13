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
static BSEMAPHORE_DECL(sendToComputer_sem, TRUE);

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
static float mic_used[2*FFT_SIZE];

static int16_t max_norm_index = -1; //pour la récupérer

#define MIN_VALUE_THRESHOLD	10000

<<<<<<< Updated upstream
#define MIN_FREQ		10	//we don't analyze before this index to not use resources for nothing
#define FREQ_FORWARD	16	//250Hz
#define FREQ_LEFT		19	//296Hz
#define FREQ_RIGHT		23	//359HZ
#define FREQ_BACKWARD	26	//406Hz
#define MAX_FREQ		500	//we don't analyze after this index to not use resources for nothing
#define LEFT 			1
#define RIGHT	 		2
#define FRONT			3
#define BACK 			4
#define MIC				LEFT //Lets you choose which mic you want to work with
=======
#define MIN_POS 		0	//we don't analyze before this index to not use resources for nothing
#define MIN_FREQ		15000
#define BUENO			400 //Hz
#define MARS 			500 //Hz
#define SNICKERS		600 //Hz
#define STOP_SEARCH		700 //Hz
#define LOCK_SEARCH		900
#define ERROR_FREQ		30  //Hz
#define BUENO_CODE		2
#define MARS_CODE		3
#define SNICKERS_CODE	5
#define STOP_CODE 		7
#define NO_CODE 		0
#define LOCK_CODE		8
>>>>>>> Stashed changes


#define FREQ_FORWARD_L		(FREQ_FORWARD-1)
#define FREQ_FORWARD_H		(FREQ_FORWARD+1)
#define FREQ_LEFT_L			(FREQ_LEFT-1)
#define FREQ_LEFT_H			(FREQ_LEFT+1)
#define FREQ_RIGHT_L		(FREQ_RIGHT-1)
#define FREQ_RIGHT_H		(FREQ_RIGHT+1)
#define FREQ_BACKWARD_L		(FREQ_BACKWARD-1)
#define FREQ_BACKWARD_H		(FREQ_BACKWARD+1)
/*
*	Callback called when the demodulation of the four microphones is done.
*	We get 160 samples per mic every 10ms (16kHz)
*	
*	params :
*	int16_t *data			Buffer containing 4 times 160 samples. the samples are sorted by micro
*							so we have [micRight1, micLeft1, micBack1, micFront1, micRight2, etc...]
*	uint16_t num_samples	Tells how many data we get in total (should always be 640)
*/

//void mic_init()
//{
//	switch(MIC)
//	{
//	case LEFT:
//		mic_used = micLeft_cmplx_input;
//		break;
//	case RIGHT:
//		mic_used = micRight_cmplx_input;
//		break;
//	case FRONT:
//		mic_used = micFront_cmplx_input;
//		break;
//	case BACK:
//		mic_used = micBack_cmplx_input;
//		break;
//	}
//}

<<<<<<< Updated upstream
uint16_t get_position_freq()
{
	return micLeft_cmplx_input[max_norm_index];
}

static float current_max =0;  //faire une remise a 0 plus clean que ca
static unsigned int tmp_index =0;
unsigned int detectPeak2(float data, unsigned int index)
=======

static uint8_t search = NO_CODE;
static bool waiting = 0;
static bool locked = 0;
static unsigned int size = 0;
static float frequency = 0;
static bool is_searching = 0;

unsigned int detectPeak1(float *data)
>>>>>>> Stashed changes
{
	if(data > current_max)
	{
		current_max = data;
		tmp_index = index;
	}
	return tmp_index;
}

<<<<<<< Updated upstream
void sound_remote(float* data){
	float max_norm = MIN_VALUE_THRESHOLD;

	//search for the highest peak
	for(uint16_t i = MIN_FREQ ; i <= MAX_FREQ ; i++){
		if(data[i] > max_norm){
			max_norm = data[i];
			max_norm_index = i;
		}
	}

	//go forward
//	if(max_norm_index >= FREQ_FORWARD_L && max_norm_index <= FREQ_FORWARD_H){
//		left_motor_set_speed(600);
//		right_motor_set_speed(600);
//	}
//	//turn left
//	else if(max_norm_index >= FREQ_LEFT_L && max_norm_index <= FREQ_LEFT_H){
//		left_motor_set_speed(-600);
//		right_motor_set_speed(600);
//	}
//	//turn right
//	else if(max_norm_index >= FREQ_RIGHT_L && max_norm_index <= FREQ_RIGHT_H){
//		left_motor_set_speed(600);
//		right_motor_set_speed(-600);
//	}
//	//go backward
//	else if(max_norm_index >= FREQ_BACKWARD_L && max_norm_index <= FREQ_BACKWARD_H){
//		left_motor_set_speed(-600);
//		right_motor_set_speed(-600);
//	}
//	else{
//		left_motor_set_speed(0);
//		right_motor_set_speed(0);
//	}

}

static unsigned int temp = 0;
static unsigned int size = 0;
static float frequency = 0;
void processAudioData(int16_t *data, uint16_t num_samples){
=======
void processAudioData(int16_t *data){
>>>>>>> Stashed changes

	/*
	*
	*	We get 160 samples per mic every 10ms
	*	So we fill the samples buffers to reach
	*	1024 samples, then we compute the FFTs.
	*
	*/

	unsigned int pos_tmp = 01;
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

		pos_tmp = detectPeak2(micLeft_cmplx_input[2*size], 2*size);// we do it here to avoid double checking the table of input.

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
<<<<<<< Updated upstream
		temp++;
		if(pos_tmp < 1 && pos_tmp > FFT_SIZE)
		{
			pos_tmp = 1;
		}
		if(temp == 10)
		{
			temp = 0;
			chBSemSignal(&sendToComputer_sem);

			frequency = (float)pos_tmp*7.7-10; // optimiser en faisant une recherche sur le domaine qui nous interesse, par exemple entre 100 et 1000Hz.
			chprintf((BaseSequentialStream *)&SDU1, "freq %f\n", frequency);

			current_max =0;
			tmp_index =0;
		}

		if(frequency > 450 && frequency < 550)
		{
			// code pour rigoler led mdr lol
			set_body_led(1);
			chThdSleep(500);
			set_body_led(0);
		}
		if(frequency > 650 && frequency < 750)
		{
			set_rgb_led(LED7, 100, 100, 100);
		}
	}
}


void wait_send_to_computer(void){
	chBSemWait(&sendToComputer_sem);
}

=======
		pos_tmp = detectPeak1(micLeft_output);
			
		frequency = pos_tmp*15.1925; 

		if(!waiting && frequency !=0 && !locked) // FAIRE UN LIEN ENTRE LOCK ET GO SEARCHING
		{
			chprintf((BaseSequentialStream *)&SDU1, "frequency = %f\n", frequency);
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
		
		if(frequency > STOP_SEARCH - ERROR_FREQ && frequency < STOP_SEARCH + ERROR_FREQ)
		{
			search = STOP_CODE;
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
	}
}

void mic_wait(void){
	waiting = TRUE;
}

void mic_standby(void){
	search = NO_CODE;
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

>>>>>>> Stashed changes
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

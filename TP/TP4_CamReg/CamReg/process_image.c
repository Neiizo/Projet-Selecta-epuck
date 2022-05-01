#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>
#define BITRED  15
#define BITGREEN  10
#define BITBLUE  4
#define NB_BAR 3
#define ERROR_PIXEL 20

static float distance_cm = 0;
static float PXTOCM = 1400;
static uint16_t line_pos =0;
static uint32_t posList[NB_BAR*2];


//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){

    	//chThdSleep(12);
    	systime_t time;
    	time = chVTGetSystemTime();
    	//starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
		chThdSleep(100);


    }
}

uint16_t detection(uint8_t *image){
	bool found = 0;
	uint8_t count =0;
	uint32_t moy=0;
	uint8_t lineCount = 0;
	bool codeStart =0;
	for(unsigned int i=0; i < IMAGE_BUFFER_SIZE; i++){
		moy += image[i];
		count++;
		if(count == 5)
		{
			moy = moy/count;
			if(moy <4 || moy == 4)
			{
				if(found == 0)
				{
					if(!codeStart)
					{
						codeStart = 1;
						line_pos= i-count+1;
					}
					posList[2*lineCount] = i;
					found = 1;
				}
			}
			else if((found == 1) && (moy > 5))
			{
				found = 0;
				lineCount ++;
				posList[2*lineCount-1] = i;
				if(lineCount == NB_BAR)
				{
					return i-line_pos;
				}
			}
			moy =0;
			count=0;
		}
	}
	return 0; // ajouter un message d'erreur ou ajouter un retour de la largeur actuelle
}

uint8_t detect_codebarre(uint32_t width)
{
	uint8_t width_1 = posList[1]- posList[0];
	uint8_t width_2 = posList[3]- posList[2];
	uint8_t width_3 = posList[5]- posList[4];

	float big_width = (float)width*1.2/3.9;
	float small_width = (float)width*0.6/3.9;
	uint8_t code = 0b000;
	if((width_1 < big_width+ERROR_PIXEL)&&(width_1 > big_width-ERROR_PIXEL))
	{
		code |= 0b100;
	}
	if((width_2 < big_width+ERROR_PIXEL)&&(width_2 > big_width-ERROR_PIXEL))
	{
		code |= 0b010;
	}
	if((width_3 < big_width+ERROR_PIXEL)&&(width_3 > big_width-ERROR_PIXEL))
	{
		code |= 0b001;
	}
	return code ;
}



static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};
	bool boucle = 0;
	uint16_t width =0;
    while(1){
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
        //gets the pointer to the array filled with the last image in RGB565
		img_buff_ptr = dcmi_get_last_image_ptr();

		uint8_t temp = 0;
		boucle = !boucle;
		if (boucle) {
			for(unsigned int i = 0; i < IMAGE_BUFFER_SIZE; ++i){
				image[i] = img_buff_ptr[2*i+1];
				image[i] &= 0b00011111; // blue

//				temp = img_buff_ptr[2*i+1];
//				temp = temp >> 5;
//				temp &= 0b00000111;
//				image[i] = temp;
//				temp = img_buff_ptr[2*i];
//				temp = temp << 3;
//				temp &= 0b00111000;
//				image[i] |= temp;

//				image[i] =img_buff_ptr[2*i]>>3;
//				image[i] &= 0b00011111; //rouge
			}
			width = detection(image);
			if(width ==0)
			{
				chprintf((BaseSequentialStream *)&SDU1, "Distance not found\n");

			}
			else
			{
				distance_cm = PXTOCM/width;
				distance_cm = distance_cm*1.925;
				// ici faire pour différentes lignes
				uint8_t code = detect_codebarre(width);

//				uint8_t width_1 = posList[1]- posList[0];
//				uint8_t width_2 = posList[3]- posList[2];
//				uint8_t width_3 = posList[5]- posList[4];

				chprintf((BaseSequentialStream *)&SDU1, "Distance = %f, code = %d\n", distance_cm, code);
//				chprintf((BaseSequentialStream *)&SDU1, "Distance = %f, width1 = %d, width2 = %d, width3 = %d\n", distance_cm, width_1, width_2, width_3);

			}
			SendUint8ToComputer(image, IMAGE_BUFFER_SIZE); // x pixels * 2 bytes !!!
		}
    }
}

float get_distance_cm(void){
	return distance_cm;
}

uint16_t get_line_pos(void){
	return line_pos;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}

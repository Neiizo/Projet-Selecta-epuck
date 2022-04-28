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

static float distance_cm = 0;
static float PXTOCM = 1400;
static uint16_t line_pos =0;
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
	uint16_t width = 0;
	bool found = 0;
	uint8_t count =0;
	uint32_t moy=0;
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
					line_pos= i-count+1;
					found = 1;
				}
				width +=count;
			}
			else if((found == 1) && (moy > 5))
			{
				return width;
			}
			moy =0;
			count=0;
		}
	}
	return 0;
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

		/*
		*	To complete
		*/
		boucle = !boucle;
		if (boucle) {
			for(unsigned int i = 0; i < IMAGE_BUFFER_SIZE; ++i){
				image[i] = img_buff_ptr[2*i+1];
				image[i] &= 0b00011111;

			}
			width = detection(image);
			//PXTOCM = data.width/realLineSize;
			distance_cm = PXTOCM/width;
			// ici faire pour différentes lignes
			chprintf((BaseSequentialStream *)&SDU1, "Distance = %f, Position = %d, width = %d\n", distance_cm, line_pos, width);

//			chprintf((BaseSequentialStream *)&SDU1, "Position = %d, width = %d\n", data.pos, data.width);

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

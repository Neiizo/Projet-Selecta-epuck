#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <motors.h>
#include <pi_regulator.h>
#include <process_image.h>

static uint16_t sampling_time = 0 ;
static const float Kp = 0; // mettre en define ?
static const float Ki = 0;

uint16_t pi_regulator(float distance, float goal_dist){
	float erreur = 0;
	float vitesse = 0;

	static float somme_erreurs = 0;

	erreur = goal_dist - distance ;

	somme_erreurs += erreur;

	vitesse = Kp * erreur + Ki * somme_erreurs;

	return (int16_t)vitesse;

}

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;
    int16_t speed_correction = 0 ;

    while(1){
        time = chVTGetSystemTime();

        /*
		*	To complete
		*/
        speed = pi_regulator(get_distance_cm(), 10);
        speed_correction = (get_line_pos(), IMAGE_BUFFER_SIZE/2);

        if(abs(speed_correction) < ROTATION_THRESHOLD){
        	speed_correction = 0;
        }

//        chThdSleep(sampling_time);
        
        //applies the speed from the PI regulator
		 right_motor_set_speed(speed);
		 left_motor_set_speed(speed);

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}

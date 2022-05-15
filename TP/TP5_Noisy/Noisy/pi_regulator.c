#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>

#include <main.h>
#include <motors.h>
#include <pi_regulator.h>
#include <process_image.h>

#include <communications.h>

static bool en_pi_regulator = 0;
static bool stop_found = 0;

uint16_t pi_regulator(float distance, float goal_dist){

	float erreur = 0;
	float vitesse = 0;

	static float somme_erreurs = 0;

	erreur = - goal_dist + distance ;

	if(fabs(erreur) < ERROR_THRESHOLD){
		return 0;
	}

	somme_erreurs += erreur;

	if(somme_erreurs > MAX_SUM_ERROR){
		somme_erreurs = MAX_SUM_ERROR;
	}
    else if(somme_erreurs < -MAX_SUM_ERROR)
    {
    	somme_erreurs = -MAX_SUM_ERROR;
    }

	vitesse = KP * erreur + KI * somme_erreurs;

	return (int16_t)vitesse;

}
static THD_WORKING_AREA(waPiRegulator, 1024);
static THD_FUNCTION(PiRegulator, arg) {

	chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;
    int16_t speed_correction = 0 ;

    while(1){
        time = chVTGetSystemTime();

        speed = pi_regulator(get_distance_cm(), GOAL_DISTANCE );
        speed_correction = (get_line_pos() - IMAGE_BUFFER_SIZE/2);

        if(abs(speed_correction) < ROTATION_THRESHOLD){
        	speed_correction = 0;
        }

        //applies the speed from the PI regulator

		 if(abs(speed) < SPEED_MIN && !stop_found){
			 stop_found = 1;
		 }
		 if(!stop_found && en_pi_regulator) {
			 right_motor_set_speed(speed - ROTATION_COEFF * speed_correction);
			 left_motor_set_speed(speed + ROTATION_COEFF * speed_correction);
		 }

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void re_enable_pi_regulator(void){
	en_pi_regulator = 1;
}

void disable_pi_regulator(void){
	en_pi_regulator = 0;
	stop_found = 0;
	right_motor_set_speed(0);
	left_motor_set_speed(0);
}

bool get_pi_status(void){
	return en_pi_regulator;
}

bool get_found_status(void){
	return stop_found;
}


void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL); //-1
}

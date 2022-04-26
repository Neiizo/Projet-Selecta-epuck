#include <stm32f4xx.h>
#include <gpio.h>

#define TIMER_CLOCK 0    	// TODO: configure APB1 clock
#define PRESCALER   0       // TODO: configure timer frequency
#define COUNTER_MAX 0       // TODO: configure timer max counter

void timer7_start(void)
{
    / enable TIM4 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    // configure TIM4
    TIM4->PSC = PRESCALER_TIM4 - 1; // Note: final timer clock = timer clock / (prescaler + 1)
    TIM4->ARR = COUNTER_MAX_TIM4 - 1; // Note: timer reload takes 1 cycle, thus -1
    // enable TIM4
    TIM4->CR1 |= TIM_CR1_CEN;
}

// Timer 7 Interrupt Service Routine
void TIM7_IRQHandler(void)
{
    /*
    *
    *   BEWARE !!
    *   Based on STM32F40x and STM32F41x Errata sheet - 2.1.13 Delay after an RCC peripheral clock enabling
    *
    *   As there can be a delay between the instruction of clearing of the IF (Interrupt Flag) of corresponding register (named here CR) and
    *   the effective peripheral IF clearing bit there is a risk to enter again in the interrupt if the clearing is done at the end of ISR.
    *
    *   As tested, only the workaround 3 is working well, then read back of CR must be done before leaving the ISR
    *
    */

    /* do something ... */

    // Clear interrupt flag
    TIM7->SR &= ~TIM_SR_UIF;
    TIM7->SR;	// Read back in order to ensure the effective IF clearing
}

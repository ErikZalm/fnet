#include "fnet_config.h"

#include "fnet.h"
#include "fnet_timer_prv.h"
#include "fnet_isr.h"

#include "lpc_debug.h"

#ifdef FNET_LPC



#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#define PCONP_ENABLE_PCTIM2 (1<<22)

#define enable_timer2_power() LPC_SC->PCONP |= PCONP_ENABLE_PCTIM2
#define disable_timer2_power() LPC_SC->PCONP &= ~PCONP_ENABLE_PCTIM2

#define timer2_set_pclk4() LPC_SC->PCLKSEL1 &= ~(3<<12)
#define timer2_counter_reset() LPC_TIM2->TCR = 0x2

#define timer2_set_timer_mode() LPC_TIM2->CTCR = 0
#define timer2_interrupt_reset_match() LPC_TIM2->MCR=0x3

#define timer2_set_interval0(interval) LPC_TIM2->MR0 = interval

#define timer2_set_enable() LPC_TIM2->TCR = 1
#define timer2_set_disable() LPC_TIM2->TCR = 0

void fnet_cpu_timer_handler_top() {
	LPC_TIM2->IR = 1;
	fnet_timer_ticks_inc();
}
void fnet_cpu_timer_handler_bottom() {
	fnet_timer_handler_bottom();
}

void TIMER2_IRQHandler(void) {
	//fnet_cpu_timer_handler_top();
	fnet_isr_handler(TIMER2_IRQn);

}
int fnet_cpu_timer_init(unsigned int period_ms) {
	uint32_t pClk4 = SystemCoreClock / 4;
	float tickTime = 1.0 / ((float) pClk4);
	float seconds = (float) period_ms * 0.001;
	uint32_t counts = seconds / tickTime;

	// Turn on the power
	enable_timer2_power();
	// Select the clock
	timer2_set_pclk4();
	// Set reset
	timer2_counter_reset();
	timer2_set_timer_mode();
	// Set prescaler to 0
	//LPC_TIM3->PR = 0;
	// Set interrupt and reset on match
	timer2_interrupt_reset_match();
	// Set the match register
	timer2_set_interval0(counts);
	//LPC_TIM3->CTCR = 0; // use counter mode
	// Enable interrupt
	NVIC_EnableIRQ(TIMER2_IRQn);
	fnet_isr_vector_init(TIMER2_IRQn,fnet_cpu_timer_handler_top,fnet_timer_handler_bottom,0);
	//NVIC_SetVector(UART0_IRQn, (uint32_t) &fnet_cpu_timer_handler_top);
	// Enable LPC timer 3
	timer2_set_enable();
	return FNET_OK;
}

/************************************************************************
* NAME: fnet_cpu_timer_release
*
* DESCRIPTION: Relaeses TCP/IP hardware timer.
*
*************************************************************************/
void fnet_cpu_timer_release( void )
{
	timer2_set_disable();
	disable_timer2_power();

}
#endif

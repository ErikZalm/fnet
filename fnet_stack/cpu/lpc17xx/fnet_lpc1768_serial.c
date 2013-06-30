/*
 * fnet_lpc1768_serial.c
 *
 *  Created on: Dec 8, 2012
 *      Author: matt
 */

#include "fnet.h"

#if FNET_LPC

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif


/********************************************************************/
void fnet_cpu_serial_putchar (long port_number, int character)
{
	while((LPC_UART0->LSR & 0x40) != 0x40);
	LPC_UART0->THR = character;
 }
/********************************************************************/
int fnet_cpu_serial_getchar (long port_number)
{
	if ((LPC_UART0->LSR & 0x01) == 0x01)
		return LPC_UART0->RBR;

    return FNET_ERR;
}

/********************************************************************/
void fnet_cpu_serial_init(long port_number, unsigned long baud_rate)
{
	// TODO: insert code here

		// Set the pinsel on txd0,rxd0
		LPC_PINCON ->PINSEL0 &= ~((0x3 << 4) | (0x3 << 6));
		LPC_PINCON ->PINSEL0 |= (1 << 4) | (1 << 6);

		// Set RXD0 to input

		// Enable UART0
		LPC_SC ->PCONP |= (1 << 3);
		LPC_GPIO0->FIODIR0 |= 0x4;
		// Set clock to PCLK/4
		//LPC_SC->PCLKSEL0 &= ~(0x3 << 6);

		// Set clock to PCLK/8
		LPC_SC->PCLKSEL0 |= (0x3 << 6);
		// 8-N-1 configuration
		LPC_UART0 ->LCR = (0x3);
		// Enable access to divisor latch access bits
		LPC_UART0 ->LCR |= (1 << 7);

		// Set bit rate to 9600
		LPC_UART0 ->DLM = 0;
		LPC_UART0 ->DLL = 54;

		LPC_UART0 ->FDR = (2 << 4) | (1 << 0);

		// Remove access to divisor latch access bits
		LPC_UART0 ->LCR &= ~(1 << 7);
		// Enable RX,TX
		LPC_UART0 ->FCR = (0x7);

		// Enable receive interrupt
		//LPC_UART0->IER = 1;
		//NVIC_EnableIRQ(UART0_IRQn);

}

#endif

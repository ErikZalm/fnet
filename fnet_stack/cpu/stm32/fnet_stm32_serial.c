 /**************************************************************************
*
* @file fnet_stm32_serial.c
*
* @author inca
*
* @date Jun-23-2013
*
* @version 0.0.0.0
*
* @brief ChibiOS "CPU-specific" Serial API implementation.
*
***************************************************************************/

#include <stdarg.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "fnet.h"

#if FNET_STM32

/********************************************************************/
void fnet_cpu_serial_putchar (long port_number, int character)
{
  (void) port_number;
  chSequentialStreamPut((BaseSequentialStream *)&SD1, (uint8_t)character);
 }
/********************************************************************/
int fnet_cpu_serial_getchar (long port_number)
{
  (void) port_number;
  return chSequentialStreamGet((BaseSequentialStream *)&SD1);
}

/********************************************************************/
void fnet_cpu_serial_init(long port_number, unsigned long baud_rate)
{
  // Already done in ChibiOS main() via hal and such
  (void) port_number;
  (void) baud_rate;
  
		// Enable receive interrupt
		//LPC_UART0->IER = 1;
		//NVIC_EnableIRQ(UART0_IRQn);
}

#endif /*FNET_STM32*/
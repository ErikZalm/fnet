/*
 * fnet_lpc.c
 *
 *  Created on: Dec 9, 2012
 *      Author: matt
 */

#include "fnet.h"

#if FNET_LPC
/************************************************************************
* NAME: fnet_cpu_irq_disable
*
* DESCRIPTION: Disable IRQs
*************************************************************************/
fnet_cpu_irq_desc_t fnet_cpu_irq_disable(void)
{
     return 0;
}

/************************************************************************
* NAME: fnet_cpu_irq_enable
*
* DESCRIPTION: Enables IRQs.
*************************************************************************/
void fnet_cpu_irq_enable(fnet_cpu_irq_desc_t irq_desc)
{
	char x;
	x++; // stub
}

// stub function, we use CMSIS for now
int fnet_cpu_isr_install(unsigned int vector_number, unsigned int priority)
{
	return FNET_OK;
}
#endif

/**************************************************************************
*
* @file fnet_stm32.c
*
* @author inca
*
* @date Jun-23-2013
*
* @version 0.0.0.0
*
* @brief ChibiOS "CPU-specific" API implementation.
*
***************************************************************************/
#include "fnet.h"

#if FNET_STM32
#if 0
/************************************************************************
* NAME: fnet_cpu_irq_disable
*
* DESCRIPTION: Disable IRQs
*************************************************************************/
fnet_cpu_irq_desc_t fnet_cpu_irq_disable(void)
{
  //nvicDisableVector(uint32_t interrupt_number)
  return 0;
}

/************************************************************************
* NAME: fnet_cpu_irq_enable
*
* DESCRIPTION: Enables IRQs.
*************************************************************************/
void fnet_cpu_irq_enable(fnet_cpu_irq_desc_t irq_desc)
{
  // CMSIS:
  //nvicEnableVector(uint32_t interrupt_number, uint32_t prio)
  (void) irq_desc;
  char x;
  x++; // stub
}

// stub function from lpc example, we use ChibiOS for IRQ and ISR
int fnet_cpu_isr_install(unsigned int vector_number, unsigned int priority)
{
  //nvicEnableVector(uint32_t interrupt_number, uint32_t prio)
  (void) vector_number, priority;
  return FNET_OK;
}
#endif
#endif /*FNET_STM32*/

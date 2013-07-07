/**************************************************************************
*
* Copyright 2013 by Jon Elliott, Andrey Butok. FNET Community.
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License Version 3 
* or later (the "LGPL").
*
* As a special exception, the copyright holders of the FNET project give you
* permission to link the FNET sources with independent modules to produce an
* executable, regardless of the license terms of these independent modules,
* and to copy and distribute the resulting executable under terms of your 
* choice, provided that you also meet, for each linked independent module,
* the terms and conditions of the license of that module.
* An independent module is a module which is not derived from or based 
* on this library. 
* If you modify the FNET sources, you may extend this exception 
* to your version of the FNET sources, but you are not obligated 
* to do so. If you do not wish to do so, delete this
* exception statement from your version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License
* and the GNU Lesser General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*
**********************************************************************/ /*!
*
* @file fnet_chibios.c
*
* @author inca
*
* @brief Default ChibiOS specific functions. @n
*        Experimental. Not supported.
*
***************************************************************************/ 

#include "fnet.h"
#include "ch.h"
#include "hal.h"

#if FNET_CFG_OS && FNET_CFG_OS_CHIBIOS

#if FNET_CFG_OS_EVENT

static BSEMAPHORE_DECL(FnetSemaphore, 0);

/************************************************************************
* NAME: fnet_os_event_init
*
* DESCRIPTION:
*************************************************************************/
int fnet_os_event_init(void)
{
   chBSemInit(&FnetSemaphore, FALSE);
   return FNET_OK;

}

/************************************************************************
* NAME: fnet_os_event_wait
*
* DESCRIPTION:
*************************************************************************/
void fnet_os_event_wait(void)
{
   chBSemWait(&FnetSemaphore);
}

/************************************************************************
* NAME: fnet_os_event_raise
*
* DESCRIPTION:
*************************************************************************/
void fnet_os_event_raise(void)
{
   chBSemSignal(&FnetSemaphore);
}

#endif /* FNET_CFG_OS_EVENT */

#if FNET_CFG_OS_ISR

/************************************************************************
* NAME: fnet_os_isr;
*
* DESCRIPTION: This handler is executed on every FNET interrupt 
*              (from ethernet and timer module).
*              Extructs vector number and calls fnet_isr_handler().
*************************************************************************/
void fnet_os_isr(void)
{
  /*******************************
   * OS-specific Interrupt Enter.
   *******************************/
  CH_IRQ_PROLOGUE();
  //chSysLockFromIsr();

//brtos stuff...
//  OS_SAVE_ISR();
//  OS_INT_ENTER();

  /* brtos: Call original CPU handler*/
  //fnet_cpu_isr();
  
  // TODO: Open ?
  // CH_IRQ_HANDLER(irq_handler) {
  // serve_interrupt(); ?
  
  /* Index the interrupt vector from the ICSR via CMSIS NVIC. 
  ICSR: Interrupt Control and State Register 
  http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0553a/Cihfaaha.html */
  fnet_uint16 vector_number = (fnet_uint16) (ICSR_VECTACTIVE_MASK & SCB_ICSR);
  fnet_isr_handler(vector_number);


  /*******************************
   * Interrupt Exit.
   *******************************/
  //chSysUnlockFromIsr();
  CH_IRQ_EPILOGUE();

//brtos stuff...
//  OS_INT_EXIT();  
//  OS_RESTORE_ISR();
}

#endif

#if FNET_CFG_OS_MUTEX

static MUTEX_DECL(FnetMutex);
static int FnetMutexCount;

/************************************************************************
* NAME: fnet_os_mutex_init
*
* DESCRIPTION:
*************************************************************************/
int fnet_os_mutex_init(void)
{
   chMtxInit(&FnetMutex);
   FnetMutexCount=0;
   return FNET_OK;
}

/************************************************************************
* NAME: fnet_os_mutex_lock;
*
* DESCRIPTION:
*************************************************************************/
void fnet_os_mutex_lock(void)
{
     chSysLock();
     if (chThdSelf() != FnetMutex.m_owner) {
       // Not owned. Lock.
       chMtxLockS(&FnetMutex);
     }
     FnetMutexCount++;
     chSysUnlock();
}

/************************************************************************
* NAME: fnet_os_mutex_unlock;
*
* DESCRIPTION:
*************************************************************************/
void fnet_os_mutex_unlock(void)
{
   chSysLock();
   if (--FnetMutexCount == 0) {
     // Last owned. Unlock.
     chMtxUnlockS();
   }
   chSysUnlock();
}

/************************************************************************
* NAME: fnet_os_mutex_release;
*
* DESCRIPTION:
*************************************************************************/
void fnet_os_mutex_release(void)
{

}
#endif /* FNET_CFG_OS_MUTEX */

#if FNET_CFG_OS_TIMER

/*
 * GPT2 callback.
 */
static void gpt_fnet_cb(GPTDriver *gptp) {

  (void)gptp;
  chSysLockFromIsr();
  fnet_timer_ticks_inc();
  chSysUnlockFromIsr();
}

/*
 * GPT2 configuration.
 */
static const GPTConfig gpt_fnet_cfg = {
  100000,         /* 100kHz timer clock.*/
  gpt_fnet_cb     /* Timer callback.*/
};

/************************************************************************
* NAME: fnet_os_timer_init
*
* DESCRIPTION: Starts OS-Timer/Event. delay_ms - period of timer (ms).
*************************************************************************/
int fnet_os_timer_init( unsigned int period_ms )
{
   gptStart(&FNET_CHIBIOS_TIMER, &gpt_fnet_cfg);
   gptStartContinuous(&FNET_CHIBIOS_TIMER, 100); /* 1ms tick */
}

/************************************************************************
* NAME: fnet_os_timer_release
*
* DESCRIPTION: Releases OS-Timer/Event.
*              
*************************************************************************/
void fnet_os_timer_release( void )
{
   gptStopTimer(&FNET_CHIBIOS_TIMER);
}

#endif /* FNET_CFG_OS_TIMER */

#endif /* FNET_CFG_OS && FNET_CFG_OS_CHIBIOS */

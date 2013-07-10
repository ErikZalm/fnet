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
#include "evtimer.h"

#include "fnet_stm32_eth.h"
#include "fnet_eth_prv.h"

#if FNET_CFG_OS && FNET_CFG_OS_CHIBIOS

/************************************************************************
* NAME: fnet_thread
*
* DESCRIPTION: FNET thread. Wait for incoming packages.
*************************************************************************/
#define PERIODIC_TIMER_ID       1
#define FRAME_RECEIVED_ID       2

#ifndef FNET_THREAD_PRIORITY
#define FNET_THREAD_PRIORITY    LOWPRIO
#endif

static WORKING_AREA(wa_fnet_thread, 2048);
EvTimer fnetEventTimer;

static msg_t fnet_thread(void *arg) {
   (void) arg;
   EventListener el0, el1;

   chRegSetThreadName("FNET thread");

   evtInit(&fnetEventTimer, MS2ST(100) );
   evtStart(&fnetEventTimer);
   chEvtRegisterMask(&fnetEventTimer.et_es, &el0, PERIODIC_TIMER_ID);
   chEvtRegisterMask(macGetReceiveEventSource(&ETHD1), &el1, FRAME_RECEIVED_ID);
   chEvtAddEvents(PERIODIC_TIMER_ID | FRAME_RECEIVED_ID);

   while (TRUE) {
      eventmask_t mask = chEvtWaitAny(ALL_EVENTS );
      if (mask & PERIODIC_TIMER_ID) {
         fnet_timer_ticks_inc();
         fnet_timer_handler_bottom(NULL );
      }
      if (mask & FRAME_RECEIVED_ID) {
         fnet_stm32_input();
      }
   }
   return RDY_OK;
}

void fnetThdStart(void) {
   chThdCreateStatic(wa_fnet_thread, sizeof(wa_fnet_thread), FNET_THREAD_PRIORITY, fnet_thread, NULL);
}

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

#ifdef FNET_CFG_OS_TIMER

/************************************************************************
* NAME: fnet_os_timer_init
*
* DESCRIPTION: Starts OS-Timer/Event. delay_ms - period of timer (ms).
*************************************************************************/
int fnet_os_timer_init( unsigned int period_ms )
{
//   evtStop(&fnetEventTimer);
//   evtInit(&fnetEventTimer, MS2ST(period_ms) );
//   evtStart(&fnetEventTimer);
   return FNET_OK;
}

/************************************************************************
* NAME: fnet_os_timer_release
*
* DESCRIPTION: Releases OS-Timer/Event.
*
*************************************************************************/
void fnet_os_timer_release( void )
{
   evtStop(&fnetEventTimer);
}

#endif /* FNET_CFG_OS_TIMER */

#endif /* FNET_CFG_OS && FNET_CFG_OS_CHIBIOS */

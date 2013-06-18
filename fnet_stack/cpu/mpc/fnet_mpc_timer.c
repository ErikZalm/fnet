/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2011 by Andrey Butok and Gordon Jahn. Freescale Semiconductor, Inc.
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
* @file fnet_mpc_timer.c
*
* @author Andrey Butok
*
* @date Dec-17-2012
*
* @version 0.1.1.0
*
* @brief MPC specific timers implementation.
*
***************************************************************************/
#include "fnet_config.h"

#if FNET_MPC
#include "fnet.h"
#include "fnet_timer_prv.h"
#include "fnet_isr.h"
#include "fnet_mpc.h"

#define FNET_TIMER_VECTOR_NUMBER FNET_CFG_CPU_TIMER_VECTOR_NUMBER   /* Number of timer interrupt vector.*/
#define FNET_TIMER_INT_LEVEL   (FNET_CFG_CPU_TIMER_VECTOR_PRIORITY) /* Timer interrupt level. */
#define FNET_TIMER_NUMBER      (FNET_CFG_CPU_TIMER_NUMBER)    /* Timer number (according to UM) */
#define FNET_TIMER_CLKIN_PER_MS (FNET_CPU_CLOCK_KHZ)


/************************************************************************
* NAME: fnet_cpu_timer_handler_top
*
* DESCRIPTION: Top interrupt handler. Increment fnet_current_time 
*              and interrupt flag. 
*************************************************************************/
static void fnet_cpu_timer_handler_top( void *cookie )
{
    FNET_COMP_UNUSED_ARG(cookie);
    
    /* Clear timer event condition.*/
   	FNET_MPC_PITRTI_TFLG(FNET_TIMER_NUMBER) = 0x1;
    
    /* Update RTC counter.*/
    fnet_timer_ticks_inc(); 
}

/************************************************************************
* NAME: fnet_cpu_timer_init
*
* DESCRIPTION: Starts TCP/IP hardware timer. delay_ms - period of timer (ms)
*         e.g. Time-out period = (1/FNET_CFG_SYSTEM_CLOCK_KHZ)x(1)x(124+1)x528x100 = 100 ms
*************************************************************************/
int fnet_cpu_timer_init( unsigned int period_ms )
{
    int result;
    
    /* Install interrupt handler.
     */
    result = fnet_isr_vector_init(FNET_TIMER_VECTOR_NUMBER, fnet_cpu_timer_handler_top,
                                              fnet_timer_handler_bottom, FNET_TIMER_INT_LEVEL, 0);
    
    if(result == FNET_OK)
    {
		FNET_MPC_PITRTI_MCR = 0x04;
		FNET_MPC_PITRTI_TCTRL(FNET_TIMER_NUMBER) = 0x0;
		FNET_MPC_PITRTI_LDVAL(FNET_TIMER_NUMBER) = period_ms * FNET_TIMER_CLKIN_PER_MS;
		FNET_MPC_PITRTI_TCTRL(FNET_TIMER_NUMBER) = 0x3;
    }

    return result;
}

/************************************************************************
* NAME: fnet_cpu_timer_release
*
* DESCRIPTION: Relaeses TCP/IP hardware timer.
*              
*************************************************************************/
void fnet_cpu_timer_release( void )
{
	FNET_MPC_PITRTI_TCTRL(FNET_TIMER_NUMBER) = 0x0;
   	FNET_MPC_PITRTI_TFLG(FNET_TIMER_NUMBER) = 0x1;
    
    /* Free interrupt handler res.
     */
    fnet_isr_vector_release(FNET_TIMER_VECTOR_NUMBER);
}

#endif /*FNET_MPC*/ 

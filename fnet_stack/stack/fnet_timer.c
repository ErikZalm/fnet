/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
* Copyright 2003 by Andrey Butok. Motorola SPS.
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
* @file fnet_timer.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.16.0
*
* @brief SW timer implementation.
*
***************************************************************************/

#include "fnet.h"
#include "fnet_timer_prv.h"
#include "fnet_netbuf.h"

/* Queue of the software timers*/

struct fnet_net_timer
{
    struct fnet_net_timer *next; /* Next timer in list.*/
    unsigned long timer_cnt;     /* Timer counter. */
    unsigned long timer_rv;      /* Timer reference value. */
    void (*handler)( void * );   /* Timer handler. */
    void *cookie;                /* Handler Cookie. */
};

static struct fnet_net_timer *fnet_tl_head = 0;

volatile static unsigned long fnet_current_time;

#if FNET_CFG_DEBUG_TIMER    
    #define FNET_DEBUG_TIMER   FNET_DEBUG
#else
    #define FNET_DEBUG_TIMER(...)
#endif

/************************************************************************
* NAME: fnet_timer_init
*
* DESCRIPTION: Starts TCP/IP hardware timer. delay_ms - period of timer (ms)
*************************************************************************/
int fnet_timer_init( unsigned int period_ms )
{
   int result;
   
   fnet_current_time = 0;           /* Reset RTC counter. */
   result = FNET_HW_TIMER_INIT(period_ms);  /* Start HW timer. */
   
   return result;
}

/************************************************************************
* NAME: fnet_timer_release
*
* DESCRIPTION: Frees the memory, which was allocated for all
*              TCP/IP timers, and removes hardware timer
*************************************************************************/
void fnet_timer_release( void )
{
    struct fnet_net_timer *tmp_tl;

    FNET_HW_TIMER_RELEASE();
    
    while(fnet_tl_head != 0)
    {
        tmp_tl = fnet_tl_head->next;

        fnet_free(fnet_tl_head);

        fnet_tl_head = tmp_tl;
    }
}

/************************************************************************
* NAME: fnet_timer_ticks
*
* DESCRIPTION: This function returns current value of the timer in ticks. 
*************************************************************************/
unsigned long fnet_timer_ticks( void )
{
    return fnet_current_time;
}

/************************************************************************
* NAME: fnet_timer_seconds
*
* DESCRIPTION: This function returns current value of the timer in seconds. 
*************************************************************************/
unsigned long fnet_timer_seconds( void )
{
    return (fnet_current_time/FNET_TIMER_TICK_IN_SEC);
}

/************************************************************************
* NAME: fnet_timer_ms
*
* DESCRIPTION: This function returns current value of the timer 
* in milliseconds. 
*************************************************************************/
unsigned long fnet_timer_ms( void )
{
    return (fnet_current_time*FNET_TIMER_PERIOD_MS);
}

/************************************************************************
* NAME: fnet_timer_ticks_inc
*
* DESCRIPTION: This function increments current value of the RTC counter. 
*************************************************************************/
void fnet_timer_ticks_inc( void )
{
    fnet_current_time++; 
    
#if FNET_CFG_DEBUG_TIMER
   if((fnet_current_time%10) == 0)    
	    FNET_DEBUG_TIMER("!");
#endif	    
}

/************************************************************************
* NAME: fnet_timer_handler_bottom
*
* DESCRIPTION: Handles timer interrupts 
*              
*************************************************************************/
void fnet_timer_handler_bottom(void *cookie)
{
    struct fnet_net_timer *timer = fnet_tl_head;
    
    FNET_COMP_UNUSED_ARG(cookie);
    
    while(timer)
    {
        if(fnet_timer_get_interval(timer->timer_cnt, fnet_current_time) >= timer->timer_rv)
        {
            timer->timer_cnt = fnet_current_time;

            if(timer->handler)
                timer->handler(timer->cookie);
        }

        timer = timer->next;
    }
}

/************************************************************************
* NAME: fnet_timer_new
*
* DESCRIPTION: Creates new software timer with the period 
*              
*************************************************************************/
fnet_timer_desc_t fnet_timer_new( unsigned long period_ticks, void (*handler)( void * ), void *cookie )
{
    struct fnet_net_timer *timer = FNET_NULL;

    if( period_ticks && handler )
    {
        timer = (struct fnet_net_timer *)fnet_malloc(sizeof(struct fnet_net_timer));

        if(timer)
        {
            timer->next = fnet_tl_head;

            fnet_tl_head = timer;

            timer->timer_cnt = 0;

            timer->timer_rv = period_ticks;

            timer->handler = handler;
            timer->cookie = cookie;
        }
    }

    return (fnet_timer_desc_t)timer;
}

/************************************************************************
* NAME: fnet_timer_free
*
* DESCRIPTION: Frees software timer, which is pointed by tl_ptr 
*              
*************************************************************************/
void fnet_timer_free( fnet_timer_desc_t timer )
{
    struct fnet_net_timer *tl = timer;
    struct fnet_net_timer *tl_temp;

    if(tl)
    {
        if(tl == fnet_tl_head)
            fnet_tl_head = fnet_tl_head->next;
        else
        {
            tl_temp = fnet_tl_head;

            while(tl_temp->next != tl)
              tl_temp = tl_temp->next;

            tl_temp->next = tl->next;
        }

        fnet_free(tl);
    }
}

/************************************************************************
* NAME: fnet_timer_reset_all
*
* DESCRIPTION: Resets all timers' counters
*              
*************************************************************************/
void fnet_timer_reset_all( void )
{
    struct fnet_net_timer *tl;

    tl = fnet_tl_head;

    while(tl != 0)
    {
        tl->timer_cnt = fnet_current_time;
        tl = tl->next;
    }
}

/************************************************************************
* NAME: fnet_timer_get_interval
*
* DESCRIPTION: Calaculates an interval between two moments of time
*              
*************************************************************************/
unsigned long fnet_timer_get_interval( unsigned long start, unsigned long end )
{
    if(start <= end)
        return (end - start);
    else
        return (0xffffffff - start + end + 1);
}

/************************************************************************
* NAME: fnet_timer_delay
*
* DESCRIPTION: Do delay for a given number of timer ticks.
*              
*************************************************************************/
void fnet_timer_delay( unsigned long delay_ticks )
{
    unsigned long start = fnet_current_time;

    while(fnet_timer_get_interval(start, fnet_timer_ticks()) < delay_ticks)
    { };
}

/************************************************************************
* NAME: fnet_timer_ms2ticks
*
* DESCRIPTION: Convert milliseconds to timer ticks.
*              
*************************************************************************/
unsigned long fnet_timer_ms2ticks( unsigned long time_ms )
{
    return time_ms / FNET_TIMER_PERIOD_MS;
}

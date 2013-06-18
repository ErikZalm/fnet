/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
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
* @file fnet_timer_prv.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.12.0
*
* @brief Private. FNET Timers API.
*
***************************************************************************/

#ifndef _FNET_TIMER_PRV_H_

#define _FNET_TIMER_PRV_H_

#include "fnet_timer.h"

/* SW Timer descriptor.*/
typedef void *fnet_timer_desc_t;

/************************************************************************
*     Function Prototypes
*************************************************************************/
int fnet_timer_init( unsigned int period_ms );
void fnet_cpu_timer_release( void );
void fnet_timer_release( void );
void fnet_timer_reset_all( void );
fnet_timer_desc_t fnet_timer_new( unsigned long period_ticks, void (*handler)( void *cookie ), void *cookie );
void fnet_timer_free( fnet_timer_desc_t timer );
void fnet_timer_ticks_inc( void );
void fnet_timer_handler_bottom(void *cookie);
int fnet_cpu_timer_init( unsigned int period_ms );

#endif

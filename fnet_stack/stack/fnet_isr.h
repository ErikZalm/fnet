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
* @file fnet_isr.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.24.0
*
* @brief Private. Interrupt dispatcher API.
*
***************************************************************************/

#ifndef _FNET_ISR_H_

#define _FNET_ISR_H_

#include "fnet_config.h"


/************************************************************************
*     Events
*************************************************************************/
typedef long fnet_event_desc_t;

/************************************************************************
*     Function Prototypes
*************************************************************************/
int fnet_isr_vector_init( unsigned int vector_number, void (*handler_top)(void *cookie), void (*handler_bottom)(void *cookie), unsigned int priority, void *cookie );
fnet_event_desc_t fnet_event_init(void (*event_handler)(void *cookie), void *cookie);
void fnet_event_raise(fnet_event_desc_t event_number);                                   
void fnet_isr_vector_release(unsigned int vector_number);
void fnet_isr_lock(void);
void fnet_isr_unlock(void);
void fnet_isr_init(void);
void fnet_isr_handler(int vector_number);
int fnet_cpu_isr_install(unsigned int vector_number, unsigned int priority);

#if FNET_CFG_OS_ISR
    #define FNET_ISR_HANDLER    fnet_os_isr
#else /* By default */
    #define FNET_ISR_HANDLER    fnet_cpu_isr
#endif /* FNET_CFG_OS_ISR */

/* Defines number of the first event handler. MUST be higher than any HW-vector number. */
#define FNET_EVENT_VECTOR_NUMBER (500) 

#endif

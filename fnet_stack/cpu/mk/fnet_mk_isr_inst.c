/**************************************************************************
* 
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
* @file fnet_mk_isr_inst.c
*
* @author Andrey Butok
*
* @date Aug-2-2012
*
* @version 0.1.8.0
*
* @brief Interrupt service dispatcher implementation.
*
***************************************************************************/

#include "fnet_config.h" 
#if FNET_MK 

#if !FNET_OS

#include "fnet.h"
#include "fnet_isr.h"
#include "fnet_timer.h"
#include "fnet_netbuf.h"

/************************************************************************
* NAME: fnet_cpu_isr_install
*
* DESCRIPTION: 
*************************************************************************/
int fnet_cpu_isr_install(unsigned int vector_number, unsigned int priority)
{
    int result;
    unsigned long *irq_vec;
    int div;
    int irq_number; /* The irq number NOT the vector number.*/

    irq_vec = (unsigned long *)(FNET_CFG_CPU_VECTOR_TABLE) + vector_number;

    if(*irq_vec != (unsigned long) FNET_ISR_HANDLER)
    { /* It's not installed yet.*/
        *irq_vec = (unsigned long) FNET_ISR_HANDLER;
    }
        
    if(*irq_vec == (unsigned long) FNET_ISR_HANDLER)
    {        
        /* Make sure that the IRQ is an allowable number. */
        irq_number = vector_number - 16;
        div = irq_number/32;
        if((div >= 0) && (div < 3))
        {
            /* Initialize the NVIC to enable the specified IRQ.*/
            FNET_MK_NVIC_ICPR(div) |= 1 << (irq_number%32); /* Clear-pending. */
            FNET_MK_NVIC_ISER(div) |= 1 << (irq_number%32); /* Set-enable.*/
            /* Set priority.*/
            FNET_MK_NVIC_IP(irq_number) = (0x7-priority&0x7)<<4;
        }
        result = FNET_OK;
    }
    else
        result = FNET_ERR;
   
   return result;            
}

#endif /* !FNET_OS */

#endif /*FNET_MK*/

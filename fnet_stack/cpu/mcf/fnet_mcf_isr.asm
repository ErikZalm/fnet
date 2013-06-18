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
**********************************************************************//*!
*
* @file fnet_mcf_isr.asm
*
* @author Andrey Butok
*
* @date Aug-2-2012
*
* @version 0.1.9.0
*
* @brief Lowest level routines for ColdFire.
*
***************************************************************************/

#include "fnet_comp_asm.h"

#if FNET_MCF


FNET_COMP_ASM_EXTERN    FNET_COMP_ASM_PREFIX(fnet_isr_handler)

FNET_COMP_ASM_GLOBAL    FNET_COMP_ASM_PREFIX(fnet_cpu_isr)



FNET_COMP_ASM_CODE

/************************************************************************
* NAME: void fnet_cpu_isr(void);
*
* DESCRIPTION: This handler is executed on every FNET interrupt 
*              (from ethernet and timer module).
*              Extructs vector number and calls fnet_isr_handler().
*************************************************************************/
FNET_COMP_ASM_ALIGN 4

FNET_COMP_ASM_PREFIX(fnet_cpu_isr):
	lea      (-60,a7),a7        
	movem.l  d0-d7/a0-a6,(a7)
	
    move.l   (60,a7),d0 /* Extract the irq number from */
    lsr.l    #8,d0      /* The exception stack frame */
    lsr.l    #8,d0     
    lsr.l    #2,d0
    and.l    #0xff,d0
   
	jsr     (FNET_COMP_ASM_PREFIX(fnet_isr_handler))

	movem.l  (a7),d0-d7/a0-a6
    lea      (60,a7),a7	
	rte


#endif /*FNET_MCF*/


    FNET_COMP_ASM_END
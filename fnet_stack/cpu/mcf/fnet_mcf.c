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
* @file fnet_mcf.c
*
* @author Andrey Butok
*
* @date Aug-2-2012
*
* @version 0.1.17.0
*
* @brief CPU-specific API implementation.
*
***************************************************************************/

#include "fnet_cpu.h"

#if FNET_MCF

/************************************************************************
*     Function Prototypes
*************************************************************************/
static fnet_cpu_irq_desc_t fnet_mcf_set_irqlevel(unsigned long level);

/************************************************************************
* NAME: fnet_cpu_reset
*
* DESCRIPTION: Initiate software reset.
*************************************************************************/
void fnet_cpu_reset (void)
{
#if FNET_CFG_MCF_RCM	 
	 FNET_MCF_RCM_RCR = FNET_MCF_RCM_RCR_SOFTRST | FNET_MCF_RCM_RCR_FRCRSTOUT;
#else	 
    unsigned long address = *((unsigned long *)(0x4));
  
    (( void(*)() )address)(); /* Jump. */
#endif
 
}

/************************************************************************
* NAME: fnet_cpu_reset
*
* DESCRIPTION: Initiate software reset.
*************************************************************************/
static fnet_cpu_irq_desc_t fnet_mcf_set_irqlevel(unsigned long level)
{
    fnet_uint16 csr;
    fnet_cpu_irq_desc_t oldlevel;
    
    csr = fnet_mcf_sr_rd();

    oldlevel = (fnet_cpu_irq_desc_t)((csr & FNET_MCF_SR_IPL) >> 8);

    csr = (fnet_uint16)((csr & (~FNET_MCF_SR_IPL)) | FNET_MCF_SR_S | ((level << 8)&FNET_MCF_SR_IPL));

    fnet_mcf_sr_wr(csr);

    return oldlevel;
}


/************************************************************************
* NAME: fnet_cpu_disable_irq
*
* DESCRIPTION: Disable IRQs
*************************************************************************/
fnet_cpu_irq_desc_t fnet_cpu_irq_disable(void)
{
    return fnet_mcf_set_irqlevel(7);
}

/************************************************************************
* NAME: fnet_cpu_disable_irq
*
* DESCRIPTION: Enables IRQs at interrupt level mask value.
*************************************************************************/
void fnet_cpu_irq_enable(fnet_cpu_irq_desc_t irq_desc)
{
    fnet_mcf_set_irqlevel(irq_desc);
}

#endif /*FNET_MCF*/

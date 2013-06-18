
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
* @file fnet_mpc_isr.c
*
* @author Andrey Butok
*
* @date Dec-17-2012
*
* @version 0.1.1.0
*
* @brief Lowest level ISR routine for MPC.
*
***************************************************************************/

#include "fnet_isr.h"
#include "fnet_mpc.h"

#if FNET_MPC

/************************************************************************
* NAME: void fnet_cpu_isr(void);
*
* DESCRIPTION: This handler is executed on every FNET interrupt 
*              (from ethernet and timer module).
*              Extructs vector number and calls fnet_isr_handler().
*************************************************************************/
void fnet_cpu_isr(void)
{
#if FNET_CFG_CPU_INDEX==0
    /* ICSR register [VECTACTIVE].*/ 
    fnet_uint16 vector_number = (fnet_uint16) ((FNET_MPC_INTC_IACKR_PRC0 & 0x7FC) >> 2);
#else
    /* ICSR register [VECTACTIVE].*/ 
    fnet_uint16 vector_number = (fnet_uint16) ((FNET_MPC_INTC_IACKR_PRC1 & 0x7FC) >> 2);
#endif

    /* Call FNET isr handler.*/
    fnet_isr_handler( vector_number );

#if FNET_CFG_CPU_INDEX==0
	FNET_MPC_INTC_EOIR_PRC0 = 0;
#else
	FNET_MPC_INTC_EOIR_PRC1 = 0;
#endif
}

#endif /*FNET_MPC*/ 

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
* @file fnet_mcf_low.asm
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

	FNET_COMP_ASM_GLOBAL    FNET_COMP_ASM_PREFIX(fnet_mcf_vbr_wr)

#if FNET_CFG_CPU_CACHE
	FNET_COMP_ASM_GLOBAL    FNET_COMP_ASM_PREFIX(fnet_mcf_cacr_wr)
#endif

	FNET_COMP_ASM_GLOBAL    FNET_COMP_ASM_PREFIX(fnet_mcf_sp_rd)

	FNET_COMP_ASM_GLOBAL    FNET_COMP_ASM_PREFIX(fnet_mcf_sr_rd)


	FNET_COMP_ASM_GLOBAL    FNET_COMP_ASM_PREFIX(fnet_mcf_sr_wr)


	FNET_COMP_ASM_GLOBAL    FNET_COMP_ASM_PREFIX(fnet_mcf_illegal)

	FNET_COMP_ASM_GLOBAL    FNET_COMP_ASM_PREFIX(fnet_mcf_nop)


	FNET_COMP_ASM_CODE


/* Set VBR */
/* void fnet_mcf_vbr_wr(fnet_uint32) */
FNET_COMP_ASM_PREFIX(fnet_mcf_vbr_wr):
	movec d0,VBR 
	nop
	rts	

#if FNET_CFG_CPU_CACHE
/* Set CACR */
/* void fnet_mcf_cacr_wr(fnet_uint32) */
FNET_COMP_ASM_PREFIX(fnet_mcf_cacr_wr):
	movec d0, cacr 
	nop
	rts
#endif	

/* Read SP */
/* fnet_uint32 fnet_mcf_sp_rd() */
FNET_COMP_ASM_PREFIX(fnet_mcf_sp_rd):
	move.l     sp,d0
    rts  


/* Read SR */
/* fnet_uint16 fnet_mcf_sr_rd() */
FNET_COMP_ASM_PREFIX(fnet_mcf_sr_rd):
	move.w     sr,d0
	rts    

/* Write SR */
/* void fnet_mcf_sr_wr(fnet_uint16) */
FNET_COMP_ASM_PREFIX(fnet_mcf_sr_wr):
	move.w     d0,sr
	rts


/* Generates an illegal instruction exception */
FNET_COMP_ASM_PREFIX(fnet_mcf_illegal):
	FNET_COMP_ASM_DC16 0x4afc ;illegal
	rts


/* NOP */
FNET_COMP_ASM_PREFIX(fnet_mcf_nop):
	nop
	rts

#endif /*FNET_MCF*/

	FNET_COMP_ASM_END


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
* @file fnet_mcf_checksum.asm
*
* @author Andrey Butok
*
* @date Aug-2-2012
*
* @version 0.1.16.0
*
* @brief ColdFire-specific Internet checksum calculation.
*
***************************************************************************/

#include "fnet_comp_asm.h"

#if FNET_MCF

#if FNET_CFG_OVERLOAD_CHECKSUM_LOW
	FNET_COMP_ASM_GLOBAL FNET_COMP_ASM_PREFIX(fnet_checksum_low) 

	FNET_COMP_ASM_CODE
	FNET_COMP_ASM_ALIGN 4
	
/************************************************************************
* NAME: fnet_checksum_low
*
* DESCRIPTION: Calculates checksum.
*
*************************************************************************
* unsigned long fnet_checksum_low(unsigned long sum, int current_length, unsigned short *d_ptr);
*   Arguments:
*        D0: unsigned long sum
*        D1: int current_length
*        A0: unsigned short *d_ptr
*/
FNET_COMP_ASM_PREFIX(fnet_checksum_low):  
        move.l  a1,-(sp)
        move.l  d2,-(sp)
        move.l  d3,-(sp)

        cmp.l   #3,d1           /* Do we have 3 or less bytes?*/
        ble.w   try_words

        move.l  a0,d2           /* Try to get onto even word.*/
        btst.l  #1,d2
        beq.b   try_longs       /* YES, address is long aligned already.*/
        clr.l   d3              /* NO, do align that. */
        move.w  (a0)+,d3
        add.l   d3,d0
        subq.l  #2,d1

try_longs:
        move.l  d1,d2
        lsr.l   #6,d2           /* 64 bytes summed per loop. */
        movea.l d2,a1

        move.l  d1,d3           /* 1 long processed by 4 bytes.*/
        and.l   #0x3C,d3
        neg.l   d3
        move  #0,ccr            /* Clear X.*/


#if FNET_CFG_COMP_IAR           /* IAR does not understand this command.*/
        DC32   0x4EFB3842
#else
        jmp     (pc,d3.l,66)
#endif
do_longs:
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0

        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0
        move.l  (a0)+,d3
        addx.l  d3,d0

do_longs_end:
        suba.l  #1,a1
        tst.l   a1
        bge.b     do_longs

do_word:
        btst    #1,d1           /* Sum last word.*/
        beq.b     do_byte
        clr.l   d3
        move.w  (a0)+,d3
        addx.l  d3,d0

do_byte:
        btst    #0,d1           /* Sum last byte.*/
        beq.b   done
        clr.l   d3
        addx.l  d3,d0           /* Add the carry if there is one.*/
        move.b  (a0)+,d3
        asl.l   #8,d3
        add.l   d3,d0

done:   move.l  d0,d1           /* Fold 32 bit sum into 16.*/
        swap    d1
        and.l   #0xFFFF,d0
        and.l   #0xFFFF,d1
        addx.l  d1,d0
        move.l  d0,d1
        swap    d1
        add.l   d1,d0
        and.l   #0xFFFF,d0

        move.l  (sp)+,d3
        move.l  (sp)+,d2
        movea.l  (sp)+,a1
        rts

try_words:
        move  #0,ccr            /* Clear X.*/
        bra.b   do_word
        
#endif  /* FNET_CFG_OVERLOAD_CHECKSUM_LOW */

#endif /*FNET_MCF*/

	FNET_COMP_ASM_END

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
* @file fnet_comp_asm.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.12.0
*
* @brief Compiler-specific definitions that resolve
* differences between different assemblers.
*
***************************************************************************/


#ifndef _FNET_COMP_ASM_H_

#define _FNET_COMP_ASM_H_

#define __FNET_ASM_CODE /* To eliminate non-assembler code.*/

#include "fnet_config.h"

/* --------------- CodeWarrior ---------------*/
#if FNET_CFG_COMP_CW

/* Code section. */
#if FNET_MCF    
	/* CW compiler adds leading underscores to assembly symbols.*/
    #define FNET_COMP_ASM_PREFIX(x) _##x
#endif      

#if FNET_MK      
    #define FNET_COMP_ASM_PREFIX(x) x
#endif  


/* Macro for the equate directive. */    
FNET_COMP_ASM_EQU:  .macro  label,  value
label   .equ    value
        .endm    

/* Extern. */
FNET_COMP_ASM_EXTERN:  .macro  value
	.extern  value
        .endm
        
/* Global.*/
FNET_COMP_ASM_GLOBAL: .macro label
	.global label
	.endm
            
/* Align. */
FNET_COMP_ASM_ALIGN:  .macro  value
	.align  value
        .endm

/* DC.W */
FNET_COMP_ASM_DC16:   .macro value
	.word value
	.endm
/* DC.L */
FNET_COMP_ASM_DC32:    .macro value
	.long value
	.endm

#define FNET_COMP_ASM_LABEL(x)    FNET_COMP_ASM_PREFIX(x):

/* Code section. */
#define FNET_COMP_ASM_CODE  .text            

/* No END in CW.*/
#define FNET_COMP_ASM_END
   
#endif /* FNET_CFG_COMP_CW */

/* --------------- IAR ----------------------*/
#if FNET_CFG_COMP_IAR

/* IAR compiler does not add any leading underscores to assembly symbols*/
#define FNET_COMP_ASM_PREFIX(x) x

/* Macro for the equate directive */
FNET_COMP_ASM_EQU MACRO label, value
label   EQU  value
	ENDM

/* Extern. */
FNET_COMP_ASM_EXTERN  MACRO value
	EXTERN  value
	ENDM

/* Global.*/
FNET_COMP_ASM_GLOBAL MACRO label
	PUBLIC label
	ENDM

/* Align. */
FNET_COMP_ASM_ALIGN  MACRO value
	ALIGN  value
	ENDM
/*DC.W*/
FNET_COMP_ASM_DC16  MACRO value
	DC16 value
	ENDM

/* DC.L */
FNET_COMP_ASM_DC32 MACRO value
	DC32 value
	ENDM


/* Code section. */
#if FNET_MCF      
    #define FNET_COMP_ASM_CODE  RSEG CODE 
#endif      

#if FNET_MK      
    #define FNET_COMP_ASM_CODE  RSEG .text:CODE
#endif          

#define FNET_COMP_ASM_LABEL(x)    FNET_COMP_ASM_PREFIX(x):

/*END - END directive is placed after the last
statement of a program to tell the assembler that this is the end
of the program module. The assembler will ignore any
statement after an END directive. Carriage return is required
after the END directive.*/
#define FNET_COMP_ASM_END   END 

/* Registers.*/
#if FNET_MCF       
FLASHBAR  DEFINE  0xC04
RAMBAR    DEFINE  0xC05
#endif
                                     
    
#endif /* FNET_CFG_COMP_IAR */



/* --------------- Keil uVision ----------------------*/
#if FNET_CFG_COMP_UV

/* uVision compiler does not add any leading underscores to assembly symbols*/
#define FNET_COMP_ASM_PREFIX(x) x

/* Macro for the equate directive */
	MACRO
	FNET_COMP_ASM_EQU $label, $value
$label   EQU  $value
	MEND

/* Extern. */
	MACRO
	FNET_COMP_ASM_EXTERN  $value
	EXTERN  $value   ; IMPORT
	MEND

/* Global.*/
	MACRO
	FNET_COMP_ASM_GLOBAL $label
	EXPORT $label
	MEND

/* Align. */
	MACRO
	FNET_COMP_ASM_ALIGN $value
	ALIGN  $value
	MEND
/*DC.W*/
	MACRO
	FNET_COMP_ASM_DC16  $value
	DCWU $value
	MEND

/* DC.L */
	MACRO
	FNET_COMP_ASM_DC32 $value
	DCDU $value
	MEND

#define FNET_COMP_ASM_LABEL(x)    FNET_COMP_ASM_PREFIX(x)

/* Code section. */
#define FNET_COMP_ASM_CODE	AREA    |.text|, CODE, READONLY
       
      

/*END - END directive is placed after the last
statement of a program to tell the assembler that this is the end
of the program module. The assembler will ignore any
statement after an END directive. Carriage return is required
after the END directive.*/
#define FNET_COMP_ASM_END   END 


                                     
    
#endif /* FNET_CFG_COMP_UV */



/* --------------- GCC ----------------------*/
#if FNET_CFG_COMP_GNUC

/* Code section. */
/* GCC compiler adds leading underscores to assembly symbols.*/
#define FNET_COMP_ASM_PREFIX(x) _##x

/* Macro for the equate directive. */
.macro FNET_COMP_ASM_EQU label, value
   \label   .equ    \value
   .endm

/* Extern. */
.macro FNET_COMP_ASM_EXTERN value
   .extern \value
   .endm

/* Global.*/
.macro FNET_COMP_ASM_GLOBAL label
   .global \label
   .endm

/* Align. */
.macro FNET_COMP_ASM_ALIGN value
   .align \value
   .endm

/* DC.W */
.macro FNET_COMP_ASM_DC16 value
   .word \value
   .endm

/* DC.L */
.macro FNET_COMP_ASM_DC32 value
   .long \value
   .endm

#define FNET_COMP_ASM_LABEL(x)    FNET_COMP_ASM_PREFIX(x):


/* Code section. */
#define FNET_COMP_ASM_CODE  .text

/* No END in CW.*/
#define FNET_COMP_ASM_END

#endif /* FNET_CFG_COMP_GNUC */


#endif /* _FNET_COMP_ASM_H_ */


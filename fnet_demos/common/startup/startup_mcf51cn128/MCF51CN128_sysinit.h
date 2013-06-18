/**************************************************************************
* 
* Copyright 2009 by Andrey Butok. Freescale Semiconductor, Inc. 
*     
**********************************************************************/ /*!
*
* @file MCF51CN128_sysinit.h
*
* @author Andrey Butok
*
* @date Oct-6-2010
*
* @version 0.0.1.0
*
* @brief General configuration of the MCF51CN128 (MDN TOWER board).
*
***************************************************************************/

#ifndef _MCF51CN128_SYSINIT_H_
#define _MCF51CN128_SYSINIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * System Clock Info
 */
#define	SYSTEM_CLOCK		50331648UL  /* system frequency in Hz */

#define UART_BAUD           115200   	

/* Determine which clock source is used.
 * Depends on J11 & J12 jumper setting. 
 * Only one xtal must be selected */
#define XTAL_25MHZ          1   
#define XTAL_32KHZ          0


/********************************************************************/
/* __initialize_hardware Startup code routine
 * 
 * __initialize_hardware is called by the startup code right after reset, 
 * with interrupt disabled and SP pre-set to a valid memory area.
 * Here you should initialize memory and some peripherics;
 * at this point global variables are not initialized yet.
 * The startup code will initialize SP on return of this function.
 */
void __initialize_hardware(void);



#ifdef __cplusplus
}
#endif

#endif /* _MCF51CN128_SYSINIT_H_ */



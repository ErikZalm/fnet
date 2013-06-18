/*
* File:		support_common.h
 * Purpose:		Various project configurations.
 *
 * Notes:
 */

#ifndef _SUPPORT_COMMON_H_
#define _SUPPORT_COMMON_H_

/* Enable UART Support. */
#define ENABLE_UART_SUPPORT  1


#define MEMORY_INIT \
	/* Initialize RAMBAR: locate SRAM and validate it */ \
	move.l	%#__RAMBAR + 0x21,d0; \
	movec	d0,RAMBAR;

#define SUPPORT_ROM_TO_RAM 1

/*
 * Include the derivative header files
 */
#include "MCF52235.h"

/*
 * Include the board specific header files
 */
#include "M52235EVB_sysinit.h"

/********************************************************************/

#endif /* _SUPPORT_COMMON_H_ */


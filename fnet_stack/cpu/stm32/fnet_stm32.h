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
* @file fnet_mk.h
*
* @author Andrey Butok
*
* @date Aug-2-2012
*
* @version 0.1.11.0
*
* @brief Private. Kinetis Peripheral Registers definitions.
*
***************************************************************************/

#ifndef _FNET_STM32_H_

#define _FNET_STM32_H_


//#include "fnet_config.h"
//#include "fnet_comp.h"

#if FNET_STM32 

/*********************************************************************
*
* The basic data types.
*
*********************************************************************/
typedef unsigned char fnet_uint8;       /*  8 bits */

typedef unsigned short int fnet_uint16; /* 16 bits */
typedef unsigned long int fnet_uint32;  /* 32 bits */

typedef signed char fnet_int8;          /*  8 bits */
typedef signed short int fnet_int16;    /* 16 bits */
typedef signed long int fnet_int32;     /* 32 bits */

typedef volatile fnet_uint8 fnet_vuint8;     /*  8 bits */
typedef volatile fnet_uint16 fnet_vuint16;   /* 16 bits */
typedef volatile fnet_uint32 fnet_vuint32;   /* 32 bits */


void fnet_mk_irq_enable(fnet_uint32 irq_desc);
fnet_uint32 fnet_mk_irq_disable(void);


/* Ensure that the Thumb bit is set.*/
//#define FNET_CPU_INSTRUCTION_ADDR(addr)    ((addr)|0x1)

/************************************************************************
* Kinetis peripheral clock in KHZ.
*************************************************************************/


/*********************************************************************
* Fast Ethernet Controller (FEC)
*********************************************************************/


#endif /* FNET_STM32 */

#endif /*_FNET_STM32_H_*/

























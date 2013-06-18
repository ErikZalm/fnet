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
* @file fnet_mpc.h
*
* @author Andrey Butok
*
* @date Dec-17-2012
*
* @version 0.1.1.0
*
* @brief MPC Registers definitions.
*
***************************************************************************/

#ifndef _FNET_MPC_H_

#define _FNET_MPC_H_

#include "fnet_config.h"

#if FNET_MPC


/*****************************************************************************
 *  Flash driver check - not available on PPC
 ******************************************************************************/ 

#if FNET_CFG_FLASH
	#error PPC Flash drivers are not included in this release - drivers are available from Freescale.com 
#endif /* FNET_CFG_CPU_FLASH */

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



/*********************************************************************
*
* MPC Mode Entry Fields
*
*********************************************************************/

#define	FNET_MPC_MC_MCTL					(*(fnet_vuint32*)(void*)(0xC3FDC004))

/*********************************************************************
*
* Fast Ethernet Controller (FEC)
*
*********************************************************************/

/* Register read/write macros */
#define FNET_FEC0_BASE_ADDR                 ((fnet_vuint32*)(void*)(0xFFF4C000))


/*********************************************************************
*
* Interrupt Controller 0 (INTC0)
*
*********************************************************************/

/* Register read/write macros */
#define FNET_MPC_INTC_BASE_ADDR     ((fnet_vuint8*)(void*)(0xFFF48000UL))

#define FNET_MPC_INTC_MCR	     	(*(fnet_vuint32 *)(void *)(&FNET_MPC_INTC_BASE_ADDR[0x000]))
#define FNET_MPC_INTC_CPR_PRC0     	(*(fnet_vuint32 *)(void *)(&FNET_MPC_INTC_BASE_ADDR[0x008]))
#define FNET_MPC_INTC_CPR_PRC1     	(*(fnet_vuint32 *)(void *)(&FNET_MPC_INTC_BASE_ADDR[0x00C]))
#define FNET_MPC_INTC_IACKR_PRC0    (*(fnet_vuint32 *)(void *)(&FNET_MPC_INTC_BASE_ADDR[0x010]))
#define FNET_MPC_INTC_IACKR_PRC1    (*(fnet_vuint32 *)(void *)(&FNET_MPC_INTC_BASE_ADDR[0x014]))
#define FNET_MPC_INTC_EOIR_PRC0     (*(fnet_vuint32 *)(void *)(&FNET_MPC_INTC_BASE_ADDR[0x018]))
#define FNET_MPC_INTC_EOIR_PRC1     (*(fnet_vuint32 *)(void *)(&FNET_MPC_INTC_BASE_ADDR[0x01C]))

#define FNET_MPC_INTC_PSR(x)      (*(fnet_vuint8 *)(void *)(((fnet_vuint8*)(void*)FNET_MPC_INTC_BASE_ADDR) + 0x40 + x))

/********************************************************************/

/*********************************************************************
*
* PIT Timers (TIMER)
*
*********************************************************************/


#if FNET_CFG_CPU_MPC5668G
	#define FNET_MPC_PITRTI_BASE_ADDR             ((fnet_vuint8*)(void*)(0xFFFE0000UL))
	#define FNET_MPC_PITRTI_TIMERS_BASE_ADDR      ((fnet_vuint8*)(void*)(0xFFFE0100UL))
#endif

#if FNET_CFG_CPU_MPC564xBC
	#define FNET_MPC_PITRTI_BASE_ADDR             ((fnet_vuint8*)(void*)(0xC3FF0000UL))
	#define FNET_MPC_PITRTI_TIMERS_BASE_ADDR      ((fnet_vuint8*)(void*)(0xC3FF0100UL))
#endif


#define FNET_MPC_PITRTI_MCR             		(*(fnet_vuint32*)(void*)FNET_MPC_PITRTI_BASE_ADDR)

#define FNET_MPC_PITRTI_OFFSET_LDVAL          (fnet_vuint32) (0x00)
#define FNET_MPC_PITRTI_OFFSET_CVAL           (fnet_vuint32) (0x04)
#define FNET_MPC_PITRTI_OFFSET_TCTRL          (fnet_vuint32) (0x08)
#define FNET_MPC_PITRTI_OFFSET_TFLG           (fnet_vuint32) (0x0C)

#define FNET_MPC_PITRTI_LDVAL(x)   (*(fnet_vuint32 *)(FNET_MPC_PITRTI_TIMERS_BASE_ADDR + FNET_MPC_PITRTI_OFFSET_LDVAL + ((x)*0x10)))
#define FNET_MPC_PITRTI_CVAL(x)    (*(fnet_vuint32 *)(FNET_MPC_PITRTI_TIMERS_BASE_ADDR + FNET_MPC_PITRTI_OFFSET_CVAL + ((x)*0x10)))
#define FNET_MPC_PITRTI_TCTRL(x)   (*(fnet_vuint32 *)(FNET_MPC_PITRTI_TIMERS_BASE_ADDR + FNET_MPC_PITRTI_OFFSET_TCTRL + ((x)*0x10)))
#define FNET_MPC_PITRTI_TFLG(x)    (*(fnet_vuint32 *)(FNET_MPC_PITRTI_TIMERS_BASE_ADDR + FNET_MPC_PITRTI_OFFSET_TFLG + ((x)*0x10)))

/*********************************************************************
*
* Flash Module (BFM)
*
*********************************************************************/

#define FNET_MPC_BFM_LOWMID_ADDR_KEY	(0xA1A11111)
#define FNET_MPC_BFM_S_LOWMID_ADDR_KEY	(0xC3C33333)
#define FNET_MPC_BFM_HIGH_ADDR_KEY		(0xB2B22222)

#define FNET_MPC_BFM_CFLA0_BASE ((fnet_vuint8*)(void*)(0xC3F88000))
#define FNET_MPC_BFM_CFLA1_BASE ((fnet_vuint8*)(void*)(0xC3FB0000))
#define FNET_MPC_BFM_DFLA_BASE  ((fnet_vuint8*)(void*)(0xC3F8C000))

#define FNET_MPC_BFM_CFLA0_MCR		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA0_BASE[0x0000]))
#define FNET_MPC_BFM_CFLA0_LML		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA0_BASE[0x0004]))
#define FNET_MPC_BFM_CFLA0_SLL		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA0_BASE[0x000C]))
#define FNET_MPC_BFM_CFLA0_HBL		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA0_BASE[0x0008]))
#define FNET_MPC_BFM_CFLA0_LMS		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA0_BASE[0x0010]))
#define FNET_MPC_BFM_CFLA0_HBS		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA0_BASE[0x0014]))

#define FNET_MPC_BFM_CFLA1_MCR		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA1_BASE[0x0000]))
#define FNET_MPC_BFM_CFLA1_LML		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA1_BASE[0x0004]))
#define FNET_MPC_BFM_CFLA1_SLL		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA1_BASE[0x000C]))
#define FNET_MPC_BFM_CFLA1_HBL		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA1_BASE[0x0008]))
#define FNET_MPC_BFM_CFLA1_LMS		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA1_BASE[0x0010]))
#define FNET_MPC_BFM_CFLA1_HBS		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_CFLA1_BASE[0x0014]))

#define FNET_MPC_BFM_DFLA_MCR		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_DFLA_BASE[0x0000]))
#define FNET_MPC_BFM_DFLA_LML		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_DFLA_BASE[0x0004]))	
#define FNET_MPC_BFM_DFLA_SLL		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_DFLA_BASE[0x000C]))
#define FNET_MPC_BFM_DFLA_LMS		(*(fnet_vuint32 *)(void*)(&FNET_MPC_BFM_DFLA_BASE[0x0010]))


/*********************************************************************
*
* Peripheral Registers     
*
*********************************************************************/


/*********************************************************************
*
* Universal Asynchronous Receiver Transmitter (UART)
*
*********************************************************************/

#if FNET_CFG_CPU_MPC564xBC

	#define FNET_MPC_LIN_BASE (fnet_vuint8*)(void*)(0xFFE40000)

	#define FNET_MPC_LIN_OFFSET_CR1 (fnet_vuint32) (0x0)
	#define FNET_MPC_LIN_OFFSET_UARTCR (fnet_vuint32) (0x10)
	#define FNET_MPC_LIN_OFFSET_UARTSR (fnet_vuint32) (0x14)
	#define FNET_MPC_LIN_OFFSET_LINIBRR (fnet_vuint32) (0x28)
	#define FNET_MPC_LIN_OFFSET_LINFBRR (fnet_vuint32) (0x24)
	#define FNET_MPC_LIN_OFFSET_LINIER (fnet_vuint32) (0x04)
	#define FNET_MPC_LIN_OFFSET_BDRL (fnet_vuint32) (0x38)
	#define FNET_MPC_LIN_OFFSET_BDRM (fnet_vuint32) (0x3C)

	#define FNET_MPC_LIN_CR1(x) (*(fnet_vuint32 *)(FNET_MPC_LIN_BASE + FNET_MPC_LIN_OFFSET_CR1 + ((x)*0x4000)))
	#define FNET_MPC_LIN_UARTCR(x) (*(fnet_vuint32 *)(FNET_MPC_LIN_BASE + FNET_MPC_LIN_OFFSET_UARTCR + ((x)*0x4000)))
	#define FNET_MPC_LIN_UARTSR(x) (*(fnet_vuint32 *)(FNET_MPC_LIN_BASE + FNET_MPC_LIN_OFFSET_UARTSR + ((x)*0x4000)))
	#define FNET_MPC_LIN_LINIBRR(x) (*(fnet_vuint32 *)(FNET_MPC_LIN_BASE + FNET_MPC_LIN_OFFSET_LINIBRR + ((x)*0x4000)))
	#define FNET_MPC_LIN_LINFBRR(x) (*(fnet_vuint32 *)(FNET_MPC_LIN_BASE + FNET_MPC_LIN_OFFSET_LINFBRR + ((x)*0x4000)))
	#define FNET_MPC_LIN_LINIER(x) (*(fnet_vuint32 *)(FNET_MPC_LIN_BASE + FNET_MPC_LIN_OFFSET_LINIER + ((x)*0x4000)))
	#define FNET_MPC_LIN_BDRL(x) (*(fnet_vuint32 *)(FNET_MPC_LIN_BASE + FNET_MPC_LIN_OFFSET_BDRL + ((x)*0x4000)))
	#define FNET_MPC_LIN_BDRM(x) (*(fnet_vuint32 *)(FNET_MPC_LIN_BASE + FNET_MPC_LIN_OFFSET_BDRM + ((x)*0x4000)))

#endif

#if FNET_CFG_CPU_MPC5668G

	#define FNET_MPC_ESCI_BASE (fnet_vuint8*)(void*)(0xFFFA0000) 

	#define FNET_MPC_ESCI_OFFSET_BRR (fnet_vuint16)(0x0)
	#define FNET_MPC_ESCI_OFFSET_CR1 (fnet_vuint16)(0x2)
	#define FNET_MPC_ESCI_OFFSET_CR2 (fnet_vuint16)(0x4)
	#define FNET_MPC_ESCI_OFFSET_SDR (fnet_vuint16)(0x6)
	#define FNET_MPC_ESCI_OFFSET_IFSR1 (fnet_vuint16)(0x8)
	#define FNET_MPC_ESCI_OFFSET_IFSR2 (fnet_vuint16)(0xA)
	#define FNET_MPC_ESCI_OFFSET_LCR1 (fnet_vuint16)(0xC)
	#define FNET_MPC_ESCI_OFFSET_LCR2 (fnet_vuint16)(0xE)
	#define FNET_MPC_ESCI_OFFSET_LTR (fnet_vuint32)(0x10)
	#define FNET_MPC_ESCI_OFFSET_LRR (fnet_vuint16)(0x14)
	#define FNET_MPC_ESCI_OFFSET_LPR (fnet_vuint16)(0x18)
	#define FNET_MPC_ESCI_OFFSET_CR3 (fnet_vuint16)(0x1A)

	#define FNET_MPC_ESCI_BRR(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_BRR + ((x)*0x4000)))
	#define FNET_MPC_ESCI_CR1(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_CR1 + ((x)*0x4000)))
	#define FNET_MPC_ESCI_CR2(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_CR2 + ((x)*0x4000)))
	#define FNET_MPC_ESCI_SDR(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_SDR + ((x)*0x4000)))
	#define FNET_MPC_ESCI_IFSR1(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_IFSR1 + ((x)*0x4000)))
	#define FNET_MPC_ESCI_IFSR2(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_IFSR2 + ((x)*0x4000)))
	#define FNET_MPC_ESCI_LCR1(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_LCR1 + ((x)*0x4000)))
	#define FNET_MPC_ESCI_LCR2(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_LCR2 + ((x)*0x4000)))
	#define FNET_MPC_ESCI_LTR(x) (*(fnet_vuint32 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_LTR + ((x)*0x4000)))
	#define FNET_MPC_ESCI_LRR(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_LRR + ((x)*0x4000)))
	#define FNET_MPC_ESCI_LPR(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_LPR + ((x)*0x4000)))
	#define FNET_MPC_ESCI_CR3(x) (*(fnet_vuint16 *)(FNET_MPC_ESCI_BASE + FNET_MPC_ESCI_OFFSET_CR3 + ((x)*0x4000)))

#endif

/*********************************************************************
*
* System Integration Unit (SIU)
*
*********************************************************************/

#define FNET_MPC_SIU_BASE       ((fnet_vuint8*)(void*)(0xFFFE8000UL))

/* System Reset Control Register (SIU_SRCR) */
#define FNET_MPC_SIU_SRCR       (*(fnet_vuint32 *)(void *)(&FNET_MPC_SIU_BASE[0x010]))


/*********************************************************************
*
* GPIO
*
*********************************************************************/

/*MPC564xBC*/
#if FNET_CFG_CPU_MPC5668G
	#define FNET_SIUL_BASE_ADDR                   ((fnet_vuint32*)(void*)(0xFFFE8000UL))
#endif

#if FNET_CFG_CPU_MPC564xBC
	#define FNET_SIUL_BASE_ADDR                   ((fnet_vuint32*)(void*)(0xC3F90000UL))
#endif

#define FNET_MPC_GPIO_PCR(x)      (*(fnet_vuint16 *)(void *)(((fnet_vuint16*)(void*)FNET_SIUL_BASE_ADDR) + 0x20 + x))

#endif /* FNET_MPC */
#endif /* _FNET_MPC_H_ */

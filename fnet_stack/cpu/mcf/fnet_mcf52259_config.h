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
* @file fnet_mcf52259_config.h
*
* @brief MCF5225x specific configuration file.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/

#ifndef _FNET_MCF5225X_CONFIG_H_

#define _FNET_MCF5225X_CONFIG_H_

/* MCF */
#define FNET_MCF                        	(1)

/* Size of the internal static heap buffer. */
#ifndef FNET_CFG_HEAP_SIZE
	#define FNET_CFG_HEAP_SIZE     			(30 * 1024)
#endif	

/* System frequency in Hz */
#ifndef FNET_CFG_CPU_CLOCK_HZ
	#define FNET_CFG_CPU_CLOCK_HZ    		(80000000)
#endif

/* The platform does not have second Ethernet Module.*/
#define FNET_CFG_CPU_ETH1        			(0)

/* Defines the maximum number of incoming frames that may 
 *           be buffered by the Ethernet module.*/
#ifndef FNET_CFG_CPU_ETH_RX_BUFS_MAX
	#define FNET_CFG_CPU_ETH_RX_BUFS_MAX 	(4)
#endif

/* The platform has ColdFire Flash Module.*/
#define FNET_CFG_CPU_FLASH              	(1)

#define FNET_CFG_CPU_FLASH_PAGE_SIZE    	(4*1024)

/* No cache. */
#define FNET_CFG_CPU_CACHE              	(0)

/* Flash size.*/
#define FNET_CFG_CPU_FLASH_SIZE         	(1024 * 512)    /* 512 KB */

/* SRAM size.*/
#define FNET_CFG_CPU_SRAM_SIZE          	(1024 * 64)     /* 64 KB */  

#endif

/**************************************************************************
* 
* Copyright 2009 by Andrey Butok. Freescale Semiconductor, Inc. 
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
* @file fnet_stm32f4_config.h
*
* @date Mar-25-2013
*
* @version 0.1.5.0
*
* @brief STM32F4 specific configuration file.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/

#ifndef _FNET_STM32F4_CONFIG_H_
#define _FNET_STM32F4_CONFIG_H_

#define FNET_STM32                          (1)


/* Size of the internal static heap buffer. */
#ifndef FNET_CFG_HEAP_SIZE
#define FNET_CFG_HEAP_SIZE                  (30 * 1024)
#endif

/* System frequency in Hz. */
#ifndef FNET_CFG_CPU_CLOCK_HZ
#define FNET_CFG_CPU_CLOCK_HZ               (120000000)
#endif

/* The platform does not have second Ethernet Module.*/
#define FNET_CFG_CPU_ETH1        			(0)

/* Defines the maximum number of incoming frames that may 
 *           be buffered by the Ethernet module.*/
#ifndef FNET_CFG_CPU_ETH_RX_BUFS_MAX
#define FNET_CFG_CPU_ETH_RX_BUFS_MAX        (4)
#endif

/* The platform has Kinetis Flash Memory Module (FTFL).*/
#define FNET_CFG_CPU_FLASH                  (1)

/*/ Smallest logical block which can be erased independently.*/
#define FNET_CFG_CPU_FLASH_PAGE_SIZE        (4*1024)        /* 4KB sector.*/

/* On-chip Flash size.*/
#define FNET_CFG_CPU_FLASH_SIZE             (1024 * 1024)    /* 1 MB */

#define FNET_CFG_CPU_FLASH_PROGRAM_SIZE     (8) /*Bytes.*/

/* SRAM size.*/
#define FNET_CFG_CPU_SRAM_SIZE              (1024 * 128)    /* 128 KB */  

/* To improve the TX performance.*/
#ifndef FNET_CFG_CPU_ETH_HW_TX_IP_CHECKSUM
    #define FNET_CFG_CPU_ETH_HW_TX_IP_CHECKSUM          (1)
#endif

/* To improve the TX performance.*/
#ifndef FNET_CFG_CPU_ETH_HW_TX_PROTOCOL_CHECKSUM
    #define FNET_CFG_CPU_ETH_HW_TX_PROTOCOL_CHECKSUM    (1)
#endif

/* To improve the RX performance.*/
#ifndef FNET_CFG_CPU_ETH_HW_RX_IP_CHECKSUM
    #define FNET_CFG_CPU_ETH_HW_RX_IP_CHECKSUM          (1)
#endif

/* To improve the RX performance.*/
#ifndef FNET_CFG_CPU_ETH_HW_RX_PROTOCOL_CHECKSUM
    #define FNET_CFG_CPU_ETH_HW_RX_PROTOCOL_CHECKSUM    (1)
#endif

/* Discard of frames with MAC layer errors.*/
#ifndef FNET_CFG_CPU_ETH_HW_RX_MAC_ERR
    #define FNET_CFG_CPU_ETH_HW_RX_MAC_ERR              (1)
#endif

#endif /* _FNET_MK60F120_CONFIG_H_ */ 

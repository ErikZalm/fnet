/**************************************************************************
* 
* Copyright 2012 by Andrey Butok.
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
* @file fnet_stm32_config.h
*
* @author Andrey Butok
*
* @date Dec-17-2012
*
* @version 0.1.8.0
*
* @brief STM32 specific default configuration.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/

#ifndef _FNET_STM32_CONFIG_H_

#define _FNET_STM32_CONFIG_H_

#include "fnet_user_config.h"


#ifndef FNET_STM32
  #define FNET_STM32   (1)
#endif

#if FNET_STM32

/**************************************************************************
 *  Reduced Media Independent Interface (RMII) support.
 ******************************************************************************/
#ifndef FNET_CFG_CPU_ETH_RMII
//    #define FNET_CFG_CPU_ETH_RMII        			(1)
#endif 

/**************************************************************************
 *  Default serial port number.
 ******************************************************************************/
#ifndef FNET_CFG_CPU_SERIAL_PORT_DEFAULT
//    #define FNET_CFG_CPU_SERIAL_PORT_DEFAULT        (3) /* TWR board uses the default port number 3.*/
#endif

/**************************************************************************
 *  Maximum Timer number that is avaiable on the used platform.
 ******************************************************************************/
#define  FNET_CFG_CPU_TIMER_NUMBER_MAX              (3)


/******************************************************************************
 *  Vector number of the timer interrupt.
 *  NOTE: User application should not change this parameter. 
 ******************************************************************************/
#ifndef FNET_CFG_CPU_TIMER_VECTOR_NUMBER
//    #define FNET_CFG_CPU_TIMER_VECTOR_NUMBER        (84  + FNET_CFG_CPU_TIMER_NUMBER)
#endif

/******************************************************************************
 *  Vector number of the Ethernet Receive Frame vector number.
 *  NOTE: User application should not change this parameter. 
 ******************************************************************************/
#ifndef FNET_CFG_CPU_ETH0_VECTOR_NUMBER
//    #define FNET_CFG_CPU_ETH0_VECTOR_NUMBER        (93)
#endif

/*****************************************************************************
 *  Byte order is little endian. 
 ******************************************************************************/ 
#undef FNET_CFG_CPU_LITTLE_ENDIAN
#define FNET_CFG_CPU_LITTLE_ENDIAN                  (1)

/*****************************************************************************
 *  On-chip Flash memory start address. 
 ******************************************************************************/ 
#ifndef FNET_CFG_CPU_FLASH_ADDRESS 
//    #define FNET_CFG_CPU_FLASH_ADDRESS              (0x0)
#endif 

/*****************************************************************************
 *   On-chip SRAM memory start address. 
 ******************************************************************************/ 
#if FNET_CFG_CPU_SRAM_SIZE
#ifndef FNET_CFG_CPU_SRAM_ADDRESS 
//    #define FNET_CFG_CPU_SRAM_ADDRESS   ((unsigned long)(0x20000000 - (FNET_CFG_CPU_SRAM_SIZE/2))) /* SRAM_L = [0x2000_0000–(SRAM_size/2)]*/
#endif
#endif


#ifndef FNET_CFG_CPU_FLASH_PROGRAM_SIZE
//    #define FNET_CFG_CPU_FLASH_PROGRAM_SIZE         (4)
#endif 

#endif /* FNET_STM32 */

#endif /* _FNET_STM32_CONFIG_H_ */

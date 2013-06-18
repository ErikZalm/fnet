/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
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
* @file fnet_mpc_config.h
*
* @author Andrey Butok
*
* @date Dec-17-2012
*
* @version 0.1.1.0
*
* @brief MPC-specific default configuration file.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/

#ifndef _FNET_MPC_CONFIG_H_

#define _FNET_MPC_CONFIG_H_

#include "fnet_user_config.h"


#if FNET_MPC

/******************************************************************************
 *  Vector number of the Ethernet Receive Frame interrupt.
 *  NOTE: User application should not change this parameter. 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_ETH0_VECTOR_NUMBER

#if FNET_CFG_CPU_MPC564xBC
	#define FNET_CFG_CPU_ETH0_VECTOR_NUMBER      (245)
#endif
#if FNET_CFG_CPU_MPC5668G
	#define FNET_CFG_CPU_ETH0_VECTOR_NUMBER      (299)
#endif

#endif
/*! @endcond */

/******************************************************************************
 *  Vector number of the timer interrupt.
 *  NOTE: User application should not change this parameter. 
 ******************************************************************************/
/*! @cond */
#ifndef FNET_CFG_CPU_TIMER_VECTOR_NUMBER
	#if FNET_CFG_CPU_MPC5668G
		#define FNET_CFG_CPU_TIMER_VECTOR_NUMBER    (149+FNET_CFG_CPU_TIMER_NUMBER)

	#elif FNET_CFG_CPU_MPC564xBC	
		#if FNET_CFG_CPU_TIMER_NUMBER < 3
			#define FNET_CFG_CPU_TIMER_VECTOR_NUMBER    (59+FNET_CFG_CPU_TIMER_NUMBER)
		#else
			#define FNET_CFG_CPU_TIMER_VECTOR_NUMBER    (124+FNET_CFG_CPU_TIMER_NUMBER)
		#endif
	#endif
#endif
/*! @endcond */

/*****************************************************************************
 *  On-chip SRAM memory start address. 
 ******************************************************************************/ 
#ifndef FNET_CFG_CPU_SRAM_ADDRESS 
    #define FNET_CFG_CPU_SRAM_ADDRESS   (0x40000000)    
#endif


#endif /* FNET_MPC */

#endif /* FNET_MPC */

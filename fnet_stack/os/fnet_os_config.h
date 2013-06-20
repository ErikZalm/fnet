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
* @file fnet_os_config.h
*
* @author Andrey Butok
*
* @brief Default OS-specific configuration. @n
*        Experimental. Not supported.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/
#ifndef _FNET_OS_CONFIG_H_

#define _FNET_OS_CONFIG_H_

/*! @addtogroup fnet_os_config  */
/*! @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_OS
 * @brief     Operation System support:
 *               - @c 1 = is enabled. The OS type is defined by the @ref FNET_CFG_OS_operation_system_type.
 *               - @b @c 0 = is disabled. It is used bare-metal FNET stack (Default value).
 ******************************************************************************/
#ifndef FNET_CFG_OS
	#define FNET_CFG_OS    (1)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_OS_operation_system_type 
 * @brief    This is the set of the @c FNET_CFG_OS_[operation_system_type] definitions that 
 *           define a currently used operation system. @n
 *           Current version of the FNET supports the following OS definitions:
 *            - @c FNET_CFG_OS_UCOSIII  = Used OS is the uCOS-III.
 *            - @c FNET_CFG_OS_BRTOS    = Used OS is the BRTOS (http://code.google.com/p/brtos/).
 *            - @c FNET_CFG_OS_FREERTOS = Used OS is the FreeRTOS. 
 *            - @c FNET_CFG_OS_CHIBIOS  = Used OS is the ChibiOS (http://www.chibios.org).
 *            @n @n
 *            Selected OS definition should be only one and must be defined as 1. 
 *            All others may be defined but must have the 0 value.
 *
 ******************************************************************************/
#define FNET_CFG_OS_operation_system_type /* Ignore it. Just only for Doxygen documentation */

#if FNET_CFG_OS

	/*-----------*/
	#ifndef FNET_CFG_OS_UCOSIII
		#define FNET_CFG_OS_UCOSIII  (0)
	#endif    

	#ifndef FNET_CFG_OS_BRTOS
		#define FNET_CFG_OS_BRTOS    (0)
	#endif 
	
	#ifndef FNET_CFG_OS_FREERTOS
		#define FNET_CFG_OS_FREERTOS (0)
	#endif	

   #ifndef FNET_CFG_OS_CHIBIOS
      #define FNET_CFG_OS_CHIBIOS (1)
   #endif
	/*-----------*/
    #if FNET_CFG_OS_UCOSIII /* uCOS-III */
        #ifdef FNET_OS_STR
            #error "More than one OS selected FNET_OS_XXXX"
        #endif
	   
        #include "fnet_ucosIII_config.h"
        #define FNET_OS_STR    "uCOS-III"
    #endif
    
    #if FNET_CFG_OS_BRTOS /* BRTOS */
		#ifdef FNET_OS_STR
			#error "More than one OS selected FNET_OS_XXXX"
		#endif
	   
		#include "fnet_brtos_config.h"
		#define FNET_OS_STR    "BRTOS"
	#endif

    #if FNET_CFG_OS_FREERTOS /* FREERTOS */
        #ifdef FNET_OS_STR
            #error "More than one OS selected FNET_OS_XXXX"
        #endif
			 
        #include "fnet_freertos_config.h"
        #define FNET_OS_STR    "FreeRTOS"
    #endif	

    #if FNET_CFG_OS_CHIBIOS /* ChibiOS */
        #ifdef FNET_OS_STR
            #error "More than one OS selected FNET_OS_XXXX"
        #endif

        #include "fnet_chibios_config.h"
        #define FNET_OS_STR    "ChibiOS"
    #endif

#endif /* FNET_CFG_OS*/

/*-----------*/
#ifndef FNET_OS_STR
    #define FNET_OS_STR    "NONE"
    
    #undef  FNET_CFG_OS_UCOSIII
    #define FNET_CFG_OS_UCOSIII     (0)

    #undef  FNET_CFG_OS_BRTOS
    #define FNET_CFG_OS_BRTOS       (0)  

    #undef  FNET_CFG_OS_FREERTOS
    #define FNET_CFG_OS_FREERTOS    (0)

    #undef  FNET_CFG_OS_CHIBIOS
    #define FNET_CFG_OS_CHIBIOS     (0)

#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_OS_MUTEX
 * @brief    OS Mutex support:
 *               - @c 1 = is enabled. 
 *               - @b @c 0 = is disabled (Default value). 
 ******************************************************************************/
#ifndef FNET_CFG_OS_MUTEX
	#define FNET_CFG_OS_MUTEX   (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_OS_ISR
 * @brief    OS-specific ISR handler:
 *               - @c 1 = is enabled. The fnet_os_isr() handler is used instead 
 *                        of the fnet_cpu_isr() handler. 
 *               - @b @c 0 = is disabled (Default value). 
 ******************************************************************************/
#ifndef FNET_CFG_OS_ISR
	#define FNET_CFG_OS_ISR     (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_OS_EVENT
 * @brief    OS-specific event:
 *               - @c 1 = is enabled. It is raised when new data are arrived to
 *                        the Socket layer.
 *               - @b @c 0 = is disabled (Default value). 
 ******************************************************************************/
#ifndef FNET_CFG_OS_EVENT
	#define FNET_CFG_OS_EVENT   (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_OS_TIMER
 * @brief    OS-specific timer initialization/release:
 *               - @c 1 = is enabled.@n
 *                        The fnet_os_timer_init() initialization is called instead of fnet_cpu_timer_init(),
 *                        and fnet_os_timer_relaese() is called instead of fnet_cpu_timer_release() .
 *               - @b @c 0 = is disabled (Default value). 
 ******************************************************************************/
#ifndef FNET_CFG_OS_TIMER
	#define FNET_CFG_OS_TIMER   (0)
#endif

/*! @} */

#endif /* _FNET_OS_CONFIG_H_ */

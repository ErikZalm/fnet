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
* @file fnet_comp_config.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.13.0
*
* @brief Compiler-specific default configuration file.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/

#ifndef _FNET_COMP_CONFIG_H_

#define _FNET_COMP_CONFIG_H_

#include "fnet_config.h"


/*! @addtogroup fnet_platform_config  */
/*! @{ */
/**************************************************************************/ /*!
 * @def      FNET_CFG_COMP_compiler_type 
 * @brief    This is the set of the @c FNET_CFG_COMP_[compiler_type] definitions that 
 *           define a currently used compiler. @n
 *           Current version of the FNET supports the following compiler definitions:
 *            - @c FNET_CFG_COMP_CW  = Used compiler is the CodeWarrior.
 *            - @c FNET_CFG_COMP_IAR = Used compiler is the IAR.
 *            - @c FNET_CFG_COMP_UV  = Used compiler is the Keil uVision. 
 *            @n @n
 *            Selected compiler definition should be only one and must be defined as 1. 
 *            All others may be defined but must have the 0 value.
 *            If no compiler definition is defined, the default compiler is CodeWarrior.
 * 
 ******************************************************************************/
#define FNET_CFG_COMP_compiler_type /* Ignore it. Just only for Doxygen documentation */


/* IAR compiler. */
#ifndef FNET_CFG_COMP_IAR
    #define FNET_CFG_COMP_IAR   (0)
#endif

/* CW compiler. */
#ifndef FNET_CFG_COMP_CW
    #define FNET_CFG_COMP_CW   (0)
#endif

/* Keil uVision compiler. */
#ifndef FNET_CFG_COMP_UV
    #define FNET_CFG_COMP_UV   (0)
#endif

/* GNU GCC */
#ifndef FNET_CFG_COMP_GNUC
    #define FNET_CFG_COMP_GNUC   (0)
#endif

#if FNET_CFG_COMP_CW
    #define FNET_COMP_STR    "CW"
#elif FNET_CFG_COMP_IAR
    #define FNET_COMP_STR    "IAR"
#elif FNET_CFG_COMP_UV
    #define FNET_COMP_STR    "UV"
#elif FNET_CFG_COMP_GNUC
   #define FNET_COMP_STR     "GCC"
#else
    #if (defined(__MWERKS__) || defined(__CODEWARRIOR__))
        #undef FNET_CFG_COMP_CW
        #define FNET_CFG_COMP_CW    (1)
        #define FNET_COMP_STR       "CW"
    #elif (defined(IAR))
        #undef FNET_CFG_COMP_IAR 
        #define FNET_CFG_COMP_IAR   (1)
        #define FNET_COMP_STR       "IAR"
    #elif (defined(__GNUC__))
        #undef FNET_CFG_COMP_GNUC
        #define FNET_CFG_COMP_GNUC  (1)
		#define FNET_COMP_STR       "GCC"
    #elif (defined(__DCC__))
        #error "DIAB compiler is not supported."
    #elif (defined(__ghs__))
        #error "GREEN-HILLS compiler is not supported."
    #else
        #error "It is not possible to define the compiler. Please set a FNET_CFG_COMP_XXXX parameter."
    #endif
#endif



/*! @} */

#endif /* _FNET_COMP_CONFIG_H_ */

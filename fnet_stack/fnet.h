/**************************************************************************
*
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
* Copyright 2003 by Andrey Butok, Alexey Shervashidze. Motorola SPS
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
* @file fnet.h
*
* @author Andrey Butok
*
* @date Feb-5-2013
*
* @version 0.1.26.0
*
* @brief Main including header for the FNET project.
*
***************************************************************************/


#ifndef _FNET_H_

#define _FNET_H_

#include "fnet_config.h"
#include "fnet_comp.h"
#include "fnet_cpu.h"
#include "fnet_os.h"
#include "fnet_stack.h"
#include "fnet_services.h"


/*! @addtogroup fnet_define
* These definitions are used for reference purposes only.
* @n
*/
/*! @{*/

/**************************************************************************/ /*!
 * @def FNET_DESCRIPTION
 * @brief Description string of the FNET TCP/IP stack.
 * @showinitializer
 ******************************************************************************/
#define FNET_DESCRIPTION        "FNET TCP/IP Stack"

/**************************************************************************/ /*!
 * @def FNET_LICENSE
 * @brief License string of the FNET TCP/IP stack.
 * @showinitializer 
 ******************************************************************************/
#define FNET_LICENSE            "GNU LGPLv3"

/**************************************************************************/ /*!
 * @def FNET_COPYRIGHT
 * @brief Copyright string of the FNET TCP/IP stack.
 * @showinitializer 
 ******************************************************************************/
#define FNET_COPYRIGHT          "Copyright by FNET Community"

/**************************************************************************/ /*!
 * @def FNET_BUILD_DATE
 * @brief Build date and time of the project as a string.
 * @showinitializer 
 ******************************************************************************/
#define FNET_BUILD_DATE         __DATE__ " at " __TIME__

/**************************************************************************/ /*!
 * @def FNET_VERSION
 * @brief Current version number of the FNET TCP/IP stack.
 *        The resulting value format is xx.xx.xx = major.minor.revision, as a 
 *        string.
 * @showinitializer 
 ******************************************************************************/
#define FNET_VERSION            "2.5.0"

/*! @} */

#endif


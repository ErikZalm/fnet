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
* @file fnet_config.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.8.0
*
* @brief Main FNET default configuration file.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/

#ifndef _FNET_CONFIG_H_

#define _FNET_CONFIG_H_

/* !! Do not change the order !! */

#ifndef __DOXYGEN__
#include "fnet_user_config.h"           /* User configuration. Highest priority. */
#else
#include "fnet_doxygen_user_config.h"   /* Configuration used during generation of documentation.*/
#endif

#include "fnet_comp_config.h"           /* Default compiler specific configuration. */  

#include "fnet_cpu_config.h"            /* Default platform configuration. */

#include "fnet_os_config.h"             /* Default OS-specific configuration. */

#include "fnet_stack_config.h"          /* Default TCP/IP stack configuration. */

#include "fnet_services_config.h"       /* Default services configuration. */

#endif /* _FNET_CONFIG_H_ */

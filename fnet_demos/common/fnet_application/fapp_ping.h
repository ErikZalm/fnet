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
* @file fapp_ping.h
*
* @author Andrey Butok
*
* @date Dec-20-2012
*
* @version 0.1.5.0
*
* @brief FNET Shell Demo API (Ping).
*
***************************************************************************/

#ifndef _FAPP_PING_H_

#define _FAPP_PING_H_

#include "fapp_config.h"

#if FNET_CFG_PING && (FAPP_CFG_PING_CMD || FAPP_CFG_PING6_CMD)

/* Default parameters.*/
#define FAPP_PING_DEFAULT_SIZE      (63)
#define FAPP_PING_DEFAULT_NUMBER    (4)
#define FAPP_PING_DEFAULT_TIMEOUT   (1000) /* ms */
#define FAPP_PING_DEFAULT_PATTERN   (0x00) 
#define FAPP_PING_DEFAULT_HOP_LIMIT (64) 

void fapp_ping_cmd( fnet_shell_desc_t desc, int argc, char ** );

#endif /* FNET_CFG_PING && (FAPP_CFG_PING_CMD || FAPP_CFG_PING6_CMD) */

#endif /* _FAPP_PING_H_ */

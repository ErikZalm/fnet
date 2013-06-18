/**************************************************************************
*
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2009 by Andrey Butok. Freescale Semiconductor, Inc.
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
* @file fnet_telnet_config.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.13.0
*
* @brief FNET Telnet Server configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_TELNET_CONFIG_H_

#define _FNET_TELNET_CONFIG_H_


/** @addtogroup fnet_telnet_config */
/** @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_TELNET
 * @brief    Telnet server support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 ******************************************************************************/
#ifndef FNET_CFG_TELNET
    #define FNET_CFG_TELNET                     (0)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_TELNET_MAX
 * @brief   Maximum number of the Telnet Servers that can be run simultaneously.@n
 *          Default value is @b @c 1.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_TELNET_MAX
    #define FNET_CFG_TELNET_MAX                 (1)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_TELNET_SESSION_MAX
 * @brief   Maximum number of simultaneous user-session that can be handled 
 *          by the Telnet server.@n
 *          Default value is @b @c 1.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_TELNET_SESSION_MAX
    #define FNET_CFG_TELNET_SESSION_MAX         (1)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_TELNET_PORT
 * @brief   Default Telnet port number (in network byte order).@n
 *          It can be changed during the Telnet server initialization by the 
 *          @ref fnet_telnet_init() function. @n
 *          Default value is FNET_HTONS(23).
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_TELNET_PORT
    #define FNET_CFG_TELNET_PORT                (FNET_HTONS(23))
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_TELNET_SHELL_ECHO
 * @brief   Echo in the Tenet shell:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_TELNET_SHELL_ECHO
    #define FNET_CFG_TELNET_SHELL_ECHO          (0)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_TELNET_SOCKET_BUF_SIZE
 * @brief   Size of the socket RX & TX buffer used by the Telnet server. @n
 *          Default value is @b @c 60.
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_TELNET_SOCKET_BUF_SIZE
    #define FNET_CFG_TELNET_SOCKET_BUF_SIZE     (60)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_TELNET_CMD_LINE_BUF_SIZE
 * @brief   Size of the command-line buffer used by the Telnet Shell. @n
 8          It defines the maximum length of the entered input-command line.
 *          Default value is @b @c 60.
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_TELNET_CMD_LINE_BUF_SIZE
    #define FNET_CFG_TELNET_CMD_LINE_BUF_SIZE   (60)
#endif


/** @} */

#endif /* _FNET_TELNET_CONFIG_H_ */

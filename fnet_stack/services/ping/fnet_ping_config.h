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
* @file fnet_ping_config.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.3.0
*
* @brief PING service configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_PING_CONFIG_H_

#define _FNET_PING_CONFIG_H_

/*! @addtogroup fnet_ping_config */
/*! @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_PING
 * @brief    PING service support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 ******************************************************************************/
#ifndef FNET_CFG_PING
    #define FNET_CFG_PING           (0)
#endif

#if FNET_CFG_PING 
    /* Force RAW sockets, if PING is defined. */
    #undef FNET_CFG_RAW
    #define FNET_CFG_RAW            (1)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_PING_PACKET_MAX
 * @brief   The maximum size of the echo request packet. It defines send 
 *          and receive buffer size, used by the PING service.@n
 *          Default value is @b @c 64.
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_PING_PACKET_MAX
    #define FNET_CFG_PING_PACKET_MAX            (64)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_PING_IDENTIFIER
 * @brief   The ICMP Identifier (in network byte order).@n
 *          It is used to help match echo requests to the associated reply.@n
 *          Default value is @b @c 1.
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_PING_IDENTIFIER
    #define FNET_CFG_PING_IDENTIFIER            (FNET_HTONS(1))
#endif


/*! @} */

#endif /* _FNET_PING_CONFIG_H_ */

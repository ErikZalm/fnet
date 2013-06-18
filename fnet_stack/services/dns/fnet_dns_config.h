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
* @file fnet_dns_config.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.6.0
*
* @brief DNS Resolver configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_DNS_CONFIG_H_

#define _FNET_DNS_CONFIG_H_

/*! @addtogroup fnet_dns_config */
/*! @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_DNS_RESOLVER
 * @brief    DNS client/resolver support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 ******************************************************************************/
#ifndef FNET_CFG_DNS_RESOLVER
    #define FNET_CFG_DNS_RESOLVER                   (0)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_DNS_PORT
 * @brief   Default DNS port number (in network byte order).@n
 *          The DNS client uses this port for sending and receiving of DNS 
 *          messages. @n
 *          Default value is FNET_HTONS(53).
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_DNS_PORT
    #define FNET_CFG_DNS_PORT                       (FNET_HTONS(53))
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_DNS_RETRANSMISSION_MAX
 * @brief   The maximum number of times the DNS client/resolver will retransmit
 *          the request message to a DNS server.@n
 *          Default value is @b @c 5.
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_DNS_RETRANSMISSION_MAX
    #define FNET_CFG_DNS_RETRANSMISSION_MAX         (5)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_DNS_RETRANSMISSION_TIMEOUT
 * @brief   Timeout for the response from the remote DNS server (in seconds).@n
 *          @n 
 *          The recommended value is @c 1 second.@n
 *          If the DNS client does not receive any response from a DNS server, 
 *          it sends new request message.@n
 *          Default value is @b @c 1.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_DNS_RETRANSMISSION_TIMEOUT
    #define FNET_CFG_DNS_RETRANSMISSION_TIMEOUT     (1)  /* seconds */
#endif

/*! @} */

#endif /* _FNET_DNS_CONFIG_H_ */

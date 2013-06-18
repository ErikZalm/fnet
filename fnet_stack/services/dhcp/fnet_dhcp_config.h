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
* @file fnet_dhcp_config.h
*
* @author Andrey Butok
*
* @date Jan-16-2013
*
* @version 0.0.14.0
*
* @brief DHCP Client configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_DHCP_CONFIG_H_

#define _FNET_DHCP_CONFIG_H_

/*! @addtogroup fnet_dhcp_config */
/*! @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_DHCP
 * @brief    DHCP Client service support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 ******************************************************************************/
#ifndef FNET_CFG_DHCP
    #define FNET_CFG_DHCP       (0)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_DHCP_BOOTP
 * @brief   DHCP/BOOTP-service used protocol:
 *               - @c 1 = BOOTP (Bootstrap) protocol, defined by RFC951.
 *               - @b @c 0 = DHCP protocol (extension of BOOTP), defined by RFC2131 (Default value).@n
 *          @n
 *          BOOTP typically provides fixed allocation of a single IP address 
 *          for each client, permanently reserving this address in the BOOTP 
 *          server database.@n
 *          DHCP typically provides dynamic, leased allocation of available 
 *          IP addresses, reserving each DHCP client address temporarily 
 *          in the DHCP server database.@n
 *          DHCP can automatically rebind or renew their leases while BOOTP requires a system restart.@n
 *          Code size of BOOTP is less than DHCP by about 1.5KB.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_DHCP_BOOTP
    #define FNET_CFG_DHCP_BOOTP                 (0)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_DHCP_PORT_CLIENT
 * @brief   DHCP client port number (in network byte order).@n
 *          The client listen DHCP messages from a server on this port.@n
 *          Default value is @b @c 68.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_DHCP_PORT_CLIENT
    #define FNET_CFG_DHCP_PORT_CLIENT           (FNET_HTONS(68))
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_DHCP_PORT_SERVER
 * @brief   DHCP server port number (in network byte order).@n
 *          DHCP messages from a client to a server are sent to this port.@n
 *          Default value is @b @c 67.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_DHCP_PORT_SERVER
    #define FNET_CFG_DHCP_PORT_SERVER           (FNET_HTONS(67))
#endif
    
/**************************************************************************/ /*!
 * @def     FNET_CFG_DHCP_RESPONSE_TIMEOUT
 * @brief   Timeout for the response from the remote DHCP server (in seconds).@n
 *           @n 
 *          The recommended value is from @c 1 till @c 10 seconds.@n
 *          If the DHCP client does not receive any response from a DHCP server during this time, 
 *          it sends new discover message. @n
 *          Default value is @b @c 4.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_DHCP_RESPONSE_TIMEOUT
    #define FNET_CFG_DHCP_RESPONSE_TIMEOUT      (4) 
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_DHCP_OVERLOAD
 * @brief   DHCP "Option overload" support [RFC 2132 9.3]:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).@n
 *          @n
 *          It's recomended to disable this option, as it's not used in practice.@n @n
 *          [RFC 2132 9.3]: "This option is used to indicate that the DHCP 'sname' or 'file'
 *          fields are being overloaded by using them to carry DHCP options. A
 *          DHCP server inserts this option if the returned parameters will
 *          exceed the usual space allotted for options.@n
 *          If this option is present, the client interprets the specified
 *          additional fields after it concludes interpretation of the standard
 *          option fields." 
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_DHCP_OVERLOAD
    #define FNET_CFG_DHCP_OVERLOAD              (0)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_DHCP_BROADCAST
 * @brief   DHCP "Broadcast Address Option" support [RFC 2132 5.3]:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).@n
 *          @n
 *          [RFC 2132 9.3]: "This option specifies the broadcast address 
 *          in use on the client's subnet." 
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_DHCP_BROADCAST
    #define FNET_CFG_DHCP_BROADCAST             (0)
#endif

/*! @} */

#endif /* _FNET_DHCP_CONFIG_H_ */

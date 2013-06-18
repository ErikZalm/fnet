/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
* Copyright 2003 by Andrey Butok. Motorola SPS.
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
* @file fnet_socket.h
*
* @author Andrey Butok
*
* @date Jan-28-2013
*
* @version 0.1.51.0
*
* @brief Socket API.
*
***************************************************************************/

#ifndef _FNET_SOCKET_H_

#define _FNET_SOCKET_H_

#include "fnet_ip.h"
#include "fnet_ip6.h"

/*! @addtogroup fnet_socket 
* The Socket Application Program Interface (API) defines the way, in which the 
* application program interacts with the TCP/IP stack. It has the 
* BSD-like, non-blocking socket interface that makes porting of existing network applications 
* to the FNET more convenient.@n
* @n
* The following table summarizes the supported socket API functions:
* <table>
* <caption>Socket functions</caption>
* <tr class="fnet_td_grey">
* <th ALIGN=CENTER>Category</th><th ALIGN=CENTER>Routine</th>
* <th ALIGN=CENTER>Meaning</th><th ALIGN=CENTER>Server</th>
* <th ALIGN=CENTER>Client</th><th ALIGN=CENTER>@ref SOCK_STREAM</th>
* <th ALIGN=CENTER>@ref SOCK_DGRAM</th>
* </tr>
* <tr>
* <td>setup</td><td>@ref socket()</td><td>Creates a new unnamed socket within 
* a specified communication domain family.</td><td>X</td><td>X</td><td>X</td>
* <td>X</td>
* </tr>
* <tr>
* <td>setup</td><td>@ref bind()</td><td>Assigns a local address to 
* a socket.</td><td>X</td><td>X</td><td>X</td><td>X</td>
* </tr>
* <tr>
* <td>server</td><td>@ref listen()</td><td> Prepares a socket to accept 
* the incoming connections.</td><td>X</td><td>@n</td><td>X</td><td>@n</td>
* </tr>
* <tr>
* <td>server</td><td>@ref accept()</td><td>Accepts the connections.</td>
* <td>X</td><td>@n</td><td>X</td><td>@n</td>
* </tr>
* <tr>
* <td>client</td><td>@ref connect()</td><td> Establishes a connection to 
* a foreign socket.</td><td>@n</td><td>X</td><td>X</td><td>X</td>
* </tr>
* <tr>
* <td>input</td><td>@ref recv()</td><td>Receives the data.</td><td>X</td>
* <td>X</td><td>X</td><td>X</td>
* </tr>
* <tr>
* <td>input</td><td>@ref recvfrom()</td><td>Receives the data and address of 
* the sender.</td><td>X</td><td>X</td><td>@n</td><td>X</td>
* </tr>
* <tr>
* <td>output</td><td>@ref send()</td><td>Sends the data.</td><td>X</td>
* <td>X</td><td>X</td><td>X</td>
* </tr>
* <tr>
* <td>output</td><td>@ref sendto()</td><td>Sends the data to a specified 
* address.</td><td>X</td><td>X</td><td>@n</td><td>X</td>
* </tr>
* <tr>
* <td>termination</td><td>@ref shutdown()</td><td>Terminates a connection 
* in one or both directions.</td><td>X</td><td>X</td><td>X</td><td>X</td>
* </tr>
* <tr>
* <td>termination</td><td>@ref closesocket()</td><td>Terminates a connection 
* and releases the socket.</td><td>X</td><td>X</td><td>X</td><td>X</td>
* </tr>
* <tr>
* <td>administration</td><td>@ref setsockopt()</td><td>Sets socket or protocol 
* options.</td><td>X</td><td>X</td><td>X</td><td>X</td>
* </tr>
* <tr>
* <td>administration</td><td>@ref getsockopt()</td><td>Gets socket or protocol 
* options.</td><td>X</td><td>X</td><td>X</td><td>X</td>
* </tr>
* <tr>
* <td>administration</td><td>@ref getsockname()</td><td>Gets a local address 
* assigned to a socket.</td><td>X</td><td>X</td><td>X</td><td>X</td>
* </tr>
* <tr>
* <td>administration</td><td>@ref getpeername()</td><td>Gets a foreign address 
* assigned to a socket.</td><td>X</td><td>X</td><td>X</td><td>X</td>
* </tr>
* </table>
* Configuration parameters:
* - @ref FNET_CFG_SOCKET_MAX  
* - @ref FNET_CFG_SOCKET_TCP_TX_BUF_SIZE
* - @ref FNET_CFG_SOCKET_TCP_RX_BUF_SIZE  
* - @ref FNET_CFG_SOCKET_UDP_TX_BUF_SIZE 
* - @ref FNET_CFG_SOCKET_UDP_RX_BUF_SIZE
* - @ref FNET_CFG_SOCKET_TCP_MSS 
* - @ref FNET_CFG_RAW
*/
/*! @{ */


/* Special addresses */

/**************************************************************************/ /*!
 * @brief It means to use any network interface.
 ******************************************************************************/
#define INADDR_ANY       (unsigned long)(0x00000000)

/**************************************************************************/ /*!
 * @brief  Broadcast address. @n
 * It can be used to send the broadcast UDP datagrams over an IP network.
 ******************************************************************************/
#define INADDR_BROADCAST (unsigned long)(0xffffffff)

/**************************************************************************/ /*!
 * @brief Address family type.
 * @see AF_UNSPEC, AF_INET, AF_INET6, AF_SUPPORTED 
 ******************************************************************************/
typedef unsigned short fnet_address_family_t;

/**************************************************************************/ /*!
 * @brief Unspecified address family.
 ******************************************************************************/
#define AF_UNSPEC   (0) 
/**************************************************************************/ /*!
 * @brief IPv4 address family.
 ******************************************************************************/                        
#define AF_INET     (1) 
/**************************************************************************/ /*!
 * @brief IPv6 address family.
 ******************************************************************************/                         
#define AF_INET6    (2) 
/**************************************************************************/ /*!
 * @brief Bitmask of supported address families.
 * @showinitializer 
 ******************************************************************************/ 
#define AF_SUPPORTED ((AF_INET6*FNET_CFG_IP6) | (AF_INET*FNET_CFG_IP4))
    

/* Size of sa_data[]*/
#if FNET_CFG_IP6
    #define FNET_SA_DATA_SIZE   (20)
#else /* IPv4 */
    #define FNET_SA_DATA_SIZE   (4)
#endif

/**************************************************************************/ /*!
 * @def     FNET_IP_ADDR_STR_SIZE
 * @brief   Size of the string buffer that will contain 
 *          null-terminated ASCII string of an IP address. It depends on
 *          if disabled or enabled IPv6.
 * @see fnet_inet_ntoa, fnet_inet_ntop
 * @showinitializer 
 ******************************************************************************/
#if FNET_CFG_IP6 
    #define FNET_IP_ADDR_STR_SIZE       FNET_IP6_ADDR_STR_SIZE
#else /* IPv4 */
    #define FNET_IP_ADDR_STR_SIZE       FNET_IP4_ADDR_STR_SIZE
#endif

/**************************************************************************/ /*!
 * @brief Socket address structure.
 *
 * @see sockaddr_in, sockaddr_in6
 *
 * The original goal of the @ref sockaddr structure is to support multiple 
 * protocols. For the TCP/IP stack @c sa_data, it contains a destination address 
 * and port number for a socket.
 ******************************************************************************/
struct sockaddr
{
    fnet_address_family_t sa_family;/**< @brief Address family. Specifies the 
                             * address family, to which the address belongs. @n
                             * It is defined by @ref fnet_address_family_t. 
                             */
    unsigned short sa_port; /**< @brief 16-bit port number used to 
                             * demultiplex the transport-level messages
                             * (in network byte order). 
                             */                             
    char sa_data[FNET_SA_DATA_SIZE];/**< @brief Address value. For the TCP/IP stack, 
                             * it contains the destination address and port 
                             * number for a socket.
                             */
};

/**************************************************************************/ /*!
 * @brief IPv4 address structure.
 *
 * @see sockaddr_in
 *
 * Actually, it represents an IPv4 address (in network endian).
 ******************************************************************************/
struct in_addr
{
    fnet_ip4_addr_t s_addr; /**< @brief 32-bit IPv4 address (in network byte order). 
                            */
};

/**************************************************************************/ /*!
 * @brief IPv4 Socket address structure.
 *
 * @see sockaddr
 *
 * To make manipulation of the @ref sockaddr structure easier, the TCP/IPv4 stack 
 * also defines the equivalent structure @ref sockaddr_in 
 * ("in" means "Internet").
 ******************************************************************************/
struct sockaddr_in
{
    fnet_address_family_t sin_family;   /**< @brief Specifies the address family. @n 
                                 * It must ne set to @ref AF_INET.  
                                 */
    unsigned short sin_port;    /**< @brief 16-bit port number used to 
                                 * demultiplex the transport-level messages
                                 * (in network byte order). 
                                 */
    struct in_addr sin_addr;    /**< @brief 32-bit internet address. 
                                 */
};


/************************************************************************
*                           IPv6
*************************************************************************/


/**************************************************************************/ /*!
 * @brief IPv6 address structure.
 *
 * @see sockaddr_in6
 *
 * Actually, it represents an IPv6 address.
 ******************************************************************************/
struct in6_addr
{
    fnet_ip6_addr_t s6_addr;        /**< @brief 128-bit IPv6 address.*/
};

/**************************************************************************/ /*!
 * @brief IPv6 Socket address structure.
 *
 * @see sockaddr
 *
 * To make manipulation of the @ref sockaddr structure easier, the TCP/IPv6 stack 
 * also defines the equivalent structure @ref sockaddr_in6 
 * ("in" means "Internet").
 ******************************************************************************/
struct sockaddr_in6
{
    fnet_address_family_t  sin6_family; /**< @brief Specifies the address family. @n 
                                         * It must ne set to @ref AF_INET6.  
                                         */
    unsigned short  sin6_port;          /**< @brief 16-bit port number used to 
                                         * demultiplex the transport-level messages
                                         * (in network byte order). 
                                         */
    struct in6_addr sin6_addr;          /**< @brief 128-bit IPv6 internet address. 
                                         */
    unsigned long   sin6_scope_id;      /**< @brief Scope zone index, defining network interface.
                                         */                                      
};

/**************************************************************************/ /*!
 * @brief Multicast group information.
 *
 * The structure is used with the @ref IP_ADD_MEMBERSHIP and 
 * @ref IP_DROP_MEMBERSHIP socket options. 
 *
 * @see IP_ADD_MEMBERSHIP, IP_DROP_MEMBERSHIP
 *
 ******************************************************************************/
struct ip_mreq
{
    struct in_addr imr_multiaddr;   /**< @brief IPv4 multicast address of group. */
    struct in_addr imr_interface;   /**< @brief Local IPv4 address of interface on which 
                                     * the multicast group should be joined or dropped.@n
                                     * If this member specifies an IPv4 address of 0.0.0.0 
                                     * or @ref INADDR_ANY, 
                                     * the default interface is used. */
};

/**************************************************************************/ /*!
 * @brief Socket types.
 ******************************************************************************/
typedef enum
{
    SOCK_UNSPEC = (0),  /**< @brief Unspecified socket type. 
                         */
    SOCK_STREAM = (1),  /**< @brief Stream socket.@n
                         * Provides reliable, two-way, connection-based 
                         * byte stream. It corresponds to the TCP protocol    
                         */
    SOCK_DGRAM  = (2),  /**< @brief Datagram socket.@n
                         * Provides unreliable, connectionless datagrams. 
                         * It corresponds to the UDP protocol. 
                         */
    SOCK_RAW    = (3)   /**< @brief Raw socket.@n
                         * Raw sockets allow an application to have direct access to 
                         * lower-level communication protocols. 
                         * Raw sockets are intended to take advantage of some protocol feature 
                         * that is not directly accessible through a normal interface, 
                         * or to build new protocols on top of existing low-level protocols.@n
                         * It can be enabled by the @ref FNET_CFG_RAW option.
                         */
} fnet_socket_type_t;

/**************************************************************************/ /*!
 * @brief Socket state.
 ******************************************************************************/
typedef enum
{
    SS_UNCONNECTED = (0),   /**< @brief Not connected to any socket. 
                             */
    SS_CONNECTING  = (1),   /**< @brief In process of connecting. 
                             */
    SS_CONNECTED   = (2),   /**< @brief Connected to a socket. 
                             */
    SS_LISTENING   = (3)    /**< @brief In listening state. 
                             */
} fnet_socket_state_t;

/**************************************************************************/ /*!
 * @brief Protocol numbers and Level numbers for the @ref setsockopt() 
 * and the @ref getsockopt().
 ******************************************************************************/
typedef enum
{
    IPPROTO_IP      = (0), /**< @brief IPv4 options level number 
                            *   for @ref getsockopt() and @ref setsockopt().
                            */
    IPPROTO_ICMP    = (1), /**< @brief ICMPv4 protocol number.
                            */
    IPPROTO_IGMP    = (2), /**< @brief IGMP protocol number.
                            */                            
    IPPROTO_TCP     = (6), /**< @brief TCP protocol number; TCP options level number
                            *   for @ref getsockopt() and @ref setsockopt().
                            */
    IPPROTO_UDP     = (17),/**< @brief UDP protocol number.
                            */
    IPPROTO_IPV6    = (41), /**< @brief IPv6 options level number 
                            *    for @ref getsockopt() and @ref setsockopt().
                            */                            
    IPPROTO_ICMPV6  = (58),/**< @brief ICMPv6 protocol number.
                            */                         
    SOL_SOCKET      = (0xFFFF)  /**< @brief Socket options level number for 
                                 * @ref getsockopt() and @ref setsockopt().
                                 */
} fnet_protocol_t;

/**************************************************************************/ /*!
 * @brief Socket level (@ref SOL_SOCKET) options for the @ref setsockopt() and the @ref getsockopt().
 *
 * <table>
 *<caption>Socket SOL_SOCKET level options</caption>
 *<tr class="fnet_td_grey">
 *<th ALIGN=CENTER>Option</th><th ALIGN=CENTER>Option Type</th><th ALIGN=CENTER>
 * Default Value</th><th ALIGN=CENTER>Read/Write</th>
 *</tr>
 *<tr>
 *<td>@ref SO_ACCEPTCONN</td><td>int</td><td>0</td><td>R</td>
 *</tr>
 *<tr>
 *<td>@ref SO_KEEPALIVE</td><td>int</td><td>1</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref SO_DONTROUTE</td><td>int</td><td>0</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref SO_LINGER</td><td>struct @ref linger</td><td>{0,0}</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref SO_OOBINLINE</td><td>int</td><td>0</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref SO_SNDBUF</td><td>unsigned long</td>
 *<td>@ref FNET_CFG_SOCKET_UDP_TX_BUF_SIZE for UDP @n @ref FNET_CFG_SOCKET_TCP_TX_BUF_SIZE for TCP</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref SO_RCVBUF</td><td>unsigned long</td>
 *<td>@ref FNET_CFG_SOCKET_UDP_RX_BUF_SIZE for UDP @n @ref FNET_CFG_SOCKET_TCP_RX_BUF_SIZE for TCP</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref SO_STATE</td><td>@ref fnet_socket_state_t</td><td>@ref SS_UNCONNECTED</td><td>R</td>
 *</tr>
 *<tr>
 *<td>@ref SO_ERROR</td><td>@ref fnet_error_t</td><td>@ref FNET_ERR_OK</td><td>R</td>
 *</tr>
 *<tr>
 *<td>@ref SO_TYPE</td><td>@ref fnet_socket_type_t</td><td>@ref SOCK_DGRAM for 
 * UDP @n @ref SOCK_STREAM for TCP</td><td>R</td>
 *</tr>
 *<tr>
 *<td>@ref SO_RCVNUM</td><td>unsigned long</td><td>0</td><td>R</td>
 *</tr>
 *<tr>
 *<td>@ref SO_SNDNUM</td><td>unsigned long</td><td>0</td><td>R</td>
 *</tr>
 *</table>
 ******************************************************************************/
typedef enum
{
    SO_ACCEPTCONN = (0x0001), /**< @brief Returns @c 1 if a socket is in 
                               *   listening mode and returns @c 0 when vice versa. 
                               *   This is the read-only option and it is 
                               *   valid only for connection-oriented protocols (TCP). 
                               */
    SO_KEEPALIVE  = (0x0002), /**< @brief This option enables keep-alive probes 
                               *   for a socket connection. These probes are used 
                               *   to maintain a TCP connection and regularly 
                               *   test the connection to ensure that it's 
                               *   still available. It is only valid for 
                               *   connection-oriented protocols (TCP). 
                               */
    SO_DONTROUTE  = (0x0004), /**< @brief This option enables bypassing of a routing algorithm.
                               *   It means that the network interface tries to send a datagram without
                               *   a gateway. 
                               */
    SO_LINGER     = (0x0008), /**< @brief This option controls the action 
                               *   taken when unsent data is present, and 
                               *   @ref closesocket() is called. This option 
                               *   is defined by the @ref linger structure. 
                               */
#if FNET_CFG_TCP_URGENT || defined(__DOXYGEN__)
    SO_OOBINLINE  = (0x0010), /**< @brief This option specifies that out-of-band (OOB) 
                               *   data will be received in line with regular data.
                               *   It is valid only for the TCP protocol. @n
                               *   This option is avalable only if 
                               *   @ref FNET_CFG_TCP_URGENT is set to @c 1. 
                               */
#endif /* FNET_CFG_TCP_URGENT */
    SO_SNDBUF     = (0x1001), /**< @brief This option defines the maximum per-socket 
                               *   buffer size for output data.
                               */
    SO_RCVBUF     = (0x1002), /**< @brief This option defines the maximum per-socket 
                               *   buffer size for input data.
                               */
    SO_STATE      = (0x1003), /**< @brief This option defines the current state of the socket.@n
                               *   This is the read-only option and it is defined by the @ref fnet_socket_state_t type.
                               */
    SO_ERROR      = (0x1006), /**< @brief This option returns a per-socket-based error code.@n
                               *   The error code is defined by the @ref fnet_error_t type.@n
                               *   After the error value has been read in the @ref getsockopt function, it is cleared.
                               *   But a successful call using the socket routines does not clear 
                               *   the socket-based error.
                               */
    SO_TYPE       = (0x1007), /**< @brief This option defines the type of the socket.
                               *   This is a read-only option and it is defined by the @ref fnet_socket_type_t type.
                               */
    SO_RCVNUM     = (0x1008), /**< @brief This option is used to determine the amount of data 
                               *   pending in the socket-input buffer.@n
                               *   This is a read-only option.
                               */
    SO_SNDNUM     = (0x1009)  /**< @brief This option is used to determine the amount of data 
                               *   in the socket output buffer. @n
                               *   This is a read-only option.
                               */
} fnet_socket_options_t;


/**************************************************************************/ /*!
 * @brief TCP level (@ref IPPROTO_TCP) options for the @ref setsockopt() 
 * and the @ref getsockopt().
 *
 * <table>
 *<caption>Socket IPPROTO_TCP level options</caption>
 *<tr class="fnet_td_grey">
 *<th ALIGN=CENTER>Option</th><th ALIGN=CENTER>Option Type</th><th ALIGN=CENTER>
 * Default Value</th><th ALIGN=CENTER>Read/Write</th>
 *</tr>
 *<tr>
 *<td>@ref TCP_MSS</td><td>unsigned short</td><td>536</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref TCP_BSD</td><td>int</td><td>1</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref TCP_NODELAY</td><td>int</td><td>1</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref TCP_FINRCVD</td><td>int</td><td>0</td><td>R</td>
 *</tr>
 *<tr>
 *<td>@ref TCP_URGRCVD</td><td>int</td><td>0</td><td>R</td>
 *</tr>
 *<tr>
 *<td>@ref TCP_KEEPIDLE</td><td>int</td><td>7200</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref TCP_KEEPINTVL</td><td>int</td><td>75</td><td>RW</td>
 *</tr> 
 *<tr>
 *<td>@ref TCP_KEEPCNT</td><td>int</td><td>8</td><td>RW</td>
 *</tr>  
 *</table>
 ******************************************************************************/
typedef enum
{
    TCP_MSS     = (0x01),   /**< @brief This option defines the maximum size of 
                             *   the input segments (MSS). @n
                             * The TCP Maximum Segment Size (MSS) defines the maximum amount 
                             * of data that a host is willing to accept in a single TCP segment.@n
                             * This Maximum Segment Size (MSS) announcement is sent from the
                             * data receiver to the data sender and says "I can accept TCP segments
                             * up to size X". The size (X) may be larger or smaller than the
                             * default.@n
                             * The MSS counts only data octets in the segment, it does not count the
                             * TCP header or the IP header.@n
                             * The default value is defined by the @ref FNET_CFG_SOCKET_TCP_MSS 
                             * user-configuration option. @n
                             * This option can be set to:
                             *     - @c 0 = The selection of the MSS is 
                             *              automatic and is based on the MTU of the outgoing 
                             *              interface minus 40 (does not include 
                             *              the 20 byte IP header and the 20 byte TCP header).@n
                             *              It is done to assist in avoiding of IP fragmentation 
                             *              at the endpoints of the TCP connection.
                             *     - Non-zero value (up to 64K) = The TCP segment could be as large as 64K 
                             *              (the maximum IP datagram size), but it could be fragmented 
                             *              at the IP layer in order to be transmitted 
                             *              across the network to the receiving host.
                             */
#if FNET_CFG_TCP_URGENT || defined(__DOXYGEN__)
    TCP_BSD     = (0x02),   /**< @brief If this option is set to @c 1, the BSD interpretation of 
                             *   the urgent pointer is used. In this case the 
                             *   urgent pointer of the TCP segment points to 
                             *   the next byte following after the urgent byte. 
                             *   Most of the TCP implementations use this 
                             *   interpretation by default.@n 
                             *   If this option is set to @c 0, the interpretation of
                             *   the TCP specification is used. In this case the 
                             *   urgent pointer of the TCP segment points 
                             *   to the urgent byte. @n
                             *   This option is avalable only if 
                             *   @ref FNET_CFG_TCP_URGENT is set to @c 1.
                             */
#endif
    TCP_NODELAY = (0x04),   /**< @brief If this option is set to @c 1, the Nagle algorithm 
                             *   is disabled (and vice versa). @n
                             *   The Nagle algorithm is effective in reducing the number 
                             *   of small packets sent by the host by essentially buffering 
                             *   send data, if there is unacknowledged data already "in flight", 
                             *   or until a full-size packet can be sent.@n
                             *   But for some applications this algorithm can impede 
                             *   performance, especially for a bulky data transfer.
                             */
    TCP_FINRCVD = (0x08),   /**< @brief This option is set when the final (FIN) segment arrives. @n 
                             *   This option indicates that another side will not send any data 
                             *   in the current connection.@n
                             *   This is the read-only option.
                             */
#if FNET_CFG_TCP_URGENT || defined(__DOXYGEN__)
    TCP_URGRCVD = (0x10),   /**< @brief This option is set when the urgent byte arrives, and 
                             *   reset when this byte is read.@n
                             *   This option can be set only if the @ref SO_OOBINLINE option is set to @c 0.@n
                             *   This is the read-only option. @n 
                             *   This option is avalable only if 
                             *   @ref FNET_CFG_TCP_URGENT is set to @c 1.                             
                             */
#endif
    TCP_KEEPIDLE = (0x20),  /**< @brief When the @ref SO_KEEPALIVE option is enabled, TCP probes a connection that
                             *   has been idle for some amount of time.  The default value for this idle
                             *   period is @c 14400 seconds (2 hours).  The @ref TCP_KEEPIDLE option can be used to affect this
                             *   value for a given socket, and specifies the number of seconds of idle
                             *   time between keepalive probes.   
                             */                           
    TCP_KEEPINTVL = (0x40), /**< @brief When the @ref SO_KEEPALIVE option is enabled, TCP probes a connection that
                             *   has been idle for some amount of time.  If the remote system does not
                             *   respond to a keepalive probe, TCP retransmits the probe after some
                             *   amount of time.  The default value for this retransmit interval is @c 75
                             *   seconds. The @ref TCP_KEEPINTVL option can be used to affect this value for
                             *   a given socket, and specifies the number of seconds to wait before
                             *   retransmitting a keepalive probe.  
                             */                           
    TCP_KEEPCNT = (0x80)    /**< @brief When the @ref SO_KEEPALIVE option is enabled, TCP probes a connection that
                             *   has been idle for some amount of time.  If the remote system does not
                             *   respond to a keepalive probe, TCP retransmits the probe a certain
                             *   number of times before a connection is considered to be broken.  The
                             *   default value for this keepalive probe retransmit limit is @c 8.  The
                             *   @ref TCP_KEEPCNT option can be used to affect this value for a given socket,
                             *   and specifies the maximum number of keepalive probes to be sent.
                             */                            
} fnet_tcp_options_t;

/**************************************************************************/ /*!
 * @brief IP level (@ref IPPROTO_IP) options for the @ref setsockopt() and 
 * the @ref getsockopt().
 *
 * <table>
 *<caption>Socket IPPROTO_IP level options</caption>
 *<tr class="fnet_td_grey">
 *<th ALIGN=CENTER>Option</th><th ALIGN=CENTER>Option Type</th><th ALIGN=CENTER>
 * Default Value</th><th ALIGN=CENTER>Read/Write</th>
 *</tr>
 *<tr>
 *<td>@ref IP_TOS</td><td>int</td><td>0</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref IP_TTL</td><td>int</td><td>64</td><td>RW</td>
 *</tr>
 *<tr>
 *<td>@ref IP_MULTICAST_TTL</td><td>int</td><td>1</td><td>RW</td>
 *</tr>
 *<tr> 
 *<td>@ref IP_ADD_MEMBERSHIP</td><td>@ref ip_mreq</td><td>N/A</td><td>W</td>
 *</tr> 
 *<tr>  
 *<td>@ref IP_DROP_MEMBERSHIP</td><td>@ref ip_mreq</td><td>N/A</td><td>W</td>
 *</tr> 
 *</table>
 ******************************************************************************/
typedef enum
{
    IP_TOS      = (3), /**< @brief  This option defines the IP TOS 
                        *   (type-of-service) field for outgoing datagrams.
                        */
    IP_TTL      = (4)  /**< @brief  This option defines the IP TTL 
                        *   (time-to-live) vlaue for outgoing datagrams.
                        */
#if FNET_CFG_MULTICAST || defined(__DOXYGEN__)
    ,
    IP_MULTICAST_TTL = (6),     /**< @brief This option allows to change IP "time to live" (TTL) 
                                 * value for outgoing multicast datagrams. 
                                 * Otherwise, multicast datagrams are sent with a default value of 1, 
                                 * to prevent them to be forwarded beyond the local network.@n
                                 * This option is available only if @ref FNET_CFG_MULTICAST is set to @c 1.
                                 */ 
    IP_ADD_MEMBERSHIP = (7),    /**< @brief  Join the socket to the supplied multicast group on 
                                 * the specified interface. It tells the system to receive packets on 
                                 * the network whose destination is the group address (but not its own).
                                 * It is valid only for the SOCK_DGRAM (UDP) sockets.@n
                                 * This option is available only if @ref FNET_CFG_MULTICAST is set to @c 1.
                                 */
    IP_DROP_MEMBERSHIP = (8)    /**< @brief  Drops membership to the given multicast group and interface.@n
                                 * This option is available only if @ref FNET_CFG_MULTICAST is set to @c 1.
                                 */
#endif /* FNET_CFG_MULTICAST */
} fnet_ip_options_t;

/**************************************************************************/ /*!
 * @brief IPv6 level (@ref IPPROTO_IPV6) options for the @ref setsockopt() 
 * and the @ref getsockopt().
 *
 * <table>
 *<caption>Socket IPPROTO_IPV6 level options</caption>
 *<tr class="fnet_td_grey">
 *<th ALIGN=CENTER>Option</th><th ALIGN=CENTER>Option Type</th><th ALIGN=CENTER>
 * Default Value</th><th ALIGN=CENTER>Read/Write</th>
 *</tr>
 *<tr>
 *<td>@ref IPV6_UNICAST_HOPS</td><td>int</td><td>64</td><td>RW</td>
 *</tr>
 *</table>
 ******************************************************************************/
typedef enum
{
    IPV6_UNICAST_HOPS = (4)  /**< @brief  This option defines hop limit used 
                              * for outgoing unicast IPv6 packets. @n
                              * Its value can be from 0 till 255.
                              */

} fnet_ip6_options_t;

/**************************************************************************/ /*!
 * @brief This structure is used for the @ref SO_LINGER option.
 ******************************************************************************/
struct linger
{
    unsigned short l_onoff;  /**< @brief Determines, whether the option will be 
                              *   turned on @c 1, or off @c 0.
                              */
    unsigned short l_linger; /**< @brief Specifies the amount of time (in seconds) 
                              *   to wait when the connection is closed and unsent data 
                              *   is discarded. 
                              *   If @c l_onoff is @c 0, the @c l_linger value is ignored
                              *   and the stack continues to try to send the data as usually.
                              */
};

/**************************************************************************/ /*!
 * @brief Socket descriptor.
 ******************************************************************************/
typedef int SOCKET;

#define SOCKET_INVALID  (FNET_ERR) /**< @brief Invalid socket. @n
                                    *   This is the return value of the @ref 
                                    *   socket() and @ref accept() functions
                                    *   if an error occurs.
                                    */
#define SOCKET_ERROR    (FNET_ERR) /**< @brief Socket error.@n
                                    *   This is the return value of 
                                    *   the socket API functions if an error occurs.
                                    */

/**************************************************************************/ /*!
 * @brief The flags parameters for receiving and sending functions 
 * @ref recv(), @ref recvfrom(), @ref send(), and @ref sendto().
 *
 * The optional flag specifies the way, in which the call is made. 
 * It can be constructed by using the bitwise OR.
 ******************************************************************************/
typedef enum
{
#if FNET_CFG_TCP_URGENT || defined(__DOXYGEN__)
    MSG_OOB       = (0x1),  /**< @brief Process out-of-band 
                             * data instead of regular data. @n 
                             *   This option is avalable only if 
                             *   @ref FNET_CFG_TCP_URGENT is set to @c 1. 
                             */
#endif /* FNET_CFG_TCP_URGENT */
    MSG_PEEK      = (0x2),  /**< @brief Receive a copy of the 
                             * data without consuming it.
                             */
    MSG_DONTROUTE = (0x4)   /**< @brief Send without using 
                             * routing tables.
                             */
} fnet_flags_t;

/**************************************************************************/ /*!
 * @brief The flags used by @ref shutdown().
 *
 * They describe what types of socket operation will no longer be allowed.
 ******************************************************************************/
typedef enum
{
    SD_READ  = (0x1), /**< @brief Data receiving is disabled.
                       */
    SD_WRITE = (0x2), /**< @brief Data sending is disabled.
                       */
    SD_BOTH  = (SD_READ | SD_WRITE) /**< @brief Both receiving and 
                                     * sending are disabled.
                                     */
} fnet_sd_flags_t;


/***************************************************************************/ /*!
 *
 * @brief    Creates a socket. 
 *
 *
 * @param family   Address family that the socket will use, defined 
 *                 by the @ref fnet_address_family_t.
 *                 Only Internet address family (@ref AF_INET) is supported.
 *
 * @param type     Type specification for the new socket, defined by @ref
 *                 fnet_socket_type_t. It can be @ref SOCK_STREAM (TCP) or
 *                 @ref SOCK_DGRAM (UDP).
 *
 * @param protocol Protocol to be used with the socket that is specific to 
 *                 the indicated address family. This stack supports 
 *                 @ref IPPROTO_TCP and @ref IPPROTO_UDP.@n
 *                 This parameter is optional, and can be set to zero, as 
 *                 the @c type already defines the proper protocol.
 *
 * @return This function returns:
 *   - Socket descriptor referencing the new socket, if no error occurs.
 *   - @ref SOCKET_INVALID if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see closesocket()
 *
 ******************************************************************************
 *
 * This function creates a socket and returns its descriptor.@n
 * The @ref socket() function causes a socket descriptor and any related 
 * resources to be allocated and bound to a specific transport-service 
 * provider that supports the requested combination of address family,
 * socket type, and protocol parameters.@n
 * @n
 * After a socket is created:
 * - Connection-oriented sockets, such as the @ref SOCK_STREAM, provide 
 *   full-duplex connections. Before any data can be sent or received,
 *   it must be in a connected state . A connection to 
 *   another socket is established with the @ref connect() call. 
 *   Once connected, the data can be transferred using the @ref send() and  the @ref 
 *   recv() calls. When a session has been completed, the @ref closesocket() 
 *   must be performed.
 * - Connectionless, message-oriented sockets, such as the @ref SOCK_DGRAM, allow 
 *   sending and receiving of datagrams to and from arbitrary peers using 
 *   the @ref sendto() and the @ref recvfrom(). If such a socket is connected 
 *   to a specific peer, datagrams can be sent to that peer using the 
 *   @ref send(), and can be received only from this peer using the @ref recv().
 *
 ******************************************************************************/
SOCKET socket( fnet_address_family_t family, fnet_socket_type_t type, int protocol );

/***************************************************************************/ /*!
 *
 * @brief    Assigns a local address to a socket.
 *
 *
 * @param s       Descriptor, identifying a socket to bind.
 *
 * @param name    The address to be assigned to the socket, from the @ref 
 *                sockaddr structure.
 *
 * @param namelen The length of the @c name parameter. 
 *                Normally @c namelen is set to @c sizeof(name).
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see socket()
 *
 ******************************************************************************
 *
 * This function associates a local address with the socket.@n
 * The @ref bind() function is used on an unconnected socket before 
 * subsequent calls to the @ref connect() or the @ref listen() functions. 
 * It is used to bind to either connection-oriented (stream) or connectionless 
 * (datagram) sockets.@n
 * Within the @ref sockaddr structure, the address may be the address assigned 
 * to a network interface on the host or the @ref INADDR_ANY. 
 * Using the @ref INADDR_ANY causes the stack to use the default network 
 * interface address. Using a port number of 0 causes the service provider 
 * to assign a unique port to the socket with a value between 1024 and 5000.
 *
 ******************************************************************************/
int bind( SOCKET s, const struct sockaddr *name, int namelen );

/***************************************************************************/ /*!
 *
 * @brief    Places the socket into a state, where it is listening for 
 *           an incoming connection.
 *
 *
 * @param s          Descriptor identifying a socket that will be 
 *                   used for listening.
 *
 * @param backlog    Maximum length of the queue of pending connections.
 *
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see socket(), bind(), accept()
 *
 ******************************************************************************
 *
 * Only connection-oriented sockets (@ref SOCK_STREAM) are used with the
 * @ref listen().@n
 * The socket is put into passive mode, where the incoming connection 
 * requests are acknowledged and queued pending acceptance by the listening socket.
 * The @ref listen() function is typically used by servers that can have 
 * more than one connection request at a time. @n 
 * An application can call @ref listen() more than once on the same socket. 
 * If there are more pending connections than the new backlog value,
 * the excess pending connections will be reset and dropped.
 *
 ******************************************************************************/
int listen( SOCKET s, int backlog );

/***************************************************************************/ /*!
 *
 * @brief    Accepts a connection on the specified socket.
 *
 *
 * @param s          Descriptor, identifying an unconnected socket.
 *
 * @param addr       Optional pointer to a buffer that receives the address 
 *                   of the remote host at the other end of the connection.
 *
 * @param addrlen    Optional pointer to an integer that contains the 
 *                   length of the @c addr parameter.
 *
 *
 * @return This function returns:
 *   - Socket descriptor referencing the new socket, if no error occurs.
 *   - @ref SOCKET_INVALID if an error occurs. @n 
 *     The specific error code can be retrieved using @ref fnet_error_get().
 *
 * @see socket(), bind()
 *
 ******************************************************************************
 *
 * The function extracts the first connection in the queue of pending 
 * connections on the listening socket @c s, and returns the new socket descriptor.@n
 * The newly-created socket is the socket that will handle the actual 
 * connection and has the same properties as the socket @c s. 
 * The original socket remains open and listens for new connection requests.
 * If no pending connections are present in the queue of the socket, 
 * the @ref accept() returns an error and the specific error code is set to 
 * @ref FNET_ERR_AGAIN.@n 
 * The parameter @c addr is a result parameter that is filled in with the 
 * address of the remote host. The @c addrlen should initially contain the 
 * amount of space pointed to by the @c addr; on return it will contain the 
 * actual length of the address returned (in bytes).@n
 * The @ref accept() function is used only with connection-oriented socket 
 * types (@ref SOCK_STREAM).@n
 * If @c addr and/or @c addrlen are equal to @c 0, then no information
 * about the remote address of the accepted socket is returned.
 *
 ******************************************************************************/
SOCKET accept( SOCKET s, struct sockaddr *addr, int *addrlen );

/***************************************************************************/ /*!
 *
 * @brief    Establishes a connection with the specified socket.
 *
 *
 * @param s          Descriptor identifying an unconnected socket.
 *
 * @param name       Address (name) of the socket, with which the connection 
 *                   should be established.
 *
 * @param namelen    Length of the @c name.
 *
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see socket(), bind()
 *
 ******************************************************************************
 *
 * For connection-oriented sockets (@ref SOCK_STREAM), this function activates 
 * an active connection with the foreign host. 
 * 
 * When the socket call completes successfully, the socket is ready to send 
 * and receive data. Any attempt to reconnect the active connection will fail 
 * with the error code @ref FNET_ERR_ISCONN. For connection-oriented 
 * sockets, it is often not possible to complete the connection 
 * immediately. Until the connection attempt 
 * completes on a socket, all subsequent calls to @ref connect() 
 * on the same socket will fail with the error code @ref FNET_ERR_INPROGRESS, 
 * or succeed with @ref FNET_ERR_ISCONN when the connection completes successfully.
 * Use the @ref getsockopt() function to determine the completion of the 
 * connection request by checking the @ref SO_STATE to see if the socket is 
 * connected (@ref SS_CONNECTED), is still connecting (@ref SS_CONNECTING) 
 * or the connection has failed (@ref SS_UNCONNECTED). @n
 * @n
 * For a connectionless socket (@ref SOCK_DGRAM), the operation performed 
 * by @ref connect() is merely to establish a default destination address 
 * that can be used on subsequent @ref send() and @ref recv() calls. 
 * Any datagrams received from an address other than the destination address 
 * specified will be discarded. The default destination can be changed by 
 * simply calling the @ref connect() again, even if the socket is connected already.
 * Any datagrams queued for receipt are discarded from the previous 
 * @ref connect(). For connectionless sockets, the @c name can indicate any 
 * valid address, including a broadcast address.@n
 * @n
 * If the socket @c s is unbound, unique values are assigned to the local 
 * association by the stack, and the socket is marked as bound.
 *
 ******************************************************************************/
int connect( SOCKET s, struct sockaddr *name, int namelen );

/***************************************************************************/ /*!
 *
 * @brief    Receives the data from a connected socket.
 *
 *
 * @param s      Descriptor identifying a connected socket.
 *
 * @param buf    Buffer for the incoming data.
 *
 * @param len    Length of the @c buf.
 *
 * @param flags  Optional flag specifying the way, in which the call is made. 
 *               It can be constructed by using the bitwise OR operator with
 *               any of the values defined by the @ref fnet_flags_t.
 *
 *
 * @return This function returns:
 *   - The number of bytes received, if no error occurs. 
 *     The return value is set to zero, if there
 *     is no input data.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see connect(), recvfrom()
 *
 ******************************************************************************
 *
 * For connection-oriented sockets (@ref SOCK_STREAM), calling the @ref recv() 
 * will return as much information as is currently available up to the size 
 * of the buffer supplied.@n
 * @n
 * For connectionless sockets (@ref SOCK_DGRAM), the data is extracted from the 
 * first enqueued datagram (message), whose destination address is specified 
 * by the @ref connect() function.
 * If the datagram is larger than the supplied buffer, the buffer is filled
 * with the first part of the datagram, and the @ref recv() generates the error 
 * @ref FNET_ERR_MSGSIZE and the excess data is lost.@n
 * @n
 * The @c flags parameter can be used to influence the behavior of the function
 * invocation beyond the options specified for the associated socket. 
 *
 ******************************************************************************/
int recv( SOCKET s, char *buf, int len, int flags );

/***************************************************************************/ /*!
 *
 * @brief    Receives the data and captures the address, from which the data 
 *           was sent.
 *
 *
 * @param s      Descriptor, identifying a bound socket.
 *
 * @param buf    Buffer for the incoming data.
 *
 * @param len    Length of the @c buf.
 *
 * @param flags  Optional flag specifying the way in which the call is made. 
 *               It can be constructed by using the bitwise OR operator with
 *               any of the values defined by the @ref fnet_flags_t.
 *
 * @param from       Optional pointer to a buffer that will hold the 
 *                   source address upon return.
 *
 * @param fromlen    Optional pointer to the size of the @c from buffer.
 *
 *
 * @return This function returns:
 *   - The number of bytes received, if no error occurs. 
 *     The return value is set to zero, if the socket is nonblocking and there
 *     is no input data.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see bind(), recv()
 *
 ******************************************************************************
 *
 * For connection-oriented sockets (@ref SOCK_STREAM), calling the @ref recvfrom() 
 * will return as much information as is currently available up to the size of
 * the buffer supplied. The @ref recvfrom() call is rarely used for 
 * stream sockets, because the routine provides the peer-socket information,
 * and stream sockets are tied to a specific peer.@n
 * @n
 * For connectionless sockets (@ref SOCK_DGRAM), the data is extracted from the 
 * first enqueued datagram.
 * If the datagram is larger than the buffer supplied, the buffer is filled 
 * with the first part of the datagram, and the @ref recvfrom() generates the 
 * error @ref FNET_ERR_MSGSIZE and the excess data is lost.@n
 * @n
 * If the @c from parameter is non-zero, the network address of the peer 
 * that sent the data is copied to the corresponding @ref sockaddr structure. 
 * The value pointed to by @c fromlen is initialized to the size of this 
 * structure and is modified on return, to indicate the actual size of 
 * the address stored in the @ref sockaddr structure.@n
 * @n
 * The flags parameter can be used to influence the behavior of the function 
 * invocation beyond the options specified for the associated socket.
 *
 ******************************************************************************/
int recvfrom( SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen );

/***************************************************************************/ /*!
 *
 * @brief    Sends the data on a connected socket.
 *
 *
 * @param s      Descriptor, identifying a connected socket.
 *
 * @param buf    Buffer containing the data to be transmitted.
 *
 * @param len    Length of the data in @c buf.
 *
 * @param flags  Optional flag specifying the way in which the call is made. 
 *               It can be constructed by using the bitwise OR operator with
 *               any of the values defined by the @ref fnet_flags_t.
 *
 *
 * @return This function returns:
 *   - The total number of bytes sent, if no error occurs. 
 *     It can be less than the number indicated by @c len.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see connect(), sendto()
 *
 ******************************************************************************
 *
 * For stream-oriented sockets (@ref SOCK_STREAM), the number of 
 * actually sent bytes can be between @c 1 and the requested length, depending on 
 * buffer availability on both client and server machines.@n
 * @n
 * For message-oriented sockets (@ref SOCK_DGRAM), care must be taken not to  
 * exceed the maximum datagram size, which can be obtained by using the @ref 
 * getsockopt() to retrieve the value of the socket option @ref SO_SNDBUF. 
 * If the data is too long to pass atomically through the underlying 
 * datagram protocol, the error @ref FNET_ERR_MSGSIZE is returned 
 * and no data is transmitted.@n
 * Calling the @ref send() with a zero @c len parameter is permissible. 
 * In such cases, the @ref send() will return zero as a valid value, and 
 * for message-oriented sockets (@ref SOCK_DGRAM) a zero-length transport 
 * datagram is sent.@n
 * @n
 * The @c flags parameter can be used to influence the behavior of the function
 * invocation beyond the options specified for the associated socket. 
 *
 ******************************************************************************/
int send( SOCKET s, char *buf, int len, int flags );

/***************************************************************************/ /*!
 *
 * @brief    Sends the data to a specific destination.
 *
 *
 * @param s      Descriptor, identifying a socket.
 *
 * @param buf    Buffer containing the data to be transmitted.
 *
 * @param len    Length of the data in @c buf.
 *
 * @param flags  Optional flag specifying the way, in which the call is made. 
 *               It can be constructed by using the bitwise OR operator with
 *               any of the values defined by the @ref fnet_flags_t.
 *
 * @param to     Pointer to the address of the target socket.
 *
 * @param tolen  Size of the address in @c to.
 *
 *
 * @return This function returns:
 *   - The total number of bytes sent, if no error occurs. 
 *     It can be lesser than the number indicated by @c len.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see connect(), send()
 *
 ******************************************************************************
 *
 * For nonb-locking stream-oriented sockets (@ref SOCK_STREAM), the number of 
 * actually sent bytes can be between @c 1 and the requested length, depending on 
 * buffer availability on both client and server machines.@n
 * @n
 * For message-oriented sockets (@ref SOCK_DGRAM), care must be taken not not 
 * exceed the maximum datagram size, which can be obtained by using the @ref 
 * getsockopt() to retrieve the value of the socket option @ref SO_SNDBUF. 
 * If the data is too long to pass atomically through the underlying 
 * datagram protocol, the error @ref FNET_ERR_MSGSIZE is returned 
 * and no data is transmitted.@n
 * Calling @ref sendto() with a zero @c len parameter is permissible. 
 * In such cases, the @ref sendto() will return zero as a valid value, and 
 * for message-oriented sockets (@ref SOCK_DGRAM), a zero-length transport 
 * datagram is sent.@n
 * @n
 * The @ref sendto() function is normally used on a connectionless socket 
 * (@ref SOCK_DGRAM) to send a datagram to a specific peer socket identified 
 * by the @c to parameter. Even if the connectionless socket has been previously 
 * connected to a specific address, the @c to parameter overrides the 
 * destination address for that particular datagram only.@n 
 * On a connection-oriented socket (@ref SOCK_STREAM), the @c to and @c tolen 
 * parameters are ignored, making the @ref sendto() equivalent to @ref send().@n
 * @n
 * The @c flags parameter can be used to influence the behavior of the function
 * invocation beyond the options specified for the associated socket. 
 *
 ******************************************************************************/
int sendto( SOCKET s, char *buf, int len, int flags, const struct sockaddr *to, int tolen );

/***************************************************************************/ /*!
 *
 * @brief    Terminates the connection in one or both directions.
 *
 *
 * @param s      Descriptor, identifying a socket.
 *
 * @param how    Flag that describes what types of operation will no longer 
 *               be allowed, it's defined by the @ref fnet_sd_flags_t.
 *
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see closesocket()
 *
 ******************************************************************************
 *
 * This function is used on all types of sockets to disable reception, 
 * transmission, or both.@n
 * If the @c how parameter is @ref SD_READ, subsequent calls to the @ref recv()
 * function on the socket will be unallowed. If the @c how parameter is 
 * @ref SD_WRITE, subsequent calls to the @ref send() function are unallowed. 
 * Setting the @c how to the @ref SD_BOTH disables both sends and receives as 
 * described above.@n
 * The @ref shutdown() function does not close the socket. Any resources 
 * attached to the socket will not be freed until the @ref closesocket() is invoked.
 *
 ******************************************************************************/
int shutdown( SOCKET s, int how );

/***************************************************************************/ /*!
 *
 * @brief    Closes an existing socket.
 *
 *
 * @param s      Descriptor, identifying a socket to close.
 *
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see socket()
 *
 ******************************************************************************
 *
 * This function releases the socket descriptor @c s, so that further 
 * references to @c s fail with the error @ref FNET_ERR_BAD_DESC.@n
 * An application should always have a matching call to the @ref closesocket() for 
 * each successful call to the @ref socket() to return any socket resources to
 * the system.@n
 * @n
 * For the @ref SOCK_STREAM sockets, the @ref SO_LINGER option controls the action 
 * of the closing, when unsent data is present on the socket and the @ref 
 * closesocket() function is performed. This option has effect on 
 * TCP sockets only. To enable or disable the @ref SO_LINGER option, the application 
 * should use the @ref linger structure. This structure includes the 
 * @c l_onoff and the @c l_linger fields. If the @c l_onoff has a non-zero value, 
 * then the @ref SO_LINGER is enabled. Otherwise, the @ref SO_LINGER option is 
 * disabled. The @c l_linger field includes the timeout in seconds.
 * If the @ref SO_LINGER option is not used 
 * , the remaining data of the output buffer can be sent after the execution 
 * of the @ref closesocket() in the background.@n
 * If a timeout expires and unsent data is present, the hard reset occurs and
 * unsent data will be deleted.@n
 * If the @ref SO_LINGER option is used with a zero timeout interval, the 
 * hard reset occurs immediately and unsent data will be deleted.
 *
 ******************************************************************************/
int closesocket( SOCKET s );


/***************************************************************************/ /*!
 *
 * @brief    Sets a socket option.
 *
 *
 * @param s          Descriptor identifying a socket.
 *
 * @param level      Level, at which the option is defined.
 *                   The supported levels are @ref SOL_SOCKET, 
 *                   @ref IPPROTO_IP, and @ref IPPROTO_TCP.
 *
 * @param optname    Socket option for which the value is to be set.
 *                   - for the @ref SOL_SOCKET level the options are defined by 
 *                     @ref fnet_socket_options_t.
 *                   - for the @ref IPPROTO_IP level the options are defined by 
 *                     @ref fnet_ip_options_t.
 *                   - for the @ref IPPROTO_TCP level the options are defined by 
 *                     @ref fnet_tcp_options_t.
 *
 * @param optval     Pointer to a buffer, in which the value for the requested 
 *                   option is supplied.
 *
 * @param optlen     Size of the @c optval buffer.
 *
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see getsockopt()
 *
 ******************************************************************************
 *
 * This function sets the current value for a socket option associated with the 
 * specified socket.@n
 * The various socket options are described by the @ref fnet_socket_options_t, 
 * @ref fnet_ip_options_t, and @ref fnet_tcp_options_t.
 *
 ******************************************************************************/
int setsockopt( SOCKET s, int level, int optname, char *optval, int optlen );

/***************************************************************************/ /*!
 *
 * @brief    Gets a socket option.
 *
 *
 * @param s          Descriptor identifying a socket.
 *
 * @param level      Level, at which the option is defined.
 *                   The supported levels are the @ref SOL_SOCKET, 
 *                   @ref IPPROTO_IP, and @ref IPPROTO_TCP.
 *
 * @param optname    Socket option, for which the value is to be retrievied.
 *                   - for the @ref SOL_SOCKET level the options are defined by 
 *                     @ref fnet_socket_options_t.
 *                   - for the @ref IPPROTO_IP level the options are defined by 
 *                     @ref fnet_ip_options_t.
 *                   - for the @ref IPPROTO_TCP level the options are defined by 
 *                     @ref fnet_tcp_options_t.
 *
 * @param optval     Pointer to a buffer in which the value for the requested 
 *                   option has to be returned.
 *
 * @param optlen     Pointer to the size of the @c optval buffer.
 *
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see setsockopt()
 *
 ******************************************************************************
 *
 * This function retrieves the current value for a socket option associated 
 * with the socket @c s, and stores the result in the @c optval.@n
 * The various socket options are described by the @ref fnet_socket_options_t, 
 * @ref fnet_ip_options_t, and @ref fnet_tcp_options_t.
 *
 ******************************************************************************/
int getsockopt( SOCKET s, int level, int optname, char *optval, int *optlen );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the name of a peer connected to a socket.
 *
 *
 * @param s          Descriptor identifying a connected socket.
 *
 * @param name       Structure that receives the name (address) of the peer.
 *
 * @param namelen    Pointer to the size of the @c name structure.
 *
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see getsockname()
 *
 ******************************************************************************
 *
 * This function retrieves the name of the peer connected to the socket @c s 
 * and stores it in the @ref sockaddr structure identified by @c name.@n
 * The @ref getpeername() function can be used only with a connected socket.
 * For datagram sockets, only the name of a peer specified in a previous 
 * @ref connect() call will be returned - any name specified by a previous 
 * @ref sendto() call will not be returned by the @ref getpeername().
 *
 ******************************************************************************/
int getpeername( SOCKET s, struct sockaddr *name, int *namelen );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the current name for the specified socket.
 *
 *
 * @param s          Descriptor identifying a socket.
 *
 * @param name       Structure that receives the name (address) of the 
 *                   socket @c s.
 *
 * @param namelen    Pointer to the size of the @c name structure.
 *
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref SOCKET_ERROR if an error occurs. @n 
 *     The specific error code can be retrieved using the @ref fnet_error_get().
 *
 * @see getpeername()
 *
 ******************************************************************************
 *
 * This function retrieves the current name for the specified socket @c s
 * and stores it in the @c name.@n
 * It is used on a bound or connected socket. 
 *
 ******************************************************************************/
int getsockname( SOCKET s, struct sockaddr *name, int *namelen );

/***************************************************************************/ /*!
 *
 * @brief    Compares socket addresses.
 *
 * @param addr1     First socket address to be compared.
 *
 * @param addr2     Second socket address to be compared.
 *
 * @return This function returns:
 *   - @ref FNET_TRUE if addresses are equal.
 *   - @ref FNET_FALSE if addresses are not equal. 
*
 ******************************************************************************
 *
 * This function compares specified socket addresses, depending on address family. @n
 * Port number and Scope Id do not take part in process of comparison.
 *
 ******************************************************************************/
int fnet_socket_addr_are_equal(const struct sockaddr *addr1, const struct sockaddr *addr2);

/***************************************************************************/ /*!
 *
 * @brief    Determines, if socket address is unspecified.
 *
 * @param addr  Socket address.
 *
 * @return       This function returns:
 *   - @c FNET_TRUE if the address is unspecified.
 *   - @c FNET_FALSE if the address is specified.
 *
 ******************************************************************************
 *
 * This function determines if the socket address is unspecified 
 *  (IP address is set to all zeros) or not.
 *
 ******************************************************************************/
int fnet_socket_addr_is_unspecified(const struct sockaddr *addr);

/*! @} */

#endif /* _FNET_SOCKET_H_ */

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
* @file fnet_stack_config.h
*
* @author Andrey Butok
*
* @date Feb-5-2013
*
* @version 0.1.57.0
*
* @brief Main TCP/IP stack default configuration file.
*
***************************************************************************/

/************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 ************************************************************************/

#ifndef _FNET_STACK_CONFIG_H_

#define _FNET_STACK_CONFIG_H_

/*****************************************************************************
*     IP6-specific parameters.
******************************************************************************/
/*! @addtogroup fnet_platform_stack_ip6_config  */
/*! @{ */

/**************************************************************************/ /*!
 * @def     FNET_CFG_IP6
 * @brief   IPv6 protocol support:
 *               - @b @c 1 = is enabled.
 *               - @c 0 = is disabled (Default value).@n
 * 
 * @see FNET_CFG_IP4
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_IP6
    #define FNET_CFG_IP6                        (0)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_ND6_NEIGHBOR_CACHE_SIZE
 * @brief   Maximum number of entries in neighbor cache (per interface).
 *           
 * @note    A small cache size may result in an
 *          excessive number of Neighbor Discovery messages if entries are
 *          discarded and rebuilt in quick succession.
 * @showinitializer 
 ******************************************************************************/ 
#ifndef FNET_CFG_ND6_NEIGHBOR_CACHE_SIZE
   #define FNET_CFG_ND6_NEIGHBOR_CACHE_SIZE     (5)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_ND6_PREFIX_LIST_SIZE
 * @brief   Maximum number of entries in the Prefix list (per interface).
 * @showinitializer 
 ******************************************************************************/ 
#ifndef FNET_CFG_ND6_PREFIX_LIST_SIZE
    #define FNET_CFG_ND6_PREFIX_LIST_SIZE       (4)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_ND6_ROUTER_LIST_SIZE
 * @brief   Maximum number of entries in the Default Router list (per interface).
 * 
 * @note    RFC4861: However, a host MUST retain at least two router
 *          addresses and SHOULD retain more.
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_ND6_ROUTER_LIST_SIZE
    #define FNET_CFG_ND6_ROUTER_LIST_SIZE       (2)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_ND6_DAD_TRANSMITS
 * @brief   (RFC4862 5.1)The number of consecutive Neighbor
 *          Solicitation messages sent while performing Duplicate Address
 *          Detection on a tentative address. A value of zero indicates that
 *          Duplicate Address Detection is not performed on tentative
 *          addresses. A value of one indicates a single transmission with no
 *          follow-up retransmissions. @n
 *          Default is @c 1.
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_ND6_DAD_TRANSMITS 
    #define FNET_CFG_ND6_DAD_TRANSMITS          (1) /* If RTCSCFG_ND6_DAD_TRANSMITS = 0, the DAD is disabled.*/
#endif 

/* Check minimum values.*/
#if FNET_CFG_ND6_ROUTER_LIST_SIZE < 1 
    #error FNET_CFG_ND6_ROUTER_LIST_SIZE must be > 0
#endif
#if FNET_CFG_ND6_NEIGHBOR_CACHE_SIZE  < 1
    #error FNET_CFG_ND6_NEIGHBOR_CACHE_SIZE must be > 0
#endif
#if FNET_CFG_ND6_PREFIX_LIST_SIZE < 1 
    #error FNET_CFG_ND6_PREFIX_LIST_SIZE must be > 0
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_IP6_FRAGMENTATION
 * @brief    IPv6 fragmentation:
 *               - @b @c 1 = is enabled (Default value). @n The IPv6 will attempt
 *                        to reassemble the IPv6 packet fragments and will be able to
 *                        generate fragmented IPv6 packets.
 *               - @c 0 = is disabled. The IPv6 will
 *                        silently discard the fragmented IPv6 packets..
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_IP6_FRAGMENTATION
    #define FNET_CFG_IP6_FRAGMENTATION      (1)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_IP6_PMTU_DISCOVERY
 * @brief    Path MTU Discovery for IPv6:
 *               - @b @c 1 = is enabled (Default value).
 *               - @c 0 = is disabled. 
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_IP6_PMTU_DISCOVERY
    #define FNET_CFG_IP6_PMTU_DISCOVERY     (1)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_NETIF_IP6_ADDR_MAX
 * @brief    Maximum number of IPv6 addresses that can be bound to an interface.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_NETIF_IP6_ADDR_MAX
    #define FNET_CFG_NETIF_IP6_ADDR_MAX     (5)
#endif

#if FNET_CFG_NETIF_IP6_ADDR_MAX <2 
    #undef FNET_CFG_NETIF_IP6_ADDR_MAX
    #define FNET_CFG_NETIF_IP6_ADDR_MAX     (2)
#endif 
 
/*! @} */

/*! @addtogroup fnet_platform_stack_ip4_config  */
/*! @{ */

/**************************************************************************/ /*!
 * @def     FNET_CFG_IP4
 * @brief   IPv4 protocol support:
 *               - @b @c 1 = is enabled (Default value).
 *               - @c 0 = is disabled.@n
 * 
 * @see FNET_CFG_IP6
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_IP4
    #define FNET_CFG_IP4                        (1)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_IP4_FRAGMENTATION
 * @brief    IP fragmentation:
 *               - @c 1 = is enabled. The IP will attempt
 *                        to reassemble the IP packet fragments and will be able to
 *                        generate fragmented IP packets.
 *               - @b @c 0 = is disabled (Default value). The IP will
 *                        silently discard the fragmented IP packets..
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_IP4_FRAGMENTATION
    #define FNET_CFG_IP4_FRAGMENTATION          (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH0_IP4_ADDR
 * @brief    Defines the default IP address for the Ethernet-0 interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_ip4_addr().
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_ETH0_IP4_ADDR
    #define FNET_CFG_ETH0_IP4_ADDR        (FNET_IP4_ADDR_INIT(192, 168, 0, 20))
#endif
    
/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH0_IP4_MASK
 * @brief    Defines the default IP Subnetmask for the Ethernet-0 interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_ip4_subnet_mask().
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_ETH0_IP4_MASK
    #define FNET_CFG_ETH0_IP4_MASK        (FNET_IP4_ADDR_INIT(255, 255, 255, 0))
#endif
    
/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH0_IP4_GW
 * @brief    Defines the default Gateway IP address for the Ethernet-0 interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_ip4_gateway().
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_ETH0_IP4_GW
    #define FNET_CFG_ETH0_IP4_GW          (FNET_IP4_ADDR_INIT(0, 0, 0, 0))
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH0_IP4_DNS
 * @brief    Defines the default DNS IP address for the Ethernet-0 interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_ip4_dns().
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_ETH0_IP4_DNS
    #define FNET_CFG_ETH0_IP4_DNS        (FNET_IP4_ADDR_INIT(0, 0, 0, 0))
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH1_IP4_ADDR
 * @brief    Defines the default IP address for the Ethernet-1 interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_ip4_addr().
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_ETH1_IP4_ADDR
    #define FNET_CFG_ETH1_IP4_ADDR        (FNET_IP4_ADDR_INIT(192, 168, 0, 21))
#endif
    
/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH1_IP4_MASK
 * @brief    Defines the default IP Subnetmask for the Ethernet-1 interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_ip4_subnet_mask().
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_ETH1_IP4_MASK
    #define FNET_CFG_ETH1_IP4_MASK        (FNET_IP4_ADDR_INIT(255, 255, 255, 0))
#endif
    
/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH1_IP4_GW
 * @brief    Defines the default Gateway IP address for the Ethernet-1 interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_ip4_gateway().
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_ETH1_IP4_GW
    #define FNET_CFG_ETH1_IP4_GW          (FNET_IP4_ADDR_INIT(0, 0, 0, 0))
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_ETH1_IP4_DNS
 * @brief    Defines the default DNS IP address for the Ethernet-1 interface.
 *           At runtime, it can be changed by @ref fnet_netif_set_ip4_dns().
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_ETH1_IP4_DNS
    #define FNET_CFG_ETH1_IP4_DNS        (FNET_IP4_ADDR_INIT(0, 0, 0, 0))
#endif



/**************************************************************************/ /*!
 * @def      FNET_CFG_LOOPBACK_IP4_ADDR
 * @brief    Defines the IP address for the Loopback interface.
 *           By default it is set to 127.0.0.1.
 *           At runtime, it can be changed by @ref fnet_netif_set_ip4_addr().
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_LOOPBACK_IP4_ADDR
    #define FNET_CFG_LOOPBACK_IP4_ADDR   (FNET_IP4_ADDR_INIT(127, 0, 0, 1))
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_IGMP
 * @brief    Internet Group Management Protocol (IGMP) support:
 *               - @c 1 = is enabled. It sets @ref FNET_CFG_MULTICAST to 1 automatically.
 *               - @b @c 0 = is disabled (Default value).@n
 * @see FNET_CFG_IGMP_VERSION
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_IGMP
    #define FNET_CFG_IGMP                   (0)
#endif

#if FNET_CFG_IGMP
    /* IGMP requires IPv4 multicast support.*/
    #undef FNET_CFG_MULTICAST
    #define  FNET_CFG_MULTICAST             (1)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_IGMP_VERSION
 * @brief    Internet Group Management Protocol (IGMP) version:
 *               - @c 1 = IGMPv1 - RFC1112.
 *               - @b @c 2 = IGMPv2 - RFC2236 (Default value).@n
 * @see FNET_CFG_IGMP 
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_IGMP_VERSION
    #define FNET_CFG_IGMP_VERSION           (2)
#endif

#if (FNET_CFG_IGMP_VERSION < 1) || (FNET_CFG_IGMP_VERSION > 2)
    #error "FNET_CFG_IGMP_VERSION must be set to 1 or to 2"
#endif


/*! @} */

/*! @addtogroup fnet_stack_config */
/*! @{ */

/*****************************************************************************
*     TCP/IP stack general parameters.
******************************************************************************/

/**************************************************************************/ /*!
 * @def      FNET_CFG_TCP
 * @brief    TCP protocol support:
 *               - @b @c 1 = is enabled (Default value).
 *               - @c 0 = is disabled.@n
 *           @n
 *           You can disable it to save a substantial amount of code, if
 *           your application needs the UDP only. By default, it is enabled.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_TCP
    #define FNET_CFG_TCP                    (1)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_LOOPBACK
 * @brief    Loopback interface:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_LOOPBACK
    #define FNET_CFG_LOOPBACK              (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_LOOPBACK_MULTICAST
 * @brief    Local loopback of multicast datagrams:
 *               - @c 1 = is enabled.@n
 *                        It means that the sending system should receive a
 *                        copy of the multicast datagrams that are transmitted.@n
 *                        It is valid only if @ref FNET_CFG_LOOPBACK is ser to @c 1.
 *               - @b @c 0 = is disabled (Default value).@n
 * @see FNET_CFG_LOOPBACK
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_LOOPBACK_MULTICAST
    #define FNET_CFG_LOOPBACK_MULTICAST     (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_LOOPBACK_BROADCAST
 * @brief    Local loopback of broadcast datagrams:
 *               - @c 1 = is enabled.@n
 *                        It means that the sending system should receive a
 *                        copy of the broadcast datagrams that are transmitted.@n
 *                        It is valid only if @ref FNET_CFG_LOOPBACK is ser to @c 1.
 *               - @b @c 0 = is disabled (Default value).@n
 * @see FNET_CFG_LOOPBACK
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_LOOPBACK_BROADCAST
    #define FNET_CFG_LOOPBACK_BROADCAST     (0)
#endif
    
/**************************************************************************/ /*!
 * @def      FNET_CFG_LOOPBACK_MTU
 * @brief    Defines the Maximum Transmission Unit for the Loopback interface.
 *           By default, it is set to 1576.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_LOOPBACK_MTU
    #define FNET_CFG_LOOPBACK_MTU           (1576)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_DEFAULT_IF
 * @brief    Descriptor of a default network interface set during stack initialisation.@n
 *           For example, it can be set to FNET_ETH0_IF, FNET_ETH1_IF or FNET_LOOP_IF. @n
 *           During run time it can be changed by ref@ fnet_netif_set_default().
 * @see fnet_netif_get_default(), net_netif_set_default()           
 ******************************************************************************/
#ifndef FNET_CFG_DEFAULT_IF
	#if FNET_CFG_CPU_ETH0
		#define FNET_CFG_DEFAULT_IF             (FNET_ETH0_IF)
	#elif FNET_CFG_CPU_ETH1
		#define FNET_CFG_DEFAULT_IF             (FNET_ETH1_IF)
	#elif FNET_CFG_LOOPBACK
		#define FNET_CFG_DEFAULT_IF             (FNET_LOOP_IF)
	#else
		#define FNET_CFG_DEFAULT_IF             ((fnet_netif_desc_t)FNET_NULL)
	#endif
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_MULTICAST
 * @brief    Multicast group support:
 *               - @c 1 = is enabled. @n 
 *                 It is set automatically  to @c 1, if @c FNET_CFG_IP6 is set to @c 1.
 *               - @c 0 = is disabled (default value, if only IPv4 is enabled).@n
 * @see FNET_CFG_MULTICAST_MAX, FNET_CFG_MULTICAST_SOCKET_MAX, FNET_CFG_IGMP
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_MULTICAST
    #define FNET_CFG_MULTICAST              (0)
#endif

#if FNET_CFG_IP6 /* Multicast must be enabled for IPv6. */
    #undef FNET_CFG_MULTICAST
    #define FNET_CFG_MULTICAST              (1)
#endif    

/**************************************************************************/ /*!
 * @def      FNET_CFG_MULTICAST_MAX
 * @brief    Maximum number of unique multicast memberships may exist at 
 *           the same time in the whole system.@n
 *           You can also join the same host group address on multiple interfaces.
 * @see FNET_CFG_MULTICAST, FNET_CFG_MULTICAST_SOCKET_MAX
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_MULTICAST_MAX
    #define FNET_CFG_MULTICAST_MAX          (5)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_MULTICAST_SOCKET_MAX
 * @brief    Maximum number of multicast memberships may exist at 
 *           the same time per one socket.
 * @see FNET_CFG_MULTICAST, FNET_CFG_MULTICAST_MAX
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_MULTICAST_SOCKET_MAX
    #define FNET_CFG_MULTICAST_SOCKET_MAX   (1)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_DNS
 * @brief    DNS IPv4 address support, by network interface:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).@n
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_DNS
    #define FNET_CFG_DNS                    (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_TCP_DISCARD_OUT_OF_ORDER
 * @brief    Discarding of TCP segments that are received out of order:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).@n
 *           @n
 *           But you may enable it, to save RAM.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_TCP_DISCARD_OUT_OF_ORDER
    #define FNET_CFG_TCP_DISCARD_OUT_OF_ORDER   (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_TCP_URGENT
 * @brief    TCP "urgent" (out-of-band) data processing:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 * @see SO_OOBINLINE, TCP_URGRCVD, TCP_BSD, MSG_OOB
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_TCP_URGENT
    #define FNET_CFG_TCP_URGENT                 (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_UDP
 * @brief    UDP protocol support:
 *               - @b @c 1 = is enabled (Default value).
 *               - @c 0 = is disabled.@n
 *           @n
 *           You can disable it to save some amount of code, if your
 *           application needs the TCP only. By default, it is enabled.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_UDP
    #define FNET_CFG_UDP                        (1)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_UDP_CHECKSUM
 * @brief    UDP checksum:
 *               - @b @c 1 = The UDP checksum will be generated for transmitted
 *                        datagrams and verified on received UDP datagrams (Default value).
 *               - @c 0 = The UDP checksum will not be generated for transmitted
 *                        datagrams and won't be verified on received UDP datagrams.
 *           @n@n
 *           You can disable it to speed the UDP applications up.
 *           By default, it is enabled.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_UDP_CHECKSUM
    #define FNET_CFG_UDP_CHECKSUM               (1)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_RAW
 * @brief    RAW socket support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).@n
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_RAW
    #define FNET_CFG_RAW                        (0)
#endif


/*****************************************************************************
* 	TCP/IP stack parameters.
******************************************************************************/

/**************************************************************************/ /*!
 * @def      FNET_CFG_HEAP_SIZE
 * @brief    Size of the internal static heap buffer.
 *           It is used only if @ref fnet_init_static() was
 *           called for FNET initialization.
 * @hideinitializer
 ******************************************************************************/
#ifndef FNET_CFG_HEAP_SIZE
    #define FNET_CFG_HEAP_SIZE                  (50 * 1024)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_MAX
 * @brief    Maximum number of sockets that can exist at the same time.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_SOCKET_MAX
    #define FNET_CFG_SOCKET_MAX                 (10)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_TCP_MSS
 * @brief    The default value of the @ref TCP_MSS option 
 *           (TCP Maximum Segment Size).
 *      The TCP Maximum Segment Size (MSS) defines the maximum amount 
 *      of data that a host is willing to accept in a single TCP segment.@n
 *      This Maximum Segment Size (MSS) announcement is sent from the
 *      data receiver to the data sender and says "I can accept TCP segments
 *      up to size X". The size (X) may be larger or smaller than the
 *      default.@n
 *      The MSS counts only data octets in the segment, it does not count the
 *      TCP header or the IP header.@n
 *      This option can be set to:
 *          - @b @c 0 = This is the default value. The selection of the MSS is 
 *              automatic and is based on the MTU of the outgoing 
 *              interface minus 40 (does not include 
 *              the 20 byte IP header and the 20 byte TCP header).@n
 *              It is done to assist in avoiding of IP fragmentation 
 *              at the endpoints of the TCP connection.
 *          - Non-zero value (up to 64K) = The TCP segment could be as large as 64K 
 *              (the maximum IP datagram size), but it could be fragmented 
 *              at the IP layer in order to be transmitted 
 *              across the network to the receiving host.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_SOCKET_TCP_MSS
    #define FNET_CFG_SOCKET_TCP_MSS             (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_TCP_TX_BUF_SIZE
 * @brief    Default maximum size for the TCP send-socket buffer.
 *           At runtime, it can be changed by @ref setsockopt()
 *           using the @ref SO_SNDBUF socket option.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_SOCKET_TCP_TX_BUF_SIZE
    #define FNET_CFG_SOCKET_TCP_TX_BUF_SIZE     (2 * 1024)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_TCP_RX_BUF_SIZE
 * @brief    Default maximum size for the TCP receive-socket buffer.
 *           At runtime, it can be changed by @ref setsockopt()
 *           using the @ref SO_RCVBUF socket option.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_SOCKET_TCP_RX_BUF_SIZE
    #define FNET_CFG_SOCKET_TCP_RX_BUF_SIZE     (2 * 1024)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_UDP_TX_BUF_SIZE
 * @brief    Default maximum size for the UDP send-socket buffer.
 *           At runtime, it can be changed by @ref setsockopt()
 *           using the @ref SO_SNDBUF socket option.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_SOCKET_UDP_TX_BUF_SIZE
    #define FNET_CFG_SOCKET_UDP_TX_BUF_SIZE     (2 * 1024)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_UDP_RX_BUF_SIZE
 * @brief    Default maximum size for the UDP receive-socket buffer.
 *           At runtime, it can be changed by @ref setsockopt()
 *           using the @ref SO_RCVBUF socket option.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_SOCKET_UDP_RX_BUF_SIZE
    #define FNET_CFG_SOCKET_UDP_RX_BUF_SIZE     (2 * 1024)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_RAW_TX_BUF_SIZE
 * @brief    Default maximum size for the RAW send-socket buffer.
 *           At runtime, it can be changed by @ref setsockopt()
 *           using the @ref SO_SNDBUF socket option.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_SOCKET_RAW_TX_BUF_SIZE
    #define FNET_CFG_SOCKET_RAW_TX_BUF_SIZE     (2 * 1024)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_SOCKET_RAW_RX_BUF_SIZE
 * @brief    Default maximum size for the RAW receive-socket buffer.
 *           At runtime, it can be changed by @ref setsockopt()
 *           using the @ref SO_RCVBUF socket option.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_SOCKET_RAW_RX_BUF_SIZE
    #define FNET_CFG_SOCKET_RAW_RX_BUF_SIZE     (2 * 1024)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_IP_MAX_PACKET
 * @brief    Maximum size for the IPv4 and IPv6 datagram, 
 *           the largest value is 65535. @n
 *           Default value is 10 KB.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_IP_MAX_PACKET
    #define FNET_CFG_IP_MAX_PACKET              (10*1024)  
#endif

/*****************************************************************************
 * Function Overload
 *****************************************************************************/
#ifndef FNET_CFG_OVERLOAD_CHECKSUM_LOW
    #define FNET_CFG_OVERLOAD_CHECKSUM_LOW      (0)
#endif

#ifndef FNET_CFG_OVERLOAD_MEMCPY
    #define FNET_CFG_OVERLOAD_MEMCPY            (0)
#endif

/* IPv4 and/or IPv6 must enaqbled.*/
#if !FNET_CFG_IP4 && !FNET_CFG_IP6
    #error "Please enable IPv4 or/and IPv6, by FNET_CFG_IP4 or/and FNET_CFG_IP6."
#endif


/*****************************************************************************
 * DEBUGING INFO OUTPUT
 *****************************************************************************/


/**************************************************************************/ /*!
 * @internal
 * @brief    Debugging output:
 *               - @c 1 = is enabled.
 *               - @c 0 = is disabled.
 * @internal
 ******************************************************************************/
#ifndef FNET_CFG_DEBUG
    #define FNET_CFG_DEBUG              (0)
#endif

#ifndef FNET_CFG_DEBUG_TIMER  /* It will be printed to the UART '!' sign every second. */
    #define FNET_CFG_DEBUG_TIMER        (0)
#endif

#ifndef FNET_CFG_DEBUG_HTTP
    #define FNET_CFG_DEBUG_HTTP         (0)
#endif

#ifndef FNET_CFG_DEBUG_DHCP
    #define FNET_CFG_DEBUG_DHCP         (0)
#endif

#ifndef FNET_CFG_DEBUG_ARP
    #define FNET_CFG_DEBUG_ARP          (0)
#endif

#ifndef FNET_CFG_DEBUG_MEMPOOL
    #define FNET_CFG_DEBUG_MEMPOOL      (0)
#endif

#ifndef FNET_CFG_DEBUG_TFTP_CLN
    #define FNET_CFG_DEBUG_TFTP_CLN     (0)
#endif

#ifndef FNET_CFG_DEBUG_TFTP_SRV
    #define FNET_CFG_DEBUG_TFTP_SRV     (0)
#endif

#ifndef FNET_CFG_DEBUG_STACK
    #define FNET_CFG_DEBUG_STACK        (0)
#endif

#ifndef FNET_CFG_DEBUG_TELNET
    #define FNET_CFG_DEBUG_TELNET       (0)
#endif

#ifndef FNET_CFG_DEBUG_SHELL
    #define FNET_CFG_DEBUG_SHELL        (0)
#endif

#ifndef FNET_CFG_DEBUG_DNS
    #define FNET_CFG_DEBUG_DNS          (0)
#endif

#ifndef FNET_CFG_DEBUG_BENCH
    #define FNET_CFG_DEBUG_BENCH        (0)
#endif

#ifndef FNET_CFG_DEBUG_STARTUP_MS
    #define FNET_CFG_DEBUG_STARTUP_MS   (0)
#endif

#ifndef FNET_CFG_DEBUG_IP6
    #define FNET_CFG_DEBUG_IP6          (0)
#endif

#ifndef FNET_CFG_DEBUG_TRACE
    #define FNET_CFG_DEBUG_TRACE        (0)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_IP
    #define FNET_CFG_DEBUG_TRACE_IP     (0)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_ICMP
    #define FNET_CFG_DEBUG_TRACE_ICMP   (0)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_IGMP
    #define FNET_CFG_DEBUG_TRACE_IGMP   (0)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_ETH
    #define FNET_CFG_DEBUG_TRACE_ETH    (0)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_ARP
    #define FNET_CFG_DEBUG_TRACE_ARP    (0)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_UDP
    #define FNET_CFG_DEBUG_TRACE_UDP    (0)
#endif

#ifndef FNET_CFG_DEBUG_TRACE_TCP
    #define FNET_CFG_DEBUG_TRACE_TCP    (0)
#endif


 

#if !FNET_CFG_DEBUG /* Clear all debuging flags. */
    #undef  FNET_CFG_DEBUG_TIMER
    #define FNET_CFG_DEBUG_TIMER        (0)
    #undef  FNET_CFG_DEBUG_HTTP
    #define FNET_CFG_DEBUG_HTTP         (0)
    #undef  FNET_CFG_DEBUG_DHCP
    #define FNET_CFG_DEBUG_DHCP         (0)        
    #undef  FNET_CFG_DEBUG_TELNET
    #define FNET_CFG_DEBUG_TELNET       (0)     
    #undef  FNET_CFG_DEBUG_ARP
    #define FNET_CFG_DEBUG_ARP          (0)    
    #undef  FNET_CFG_DEBUG_MEMPOOL
    #define FNET_CFG_DEBUG_MEMPOOL      (0) 
    #undef  FNET_CFG_DEBUG_TFTP_CLN
    #define FNET_CFG_DEBUG_TFTP_CLN     (0)          
    #undef  FNET_CFG_DEBUG_TFTP_SRV
    #define FNET_CFG_DEBUG_TFTP_SRV     (0)    
    #undef  FNET_CFG_DEBUG_STACK
    #define FNET_CFG_DEBUG_STACK        (0) 
    #undef  FNET_CFG_DEBUG_SHELL
    #define FNET_CFG_DEBUG_SHELL        (0)     
    #undef  FNET_CFG_DEBUG_DNS
    #define FNET_CFG_DEBUG_DNS          (0)         
    #undef  FNET_CFG_DEBUG_BENCH
    #define FNET_CFG_DEBUG_BENCH        (0)
    #undef  FNET_CFG_DEBUG_STARTUP_MS
    #define FNET_CFG_DEBUG_STARTUP_MS   (0)  
    #undef  FNET_CFG_DEBUG_TRACE
    #define FNET_CFG_DEBUG_TRACE        (0)    
    #undef  FNET_CFG_DEBUG_IP6
    #define FNET_CFG_DEBUG_IP6          (0)    
    
#endif

#if !FNET_CFG_DEBUG_TRACE /* Clear all trace flags. */
    #undef  FNET_CFG_DEBUG_TRACE_IP
    #define FNET_CFG_DEBUG_TRACE_IP     (0)
    #undef  FNET_CFG_DEBUG_TRACE_ICMP
    #define FNET_CFG_DEBUG_TRACE_ICMP   (0)    
    #undef  FNET_CFG_DEBUG_TRACE_IGMP
    #define FNET_CFG_DEBUG_TRACE_IGMP   (0)    
    #undef  FNET_CFG_DEBUG_TRACE_ETH
    #define FNET_CFG_DEBUG_TRACE_ETH    (0)
    #undef  FNET_CFG_DEBUG_TRACE_ARP
    #define FNET_CFG_DEBUG_TRACE_ARP    (0)      
    #undef  FNET_CFG_DEBUG_TRACE_UDP
    #define FNET_CFG_DEBUG_TRACE_UDP    (0)    
    #undef  FNET_CFG_DEBUG_TRACE_TCP
    #define FNET_CFG_DEBUG_TRACE_TCP    (0)    
#endif


/******************************************************************************
 * Obsolete configuration parameters
 ******************************************************************************/
#ifdef FNET_CFG_ETH_IP4_ADDR  
	#error "FNET_CFG_ETH_IP4_ADDR parameter is obsolete, use FNET_CFG_ETH0_IP4_ADDR ."
#endif
#ifdef FNET_CFG_ETH_IP4_MASK  
	#error "FNET_CFG_ETH_IP4_MASK parameter is obsolete, use FNET_CFG_ETH0_IP4_MASK ."
#endif
#ifdef FNET_CFG_ETH_IP4_GW  
	#error "FNET_CFG_ETH_IP4_GW parameter is obsolete, use FNET_CFG_ETH0_IP4_GW ."
#endif
#ifdef FNET_CFG_ETH_IP4_DNS  
	#error "FNET_CFG_ETH_IP4_DNS parameter is obsolete, use FNET_CFG_ETH0_IP4_DNS ."
#endif
#ifdef FNET_CFG_ETH  
	#error "FNET_CFG_ETH parameter is obsolete, use FNET_CFG_CPU_ETH0 ."
#endif
#ifdef FNET_CFG_ETH_MAC_ADDR  
	#error "FNET_CFG_ETH_MAC_ADDR parameter is obsolete, use FNET_CFG_CPU_ETH0_MAC_ADDR ."
#endif
#ifdef FNET_CFG_ETH_MTU  
	#error "FNET_CFG_ETH_MTU parameter is obsolete, use FNET_CFG_CPU_ETH0_MTU ."
#endif



/**************************************************************************/ /*!
 * @internal
 ******************************************************************************/
#ifndef FNET_OS
    #define FNET_OS                     (0)
#endif


/*! @} */

#endif

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
* @file fnet_netif.h
*
* @author Andrey Butok
*
* @date Jan-28-2013
*
* @version 0.1.27.0
*
* @brief FNET Network interface API.
*
***************************************************************************/

#ifndef _FNET_NETIF_H_

#define _FNET_NETIF_H_

#include "fnet_config.h"
#include "fnet_ip.h"
#include "fnet_ip6.h"
#include "fnet_eth.h"
#include "fnet_socket.h"

/*! @addtogroup fnet_netif
* The Network Interface API allows an application to control 
* various interface parameters, such as the IP address, the gateway address, 
* the subnet mask, and others.
*/
/*! @{ */

/**************************************************************************/ /*!
 * @brief  Network interface types.
 ******************************************************************************/
typedef enum
{
    FNET_NETIF_TYPE_OTHER,      /**< @brief Unspecified interface. 
                                 */
    FNET_NETIF_TYPE_ETHERNET,   /**< @brief Ethernet interface
                                 */
    FNET_NETIF_TYPE_LOOPBACK    /**< @brief Loopback interface.
                                 */
} fnet_netif_type_t;

/**************************************************************************/ /*!
 * @brief  Network interface statistics, used by the @ref fnet_netif_get_statistics().
 ******************************************************************************/
struct fnet_netif_statistics
{
    unsigned long tx_packet; /**< @brief Tx packet count.
                              */
    unsigned long rx_packet; /**< @brief Rx packet count.
                              */
};

/**************************************************************************/ /*!
 * @brief The maximum length of a network interface name.
 ******************************************************************************/
#define FNET_NETIF_NAMELEN  (8)

/**************************************************************************/ /*!
 * @brief Network interface descriptor.
 ******************************************************************************/
typedef void *fnet_netif_desc_t;


/**************************************************************************/ /*!
 * @brief Possible IPv6 address states.
 * @see fnet_netif_get_ip6_addr(), fnet_netif_ip6_addr_info
 ******************************************************************************/
typedef enum
{
    FNET_NETIF_IP6_ADDR_STATE_NOT_USED = 0,     /**< @brief Not used.*/
    FNET_NETIF_IP6_ADDR_STATE_TENTATIVE = 1,    /**< @brief Tentative address - (RFC4862) an address whose uniqueness on a link is being
                                                 * verified, prior to its assignment to an interface. A tentative
                                                 * address is not considered assigned to an interface in the usual
                                                 * sense. An interface discards received packets addressed to a
                                                 * tentative address, but accepts Neighbor Discovery packets related
                                                 * to Duplicate Address Detection for the tentative address.
                                                 */
    FNET_NETIF_IP6_ADDR_STATE_PREFERRED = 2 	/**< @brief Preferred address - (RFC4862) an address assigned to an interface whose use by
                                                 * upper-layer protocols is unrestricted. Preferred addresses may be
                                                 * used as the source (or destination) address of packets sent from
                                                 * (or to) the interface.
                                                 */
} fnet_netif_ip6_addr_state_t;

/**************************************************************************/ /*!
 * @brief Possible IPv6 address types.
 * @see fnet_netif_get_ip6_addr(), fnet_netif_ip6_addr_info 
 ******************************************************************************/    
typedef enum
{
    FNET_NETIF_IP6_ADDR_TYPE_MANUAL = 0,           /**< @brief The address is set manually.*/
    FNET_NETIF_IP6_ADDR_TYPE_AUTOCONFIGURABLE = 1  /**< @brief The address is set using autoconfiguration. */
} fnet_netif_ip6_addr_type_t;

/**************************************************************************/ /*!
 * @brief Interface IPv6 address information structure.
 * @see fnet_netif_get_ip6_addr()
 ******************************************************************************/
typedef struct fnet_netif_ip6_addr_info
{
    fnet_ip6_addr_t             address;            /**< @brief IPv6 address.*/
    fnet_netif_ip6_addr_state_t state;              /**< @brief Address current state.*/
    fnet_netif_ip6_addr_type_t  type;               /**< @brief How the address was acquired.*/
} fnet_netif_ip6_addr_info_t;

/***************************************************************************/ /*!
 *
 * @brief    Looks for a network interface according to the specified name.
 *
 *
 * @param name       The name string of a network interface.@n Maximum length of
 *                   the interface name is defined by the @ref FNET_NETIF_NAMELEN.
 *
 * @return This function returns:
 *   - Network interface descriptor that matches the @c name parameter.
 *   - @ref FNET_NULL if there is no match.
 *
 * @see fnet_netif_get_by_ip4_addr()
 *
 ******************************************************************************
 *
 * This function scans the global network interface list looking for a network 
 * interface matching the @c name parameter (for example "eth0", "loop").
 *
 ******************************************************************************/
fnet_netif_desc_t fnet_netif_get_by_name( char *name );

/***************************************************************************/ /*!
 *
 * @brief    Looks for a network interface according to its number.
 *
 *
 * @param n       Number of a network interface (from zero).
 *
 * @return This function returns:
 *   - Network interface descriptor that matches the n-th interface.
 *   - @ref FNET_NULL if n-th interface is not available..
 *
 * @see fnet_netif_get_by_name()
 *
 ******************************************************************************
 *
 * This function scans the global network interface list looking for a network 
 * interface matching the @c n parameter.
 *
 ******************************************************************************/
fnet_netif_desc_t fnet_netif_get_by_number( unsigned long n );


/***************************************************************************/ /*!
 *
 * @brief    Looks for a network interface according to the specified IPv4 address.
 *
 *
 * @param addr       The IPv4 address of a network interface.
 *
 * @return This function returns:
 *   - Network interface descriptor that matches the @c addr parameter.
 *   - @ref FNET_NULL if there is no match.
 *
 * @see fnet_netif_get_by_name()
 *
 ******************************************************************************
 *
 * This function scans the global network interface list looking for a network 
 * interface matching the @c addr parameter.
 *
 ******************************************************************************/
fnet_netif_desc_t fnet_netif_get_by_ip4_addr( fnet_ip4_addr_t addr );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves a name of the specified network interface.
 *
 *
 * @param netif          Network interface descriptor.
 *
 * @param name           String buffer that receives a name of the 
 *                       interface @c netif_desc.
 *
 * @param name_size      Size of the @c name buffer. @n Maximum length of
 *                       the interface name is defined by the @ref FNET_NETIF_NAMELEN.
 *
 * @see fnet_netif_get_by_name()
 *
 ******************************************************************************
 *
 * This function retrieves the name of the specified network interface 
 * @c netif_desc and stores it in the @c name (for example "eth0", "loop").
 *
 ******************************************************************************/
void fnet_netif_get_name( fnet_netif_desc_t netif, char *name, unsigned char name_size );

/***************************************************************************/ /*!
 *
 * @brief    Assigns the default network interface.
 *
 *
 * @param netif     Network interface descriptor to be assigned as default.
 *
 * @see fnet_netif_get_default(), FNET_CFG_DEFAULT_IF
 *
 ******************************************************************************
 *
 * This function sets the @c netif as the default network interface.@n
 * @n
 * By default, during the FNET stack initialization, the default network interface is
 * assigned to the Ethernet.
 *
 ******************************************************************************/
void fnet_netif_set_default( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the default network interface.
 *
 *
 * @return   This function returns the descriptor of the default network interface.
 *
 * @see fnet_netif_set_default(), FNET_CFG_DEFAULT_IF
 *
 ******************************************************************************
 *
 * This function returns the descriptor of the default network interface.@n
 * @n
 * By default, during the FNET stack initialization, the default network interface is
 * assigned to the Ethernet.
 *
 ******************************************************************************/
fnet_netif_desc_t fnet_netif_get_default( void );

/***************************************************************************/ /*!
 *
 * @brief    Sets the IPv4 address of the specified network interface.
 *
 *
 * @param netif     Network interface descriptor.
 *
 * @param ipaddr    The IPv4 address of the network interface.
 *
 * @see fnet_netif_get_ip4_addr()
 *
 ******************************************************************************
 *
 * This function sets the IPv4 address of the @c netif interface to the @c ipaddr value.@n
 * Also, it makes a recalculation of the subnet mask of the interface according 
 * to the @c ipaddr. 
 *
 ******************************************************************************/
void fnet_netif_set_ip4_addr( fnet_netif_desc_t netif, fnet_ip4_addr_t ipaddr );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves an IPv4 address of the specified network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns the IPv4 address of the @c netif interface.
 *
 * @see fnet_netif_set_ip4_addr()
 *
 ******************************************************************************
 *
 * This function returns the IPv4 address of the @c netif interface.
 *
 ******************************************************************************/
fnet_ip4_addr_t fnet_netif_get_ip4_addr( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Sets the subnet mask of the specified network interface.
 *
 * @param netif     Network interface descriptor.
 *
 * @param netmask   The subnet mask of the network interface.
 *
 * @see fnet_netif_get_ip4_subnet_mask()
 *
 ******************************************************************************
 *
 * This function sets the subnet mask of the @c netif interface 
 * to the @c netmask value. 
 *
 ******************************************************************************/
void fnet_netif_set_ip4_subnet_mask( fnet_netif_desc_t netif, fnet_ip4_addr_t netmask );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves a subnet mask of the specified network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns the subnet mask of the @c netif 
 *               interface.
 *
 * @see fnet_netif_set_ip4_subnet_mask()
 *
 ******************************************************************************
 *
 * This function returns the subnet mask of the @c netif interface.
 *
 ******************************************************************************/
fnet_ip4_addr_t fnet_netif_get_ip4_subnet_mask( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Sets the gateway IP address of the specified network interface.
 *
 * @param netif     Network interface descriptor.
 *
 * @param gw        The gateway IP address of the network interface.
 *
 * @see fnet_netif_get_ip4_gateway()
 *
 ******************************************************************************
 *
 * This function sets the gateway IP address of the @c netif interface 
 * to the @c gw value. 
 *
 ******************************************************************************/
void fnet_netif_set_ip4_gateway( fnet_netif_desc_t netif, fnet_ip4_addr_t gw );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves a gateway IP address of the specified network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns the gateway IP address of the @c netif 
 *               interface.
 *
 * @see fnet_netif_set_ip4_gateway()
 *
 ******************************************************************************
 *
 * This function returns the gateway IP address of the @c netif interface.
 *
 ******************************************************************************/
fnet_ip4_addr_t fnet_netif_get_ip4_gateway( fnet_netif_desc_t netif );


#if FNET_CFG_DNS || defined(__DOXYGEN__)
/***************************************************************************/ /*!
 *
 * @brief    Sets the DNS server IP address of the specified network interface.
 *
 * @param netif     Network interface descriptor.
 *
 * @param dns       The DNS server IP address of the network interface.
 *
 * @see fnet_netif_get_ip4_dns(), FNET_CFG_DNS
 *
 ******************************************************************************
 *
 * This function sets the DNS server IP address of the @c netif interface 
 * to the @c dns value. @n
 * It is present only if @ref FNET_CFG_DNS is set to 1.
 *
 ******************************************************************************/
void fnet_netif_set_ip4_dns( fnet_netif_desc_t netif, fnet_ip4_addr_t dns );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the DNS server IP address of the specified network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns the DNS server IP address of the @c netif 
 *               interface.
 *
 * @see fnet_netif_set_ip4_dns(), FNET_CFG_DNS
 *
 ******************************************************************************
 *
 * This function returns the subnet mask of the @c netif interface. @n
 * It is present only if @ref FNET_CFG_DNS is set to 1.
 *
 ******************************************************************************/
fnet_ip4_addr_t fnet_netif_get_ip4_dns( fnet_netif_desc_t netif );

#endif /*FNET_CFG_DNS*/


/***************************************************************************/ /*!
 *
 * @brief    Sets the hardware address of the specified network interface.
 *
 * @param netif     Network interface descriptor.
 *
 * @param hw_addr        Buffer containing the hardware address 
 *                      (for the @ref FNET_NETIF_TYPE_ETHERNET interface type,
 *                       it contains the MAC address).
 *
 * @param hw_addr_size   Size of the hardware address in the @c hw_addr (for the @ref 
 *                       FNET_NETIF_TYPE_ETHERNET interface type, it 
 *                       equals @c 6).
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_netif_get_hw_addr()
 *
 ******************************************************************************
 *
 * This function sets the hardware address of the @c netif interface 
 * to the @c hw_addr value.@n
 * For the @ref FNET_NETIF_TYPE_ETHERNET interface type, this hardware address is 
 * the MAC address.
 *
 ******************************************************************************/
int fnet_netif_set_hw_addr( fnet_netif_desc_t netif, unsigned char *hw_addr, unsigned int hw_addr_size );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves a hardware address of the specified network interface.
 *
 * @param netif     Network interface descriptor.
 *
 * @param hw_addr        Buffer that receives a hardware address  
 *                      (for @ref FNET_NETIF_TYPE_ETHERNET interface type,
 *                       it will contain the MAC address).
 *
 * @param hw_addr_size   Size of the hardware address in the @c hw_addr (for the @ref 
 *                       FNET_NETIF_TYPE_ETHERNET interface type, it 
 *                       equals @c 6).
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_netif_set_hw_addr()
 *
 ******************************************************************************
 *
 * This function reads the hardware address of the @c netif interface 
 * and puts it into the @c hw_addr buffer.@n
 * For the @ref FNET_NETIF_TYPE_ETHERNET interface type, this hardware address is 
 * the MAC address.
 *
 ******************************************************************************/
int fnet_netif_get_hw_addr( fnet_netif_desc_t netif, unsigned char *hw_addr, unsigned int hw_addr_size );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the type of the specified network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns the type of the @c netif 
 *               interface.@n
 *               The type is defined by the @ref fnet_netif_type_t.
 *
 ******************************************************************************
 *
 * This function returns the type of the @c netif interface that is defined 
 * by the @ref fnet_netif_type_t.
 *
 ******************************************************************************/
fnet_netif_type_t fnet_netif_type( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Determines, if the IP address parameters were obtained 
 *           automatically.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns:
 *   - @c 0 if the address IP parameters were set manually .
 *   - @c 1 if the address IP parameters were obtained automatically.
 *
 * @see fnet_netif_set_ip4_addr(), fnet_netif_set_ip4_gateway()
 *
 ******************************************************************************
 *
 * This function determines if the IP parameters of the @c netif interface 
 * were obtained automatically by the DHCP client or were set manually by the
 * @ref fnet_netif_set_ip4_addr() and @ref fnet_netif_set_ip4_gateway().
 *
 ******************************************************************************/
int fnet_netif_get_ip4_addr_automatic( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Determines the link status of the network interface.
 *
 * @param netif  Network interface descriptor.
 *
 * @return       This function returns:
 *   - @c 0 if the network link is unconnected.
 *   - @c 1 if the network link is connected.
 *
 ******************************************************************************
 *
 * This function determines if the @c netif interface is marked as connected to 
 * a network or not.
 * The Ethernet network interface gets this status from the PHY. 
 *
 ******************************************************************************/
int fnet_netif_connected( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the network interface statistics.
 *
 * @param netif  Network interface descriptor.
 *
 * @param statistics  Structure that receives the network interface statistics 
 *                    defined by the @ref fnet_netif_statistics structure.
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs or the network interface driver does 
 *          not support this statistics.
 *
 ******************************************************************************
 *
 * This function retrieves the network statistics of the @c netif interface 
 * and puts it into the @c statistics defined by the @ref fnet_netif_statistics 
 * structure.
 *
 ******************************************************************************/
int fnet_netif_get_statistics( fnet_netif_desc_t netif, struct fnet_netif_statistics *statistics );

/**************************************************************************/ /*!
 * @brief Event handler callback function prototype, that is 
 * called when there is an IP address conflict with another system 
 * on the network.
 * 
 * @param netif      Network interface descriptor that has duplicated IP 
 *                   address.
 *
 * @see fnet_socket_rx_handler_init()
 ******************************************************************************/
typedef void(*fnet_netif_dupip_handler_t)( fnet_netif_desc_t netif );

/***************************************************************************/ /*!
 *
 * @brief    Registers the "duplicated IP address" event handler.
 *
 * @param handler    Pointer to the event-handler function defined by 
 *                   @ref fnet_netif_dupip_handler_t.
 *
 ******************************************************************************
 *
 * This function registers the @c handler callback function for 
 * the "duplicated IP address" event. This event occurs when there is 
 * an IP address conflict with another system on the network. @n
 * To stop the event handling, set @c handler parameter to zero value.
 *
 ******************************************************************************/
void fnet_netif_dupip_handler_init (fnet_netif_dupip_handler_t handler);

#if (FNET_CFG_MULTICAST & FNET_CFG_IP4) || defined(__DOXYGEN__)
  
/***************************************************************************/ /*!
 *
 * @brief    Joins the specified network interface to IPv4 multicast group.
 *
 * @param netif             Network interface descriptor.
 *
 * @param multicast_addr    Multicast-group address to be joined by the interface.
 *
 * @see fnet_netif_leave_ip4_multicast()
 *
 ******************************************************************************
 *
 * This function configures the HW interface to receive 
 * particular multicast address.
 * When the network interface picks up a packet which has a destination  
 * that matches any of the joined-multicast addresses, it will pass it to 
 * the upper layers for further processing. @n
 * @note User application should not call this function. @n
 * This function is available only if @ref FNET_CFG_MULTICAST and 
 * @ref FNET_CFG_IP4 are set to @c 1.
 *
 ******************************************************************************/
void fnet_netif_join_ip4_multicast ( fnet_netif_desc_t netif, fnet_ip4_addr_t multicast_addr );

/***************************************************************************/ /*!
 *
 * @brief    Leaves the specified network interface from IPv4 multicast group.
 *
 * @param netif             Network interface descriptor.
 *
 * @param multicast_addr    Multicast-group address to be leaved by the interface.
 *
 * @see fnet_netif_join_ip4_multicast()
 *
 ******************************************************************************
 *
 * This function configures the HW interface to ignore 
 * particular multicast address. @n
 * @note User application should not call this function. @n
 * This function is available only if @ref FNET_CFG_MULTICAST and 
 * @ref FNET_CFG_IP4 are set to @c 1. 
 *
 ******************************************************************************/
void fnet_netif_leave_ip4_multicast ( fnet_netif_desc_t netif, fnet_ip4_addr_t multicast_addr );


#endif /* FNET_CFG_MULTICAST & FNET_CFG_IP4*/



#if FNET_CFG_IP6 || defined(__DOXYGEN__)
  
/***************************************************************************/ /*!
 *
 * @brief    Joins the specified network interface to IPv6 multicast group.
 *
 * @param netif             Network interface descriptor.
 *
 * @param multicast_addr    Multicast-group address to be joined by the interface.
 *
 * @see fnet_netif_leave_ip6_multicast()
 *
 ******************************************************************************
 *
 * This function configures the HW interface to receive 
 * particular multicast address.
 * When the network interface picks up a packet which has a destination  
 * that matches any of the joined-multicast addresses, it will pass it to 
 * the upper layers for further processing. @n
 * @note User application should not call this function. @n
 * This function is available only if @ref FNET_CFG_IP6 is set to @c 1.
 *
 ******************************************************************************/
void fnet_netif_join_ip6_multicast ( fnet_netif_desc_t netif, const fnet_ip6_addr_t *multicast_addr );

/***************************************************************************/ /*!
 *
 * @brief    Leaves the specified network interface from IPv6 multicast group.
 *
 * @param netif             Network interface descriptor.
 *
 * @param multicast_addr    Multicast-group address to be leaved by the interface.
 *
 * @see fnet_netif_join_ip6_multicast()
 *
 ******************************************************************************
 *
 * This function configures the HW interface to ignore 
 * particular multicast address. @n
 * @note User application should not call this function. @n
 * This function is available only if @ref FNET_CFG_IP6 is set to @c 1. 
 *
 ******************************************************************************/
void fnet_netif_leave_ip6_multicast ( fnet_netif_desc_t netif, fnet_ip6_addr_t *multicast_addr );

/***************************************************************************/ /*!
 *
 * @brief    Retrieves an IPv6 address of the specified network interface.
 *
 * @param netif_desc  Network interface descriptor.
 *
 * @param n           Sequence number of IPv6 address to retrieve (from @c 0).
 *
 * @param addr_info   Pointer to address information structure will contain the result.
 *
 * @return This function returns:
 *   - @ref FNET_TRUE if no error occurs and data structure is filled.
 *   - @ref FNET_FALSE in case of error or @c n-th address is not available.
 *
 ******************************************************************************
 *
 * This function is used to retrieve all IPv6 addresses registered with 
 * the given interface.
 *
 ******************************************************************************/
int fnet_netif_get_ip6_addr ( fnet_netif_desc_t netif_desc, unsigned int n, fnet_netif_ip6_addr_info_t *addr_info );

/***************************************************************************/ /*!
 *
 * @brief    Binds the IPv6 address to the specified network interface.
 *
 * @param netif_desc    Network interface descriptor.
 *
 * @param addr        The IPv6 address for the network interface.
 *
 * @param addr_type   The IPv6 address type that defines the way the IPv6 
 *                    address to be assigned to the interface.
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_FALSE in case of error.
 *
 * @see fnet_netif_unbind_ip6_addr()
 *
 ******************************************************************************
 *
 * This function binds the IPv6 address to the @c netif interface.@n
 * The @c addr_type parameter defines the way the IPv6 address is assigned 
 * to the interface:
 *   - @c FNET_NETIF_IP6_ADDR_TYPE_MANUAL:  value of the @c addr parameter 
 *             defines the whole IPv6 address to be bind to the interface.
 *   - @c FNET_NETIF_IP6_ADDR_TYPE_AUTOCONFIGURABLE: value of the @c addr 
 *             parameter defines the first 64bits of the bind IPv6 address. 
 *            The last 64bits of the IPv6 address are overwritten with the 
 *            Interface Identifier. In case of Ethernet interface, 
 *            the Interface Identifier is formed from 48-bit MAC address, 
 *            according to [RFC2464]. 
 *
 ******************************************************************************/
int fnet_netif_bind_ip6_addr(fnet_netif_desc_t netif_desc, fnet_ip6_addr_t *addr, fnet_netif_ip6_addr_type_t addr_type);

/***************************************************************************/ /*!
 *
 * @brief    Unbinds the IPv6 address from the specified network interface.
 *
 * @param netif_desc    Network interface descriptor.
 *
 * @param addr        The IPv6 address to unbind.
 *
 * @return This function returns:
 *   - @ref FNET_OK if successful.
 *   - @ref FNET_FALSE if failed.
 *
 * @see fnet_netif_bind_ip6_addr()
 *
 ******************************************************************************
 *
 * This function unbinds the IPv6 address from the @c netif interface.
 *
 ******************************************************************************/
int fnet_netif_unbind_ip6_addr(fnet_netif_desc_t netif_desc, fnet_ip6_addr_t *addr);

#endif /* FNET_CFG_IP6 */

/***************************************************************************/ /*!
 *
 * @brief    Retrieves an Scope ID of the specified network interface.
 *
 * @param netif_desc  Network interface descriptor.
 *
 * @return This function returns:
 *          - Scope ID value.
 *          - @c 0 if no Scope ID was assigned to the interface. 
 *
 ******************************************************************************
 *
 * This function is used to retrieve Scope ID assigned to the given interface.
 *
 ******************************************************************************/
unsigned long fnet_netif_get_scope_id(fnet_netif_desc_t netif_desc);

/***************************************************************************/ /*!
 *
 * @brief    Looks for a network interface according to the specified Scope ID.
 *
 *
 * @param scope_id       The Scope ID of a network interface.
 *
 * @return This function returns:
 *   - Network interface descriptor that matches the @c scope_id parameter.
 *   - @ref FNET_NULL if there is no match.
 *
 * @see fnet_netif_get_scope_id()
 *
 ******************************************************************************
 *
 * This function scans the global network interface list looking for a network 
 * interface matching the specified Scope ID. 
 *
 ******************************************************************************/
fnet_netif_desc_t fnet_netif_get_by_scope_id(unsigned long scope_id);

/***************************************************************************/ /*!
 *
 * @brief    Looks for a network interface according to the specified socket
 *           address.
 *
 *
 * @param addr       The socket address of a network interface.
 *
 * @return This function returns:
 *   - Network interface descriptor that matches the @c addr parameter.
 *   - @ref FNET_NULL if there is no match.
 *
 * @see fnet_netif_get_by_scope_id()
 *
 ******************************************************************************
 *
 * This function scans the global network interface list looking for a network 
 * interface matching the specified socket address. 
 *
 ******************************************************************************/
fnet_netif_desc_t fnet_netif_get_by_sockaddr( const struct sockaddr *addr );


/*! @} */

#endif /* _FNET_NETIF_H_ */

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
* @file fnet_nd6.h
*
* @author Andrey Butok
*
* @date Feb-14-2013
*
* @version 0.1.13.0
*
* @brief Neighbor Discovery for IP6 API.
*
***************************************************************************/

#ifndef _FNET_ND6_H_

#define _FNET_ND6_H_

#include "fnet_timer_prv.h"
#include "fnet_ip6.h"
#include "fnet_icmp6.h"
#include "fnet_netbuf.h"
#include "fnet_netif_prv.h"

/* Neighbor Cache and Default Router List combined to one list.*/
#define FNET_ND6_NEIGHBOR_CACHE_SIZE         (FNET_CFG_ND6_NEIGHBOR_CACHE_SIZE + FNET_CFG_ND6_ROUTER_LIST_SIZE) 
#define FNET_ND6_PREFIX_LIST_SIZE            (FNET_CFG_ND6_PREFIX_LIST_SIZE + 1) /* One more for link-local prefix.*/
#define FNET_ND6_REDIRECT_TABLE_SIZE         (4) /* TBD config parameter.*/

/**************************************************************
* RFC4861. 10. Protocol Constants.
***************************************************************
*    Host constants:
*        MAX_RTR_SOLICITATION_DELAY 1 second
*        RTR_SOLICITATION_INTERVAL 4 seconds
*        MAX_RTR_SOLICITATIONS 3 transmissions
*    Node constants:
*        MAX_MULTICAST_SOLICIT 3 transmissions
*        MAX_UNICAST_SOLICIT 3 transmissions
*        MAX_ANYCAST_DELAY_TIME 1 second
*        MAX_NEIGHBOR_ADVERTISEMENT 3 transmissions
*        REACHABLE_TIME 30,000 milliseconds
*        RETRANS_TIMER 1,000 milliseconds
*        DELAY_FIRST_PROBE_TIME 5 seconds
*        MIN_RANDOM_FACTOR .5
*        MAX_RANDOM_FACTOR 1.5
***************************************************************/

/* If a host sends MAX_RTR_SOLICITATIONS solicitations, and receives no
 * Router Advertisements after having waited MAX_RTR_SOLICITATION_DELAY
 * seconds after sending the last solicitation, the host concludes that
 * there are no routers on the link for the purpose of [ADDRCONF].
 * However, the host continues to receive and process Router
 * Advertisements messages in the event that routers appear on the link.
 *
 * A host SHOULD transmit up to MAX_RTR_SOLICITATIONS Router
 * Solicitation messages, each separated by at least
 * RTR_SOLICITATION_INTERVAL seconds.
 */
#define FNET_ND6_MAX_RTR_SOLICITATIONS       (3)        /* transmissions */
#define FNET_ND6_MAX_RTR_SOLICITATION_DELAY  (1000)     /* ms */
#define FNET_ND6_RTR_SOLICITATION_INTERVAL   (4000)     /* ms */

/* If no Neighbor Advertisement is received after MAX_MULTICAST_SOLICIT
* solicitations, address resolution has failed. The sender MUST return
* ICMP destination unreachable indications with code 3 (Address
* Unreachable) for each packet queued awaiting address resolution.
*/
#define FNET_ND6_MAX_MULTICAST_SOLICIT       (3)        /* transmissions */

/*
 * Default value of the time between retransmissions of Neighbor
 * Solicitation messages to a neighbor when
 * resolving the address or when probing the
 * reachability of a neighbor. Also used during Duplicate
 * Address Detection (RFC4862).
 */
#define FNET_ND6_RETRANS_TIMER               (1000)     /* ms */

/*
 * Default value of the time a neighbor is considered reachable after
 * receiving a reachability confirmation.
 *
 * This value should be a uniformly distributed
 * random value between MIN_RANDOM_FACTOR and
 * MAX_RANDOM_FACTOR times BaseReachableTime
 * milliseconds. A new random value should be
 * calculated when BaseReachableTime changes (due to
 * Router Advertisements) or at least every few
 * hours even if
 */
#define FNET_ND6_REACHABLE_TIME              (30000)    /* ms */ 

/*
 * If no reachability confirmation is received
 * within DELAY_FIRST_PROBE_TIME seconds of entering the
 * DELAY state, send a Neighbor Solicitation and change
 * the state to PROBE.
 */
#define FNET_ND6_DELAY_FIRST_PROBE_TIME      (5000)     /*ms*/

/*
 * If no response is
 * received after waiting RetransTimer milliseconds after sending the
 * MAX_UNICAST_SOLICIT solicitations, retransmissions cease and the
 * entry SHOULD be deleted.
 */
#define FNET_ND6_MAX_UNICAST_SOLICIT         (3)        /*times*/

/*
 * ND6 general timer resolution.
 */
#define FNET_ND6_TIMER_PERIOD                (100)      /* ms */





#define FNET_ND6_PREFIX_LENGTH_DEFAULT       (64)            /* Default prefix length, in bits.*/
#define FNET_ND6_PREFIX_LIFETIME_INFINITE    (0xFFFFFFFF)    /* A lifetime value of all one bits (0xffffffff) represents infinity. */

/**********************************************************************
 * Link-layer address.
 * For example, Ethernet interafce uses the address with size set to 6.
 ***********************************************************************/
typedef unsigned char fnet_nd6_ll_addr_t[16]; 

/* Copying Link-layer address.*/
#define FNET_ND6_LL_ADDR_COPY(from_addr, to_addr, ll_size)   \
                                    (fnet_memcpy(&to_addr[0], &from_addr[0], ll_size))

/* Link-layer address Equality. */
#define FNET_ND6_LL_ADDR_ARE_EQUAL(a, b, size)               \
        (fnet_memcmp(&a[0], &b[0], (int)size) == 0)
                                    
/***********************************************************************
* Prefix state.
***********************************************************************/
typedef enum fnet_nd6_prefix_state
{
    FNET_ND6_PREFIX_STATE_NOTUSED = 0,      /* The entry is not used - free.*/
    FNET_ND6_PREFIX_STATE_USED = 1          /* The entry is used.*/
} fnet_nd6_prefix_state_t;

/***********************************************************************
* Prefix List entry, based on RFC4861.
* Prefix List entries are created from information received in Router
* Advertisements.
***********************************************************************/
typedef struct fnet_nd6_prefix_entry
{
 
    fnet_ip6_addr_t         prefix;         /* Prefix of an IP address. */
    unsigned long           prefix_length;  /* Prefix length (in bits). The number of leading bits
                                             * in the Prefix that are valid. */
    fnet_nd6_prefix_state_t state;          /* Prefix state.*/                                 
    unsigned long           lifetime;       /* Valid Lifetime
                                             * 32-bit unsigned integer. The length of time in
                                             * seconds (relative to the time the packet is sent)
                                             * that the prefix is valid for the purpose of on-link
                                             * determination. A value of all one bits
                                             * (0xffffffff) represents infinity. The Valid
                                             * Lifetime is also used by [ADDRCONF].*/
    unsigned long           creation_time;  /* Time of entry creation, in seconds.*/                                     
} fnet_nd6_prefix_entry_t;

/**************************************************************
* Neighbor’s reachability states, based on RFC4861.
**************************************************************/
typedef enum fnet_nd6_neighbor_state
{
    FNET_ND6_NEIGHBOR_STATE_NOTUSED = 0,    /* The entry is not used - free.*/
    FNET_ND6_NEIGHBOR_STATE_INCOMPLETE = 1, /* Address resolution is in progress and the link-layer
                                             * address of the neighbor has not yet been determined.*/
    FNET_ND6_NEIGHBOR_STATE_REACHABLE = 2,  /* Roughly speaking, the neighbor is known to have been
                                             * reachable recently (within tens of seconds ago).*/
    FNET_ND6_NEIGHBOR_STATE_STALE = 3,      /* The neighbor is no longer known to be reachable but
                                             * until traffic is sent to the neighbor, no attempt
                                             * should be made to verify its reachability.*/                                         
    FNET_ND6_NEIGHBOR_STATE_DELAY = 4,      /* The neighbor is no longer known to be reachable, and
                                             * traffic has recently been sent to the neighbor.
                                             * Rather than probe the neighbor immediately, however,
                                             * delay sending probes for a short while in order to
                                             * give upper-layer protocols a chance to provide
                                             * reachability confirmation.*/                                          
    FNET_ND6_NEIGHBOR_STATE_PROBE = 5       /* The neighbor is no longer known to be reachable, and
                                             * unicast Neighbor Solicitation probes are being sent to
                                             * verify reachability.*/                                          
} fnet_nd6_neighbor_state_t;

/***********************************************************************
* Neighbor Cache entry, based on RFC4861.
***********************************************************************/
typedef struct fnet_nd6_neighbor_entry
{
 
    fnet_ip6_addr_t             ip_addr;        /* Neighbor’s on-link unicast IP address. */
    fnet_nd6_ll_addr_t          ll_addr;        /* Its link-layer address. Actual size is defiined by fnet_netif_api_t->hw_addr_size. */
    fnet_nd6_neighbor_state_t   state;          /* Neighbor’s reachability state.*/
    unsigned long               state_time;     /* Time of last state event.*/
    fnet_netbuf_t               *waiting_netbuf;/* Pointer to any queued packetwaiting for address resolution to complete.*/
                                                /* RFC 4861 7.2.2: While waiting for address resolution to complete, the sender MUST,
                                                 * for each neighbor, retain a small queue of packets waiting for
                                                 * address resolution to complete. The queue MUST hold at least one
                                                 * packet, and MAY contain more.
                                                 * When a queue  overflows, the new arrival SHOULD replace the oldest entry.*/    
    int                         solicitation_send_counter;  /* Counter - how many soicitations where sent.*/
    fnet_ip6_addr_t             solicitation_src_ip_addr;   /* IP address used during AR solicitation messages. */    
    unsigned long               creation_time;              /* Time of entry creation, in seconds.*/    
    /* Default Router list entry info.*/
    int                         is_router;          /* A flag indicating whether the neighbor is a router or a host.*/
    unsigned long               router_lifetime;    /* The lifetime associated
                                                    * with the default router in units of seconds. The
                                                    * field can contain values up to 65535 and receivers
                                                    * should handle any value, while the sending rules in
                                                    * Section 6 limit the lifetime to 9000 seconds. A
                                                    * Lifetime of 0 indicates that the router is not a
                                                    * default router and SHOULD NOT appear on the default router list.
                                                    * It is used only if "is_router" is 1.*/    
} fnet_nd6_neighbor_entry_t;

/***********************************************************************
* Redirect Table entry.
***********************************************************************/
typedef struct fnet_nd6_redirect_entry
{
    fnet_ip6_addr_t     destination_addr;   /* Destination Address. The IP address of the destination that is
                                             * redirected to the target. */
    fnet_ip6_addr_t     target_addr;        /* Target Address. An IP address that is a better first hop to use for
                                             * the ICMP Destination Address. When the target is
                                             * the actual endpoint of communication, i.e., the
                                             * destination is a neighbor, the Target Address field
                                             * MUST contain the same value as the ICMP Destination
                                             * Address field. Otherwise, the target is a better
                                             * first-hop router and the Target Address MUST be the
                                             * router’s link-local address so that hosts can
                                             * uniquely identify routers. */
    unsigned long       creation_time;      /* Time of entry creation.*/
} fnet_nd6_redirect_entry_t;


/**********************************************************************
* Neighbor Solicitation Message Format (RFC 4861)
***********************************************************************
* Nodes send Neighbor Solicitations to request the link-layer address
* of a target node while also providing their own link-layer address to
* the target.
*
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                           Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                       Target Address                          +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct fnet_nd6_ns_header
{
    fnet_icmp6_header_t icmp6_header    FNET_COMP_PACKED;               
    unsigned char       _reserved[4]    FNET_COMP_PACKED;
    fnet_ip6_addr_t     target_addr     FNET_COMP_PACKED;
} fnet_nd6_ns_header_t;
FNET_COMP_PACKED_END

/**********************************************************************
* Neighbor Advertisement Message Format (RFC 4861)
***********************************************************************
* A node sends Neighbor Advertisements in response to Neighbor
* Solicitations and sends unsolicited Neighbor Advertisements in order
* to (unreliably) propagate new information quickly.
*
*	 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |R|S|O|                     Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                       Target Address                          +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct fnet_nd6_na_header
{
    fnet_icmp6_header_t icmp6_header    FNET_COMP_PACKED; 
    unsigned char       flag            FNET_COMP_PACKED;
    unsigned char       _reserved[3]    FNET_COMP_PACKED;
    fnet_ip6_addr_t     target_addr     FNET_COMP_PACKED;    
} fnet_nd6_na_header_t;
FNET_COMP_PACKED_END

/* NA flags.*/
#define FNET_ND6_NA_FLAG_ROUTER      (0x80) /* Router flag. When set, the R-bit indicates that
                                             * the sender is a router. The R-bit is used by
                                             * Neighbor Unreachability Detection to detect a
                                             * router that changes to a host.*/
#define FNET_ND6_NA_FLAG_SOLICITED   (0x40) /* Solicited flag. When set, the S-bit indicates that
                                             * the advertisement was sent in response to a
                                             * Neighbor Solicitation from the Destination address.
                                             * The S-bit is used as a reachability confirmation
                                             * for Neighbor Unreachability Detection. It MUST NOT
                                             * be set in multicast advertisements or in
                                             * unsolicited unicast advertisements.*/
#define FNET_ND6_NA_FLAG_OVERRIDE    (0x20) /* Override flag. When set, the O-bit indicates that
                                             * the advertisement should override an existing cache
                                             * entry and update the cached link-layer address.
                                             * When it is not set the advertisement will not
                                             * update a cached link-layer address though it will
                                             * update an existing Neighbor Cache entry for which
                                             * no link-layer address is known. It SHOULD NOT be
                                             * set in solicited advertisements for anycast
                                             * addresses and in solicited proxy advertisements.
                                             * It SHOULD be set in other solicited advertisements
                                             * and in unsolicited advertisements.*/

/**********************************************************************
* Redirect Message Format (RFC 4861)
***********************************************************************
* Routers send Redirect packets to inform a host of a better first-hop
* node on the path to a destination.
*
*	 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                           Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                       Target Address                          +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +                    Destination Address                        +
*    |                                                               |
*    +                                                               +
*    |                                                               |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct fnet_nd6_rd_header
{
    fnet_icmp6_header_t icmp6_header        FNET_COMP_PACKED; 
    unsigned char       _reserved[4]        FNET_COMP_PACKED;
    fnet_ip6_addr_t     target_addr         FNET_COMP_PACKED;  
    fnet_ip6_addr_t     destination_addr    FNET_COMP_PACKED;    
} fnet_nd6_rd_header_t;
FNET_COMP_PACKED_END

 
/**********************************************************************
* Router Solicitation Message Format
***********************************************************************
* Hosts send Router Solicitations in order to prompt routers to
* generate Router Advertisements quickly.
*
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                           Reserved                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct fnet_nd6_rs_header
{
    fnet_icmp6_header_t icmp6_header    FNET_COMP_PACKED;  
    unsigned char       _reserved[4]    FNET_COMP_PACKED;
} fnet_nd6_rs_header_t;
FNET_COMP_PACKED_END

/**********************************************************************
* Router Advertisement Message Format
***********************************************************************
* Routers send out Router Advertisement messages periodically, or in
* response to Router Solicitations.
*
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |     Type      |     Code      |          Checksum             |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    | Cur Hop Limit |M|O|  Reserved |       Router Lifetime         |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                      Reachable Time                           |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |                      Retrans Timer                            |
*    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*    |   Options ...
*    +-+-+-+-+-+-+-+-+-+-+-+-
***********************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct fnet_nd6_ra_header
{
    fnet_icmp6_header_t icmp6_header    FNET_COMP_PACKED;   /* ICMPv6 header.*/
    unsigned char       cur_hop_limit   FNET_COMP_PACKED;   /* 8-bit unsigned integer. The default value that
                                                             * should be placed in the Hop Count field of the IP
                                                             * header for outgoing IP packets. A value of zero
                                                             * means unspecified (by this router). */
    unsigned char       flag            FNET_COMP_PACKED;   /* ND6_RS_FLAG_M and/or ND6_RS_FLAG_O flags.*/
    unsigned short      router_lifetime FNET_COMP_PACKED;   /* 16-bit unsigned integer. The lifetime associated
                                                             * with the default router in units of seconds. The
                                                             * field can contain values up to 65535 and receivers
                                                             * should handle any value, while the sending rules in
                                                             * Section 6 limit the lifetime to 9000 seconds. A
                                                             * Lifetime of 0 indicates that the router is not a
                                                             * default router and SHOULD NOT appear on the default
                                                             * router list. The Router Lifetime applies only to
                                                             * the router’s usefulness as a default router; it
                                                             * does not apply to information contained in other
                                                             * message fields or options. Options that need time
                                                             * limits for their information include their own
                                                             * lifetime fields.*/
    unsigned long       reachable_time  FNET_COMP_PACKED;   /* 32-bit unsigned integer. The time, in
                                                             * milliseconds, that a node assumes a neighbor is
                                                             * reachable after having received a reachability
                                                             * confirmation. Used by the Neighbor Unreachability
                                                             * Detection algorithm (see Section 7.3). A value of
                                                             * zero means unspecified (by this router). */                                       
    unsigned long       retrans_timer   FNET_COMP_PACKED;   /* 32-bit unsigned integer. The time, in
                                                             * milliseconds, between retransmitted Neighbor
                                                             * Solicitation messages. Used by address resolution
                                                             * and the Neighbor Unreachability Detection algorithm
                                                             * (see Sections 7.2 and 7.3). A value of zero means
                                                             * unspecified (by this router).*/   
   
} fnet_nd6_ra_header_t;
FNET_COMP_PACKED_END

/* RA flags */
#define FNET_ND6_RA_FLAG_M   (0x80) /* 1-bit "Managed address configuration" flag. When
                                     * set, it indicates that addresses are available via
                                     * Dynamic Host Configuration Protocol [DHCPv6].
                                     * If the M flag is set, the O flag is redundant and
                                     * can be ignored because DHCPv6 will return all
                                     * available configuration information.*/
#define FNET_ND6_RA_FLAG_O   (0x40) /* 1-bit "Other configuration" flag. When set, it
                                     * indicates that other configuration information is
                                     * available via DHCPv6. Examples of such information
                                     * are DNS-related information or information on other
                                     * servers within the network.*/
                                    /* Note: If neither M nor O flags are set, this indicates that no
                                     * information is available via DHCPv6.*/

/* Hop Limit when sending/receiving Neighbor Discovery messages. */
#define FNET_ND6_HOP_LIMIT                  (255)

/* ND option types (RFC4861). */
#define FNET_ND6_OPTION_SOURCE_LLA          (1) /* Source Link-layer Address.*/
#define FNET_ND6_OPTION_TARGET_LLA          (2) /* Target Link-layer Address.*/
#define FNET_ND6_OPTION_PREFIX              (3) /* Prefix Information.*/
#define FNET_ND6_OPTION_REDIRECTED_HEADER   (4) /* Redirected Header.*/
#define FNET_ND6_OPTION_MTU                 (5) /* MTU */

/***********************************************************************
 * ND option header
 ***********************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct fnet_nd6_option_header
{
    unsigned char type      FNET_COMP_PACKED;   /* Identifier of the type of option.*/
    unsigned char length    FNET_COMP_PACKED;   /* The length of the option
                                                 * (including the type and length fields) in units of
                                                 * 8 octets.  The value 0 is invalid.  Nodes MUST
                                                 * silently discard an ND packet that contains an
                                                 * option with length zero.*/
} fnet_nd6_option_header_t;
FNET_COMP_PACKED_END

/***********************************************************************
 * Source/Target Link-layer Address option header:
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Length    |       Link-Layer Address ...
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 ***********************************************************************/
FNET_COMP_PACKED_BEGIN 
typedef struct fnet_nd6_option_lla_header
{
    fnet_nd6_option_header_t    option_header   FNET_COMP_PACKED;   /* Option general header.*/
    unsigned char               addr[6]         FNET_COMP_PACKED;   /* The length of the option. Can be more or less than 6.*/
}fnet_nd6_option_lla_header_t;
FNET_COMP_PACKED_END

/***********************************************************************
 * MTU option header:
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Length    |           Reserved            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          MTU                                  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
FNET_COMP_PACKED_BEGIN  
typedef struct fnet_nd6_option_mtu_header
{
    fnet_nd6_option_header_t    option_header   FNET_COMP_PACKED;   /* Option general header.*/
    unsigned char               _reserved[2]    FNET_COMP_PACKED;
    unsigned long               mtu             FNET_COMP_PACKED;   /* The recommended MTU for the link.*/
}fnet_nd6_option_mtu_header_t;
FNET_COMP_PACKED_END


/***********************************************************************
 * Prefix Information option header:
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Length    | Prefix Length |L|A| Reserved1 |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         Valid Lifetime                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       Preferred Lifetime                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                           Reserved2                           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  +                                                               +
 *  |                                                               |
 *  +                             Prefix                            +
 *  |                                                               |
 *  +                                                               +
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 ***********************************************************************/
FNET_COMP_PACKED_BEGIN  
typedef struct fnet_nd6_option_prefix_header
{
    fnet_nd6_option_header_t    option_header   FNET_COMP_PACKED;   /* Option general header.*/
    unsigned char               prefix_length   FNET_COMP_PACKED;   /* The number of leading bits
                                                                     * in the Prefix that are valid. The value ranges
                                                                     * from 0 to 128. The prefix length field provides
                                                                     * necessary information for on-link determination
                                                                     * (when combined with the L flag in the prefix
                                                                     * information option). It also assists with address
                                                                     * autoconfiguration as specified in [ADDRCONF], for
                                                                     * which there may be more restrictions on the prefix
                                                                     * length.*/
    unsigned char               flag            FNET_COMP_PACKED;   /* ND6_OPTION_FLAG_L and/or ND6_OPTION_FLAG_O flags.*/
    unsigned long               valid_lifetime  FNET_COMP_PACKED;   /* The length of time in
                                                                     * seconds (relative to the time the packet is sent)
                                                                     * that the prefix is valid for the purpose of on-link
                                                                     * determination. A value of all one bits
                                                                     * (0xffffffff) represents infinity. The Valid
                                                                     * Lifetime is also used by [ADDRCONF].*/
    unsigned long               prefered_lifetime FNET_COMP_PACKED; /* The length of time in
                                                                     * seconds (relative to the time the packet is sent)
                                                                     * that addresses generated from the prefix via
                                                                     * stateless address autoconfiguration remain
                                                                     * preferred [ADDRCONF]. A value of all one bits
                                                                     * (0xffffffff) represents infinity. See [ADDRCONF].
                                                                     * Note that the value of this field MUST NOT exceed
                                                                     * the Valid Lifetime field to avoid preferring
                                                                     * addresses that are no longer valid.*/  
    unsigned long               _reserved       FNET_COMP_PACKED;        
    fnet_ip6_addr_t             prefix          FNET_COMP_PACKED;   /* An IP address or a prefix of an IP address. The
                                                                     * Prefix Length field contains the number of valid
                                                                     * leading bits in the prefix. The bits in the prefix
                                                                     * after the prefix length are reserved and MUST be
                                                                     * initialized to zero by the sender and ignored by
                                                                     * the receiver. A router SHOULD NOT send a prefix
                                                                     * option for the link-local prefix and a host SHOULD
                                                                     * ignore such a prefix option.*/                                    
                                                                
}fnet_nd6_option_prefix_header_t;
FNET_COMP_PACKED_END

#define FNET_ND6_OPTION_FLAG_L  (0x80)  /* 1-bit on-link flag. When set, indicates that this
                                         * prefix can be used for on-link determination. When
                                         * not set the advertisement makes no statement about
                                         * on-link or off-link properties of the prefix. In
                                         * other words, if the L flag is not set a host MUST
                                         * NOT conclude that an address derived from the
                                         * prefix is off-link. That is, it MUST NOT update a
                                         * previous indication that the address is on-link.*/    
#define FNET_ND6_OPTION_FLAG_A  (0x40)  /* 1-bit autonomous address-configuration flag. When
                                         * set indicates that this prefix can be used for
                                         * stateless address configuration as specified in
                                         * [ADDRCONF].*/  


/***********************************************************************
* Neighbor Descovery Configuration
***********************************************************************/
typedef struct fnet_nd6_if
{
    /*************************************************************
    * Neighbor Cache.
    * RFC4861 5.1: A set of entries about individual neighbors to 
    * which traffic has been sent recently. 
    **************************************************************/
    /*************************************************************
    * Combined with Default Router List.
    * RFC4861 5.1: A list of routers to which packets may be sent.. 
    **************************************************************/    
    fnet_nd6_neighbor_entry_t  neighbor_cache[FNET_ND6_NEIGHBOR_CACHE_SIZE];

    /*************************************************************
    * Prefix List.
    * RFC4861 5.1: A list of the prefixes that define a set of
    * addresses that are on-link. 
    **************************************************************/
    fnet_nd6_prefix_entry_t     prefix_list[FNET_ND6_PREFIX_LIST_SIZE]; 
    
    /* Redirect Table. Used only when target address != destination address. */
    fnet_nd6_redirect_entry_t   redirect_table[FNET_ND6_REDIRECT_TABLE_SIZE];  
    
    fnet_timer_desc_t           timer;                  /* General ND timer.*/
    
    /* Router Discovery variables.*/
    unsigned long               rd_transmit_counter;    /* Counter used by RD. Equals to the number 
                                                         * of RS transmits till RD is finished.*/                                                    
    unsigned long               rd_time;                /* Time of last RS transmit.*/    
    
    /* Interface variables */  
    unsigned long               mtu;                    /* The recommended MTU for the link.
                                                         * Updated by RA messages.*/
    unsigned char               cur_hop_limit;          /* The default value that
                                                         * should be placed in the Hop Count field of the IP
                                                         * header for outgoing IP packets.*/
    unsigned long               reachable_time;         /* The time, in milliseconds,
                                                         * that a node assumes a neighbor is
                                                         * reachable after having received a reachability
                                                         * confirmation. Used by the Neighbor Unreachability
                                                         * Detection algorithm.*/ 
    unsigned long               retrans_timer;          /* The time, in milliseconds,
                                                         * between retransmitted Neighbor
                                                         * Solicitation messages. Used by address resolution
                                                         * and the Neighbor Unreachability Detection algorithm
                                                         * (see Sections 7.2 and 7.3).*/  
    int                         ip6_disabled;           /* IP operation on the interface is disabled.*/
} fnet_nd6_if_t;

/* Forward declaration.*/
struct fnet_netif;
struct fnet_netif_ip6_addr;
/***********************************************************************
* Function Prototypes
***********************************************************************/
int fnet_nd6_init (struct fnet_netif *netif, fnet_nd6_if_t *nd6_if_ptr);
void fnet_nd6_release (struct fnet_netif *netif);
fnet_nd6_prefix_entry_t *fnet_nd6_prefix_list_add(struct fnet_netif *netif, const fnet_ip6_addr_t *prefix, unsigned long prefix_length, unsigned long lifetime);
void fnet_nd6_prefix_list_del(fnet_nd6_prefix_entry_t *prefix_entry);
fnet_nd6_prefix_entry_t *fnet_nd6_prefix_list_get(struct fnet_netif *netif, fnet_ip6_addr_t *prefix);
int fnet_nd6_addr_is_onlink(struct fnet_netif *netif, fnet_ip6_addr_t *addr);
fnet_nd6_neighbor_entry_t *fnet_nd6_neighbor_cache_get(struct fnet_netif *netif, fnet_ip6_addr_t *ip_addr);
void fnet_nd6_neighbor_cache_del(struct fnet_netif *netif, fnet_nd6_neighbor_entry_t *neighbor_entry);
fnet_nd6_neighbor_entry_t *fnet_nd6_neighbor_cache_add(struct fnet_netif *netif, fnet_ip6_addr_t *ip_addr, fnet_nd6_ll_addr_t ll_addr, fnet_nd6_neighbor_state_t state);
void fnet_nd6_neighbor_enqueue_waiting_netbuf(fnet_nd6_neighbor_entry_t *neighbor_entry, fnet_netbuf_t *waiting_netbuf);
void fnet_nd6_neighbor_send_waiting_netbuf(struct fnet_netif *netif, fnet_nd6_neighbor_entry_t *neighbor_entry);
void fnet_nd6_router_list_add( fnet_nd6_neighbor_entry_t *neighbor_entry, unsigned long lifetime );
void fnet_nd6_router_list_del( fnet_nd6_neighbor_entry_t *neighbor_entry );
fnet_nd6_neighbor_entry_t *fnet_nd6_default_router_get(struct fnet_netif *netif);
void fnet_nd6_neighbor_solicitation_send(struct fnet_netif *netif, fnet_ip6_addr_t *ipsrc /* NULL for, DAD */, fnet_ip6_addr_t *ipdest /*set for NUD,  NULL for DAD & AR */, fnet_ip6_addr_t *target_addr);
void fnet_nd6_neighbor_solicitation_receive(struct fnet_netif *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb);
void fnet_nd6_neighbor_advertisement_send(struct fnet_netif *netif, fnet_ip6_addr_t *ipsrc, fnet_ip6_addr_t *ipdest, unsigned char na_flags);
void fnet_nd6_neighbor_advertisement_receive(struct fnet_netif *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb);
void fnet_nd6_router_solicitation_send(struct fnet_netif *netif);
void fnet_nd6_router_advertisement_receive(struct fnet_netif *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb);
void fnet_nd6_redirect_receive(struct fnet_netif *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb);
void fnet_nd6_redirect_addr(struct fnet_netif *if_ptr, fnet_ip6_addr_t **destination_addr_p);
void fnet_nd6_dad_start(struct fnet_netif *netif , struct fnet_netif_ip6_addr *addr_info);
void fnet_nd6_rd_start(struct fnet_netif *netif);
void fnet_nd6_debug_print_prefix_list(struct fnet_netif *netif );

#endif /* _FNET_ND6_H_ */

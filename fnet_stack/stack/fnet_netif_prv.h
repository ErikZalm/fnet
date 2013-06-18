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
* @file fnet_netif_prv.h
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.34.0
*
* @brief Private. FNET Network interface API.
*
***************************************************************************/

#ifndef _FNET_NETIF_PRV_H_

#define _FNET_NETIF_PRV_H_

#include "fnet_config.h"
#include "fnet_netbuf.h"
#include "fnet_netif.h"
#include "fnet_eth.h"
#include "fnet_nd6.h"



/**************************************************************************/ /*!
 * @internal
 * @brief    Netif features.
 ******************************************************************************/
typedef enum
{
    FNET_NETIF_FEATURE_NONE                     = 0x00, /* No special feature.*/
    FNET_NETIF_FEATURE_HW_TX_IP_CHECKSUM        = 0x01, /* If an IP frame is transmitted, the checksum is inserted automatically. The IP header checksum field
                                                         * must be cleared. If a non-IP frame is transmitted the frame is not modified.*/
    FNET_NETIF_FEATURE_HW_TX_PROTOCOL_CHECKSUM  = 0x02, /* If an IP frame with a known protocol is transmitted (UDP,TCP,ICMP), the checksum is inserted automatically into the
                                                         * frame. The checksum field must be cleared. The other frames are not modified.*/
    FNET_NETIF_FEATURE_HW_RX_IP_CHECKSUM        = 0x04, /* If an IPv4 frame is received with a mismatching header checksum, 
                                                         * the frame is discarded.*/
    FNET_NETIF_FEATURE_HW_RX_PROTOCOL_CHECKSUM  = 0x08  /* If a TCP/IP, UDP/IP, or ICMP/IP frame is received that has a wrong TCP, UDP, or ICMP checksum,
                                                         * the frame is discarded.*/
} fnet_netif_feature_t;


/**************************************************************************/ /*!
 * Interface IPv4 address structure.
 ******************************************************************************/
typedef struct
{
    fnet_ip4_addr_t address;            /**< The IP address.*/
    fnet_ip4_addr_t net;                /**< Network address.*/
    fnet_ip4_addr_t netmask;            /**< Network mask.*/
    fnet_ip4_addr_t subnet;             /**< Network and subnet address.*/
    fnet_ip4_addr_t subnetmask;         /**< Network and subnet mask.*/
    fnet_ip4_addr_t netbroadcast;       /**< Network broadcast address.*/
    fnet_ip4_addr_t subnetbroadcast;    /**< Subnet broadcast address.*/
    fnet_ip4_addr_t gateway;            /**< Gateway.*/
#if FNET_CFG_DNS    
    fnet_ip4_addr_t dns;                /**< DNS address.*/    
#endif    
    int is_automatic;                   /**< 0 if it's set statically/manually.*/
                                        /**< 1 if it's obtained automatically (by DHCP).*/
} fnet_netif_ip4_addr_t;

/* Maxinmum number of IPv6 addresses per interface.*/
#define FNET_NETIF_IP6_ADDR_MAX                 FNET_CFG_NETIF_IP6_ADDR_MAX   
/* A lifetime value of all one bits (0xffffffff) represents infinity. */
#define FNET_NETIF_IP6_ADDR_LIFETIME_INFINITE   FNET_ND6_PREFIX_LIFETIME_INFINITE   



/*********************************************************************
 * Interface IPv6 address structure.
 *********************************************************************/
typedef struct fnet_netif_ip6_addr
{
    fnet_ip6_addr_t             address;                    /* IPv6 address.*/
    fnet_netif_ip6_addr_state_t state;                      /* Address current state.*/
    fnet_netif_ip6_addr_type_t  type;                       /* How the address was acquired.*/
    fnet_ip6_addr_t             solicited_multicast_addr;   /* Solicited-node multicast */ 
                                                    
    unsigned long               creation_time;          /* Time of entry creation (in seconds).*/    
    unsigned long               lifetime;               /* Address lifetime (in seconds). 0xFFFFFFFF = Infinite Lifetime
                                                         * RFC4862. A link-local address has an infinite preferred and valid lifetime; it
                                                         * is never timed out.*/
    unsigned long               prefix_length;          /* Prefix length (in bits). The number of leading bits
                                                         * in the Prefix that are valid. */                                        
    unsigned long               dad_transmit_counter;   /* Counter used by DAD. Equals to the number 
                                                         * of NS transmits till DAD is finished.*/                                                    
    unsigned long               state_time;             /* Time of last state event.*/  
} fnet_netif_ip6_addr_t;


struct fnet_netif; /* Forward declaration.*/
/**************************************************************************/ /*!
 * @internal
 * @brief    Network-interface general API structure.
 ******************************************************************************/
typedef struct fnet_netif_api
{
    fnet_netif_type_t   type;                                       /* Data-link type. */
    unsigned int        hw_addr_size;
    int                 (*init)( struct fnet_netif * );             /* Initialization function.*/
    void                (*release)( struct fnet_netif * );          /* Shutdown function.*/
#if FNET_CFG_IP4    
    void                (*output_ip4)(struct fnet_netif *netif, fnet_ip4_addr_t dest_ip_addr, fnet_netbuf_t* nb); /* Transmit function.*/
#endif    
    void                (*set_addr_notify)( struct fnet_netif * );  /* Address change notification function.*/
    void                (*drain)( struct fnet_netif * );            /* Drain function.*/
    int                 (*get_hw_addr)( struct fnet_netif *netif, unsigned char *hw_addr);
    int                 (*set_hw_addr)( struct fnet_netif *netif, unsigned char *hw_addr);
    int                 (*is_connected)( struct fnet_netif *netif );
    int                 (*get_statistics)( struct fnet_netif *netif, struct fnet_netif_statistics *statistics );
#if FNET_CFG_MULTICAST  
    #if FNET_CFG_IP4
	    void (*multicast_join_ip4)( struct fnet_netif *netif, fnet_ip4_addr_t multicast_addr  );
    	void (*multicast_leave_ip4)( struct fnet_netif *netif, fnet_ip4_addr_t multicast_addr  );
	#endif
	#if FNET_CFG_IP6
	    void (*multicast_join_ip6)( struct fnet_netif *netif, const fnet_ip6_addr_t *multicast_addr  );
	    void (*multicast_leave_ip6)( struct fnet_netif *netif, fnet_ip6_addr_t *multicast_addr  );
    #endif	    
#endif	
#if FNET_CFG_IP6
    void                (*output_ip6)(struct fnet_netif *netif, fnet_ip6_addr_t *src_ip_addr,  fnet_ip6_addr_t *dest_ip_addr, fnet_netbuf_t* nb); /* IPv6 Transmit function.*/
#endif
} fnet_netif_api_t;



/* Forward declaration.*/
struct fnet_nd6_if;

/**************************************************************************/ /*!
 * @internal
 * @brief    Network interface structure.
 ******************************************************************************/
typedef struct fnet_netif
{
    struct fnet_netif       *next;                              /* Pointer to the next net_if structure. */
    struct fnet_netif       *prev;                              /* Pointer to the previous net_if structure. */
    char                    name[FNET_NETIF_NAMELEN];           /* Network interface name (e.g. "eth0", "loop"). */
    unsigned long           mtu;                                /* Maximum transmission unit. */
    void                    *if_ptr;                            /* Points to specific control data structure of current interface. */
    const fnet_netif_api_t  *api;                               /* Pointer to Interafce API structure.*/
    unsigned long           scope_id;                           /* Scope zone index, defining network interface. Used by IPv6 sockets.*/
    unsigned long           features;                           /* Supported features. Bitwise of fnet_netif_feature_t.*/
#if FNET_CFG_IP4    
    fnet_netif_ip4_addr_t   ip4_addr;                           /* The interface IPv4 address structure. */    
#endif
#if FNET_CFG_IP6
    fnet_netif_ip6_addr_t   ip6_addr[FNET_NETIF_IP6_ADDR_MAX];  /* The interface IPv6 address structure. */
    struct fnet_nd6_if      *nd6_if_ptr;                        /* Pointer to the ND structure, if the interface supports ND. */ 
#if FNET_CFG_IP6_PMTU_DISCOVERY    
    unsigned long           pmtu;                               /* Path MTU, changed by Path MTU Discovery for IPv6. If 0 - is disabled.*/
    unsigned long           pmtu_timestamp;                     /* The timestamp, in milliseconds, when PMTU was changed last time.*/ 
    fnet_timer_desc_t       pmtu_timer;                         /* PMTU timer,used to detect increases in PMTU.*/
#endif    
#endif    
 } fnet_netif_t;


/************************************************************************
*     Global Data Structures
*************************************************************************/
extern fnet_netif_t *fnet_netif_list;   /* The list of network interfaces.*/


/************************************************************************
*     Function Prototypes
*************************************************************************/
int fnet_netif_init_all( void );
void fnet_netif_release_all( void );
int fnet_netif_init(fnet_netif_t *netif, unsigned char *hw_addr, unsigned int hw_addr_size);
void fnet_netif_release( fnet_netif_t *netif );
void fnet_netif_drain( void );
void fnet_netif_set_ip4_addr_automatic( fnet_netif_desc_t netif );
void fnet_netif_dupip_handler_signal( fnet_netif_desc_t netif );


#if FNET_CFG_IP6
    fnet_netif_ip6_addr_t *fnet_netif_get_ip6_addr_info(fnet_netif_t *netif, fnet_ip6_addr_t *ip_addr);
    int fnet_netif_bind_ip6_addr_prv( fnet_netif_t *netifnetif_desc, fnet_ip6_addr_t *addr, fnet_netif_ip6_addr_type_t addr_type, 
                                        unsigned long lifetime /*in seconds*/, unsigned long prefix_length /* bits */ );
    int fnet_netif_unbind_ip6_addr_prv ( fnet_netif_t *netif, fnet_netif_ip6_addr_t *if_addr );                                        
    int fnet_netif_is_my_ip6_addr(fnet_netif_t *netif, fnet_ip6_addr_t *ip_addr);
    fnet_netif_desc_t fnet_netif_get_by_ip6_addr( fnet_ip6_addr_t *ip_addr );
    int fnet_netif_is_my_ip6_solicited_multicast_addr(fnet_netif_t *netif, fnet_ip6_addr_t *ip_addr);
    void fnet_netif_ip6_addr_timer ( fnet_netif_t *netif);
    int fnet_netif_set_ip6_addr_autoconf(fnet_netif_t *netif, fnet_ip6_addr_t *ip_addr);
#if FNET_CFG_IP6_PMTU_DISCOVERY     
    void fnet_netif_pmtu_init(fnet_netif_t *netif);
    void fnet_netif_pmtu_release(fnet_netif_t *netif);
    void fnet_netif_set_pmtu(fnet_netif_t *netif, unsigned long pmtu);
#endif
    
#endif                                      

#endif /* _FNET_NETIF_PRV_H_ */

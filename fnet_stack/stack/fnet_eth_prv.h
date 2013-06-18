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
* @file fnet_eth_prv.h
*
* @author Andrey Butok
*
* @date Mar-4-2013
*
* @version 0.1.21.0
*
* @brief Private. Ethernet platform independent API functions.
*
***************************************************************************/

#ifndef _FNET_ETHERNET_PRV_H_

#define _FNET_ETHERNET_PRV_H_

#include "fnet_config.h"

#if (FNET_CFG_CPU_ETH0 ||FNET_CFG_CPU_ETH1)

#include "fnet_eth.h"
#include "fnet_arp.h"
#include "fnet_netif_prv.h"


/************************************************************************
*     Definitions
*************************************************************************/
/* Ethernet Frame Types */
#define FNET_ETH_TYPE_IP4       (0x0800)
#define FNET_ETH_TYPE_ARP       (0x0806)
#define FNET_ETH_TYPE_IP6       (0x86DD)

#define FNET_ETH_HDR_SIZE       (14)    /* Size of Ethernet header.*/
#define FNET_ETH_CRC_SIZE       (4)     /* Size of Ethernet CRC.*/


/************************************************************************
*    Network Layer Protocol interface control structure.
*************************************************************************/
typedef struct fnet_eth_prot_if
{
    unsigned short protocol;                                    /* Protocol number */
    void (*input)( fnet_netif_t *netif, fnet_netbuf_t *nb );    /* Protocol input function.*/
} fnet_eth_prot_if_t;


/*****************************************************************************
*     Ethernet Frame header.
******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    fnet_mac_addr_t destination_addr    FNET_COMP_PACKED;   /**< 48-bit destination address.*/
    fnet_mac_addr_t source_addr         FNET_COMP_PACKED;   /**< 48-bit source address.*/
    unsigned short  type                FNET_COMP_PACKED;    /**< 16-bit type field.*/
} fnet_eth_header_t;
FNET_COMP_PACKED_END


/*****************************************************************************
*     Ethernet Control data structure 
******************************************************************************/
typedef struct fnet_eth_if
{
    void                *if_cpu_ptr;  /* Points to CPU-specific control data structure of the interface. */
    unsigned int        mac_number;   /* MAC module number [0-1]. */
    void                ( *output)(fnet_netif_t *netif, unsigned short type, const fnet_mac_addr_t dest_addr, fnet_netbuf_t* nb);
#if FNET_CFG_MULTICAST      
    void                ( *multicast_join)(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
    void                ( *multicast_leave)(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
#endif /* FNET_CFG_MULTICAST */
    /* Internal parameters.*/
    int                 connection_flag;
    fnet_timer_desc_t   eth_timer;    /* Optional ETH timer.*/
#if FNET_CFG_IP4    
    fnet_arp_if_t       arp_if;
#endif   
#if FNET_CFG_IP6   
    fnet_nd6_if_t       nd6_if;
#endif 
#if !FNET_CFG_CPU_ETH_MIB     
    struct fnet_netif_statistics statistics;
#endif    
} fnet_eth_if_t;



/************************************************************************
*     Global Data Structures
*************************************************************************/
extern const fnet_mac_addr_t fnet_eth_null_addr;
extern const fnet_mac_addr_t fnet_eth_broadcast;

#if FNET_CFG_CPU_ETH0 
    extern fnet_netif_t fnet_eth0_if;
    #define FNET_ETH0_IF ((fnet_netif_desc_t)(&fnet_eth0_if))
#endif
#if FNET_CFG_CPU_ETH1 
    extern fnet_netif_t fnet_eth1_if;
    #define FNET_ETH1_IF ((fnet_netif_desc_t)(&fnet_eth1_if))
#endif    
    

/************************************************************************
*     Function Prototypes
*************************************************************************/
int fnet_eth_init(fnet_netif_t *netif);
void fnet_eth_release( fnet_netif_t *netif);
void fnet_eth_drain(fnet_netif_t *netif);
void fnet_eth_change_addr_notify(fnet_netif_t *netif);

#if FNET_CFG_IP4
    void fnet_eth_output_ip4(fnet_netif_t *netif, fnet_ip4_addr_t dest_ip_addr, fnet_netbuf_t* nb);
#endif 

void fnet_eth_output_low( fnet_netif_t *netif, unsigned short type, const fnet_mac_addr_t dest_addr,
                          fnet_netbuf_t *nb );
void fnet_eth_prot_input( fnet_netif_t *netif, fnet_netbuf_t *nb, unsigned short protocol ); 

#if FNET_CFG_MULTICAST
    #if FNET_CFG_IP4 
        void fnet_eth_multicast_leave_ip4(fnet_netif_t *netif, fnet_ip4_addr_t multicast_addr );
        void fnet_eth_multicast_join_ip4(fnet_netif_t *netif, fnet_ip4_addr_t  multicast_addr );
    #endif
    #if FNET_CFG_IP6
        void fnet_eth_multicast_leave_ip6(fnet_netif_t *netif, fnet_ip6_addr_t *multicast_addr );
        void fnet_eth_multicast_join_ip6(fnet_netif_t *netif, const fnet_ip6_addr_t  *multicast_addr );
    #endif        
#endif /* FNET_CFG_MULTICAST */

#if FNET_CFG_IP6
    void fnet_eth_output_ip6(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip_addr,  fnet_ip6_addr_t *dest_ip_addr, fnet_netbuf_t* nb);
#endif

#if FNET_CFG_DEBUG_TRACE_ETH
    void fnet_eth_trace(char *str, fnet_eth_header_t *eth_hdr);
#else
    #define fnet_eth_trace(str, eth_hdr)
#endif                          

#endif /* FNET_CFG_ETH */

#endif /* _FNET_ETHERNET_PRV_H_ */

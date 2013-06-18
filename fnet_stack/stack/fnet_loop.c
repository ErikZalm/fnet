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
* @file fnet_loop.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.20.0
*
* @brief Loopback driver implementation.
*
***************************************************************************/

#include "fnet_config.h" 
#if FNET_CFG_LOOPBACK

#include "fnet_loop.h"
#include "fnet_ip_prv.h"
#include "fnet_ip6_prv.h"

/************************************************************************
*     Global Data Structures
*************************************************************************/

/* Loopback general API structure. */
const struct fnet_netif_api fnet_loop_api =
{
    FNET_NETIF_TYPE_LOOPBACK,   /* Data-link type. */
    0,
    0,                          /* initialization function.*/
	0,                          /* shutdown function.*/
#if FNET_CFG_IP4 	
	fnet_loop_output_ip4,           /* transmit function.*/
#endif  	
	0,                          /* address change notification function.*/
	0,                          /* drain function.*/
	0,
	0,
	0,
	0
#if FNET_CFG_MULTICAST 
    #if FNET_CFG_IP4
    ,
	0,
    0
	#endif
	#if FNET_CFG_IP6
    ,
	0,
    0
    #endif	
#endif
#if FNET_CFG_IP6
	,
	fnet_loop_output_ip6
#endif /* FNET_CFG_IP6 */		
};


/* Loopback Interface structure.*/
fnet_netif_t fnet_loop_if = 
{
    0,                          /* pointer to the next net_if structure.*/
    0,                          /* pointer to the previous net_if structure.*/
    "loop",                     /* network interface name.*/
    FNET_LOOP_MTU,              /* maximum transmission unit.*/
    0,                          /* points to interface specific data structure.*/
    &fnet_loop_api
};

/************************************************************************
* NAME: fnet_loop_output_ip4
*
* DESCRIPTION: This function just only sends outgoing packets to IP layer.
*************************************************************************/
#if FNET_CFG_IP4
void fnet_loop_output_ip4(fnet_netif_t *netif, fnet_ip4_addr_t dest_ip_addr, fnet_netbuf_t* nb)
{
    FNET_COMP_UNUSED_ARG(dest_ip_addr);
    
    /* MTU check */
    if (nb->total_length <= netif->mtu)
        fnet_ip_input(netif, nb);
    else
        fnet_netbuf_free_chain(nb);
}
#endif /* FNET_CFG_IP4 */

/************************************************************************
* NAME: fnet_loop_output_ip6
*
* DESCRIPTION: This function just only sends outgoing packets to IPv6 layer.
*************************************************************************/
#if FNET_CFG_IP6
void fnet_loop_output_ip6(struct fnet_netif *netif, fnet_ip6_addr_t *src_ip_addr,  fnet_ip6_addr_t *dest_ip_addr, fnet_netbuf_t* nb)
{
    FNET_COMP_UNUSED_ARG(dest_ip_addr);
    FNET_COMP_UNUSED_ARG(src_ip_addr);
 
    /* MTU check */
    if (nb->total_length <= netif->mtu)
        fnet_ip6_input(netif, nb);
    else
        fnet_netbuf_free_chain(nb);
}
#endif /* FNET_CFG_IP6 */

#endif /* FNET_CFG_LOOPBACK */

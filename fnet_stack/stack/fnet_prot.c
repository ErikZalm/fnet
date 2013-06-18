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
* @file fnet_prot.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.26.0
*
* @brief Transport protocol interface implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_prot.h"
#include "fnet_tcp.h"
#include "fnet_udp.h"
#include "fnet_ip_prv.h"
#include "fnet_netif_prv.h"
#include "fnet_isr.h"
#include "fnet_debug.h"
#include "fnet_stdlib.h"
#include "fnet_icmp.h"
#include "fnet_icmp6.h"
#include "fnet_igmp.h"
#include "fnet_raw.h"

/************************************************************************
*     Global Data Structures
*************************************************************************/


/************************************************************************
*     List of Transport Layer Protocols.
*************************************************************************/
static fnet_prot_if_t * const fnet_prot_if_list[] =
{
#if FNET_CFG_IP4
    &fnet_icmp_prot_if      /* ICMP */
#endif    
#if FNET_CFG_IP6
    #if FNET_CFG_IP4
    ,
    #endif
    &fnet_icmp6_prot_if    /* ICMPv6 */
#endif    
#if FNET_CFG_TCP
    ,&fnet_tcp_prot_if      /* TCP */
#endif
#if FNET_CFG_UDP
    ,&fnet_udp_prot_if      /* UDP */
#endif    
#if FNET_CFG_IGMP && FNET_CFG_IP4
    ,&fnet_igmp_prot_if     /* IGMP */
#endif  
#if FNET_CFG_RAW
    ,&fnet_raw_prot_if     /* IGMP */
#endif  
    
    /* ADD HERE YOUR TRANSPORT LAYER PROTOCOL */
};

#define FNET_PROT_TRANSPORT_IF_LIST_SIZE  (sizeof(fnet_prot_if_list)/sizeof(fnet_prot_if_t*))





/************************************************************************
* NAME: fnet_prot_init
*
* DESCRIPTION: Transport and IP layers initialization. 
*************************************************************************/
int fnet_prot_init( void )
{
    int i;
    int result = FNET_OK;
 
    fnet_isr_lock();
    
    for(i=0; i < FNET_PROT_TRANSPORT_IF_LIST_SIZE; i++)
    {
        fnet_prot_if_list[i]->head = 0;
        if(fnet_prot_if_list[i]->prot_init)
            if(fnet_prot_if_list[i]->prot_init() == FNET_ERR)
            {
                result = FNET_ERR;
                goto ERROR;
            }
    }

#if FNET_CFG_IP4 
    if(fnet_ip_init() == FNET_ERR)
    {
        result = FNET_ERR;
        goto ERROR;
    }
#endif /* FNET_CFG_IP6 */    

#if FNET_CFG_IP6    
    if(fnet_ip6_init() == FNET_ERR)
    {
        result = FNET_ERR;
        goto ERROR;
    }    
#endif /* FNET_CFG_IP6 */

ERROR:    
    fnet_isr_unlock();
    return result;
}

/************************************************************************
* NAME: fnet_prot_release
*
* DESCRIPTION: Transport and IP layers release.
*************************************************************************/
void fnet_prot_release( void )
{
    int i;
    
    fnet_isr_lock();

    for(i=0; i < FNET_PROT_TRANSPORT_IF_LIST_SIZE; i++)
    {
        if(fnet_prot_if_list[i]->prot_release)
            fnet_prot_if_list[i]->prot_release();
    }

#if FNET_CFG_IP4 
    fnet_ip_release();
#endif
    
#if FNET_CFG_IP6 
    fnet_ip6_release();    
#endif
    
    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_prot_find
*
* DESCRIPTION: This function looks up a protocol by domain family name, 
*              by type and by protocol number.
*************************************************************************/
fnet_prot_if_t *fnet_prot_find( fnet_address_family_t family, fnet_socket_type_t type, int protocol )
{
    int i;
    
    for(i=0; i < FNET_PROT_TRANSPORT_IF_LIST_SIZE; i++)
    {
        if( (fnet_prot_if_list[i]->family & family) && 
            ( ((fnet_prot_if_list[i]->type == type) && (((protocol == 0)||(fnet_prot_if_list[i]->protocol == 0)) ? 1: (fnet_prot_if_list[i]->protocol == protocol)))
            || ((type == 0) && (fnet_prot_if_list[i]->protocol == protocol)) ) )
        {
            return (fnet_prot_if_list[i]);
        }
    }

    return (FNET_NULL);
}

/************************************************************************
* NAME: fnet_prot_drain
*
* DESCRIPTION: Tries to free not critical parts of 
*              dynamic allocated memory in the stack, if possible.
*************************************************************************/
void fnet_prot_drain( void )
{
    int i;

#if 0 /* For debug needs.*/
    fnet_println("DRAIN");
#endif

    for(i=0; i < FNET_PROT_TRANSPORT_IF_LIST_SIZE; i++)
    {
        if(fnet_prot_if_list[i]->prot_drain)
            fnet_prot_if_list[i]->prot_drain();
    }

#if FNET_CFG_IP4
    fnet_ip_drain();
#endif    

#if FNET_CFG_IP6
    fnet_ip6_drain();
#endif
    
    fnet_netif_drain();
}

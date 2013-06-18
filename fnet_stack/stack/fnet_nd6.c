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
* @file fnet_nd6.c
*
* @author Andrey Butok
*
* @date Feb-14-2013
*
* @version 0.1.26.0
*
* @brief IPv6 Neighbor Discovery implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_IP6
#include "fnet.h"
#include "fnet_nd6.h"
#include "fnet_icmp6.h"
#include "fnet_netif_prv.h"
#include "fnet_ip_prv.h"
#include "fnet_ip6_prv.h"
#include "fnet_checksum.h"

static void fnet_nd6_timer( void *cookie );
static void fnet_nd6_dad_timer( fnet_netif_t *netif );
static void fnet_nd6_dad_failed(fnet_netif_t *netif, fnet_netif_ip6_addr_t *addr_info);
static void fnet_nd6_rd_timer(fnet_netif_t *netif);
static void fnet_nd6_prefix_timer(fnet_netif_t *netif);
static void fnet_nd6_neighbor_cache_timer(fnet_netif_t *netif);
static void fnet_nd6_neighbor_cache_timer(fnet_netif_t *netif);
static void fnet_nd6_redirect_table_del(fnet_netif_t *if_ptr, const fnet_ip6_addr_t *target_addr);
static fnet_nd6_redirect_entry_t *fnet_nd6_redirect_table_add(fnet_netif_t *if_ptr, const fnet_ip6_addr_t *destination_addr, const fnet_ip6_addr_t *target_addr);
static int fnet_nd6_is_firsthop_router(fnet_netif_t *netif, fnet_ip6_addr_t *router_ip);

/************************************************************************
* NAME: fnet_nd6_init
* RETURNS: FNET_OK or FNET_ERR.
* DESCRIPTION: Initializes the Neighbor Disscovery on an interface.
*************************************************************************/
int fnet_nd6_init (fnet_netif_t *netif, fnet_nd6_if_t *nd6_if_ptr)
{
    int result = FNET_ERR;
    
    if(netif && nd6_if_ptr)
    {
        netif->nd6_if_ptr = nd6_if_ptr;
        
        /* Clear all parameters.*/
        fnet_memset_zero(nd6_if_ptr, sizeof(fnet_nd6_if_t));

        /* --- Initialize Prefix List. ----*/
        
        /* The link-local prefix is considered to be on the
         * prefix list with an infinite invalidation timer
         * regardless of whether routers are advertising a
         * prefix for it.
         */        
        fnet_nd6_prefix_list_add(netif, &fnet_ip6_addr_linklocal_prefix, FNET_ND6_PREFIX_LENGTH_DEFAULT, FNET_ND6_PREFIX_LIFETIME_INFINITE);
 
        /* ---- Initialize interface variables. ----*/
        /* The recommended MTU for the link. */
        nd6_if_ptr->mtu = netif->mtu;  
        if(nd6_if_ptr->mtu < FNET_IP6_DEFAULT_MTU)
            nd6_if_ptr->mtu = FNET_IP6_DEFAULT_MTU;

        /* Cur Hop Limit value */
        nd6_if_ptr->cur_hop_limit = FNET_IP6_HOPLIMIT_DEFAULT;            
     
        /* Reachable time. used for NU.*/
        nd6_if_ptr->reachable_time = FNET_ND6_REACHABLE_TIME;
        
        /* Time between retransmissions of Neighbor Solicitation messages.*/
        nd6_if_ptr->retrans_timer = FNET_ND6_RETRANS_TIMER;                   
        
        /* --- Register timer to check ND lists and N cache. ---*/       
        nd6_if_ptr->timer = fnet_timer_new((FNET_ND6_TIMER_PERIOD / FNET_TIMER_PERIOD_MS)+1, 
                                                 fnet_nd6_timer, netif);
        
        if(nd6_if_ptr->timer != FNET_NULL)
            result = FNET_OK;
    }
    return result;
}

/************************************************************************
* NAME: fnet_nd6_release
* RETURNS: void
* DESCRIPTION: Release the Neighbor Disscovery for the interface.
*************************************************************************/
void fnet_nd6_release (fnet_netif_t *netif)
{
    if(netif && netif->nd6_if_ptr)
    {
        fnet_timer_free(netif->nd6_if_ptr->timer);  
        netif->nd6_if_ptr = 0;     
    }
}

/************************************************************************
* NAME: fnet_nd6_timer
*
* DESCRIPTION: ND6 timer.
*************************************************************************/
static void fnet_nd6_timer( void *cookie )
{
    fnet_netif_t    *netif = (fnet_netif_t *)cookie;
    
    /* DAD timer.*/
    fnet_nd6_dad_timer(netif);   
    
    /* Neighbor Cache processing.*/
    fnet_nd6_neighbor_cache_timer(netif);

    /* Check prefix lifetime.*/
    fnet_nd6_prefix_timer(netif);
    
    /* Check address lifetime.*/
    fnet_netif_ip6_addr_timer(netif);
    
    /* RD timer. */
    fnet_nd6_rd_timer(netif);      

}

/************************************************************************
* NAME: fnet_nd6_neighbor_cache_get
*
* DESCRIPTION: Get entry from Neighbor cache that corresponds ip_addr.
*               It returns NULL if no entry is found.
*************************************************************************/
fnet_nd6_neighbor_entry_t *fnet_nd6_neighbor_cache_get(fnet_netif_t *netif, fnet_ip6_addr_t *ip_addr)
{
    fnet_nd6_if_t               *nd6_if = netif->nd6_if_ptr;
    int                         i;
    fnet_nd6_neighbor_entry_t   *result = FNET_NULL;

    if (nd6_if)
    {
        /* Find the entry in the cache. */
        for(i=0; i < FNET_ND6_NEIGHBOR_CACHE_SIZE; i++)
        {
            if( (nd6_if->neighbor_cache[i].state != FNET_ND6_NEIGHBOR_STATE_NOTUSED) &&
                FNET_IP6_ADDR_EQUAL(&nd6_if->neighbor_cache[i].ip_addr, ip_addr))
            {
                result = &nd6_if->neighbor_cache[i];
                break;
            }
        }
    }
    return result;
}

/************************************************************************
* NAME: fnet_nd6_neighbor_cache_del
*
* DESCRIPTION: Deletes an entry from the Neighbor Cache.
*************************************************************************/
void fnet_nd6_neighbor_cache_del(fnet_netif_t *netif, fnet_nd6_neighbor_entry_t *neighbor_entry)
{
    if (neighbor_entry)
    {
        /* Delete posible entry in the Redirect Table.*/
        fnet_nd6_redirect_table_del(netif, &neighbor_entry->ip_addr);

        neighbor_entry->state = FNET_ND6_NEIGHBOR_STATE_NOTUSED;
        
        /* Free waiting queue.*/
        if(neighbor_entry->waiting_netbuf) //TBD for bigger queue.
        {
            fnet_netbuf_free_chain(neighbor_entry->waiting_netbuf);
        }
        
    }
}

/************************************************************************
* NAME: fnet_nd6_neighbor_cache_add
*
* DESCRIPTION: Adds (TBD update) entry into the Neighbor cache.
*************************************************************************/
fnet_nd6_neighbor_entry_t *fnet_nd6_neighbor_cache_add(fnet_netif_t *netif, fnet_ip6_addr_t *ip_addr, fnet_nd6_ll_addr_t ll_addr, fnet_nd6_neighbor_state_t state)
{
    fnet_nd6_if_t               *nd6_if = netif->nd6_if_ptr;
    int                         i;
    fnet_nd6_neighbor_entry_t   *entry = FNET_NULL;

    if (nd6_if)
    {
        /* Find not used entry.*/
        for(i=0; i < FNET_ND6_NEIGHBOR_CACHE_SIZE; i++)
        {
            if( (nd6_if->neighbor_cache[i].state == FNET_ND6_NEIGHBOR_STATE_NOTUSED))
            {
                entry = &nd6_if->neighbor_cache[i];
                break;
            }
        }
        
        /* If no free entry is found.*/
        if(entry == FNET_NULL)
        { 
            entry = &nd6_if->neighbor_cache[0];
            /* Try to find the oldest entry.*/
            for(i=0; i < FNET_ND6_NEIGHBOR_CACHE_SIZE; i++)
            {
                if(nd6_if->neighbor_cache[i].creation_time < entry->creation_time)
                {
                    entry = &nd6_if->neighbor_cache[i];
                }
            }
        }
        
        /* Fill the informationn.*/
        
        /* Clear enty structure.*/
        fnet_memset_zero(entry, sizeof(fnet_nd6_neighbor_entry_t));
        
        FNET_IP6_ADDR_COPY(ip_addr, &entry->ip_addr);
        if(ll_addr != FNET_NULL)
            FNET_ND6_LL_ADDR_COPY(ll_addr, entry->ll_addr, netif->api->hw_addr_size);
        entry->creation_time = fnet_timer_seconds();
        entry->is_router = 0;
        entry->router_lifetime = 0;
        entry->state = state;
        
        //TBD Init timers reachable; last send
    }
    return entry;
}

/************************************************************************
* NAME: fnet_nd6_neighbor_cache_timer
* DESCRIPTION: Timer routine used for Neighbor Cache check.
*************************************************************************/
static void fnet_nd6_neighbor_cache_timer(fnet_netif_t *netif)
{
    fnet_nd6_if_t               *nd6_if = netif->nd6_if_ptr;
    int                         i;
    fnet_nd6_neighbor_entry_t   *neighbor_entry;

    /* Neighbor Cache processing.*/
    for(i=0; i < FNET_ND6_NEIGHBOR_CACHE_SIZE; i++)
    {
        neighbor_entry = &nd6_if->neighbor_cache[i];
        
        /* Check router expiration */
        if((neighbor_entry->state != FNET_ND6_NEIGHBOR_STATE_NOTUSED)
           && (neighbor_entry->is_router == 1) 
           && (fnet_timer_seconds() > (neighbor_entry->creation_time + neighbor_entry->router_lifetime)))
        {
            if(neighbor_entry->router_lifetime)
                fnet_nd6_router_list_del(neighbor_entry);
        }
        
        switch(neighbor_entry->state)
        {
            /* =========== INCOMPLETE ============= */
            case FNET_ND6_NEIGHBOR_STATE_INCOMPLETE:
                    
                if(fnet_timer_get_interval(neighbor_entry->state_time, fnet_timer_ms()) > nd6_if->retrans_timer )
                {
                    neighbor_entry->solicitation_send_counter++;
                    if(neighbor_entry->solicitation_send_counter >= FNET_ND6_MAX_MULTICAST_SOLICIT)
                    /* AR is failed.*/
                    {
                        /* RFC4861 7.3.3: If address resolution fails, the entry SHOULD be deleted, so that subsequent
                         * traffic to that neighbor invokes the next-hop determination procedure
                         * again. Invoking next-hop determination at this point ensures that
                         * alternate default routers are tried.
                         */                        
                        fnet_nd6_neighbor_cache_del(netif, neighbor_entry);
                        //TBD ICMP error to upper layer.
                    }
                    else
                    /* AR: transmit NS.*/
                    {
                        fnet_ip6_addr_t *dest_addr = &neighbor_entry->ip_addr;
                        
                        /* Select source address.*/ //PFI
                        fnet_ip6_addr_t *src_addr = &neighbor_entry->solicitation_src_ip_addr;
                        
                        neighbor_entry->state_time = fnet_timer_ms();
                        /* AR: Transmitting a Neighbor Solicitation message targeted at the neighbor.*/
                        fnet_nd6_neighbor_solicitation_send(netif, src_addr, FNET_NULL /* NULL for AR */, dest_addr);
                    }
                }
                break;
            /* =========== REACHABLE ============= */    
            case FNET_ND6_NEIGHBOR_STATE_REACHABLE:
                /*
                 * RFC 4861: When ReachableTime milliseconds have passed since receipt of the last
                 * reachability confirmation for a neighbor, the Neighbor Cache entry�s
                 * state changes from REACHABLE to STALE.
                 */
                if(fnet_timer_get_interval(neighbor_entry->state_time, fnet_timer_ms()) > nd6_if->reachable_time )
                /* REACHABLE to STALE */
                { 
                    neighbor_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
                }
                break;
           /* =========== DELAY ================ */    
           case FNET_ND6_NEIGHBOR_STATE_DELAY:     
                 /*
                 * If the entry is still in the DELAY state when the timer expires, 
                 * the entry�s state changes to PROBE.
                 */
                if(fnet_timer_get_interval(neighbor_entry->state_time, fnet_timer_ms()) > FNET_ND6_DELAY_FIRST_PROBE_TIME )                
                /* DELAY to PROBE */
                { 
                    fnet_ip6_addr_t *src_addr;
                    fnet_ip6_addr_t *dest_addr = &neighbor_entry->ip_addr;

                    
                    neighbor_entry->state = FNET_ND6_NEIGHBOR_STATE_PROBE;
                    neighbor_entry->solicitation_send_counter = 0;
                    /* 
                     * Upon entering the PROBE state, a node sends a unicast Neighbor
                     * Solicitation message to the neighbor using the cached link-layer
                     * address.
                     */

                    neighbor_entry->state_time = fnet_timer_ms();
                     
                    /* Select source address.*/ //PFI
                    src_addr = (fnet_ip6_addr_t *)fnet_ip6_select_src_addr(netif, dest_addr);
                     
                    if(src_addr)
                    {
                        fnet_nd6_neighbor_solicitation_send(netif, src_addr, dest_addr/*Unicast*/, dest_addr);
                    }
                }
                break;
            /* =========== PROBE ================ */    
            case FNET_ND6_NEIGHBOR_STATE_PROBE:  //PFI the same as INCOMPLETE
                /* RFC4861 7.3.3:
                 * While in the PROBE state, a node retransmits Neighbor
                 * Solicitation messages every RetransTimer milliseconds until
                 * reachability confirmation is obtained. Probes are retransmitted even
                 * if no additional packets are sent to the neighbor. If no response is
                 * received after waiting RetransTimer milliseconds after sending the
                 * MAX_UNICAST_SOLICIT solicitations, retransmissions cease and the
                 * entry SHOULD be deleted. Subsequent traffic to that neighbor will
                 *  recreate the entry and perform address resolution again.
                 */
                 if(fnet_timer_get_interval(neighbor_entry->state_time, fnet_timer_ms()) > nd6_if->retrans_timer )
                {
                    neighbor_entry->solicitation_send_counter++;
                    if(neighbor_entry->solicitation_send_counter >= FNET_ND6_MAX_UNICAST_SOLICIT)
                    /* AR is failed.*/
                    {
                        /* RFC4861 7.3.3: If no response is
                         * received after waiting RetransTimer milliseconds after sending the
                         * MAX_UNICAST_SOLICIT solicitations
                         */                        
                        fnet_nd6_neighbor_cache_del(netif, neighbor_entry);
                    }
                    else
                    /* NUD: transmit NS.*/
                    {
                        fnet_ip6_addr_t *dest_addr = &neighbor_entry->ip_addr;
                        
                        /* Select source address.*/ //PFI
                        fnet_ip6_addr_t *src_addr = (fnet_ip6_addr_t *)fnet_ip6_select_src_addr(netif, dest_addr);
                        if(src_addr)
                        {
                            neighbor_entry->state_time = fnet_timer_ms();
                            dest_addr = (fnet_ip6_addr_t *)&neighbor_entry->ip_addr;
                            fnet_nd6_neighbor_solicitation_send(netif, src_addr, dest_addr/*Unicast*/, dest_addr);
                        }
                    }
                }              
                break;                    
            /* case FNET_ND6_NEIGHBOR_STATE_NOTUSED:*/
            default:
                break;
        }
    }
    
    
}

/************************************************************************
* NAME: fnet_nd6_neighbor_enqueue_waiting_netbuf
*
* DESCRIPTION: Put netbuf to the queue, waiting for address resolution to complete.
*************************************************************************/
void fnet_nd6_neighbor_enqueue_waiting_netbuf(fnet_nd6_neighbor_entry_t *neighbor_entry, fnet_netbuf_t *waiting_netbuf)
{
    if (neighbor_entry && waiting_netbuf)
    {
        /* When a queue  overflows, the new arrival SHOULD replace the oldest entry.*/
        //TBD for bigger queue.
        if(neighbor_entry->waiting_netbuf != FNET_NULL)
        {
            fnet_netbuf_free_chain(neighbor_entry->waiting_netbuf); /* Free the oldest one.*/
        }
        
        neighbor_entry->waiting_netbuf = waiting_netbuf;
    }
}

/************************************************************************
* NAME: fnet_nd6_neighbor_send_waiting_netbuf
*
* DESCRIPTION: Sends waiting PCBs, if any.
*************************************************************************/
void fnet_nd6_neighbor_send_waiting_netbuf(fnet_netif_t *netif, fnet_nd6_neighbor_entry_t *neighbor_entry)
{
    if (neighbor_entry->waiting_netbuf != FNET_NULL)
    {
        /* Send.*/
        netif->api->output_ip6(netif, FNET_NULL /* not needed.*/,  &neighbor_entry->ip_addr, neighbor_entry->waiting_netbuf); /* IPv6 Transmit function.*/
        
        neighbor_entry->waiting_netbuf = FNET_NULL;
    }
}

/************************************************************************
* NAME: fnet_nd6_router_list_add
*
* DESCRIPTION: Adds entry into the Router List.
*************************************************************************/
void fnet_nd6_router_list_add( fnet_nd6_neighbor_entry_t *neighbor_entry, unsigned long lifetime )
{
    if (neighbor_entry)
    {
        if(lifetime) 
        {
            neighbor_entry->is_router = 1;
            neighbor_entry->router_lifetime = lifetime;
            neighbor_entry->creation_time = fnet_timer_seconds();
        }
        else
        /* If the address is already present in the host�s Default Router
         * List and the received Router Lifetime value is zero, immediately
         * time-out the entry.
         */
        {
            fnet_nd6_router_list_del(neighbor_entry);
        }
    }
}

/************************************************************************
* NAME: fnet_nd6_router_list_del
*
* DESCRIPTION: Deletes an entry from the Router List.
*************************************************************************/
void fnet_nd6_router_list_del( fnet_nd6_neighbor_entry_t *neighbor_entry )
{
    if (neighbor_entry)
    {
        neighbor_entry->is_router = 0;
        neighbor_entry->router_lifetime = 0;
    }
}

/************************************************************************
* NAME: fnet_nd6_default_router_get
*
* DESCRIPTION: Chooses default router. Returns FNET_NULL if no router exists.
*************************************************************************/
fnet_nd6_neighbor_entry_t *fnet_nd6_default_router_get(fnet_netif_t *netif)
{
    fnet_nd6_if_t               *nd6_if = netif->nd6_if_ptr;
    int                         i;
    fnet_nd6_neighbor_entry_t   *result = FNET_NULL;

    if(nd6_if)
    {
        /* Find the first router. */
        for(i=0; i < FNET_ND6_NEIGHBOR_CACHE_SIZE; i++)
        {
            if( (nd6_if->neighbor_cache[i].state != FNET_ND6_NEIGHBOR_STATE_NOTUSED) && (nd6_if->neighbor_cache[i].is_router == 1) && (nd6_if->neighbor_cache[i].router_lifetime != 0) )
            {
                result = &nd6_if->neighbor_cache[i];
                /* Routers that are reachable or probably reachable (i.e., in any
                 * state other than INCOMPLETE) SHOULD be preferred over routers
                 * whose reachability is unknown or suspect.
                 */
                if(nd6_if->neighbor_cache[i].state != FNET_ND6_NEIGHBOR_STATE_INCOMPLETE)
                {
                    break;
                }
            }
        }

    }
    
    /* //TBD 
     * RFFC4861: When no routers on the list are known to be reachable or
     * probably reachable, routers SHOULD be selected in a round-robin
     * fashion, so that subsequent requests for a default router do not
     * return the same router until all other routers have been
     * selected.
     * Cycling through the router list in this case ensures that all
     * available routers are actively probed by the Neighbor
     * Unreachability Detection algorithm. A request for a default
     * router is made in conjunction with the sending of a packet to a
     * router, and the selected router will be probed for reachability
     * as a side effect.
     */
    
    return result;
}

/************************************************************************
* NAME: fnet_nd6_is_firsthop_router
*
* DESCRIPTION: The IP source address of the Redirect is the same as the current
*                 first-hop router for the specified ICMP Destination Address.
*************************************************************************/
static int fnet_nd6_is_firsthop_router(fnet_netif_t *netif, fnet_ip6_addr_t *router_ip)
{
    fnet_nd6_if_t               *nd6_if = netif->nd6_if_ptr;
    int                         i;
    fnet_nd6_neighbor_entry_t   *neighbor_cache_entry;
    int                         result = FNET_FALSE;

    /* The IP source address of the Redirect is the same as the current
     * first-hop router for the specified ICMP Destination Address.*/ 
    neighbor_cache_entry = fnet_nd6_neighbor_cache_get(netif, router_ip);
    if(neighbor_cache_entry) 
    { 		
        if(neighbor_cache_entry->is_router == FNET_TRUE)
        {
            if(neighbor_cache_entry->router_lifetime == 0) 
            {
                for(i=0; i < FNET_ND6_REDIRECT_TABLE_SIZE; i++)
                {
                    if(FNET_IP6_ADDR_EQUAL(&nd6_if->redirect_table[i].target_addr, router_ip))
                    {
                        result = FNET_TRUE;
                        break;
                    }
                }
            }
            else
                result = FNET_TRUE;
        }
    }
    return result;
}

/************************************************************************
* NAME: nd6_prefix_list_get
*
* DESCRIPTION: Get entry from Prefix List that corresponds to "prefix".
* It returns NULL if no entry is found.
*************************************************************************/
fnet_nd6_prefix_entry_t *fnet_nd6_prefix_list_get(fnet_netif_t *netif, fnet_ip6_addr_t *prefix)
{
    fnet_nd6_if_t           *nd6_if = netif->nd6_if_ptr;
    int                     i;
    fnet_nd6_prefix_entry_t *result = FNET_NULL;
    
    if (nd6_if)
    {
        /* Find the entry in the list.
        */
        for(i=0; i<FNET_ND6_PREFIX_LIST_SIZE; i++)
        {
            if( (nd6_if->prefix_list[i].state != FNET_ND6_PREFIX_STATE_NOTUSED) &&
                FNET_IP6_ADDR_EQUAL(&nd6_if->prefix_list[i].prefix, prefix))
            {
                result = &nd6_if->prefix_list[i];
                break;
            }
        }
    }
    return result;
}

/************************************************************************
* NAME: fnet_nd6_prefix_list_del
*
* DESCRIPTION: Deletes an entry from the Prefix List.
*************************************************************************/
void fnet_nd6_prefix_list_del(fnet_nd6_prefix_entry_t *prefix_entry)
{
    if (prefix_entry)
    {
        prefix_entry->state = FNET_ND6_PREFIX_STATE_NOTUSED;
    }
}


/************************************************************************
* NAME: fnet_nd6_prefix_list_add
*
* DESCRIPTION: Adds (TBD update) entry into the Prefix List.
*************************************************************************/
fnet_nd6_prefix_entry_t *fnet_nd6_prefix_list_add(fnet_netif_t *if_ptr, const fnet_ip6_addr_t *prefix, unsigned long prefix_length, unsigned long lifetime)
{
    struct fnet_nd6_if      *nd6_if = if_ptr->nd6_if_ptr;
    int                     i;
    fnet_nd6_prefix_entry_t *entry = FNET_NULL;
    
    if (nd6_if)
    {
        /* Find an unused entry in the cache. Skip 1st Link_locak prefix.
         */
        for(i=1; i < FNET_ND6_PREFIX_LIST_SIZE; i++)
        {
            if(nd6_if->prefix_list[i].state == FNET_ND6_PREFIX_STATE_NOTUSED)
            {
                entry = &nd6_if->prefix_list[i];
                break;
            }
        }
        
        
        /* If no free entry is found.
         */
        if(entry == FNET_NULL)
        { 
            entry = &nd6_if->prefix_list[1];
            /* Try to find the oldest entry.
             */
            for(i=1; i < FNET_ND6_PREFIX_LIST_SIZE; i++)
            {
                if(nd6_if->prefix_list[i].creation_time < entry->creation_time)
                {
                    entry = &nd6_if->prefix_list[i];
                }
            }
        }
        
        /* Fill the informationn.
         */
        FNET_IP6_ADDR_COPY(prefix, &entry->prefix);
        entry->prefix_length = prefix_length;
        entry->lifetime = lifetime;
        entry->creation_time = fnet_timer_seconds();
        entry->state = FNET_ND6_PREFIX_STATE_USED;
    }
    
    return entry;
}

/************************************************************************
* NAME: fnet_nd6_prefix_timer
* DESCRIPTION: Timer routine used for Prefix List lifetime check.
*************************************************************************/
static void fnet_nd6_prefix_timer(fnet_netif_t *netif)
{
    fnet_nd6_if_t   *nd6_if = netif->nd6_if_ptr;
    int             i;
    
    /* Check lifetime for prefixes.*/
    for(i=1; i<FNET_ND6_PREFIX_LIST_SIZE; i++)
    {
        if((nd6_if->prefix_list[i].state != FNET_ND6_PREFIX_STATE_NOTUSED)
            && (nd6_if->prefix_list[i].lifetime != FNET_ND6_PREFIX_LIFETIME_INFINITE)
            && (fnet_timer_seconds() > (nd6_if->prefix_list[i].creation_time + nd6_if->prefix_list[i].lifetime)) ) //PFI
             
        {
            fnet_nd6_prefix_list_del(&nd6_if->prefix_list[i]);
        }
    }
}

/************************************************************************
* NAME: fnet_nd6_redirect_table_add
*
* DESCRIPTION: Add entry into the Redirect Table.
*************************************************************************/
static fnet_nd6_redirect_entry_t *fnet_nd6_redirect_table_add(fnet_netif_t *if_ptr, const fnet_ip6_addr_t *destination_addr, const fnet_ip6_addr_t *target_addr)
{
    struct fnet_nd6_if          *nd6_if = if_ptr->nd6_if_ptr;
    int                         i;
    fnet_nd6_redirect_entry_t   *entry = FNET_NULL;
    
    if (nd6_if)
    {

        /* Check if the destination address exists.*/
        for(i=0; i < FNET_ND6_REDIRECT_TABLE_SIZE; i++)
        {
            if(FNET_IP6_ADDR_EQUAL(&nd6_if->redirect_table[i].destination_addr, destination_addr))
            {
                /* Found existing destination address.*/
                entry = &nd6_if->redirect_table[i];
                break;
            }
        }

        if(entry == FNET_NULL)
        {
            /* Find an unused entry in the table.
             */
            for(i=0; i < FNET_ND6_REDIRECT_TABLE_SIZE; i++)
            {
                /* Check if it is free.*/
                if(FNET_IP6_ADDR_IS_UNSPECIFIED(&nd6_if->redirect_table[i].destination_addr))
                {
                    entry = &nd6_if->redirect_table[i];
                    break;
                }
            }
              
            /* If no free entry is found.
             */
            if(entry == FNET_NULL)
            { 
                entry = &nd6_if->redirect_table[0];
                /* Try to find the oldest entry.
                 */
                for(i=1; i < FNET_ND6_REDIRECT_TABLE_SIZE; i++)
                {
                    if(nd6_if->redirect_table[i].creation_time < entry->creation_time)
                    {
                        entry = &nd6_if->redirect_table[i];
                    }
                }
            }
        }
        /* Fill the informationn.
         */
        FNET_IP6_ADDR_COPY(destination_addr, &entry->destination_addr);
        FNET_IP6_ADDR_COPY(target_addr, &entry->target_addr);
        entry->creation_time = fnet_timer_seconds();
    }
    
    return entry;
}

/************************************************************************
* NAME: fnet_nd6_redirect_table_del
*
* DESCRIPTION: Deletes an entry from the Redirect Table.
*************************************************************************/
static void fnet_nd6_redirect_table_del(fnet_netif_t *if_ptr, const fnet_ip6_addr_t *target_addr)
{
    struct fnet_nd6_if          *nd6_if = if_ptr->nd6_if_ptr;
    int                         i;
    
    if (nd6_if)
    {
        for(i=0; i < FNET_ND6_REDIRECT_TABLE_SIZE; i++)
        {
            if(FNET_IP6_ADDR_EQUAL(&nd6_if->redirect_table[i].target_addr, target_addr))
            {
                /* Clear it.*/
                fnet_memset_zero(&nd6_if->redirect_table[i], sizeof(fnet_nd6_redirect_entry_t));
            }
        }
    }
}

/************************************************************************
* NAME: fnet_nd6_redirect_addr
*
* DESCRIPTION: Redirects destination address, if needed.
*************************************************************************/
void fnet_nd6_redirect_addr(fnet_netif_t *if_ptr, fnet_ip6_addr_t **destination_addr_p)
{
    struct fnet_nd6_if          *nd6_if = if_ptr->nd6_if_ptr;
    int                         i;
    
    if (nd6_if)
    {
        for(i=0; i < FNET_ND6_REDIRECT_TABLE_SIZE; i++)
        {
            if(FNET_IP6_ADDR_EQUAL(&nd6_if->redirect_table[i].destination_addr, *destination_addr_p))
            {
                /* Found redirection.*/
                *destination_addr_p = &nd6_if->redirect_table[i].target_addr; 
                break;
            }
        }
    }
}

/************************************************************************
* NAME: fnet_nd6_addr_is_onlink
*
* DESCRIPTION: Checks if the address is on-link. 
*              Returns FNET_TRUE if it is on-line, FNET_FALSE otherwise.
*************************************************************************/
int fnet_nd6_addr_is_onlink(fnet_netif_t *netif, fnet_ip6_addr_t *addr)
{
    fnet_nd6_if_t   *nd6_if = netif->nd6_if_ptr;
    int             i;
    int             result = FNET_FALSE;

    if (nd6_if)
    {
        /* Find the entry in the list. */
        for(i=0; i<FNET_ND6_PREFIX_LIST_SIZE; i++)
        {
            if( (nd6_if->prefix_list[i].state != FNET_ND6_PREFIX_STATE_NOTUSED) &&
                (fnet_ip6_addr_pefix_cmp(&nd6_if->prefix_list[i].prefix, addr, nd6_if->prefix_list[i].prefix_length) == FNET_TRUE) )
            {
                result = FNET_TRUE;
                break;
            }
        }
    }
    return result;
}

/************************************************************************
* NAME: fnet_nd6_neighbor_solicitation_send
* DESCRIPTION: Sends an Neighbor Solicitation Message.
*   Nodes send Neighbor Solicitations to request the link-layer address
*   of a target node while also providing their own link-layer address to
*   the target.
*************************************************************************/
void fnet_nd6_neighbor_solicitation_send(fnet_netif_t *netif /*MUST*/, fnet_ip6_addr_t *ipsrc /* NULL for, DAD */, fnet_ip6_addr_t *ipdest /*set for NUD,  NULL for DAD & AR */, fnet_ip6_addr_t *target_addr)
{
    unsigned long                   ns_packet_size;
    fnet_netbuf_t                   *ns_nb;
    fnet_nd6_ns_header_t            *ns_packet;
    fnet_nd6_option_lla_header_t    *nd_option_slla; 
    fnet_ip6_addr_t                 _ip_dest;

    /* Check if, this is DAD.
     * Duplicate Address Detection sends Neighbor Solicitation
     * messages with an unspecified source address targeting its own
     * "tentative" address and without SLLAO.*/
    ns_packet_size = sizeof(fnet_nd6_ns_header_t) + ((ipsrc == FNET_NULL /* DAD */) ? 0:(sizeof(fnet_nd6_option_header_t) + netif->api->hw_addr_size));
    if((ns_nb = fnet_netbuf_new((int)ns_packet_size, FNET_TRUE)) != 0)
    {
        /*
         * Neighbor Solicitations are multicast when the node needs
         * to resolve an address and unicast when the node seeks to verify the
         * reachability of a neighbor.
         */
        if(ipdest == FNET_NULL)
        { /* AR, DAD */
            /* Generate Solicited Multicast destination address from the target address.*/
            fnet_ip6_get_solicited_multicast_addr(target_addr, &_ip_dest);
            ipdest = &_ip_dest;
        }
        /* else  NUD */

        /* Fill ICMP Header */
        ns_packet = ns_nb->data_ptr;
        ns_packet->icmp6_header.type = FNET_ICMP6_TYPE_NEIGHBOR_SOLICITATION;
        ns_packet->icmp6_header.code = 0;
        
        /* Fill NS Header.*/
        fnet_memset_zero( ns_packet->_reserved, sizeof(ns_packet->_reserved));   /* Set to zeros the reserved field.*/
        FNET_IP6_ADDR_COPY(target_addr, &ns_packet->target_addr);               /* Set NS target address.*/  
        
        /*
         * RFC4861: The link-layer address for the sender. MUST NOT be
         * included when the source IP address is the
         * unspecified address (DAD). Otherwise, on link layers
         * that have addresses this option MUST be included in
         * multicast solicitations and SHOULD be included in
         * unicast solicitations.
         */
        if(ipsrc != FNET_NULL /* AR or NUD */)
        {
            /* RFC4861 7.2.2: If the source address of the packet prompting the solicitation is the
             * same as one of the addresses assigned to the outgoing interface, that
             * address SHOULD be placed in the IP Source Address of the outgoing
             * solicitation. Otherwise, any one of the addresses assigned to the
             * interface should be used.
             */
            if(fnet_netif_is_my_ip6_addr(netif, ipsrc) == FNET_FALSE)
            {
                ipsrc = (fnet_ip6_addr_t *)fnet_ip6_select_src_addr(netif, ipdest);
                    
                if(ipsrc == FNET_NULL)
                    goto DROP; /* Just in case. Should never happen.*/
            }
                                            
            /* Fill Source link-layer address option.*/
            nd_option_slla = (fnet_nd6_option_lla_header_t *)((unsigned long)ns_packet + sizeof(fnet_nd6_ns_header_t));
            nd_option_slla->option_header.type = FNET_ND6_OPTION_SOURCE_LLA; /* Type. */
            nd_option_slla->option_header.length = (unsigned char)((netif->api->hw_addr_size + sizeof(fnet_nd6_option_header_t))>>3); /* Option size devided by 8,*/
            
            if( fnet_netif_get_hw_addr(netif, nd_option_slla->addr, netif->api->hw_addr_size) != FNET_OK)
                goto DROP;    
   
            //TBD Padd if needed.%8
        }
        else
        {
            /* Source IP address is the
             * unspecified address for DAD.
             */
            ipsrc = (fnet_ip6_addr_t *)&fnet_ip6_addr_any;
        }  
        
        /* Send ICMPv6 message.*/
        fnet_icmp6_output( netif, ipsrc, ipdest, FNET_ND6_HOP_LIMIT, ns_nb);
    }
    
    return;

DROP:
    fnet_netbuf_free_chain(ns_nb);        
}

/************************************************************************
* NAME: fnet_nd6_neighbor_solicitation_receive
* RETURS: RTCS_OK or error code.
* DESCRIPTION: Handles received Neighbor Solicitation message.
*************************************************************************/
void fnet_nd6_neighbor_solicitation_receive(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb)
{
    fnet_icmp6_header_t             *icmp6_packet = nb->data_ptr;
    unsigned long                   icmp6_packet_size = nb->total_length;
    fnet_nd6_ns_header_t            *ns_packet = (fnet_nd6_ns_header_t *)icmp6_packet;
    fnet_nd6_option_lla_header_t    *nd_option_slla = FNET_NULL;
    unsigned long                   nd_option_offset;
    
    fnet_netif_ip6_addr_t           *target_if_addr_info;
    fnet_ip6_header_t               *ip6_packet = ip6_nb->data_ptr;

	
    /************************************************************
    * Validation.
    *************************************************************/	
    if(
        (icmp6_packet_size < sizeof(fnet_nd6_ns_header_t))      
        /* Validation RFC4861 (7.1.1).*/
        ||(ip6_packet->hop_limit != FNET_ND6_HOP_LIMIT)         /* The IP Hop Limit field has a value of 255.*/
        ||(icmp6_packet->code != 0)                             /* ICMP Code is 0.*/ 
        ||FNET_IP6_ADDR_IS_MULTICAST(&ns_packet->target_addr)   /* Target Address is not a multicast address. */
        ||fnet_netif_is_my_ip6_addr(netif, src_ip)              /* Duplicate IP address check. */ //TBD???
        )
    { 		
        goto DROP;
    }


    {   /* Validation passed.
         */
     
        /************************************************************
         * Handle posible options.
         ************************************************************/
        nd_option_offset = sizeof(fnet_nd6_ns_header_t);
        while(icmp6_packet_size > nd_option_offset + sizeof(fnet_nd6_option_header_t)) 
        {
            fnet_nd6_option_header_t *nd_option;
    				    
            nd_option =  (fnet_nd6_option_header_t *) ((unsigned char *)icmp6_packet + nd_option_offset) ;
            /* Validation RFC4861 (7.1.1). All included options have a length that is greater than zero.
             */
            if(nd_option->length == 0)
            { 		
                goto DROP;
            }
            
            /* Handle Source link-layer address option only.
             */
            if((nd_option->type == FNET_ND6_OPTION_SOURCE_LLA)
    			   && ( ((nd_option->length << 3) - sizeof(fnet_nd6_option_header_t)) >= netif->api->hw_addr_size) )
            {
                nd_option_slla = (fnet_nd6_option_lla_header_t *)nd_option; /* Source Link-layer Address option is found.*/
            }
            /* else, silently ignore any options they do not recognize
             * and continue processing the message.
             */
    				    
            nd_option_offset += (nd_option->length << 3);
        }

        if(nd_option_slla  != FNET_NULL)
        {
            /* Validation RFC4861 (7.1.1): If the IP source address is the 
             * unspecified address, there is no source link-layer address option in the message.
             * NOTE: RFC4861 (7):Duplicate Address Detection sends Neighbor Solicitation
             * messages with an unspecified source address targeting its own
             * "tentative" address.
             */
            if(FNET_IP6_ADDR_IS_UNSPECIFIED(src_ip))
            {
                /* If the Source Address is the unspecified address, the node MUST NOT
                 * create or update the Neighbor Cache entry.
                 */
                goto DROP;
            }
            else
            {
                fnet_nd6_neighbor_entry_t *neighbor_cache_entry;
                            
                /* RFC 48617.2.3: the recipient
                 * SHOULD create or update the Neighbor Cache entry for the IP Source
                 * Address of the solicitation. 
                 */
                neighbor_cache_entry = fnet_nd6_neighbor_cache_get(netif, src_ip);
                if(neighbor_cache_entry == FNET_NULL)
                {
                    /* If an entry does not already exist, the
                     * node SHOULD create a new one and set its reachability state to STALE
                     * as specified in Section 7.3.3.
                     * If a Neighbor Cache entry is created, the IsRouter flag SHOULD be set
                     * to FALSE. This will be the case even if the Neighbor Solicitation is
                     * sent by a router since the Neighbor Solicitation messages do not
                     * contain an indication of whether or not the sender is a router. In
                     * the event that the sender is a router, subsequent Neighbor
                     * Advertisement or Router Advertisement messages will set the correct
                     * IsRouter value. 
                     */
                     fnet_nd6_neighbor_cache_add(netif, src_ip, nd_option_slla->addr, FNET_ND6_NEIGHBOR_STATE_STALE);
                }
                else
                {
                    /* If an entry already exists, and the
                     * cached link-layer address differs from the one in the received Source
                     * Link-Layer option, the cached address should be replaced by the
                     * received address, and the entry�s reachability state MUST be set to
                     * STALE.
                     * If a Neighbor Cache entry already exists, its
                     * IsRouter flag MUST NOT be modified.
                     */
                    if(!FNET_ND6_LL_ADDR_ARE_EQUAL(nd_option_slla->addr, neighbor_cache_entry->ll_addr, netif->api->hw_addr_size))
                    {
                        FNET_ND6_LL_ADDR_COPY(nd_option_slla->addr, neighbor_cache_entry->ll_addr, netif->api->hw_addr_size);
                        neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
                    }
                    else
                    {
                        /* RFC4861: Appendix C 
                         */ //TBD ???
                        if(neighbor_cache_entry->state == FNET_ND6_NEIGHBOR_STATE_INCOMPLETE)
                            neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
                    }
                    
                    
                    /* - It sends any packets queued for the neighbor awaiting address
                     *   resolution.*/
                    fnet_nd6_neighbor_send_waiting_netbuf(netif, neighbor_cache_entry);
                }
            }
        }
                    
        /* Get Target Address Info, according tp Target Address of NS message.
         */
        target_if_addr_info = fnet_netif_get_ip6_addr_info(netif, &ns_packet->target_addr);
                    
        /* Check if we are the target.
         */
        if(target_if_addr_info != FNET_NULL)
        {
            /* Duplicate Address Detection (DAD) sends Neighbor Solicitation
             * messages with an unspecified source address targeting its own
             * "tentative" address.
             */
            if(FNET_IP6_ADDR_IS_UNSPECIFIED(src_ip))
            { /* === Duplicate Address Detection ===*/

                /* IP Destination address must be our solitation address.
                 */
                if(fnet_netif_is_my_ip6_solicited_multicast_addr(netif, dest_ip))
                {
                    if(target_if_addr_info->state != FNET_NETIF_IP6_ADDR_STATE_TENTATIVE)
                    {
                        /* If the source of the solicitation is the unspecified address, the
                         * node MUST set the Solicited flag to zero and multicast the
                         * advertisement to the all-nodes address.
                         */
                        fnet_nd6_neighbor_advertisement_send(netif, &target_if_addr_info->address /*ipsrc*/, (fnet_ip6_addr_t *)&fnet_ip6_addr_linklocal_allnodes/*ipdest*/, FNET_ND6_NA_FLAG_OVERRIDE);
                    }
                    else
                    {
                        fnet_nd6_dad_failed(netif , target_if_addr_info); /* => DAD is failed. */
                        /* MUST be silently discarded, if the Target Address is a "tentative" 
                         * address on which Duplicate Address Detection is being performed [ADDRCONF].*/
                    }

                }
                /* else DROP IT.
                 */
            }
            else if(    /* === Address Resolution === */
                    fnet_netif_is_my_ip6_solicited_multicast_addr(netif, dest_ip) 
                    ||  /* === Neighbor Unreachability Detection === */
                    ((FNET_IP6_ADDR_EQUAL(&ns_packet->target_addr, dest_ip))) )
            { 
                /*
                 * Sends a Neighbor Advertisement response.
                 */ 
                fnet_nd6_neighbor_advertisement_send(netif, &target_if_addr_info->address /*ipsrc*/, src_ip/*ipdest*/, FNET_ND6_NA_FLAG_SOLICITED | FNET_ND6_NA_FLAG_OVERRIDE);
            }
            /* else: Bad packet.*/

        }
        /* else: Not for us => Discarded.
         */
    }

DROP:    
    fnet_netbuf_free_chain(ip6_nb);
    fnet_netbuf_free_chain(nb);
}



/************************************************************************
* NAME: fnet_nd6_neighbor_advertisement_send
* DESCRIPTION: Sends an Neighbor Advertisement message.
*************************************************************************/
void fnet_nd6_neighbor_advertisement_send(fnet_netif_t *netif, fnet_ip6_addr_t *ipsrc, fnet_ip6_addr_t *ipdest, unsigned char na_flags)
{
    unsigned long                   na_packet_size;
    fnet_netbuf_t                   *na_nb;
    fnet_nd6_na_header_t            *na_packet;
    fnet_nd6_option_lla_header_t    *nd_option_tlla;

    na_packet_size = sizeof(fnet_nd6_na_header_t) + sizeof(fnet_nd6_option_header_t) + netif->api->hw_addr_size;
    
    if((na_nb = fnet_netbuf_new((int)na_packet_size, FNET_TRUE)) != 0)
    {
        /* Fill ICMP Header */
        na_packet = na_nb->data_ptr;
        na_packet->icmp6_header.type = FNET_ICMP6_TYPE_NEIGHBOR_ADVERTISEMENT;
        na_packet->icmp6_header.code = 0;
                                              
        /* NA header.*/
        na_packet->flag = na_flags;    /* Flag parameter.*/
        fnet_memset_zero( na_packet->_reserved, sizeof(na_packet->_reserved));  /* Set to zeros the reserved field.*/
        FNET_IP6_ADDR_COPY(ipsrc, &na_packet->target_addr);                     /* Set NA target address, the same as for NS.*/
                                                
        /* Fill Target Link-Layer Address option.*/
        nd_option_tlla = (fnet_nd6_option_lla_header_t *)((unsigned long)na_packet + sizeof(fnet_nd6_na_header_t));
        nd_option_tlla->option_header.type = FNET_ND6_OPTION_TARGET_LLA; /* Type. */
        nd_option_tlla->option_header.length = (unsigned char)((netif->api->hw_addr_size + sizeof(fnet_nd6_option_header_t))>>3); /* Option size devided by 8,*/
            
        if(fnet_netif_get_hw_addr(netif, nd_option_tlla->addr, netif->api->hw_addr_size) != FNET_OK)    /* Link-Layer Target address.*/
            goto DROP;    
        
        /* Send ICMPv6 message.*/
        fnet_icmp6_output( netif, ipsrc, ipdest, FNET_ND6_HOP_LIMIT, na_nb);
    }
    
    return;

DROP:
    fnet_netbuf_free_chain(na_nb);        
}

/************************************************************************
* NAME: fnet_nd6_router_solicitation_send
* DESCRIPTION: Sends an Router Solicitation Message.
*   Nodes send Neighbor Solicitations to request the link-layer address
*   of a target node while also providing their own link-layer address to
*   the target.
*************************************************************************/
void fnet_nd6_router_solicitation_send(fnet_netif_t *netif) 
{
    unsigned long                   rs_packet_size;
    fnet_netbuf_t                   *rs_nb;
    fnet_nd6_rs_header_t            *rs_packet;
    fnet_nd6_option_lla_header_t    *nd_option_slla;
    fnet_ip6_addr_t                 *ip_dest;
    fnet_ip6_addr_t                 *ip_src;

    /* Destination Address is the all-routers multicast address.*/
    ip_dest = (fnet_ip6_addr_t *)&fnet_ip6_addr_linklocal_allrouters;
    /* Choose source address.*/
    ip_src = (fnet_ip6_addr_t *)fnet_ip6_select_src_addr(netif, ip_dest);

    rs_packet_size = sizeof(fnet_nd6_rs_header_t) + ((ip_src == FNET_NULL /* no address */) ? 0:(sizeof(fnet_nd6_option_header_t) + netif->api->hw_addr_size));
 
    if((rs_nb = fnet_netbuf_new((int)rs_packet_size, FNET_TRUE)) != 0)
    {   
        /* Fill ICMP Header */
        rs_packet = rs_nb->data_ptr;
        rs_packet->icmp6_header.type = FNET_ICMP6_TYPE_ROUTER_SOLICITATION;
        rs_packet->icmp6_header.code = 0;
             
        /* Fill RS Header.*/
        fnet_memset_zero( rs_packet->_reserved, sizeof(rs_packet->_reserved));    /* Set to zeros the reserved field.*/

        /*
         * RFC4861: The link-layer address of the sender, if
         * known. MUST NOT be included if the Source Address
         * is the unspecified address. Otherwise, it SHOULD
         * be included on link layers that have addresses.
         */
        if(ip_src != FNET_NULL )
        {
                                           
            /* Fill Source link-layer address option.*/
            nd_option_slla = (fnet_nd6_option_lla_header_t *)((unsigned long)rs_packet + sizeof(fnet_nd6_rs_header_t));
            nd_option_slla->option_header.type = FNET_ND6_OPTION_SOURCE_LLA;    /* Type. */
            nd_option_slla->option_header.length = (unsigned char)((netif->api->hw_addr_size + sizeof(fnet_nd6_option_header_t))>>3); /* Option size devided by 8,*/
            
            if(fnet_netif_get_hw_addr(netif, nd_option_slla->addr, netif->api->hw_addr_size) != FNET_OK)    /* Link-Layer Target address.*/
                goto DROP;

            //TBD Padd if needed.%8
        }
        else
        {
            /* Source IP address is the
             * unspecified address.
             */
            ip_src = (fnet_ip6_addr_t *)&fnet_ip6_addr_any;
        }                                              
        
        /* Send ICMPv6 message.*/
        fnet_icmp6_output( netif, ip_src, ip_dest, FNET_ND6_HOP_LIMIT, rs_nb);
    }
    
    return;

DROP:
    fnet_netbuf_free_chain(rs_nb);    
}

/************************************************************************
* NAME: fnet_nd6_neighbor_advertisement_receive
* DESCRIPTION: Handles received Neighbor Advertisement message.
*************************************************************************/
void fnet_nd6_neighbor_advertisement_receive(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb)
{
    fnet_icmp6_header_t             *icmp6_packet = nb->data_ptr;
    unsigned long                   icmp6_packet_size = nb->total_length;
    fnet_nd6_na_header_t            *na_packet = (fnet_nd6_na_header_t *)icmp6_packet;
    fnet_nd6_option_lla_header_t    *nd_option_tlla = FNET_NULL;
    unsigned long                   nd_option_offset;
    fnet_netif_ip6_addr_t           *target_if_addr_info;
    fnet_ip6_header_t               *ip6_packet = ip6_nb->data_ptr;

    fnet_nd6_neighbor_entry_t       *neighbor_cache_entry;
    int                             is_solicited;
    int                             is_router;
    int                             is_override;
    int                             is_ll_addr_changed;;

	FNET_COMP_UNUSED_ARG(src_ip);
	
    /************************************************************
    * Validation.
    *************************************************************/	
    if(
        (icmp6_packet_size < sizeof(fnet_nd6_na_header_t))          /* Make sure that sizeof(ICMP6_NA_HEADER) <= RTCSPCB_SIZE(pcb).*/
        /* Validation RFC4861 (7.1.2).*/
        ||(ip6_packet->hop_limit != FNET_ND6_HOP_LIMIT)             /* The IP Hop Limit field has a value of 255.*/
        ||(icmp6_packet->code != 0)                                 /* ICMP Code is 0.*/ 
        ||FNET_IP6_ADDR_IS_MULTICAST(&na_packet->target_addr)       /* Target Address is not a multicast address. */
        ||( ((is_solicited=(na_packet->flag & FNET_ND6_NA_FLAG_SOLICITED)) != 0)
            && FNET_IP6_ADDR_IS_MULTICAST(dest_ip) )                /* If the IP Destination Address is a multicast 
                                                                     * address the Solicited flag is zero.*/
        )
    { 		
        goto DROP;
    }
    

    {   
        /************************************************************
         * Handle posible options.
         ************************************************************/
        nd_option_offset = sizeof(fnet_nd6_na_header_t);
        while(icmp6_packet_size > nd_option_offset + sizeof(fnet_nd6_option_header_t)) 
        {
            fnet_nd6_option_header_t *nd_option;
    				    
            nd_option =  (fnet_nd6_option_header_t *) ((unsigned char *)icmp6_packet + nd_option_offset) ;
            /* Validation RFC4861 (7.1.2). All included options have a length that is greater than zero.
             */
            if(nd_option->length == 0)
            { 		
                goto DROP;
            }
            
            /* Handle Target Link-Layer Address option only.
             */
            if((nd_option->type == FNET_ND6_OPTION_TARGET_LLA)
    			   && ( ((nd_option->length << 3) - sizeof(fnet_nd6_option_header_t)) >= netif->api->hw_addr_size) )
            {
                nd_option_tlla = (fnet_nd6_option_lla_header_t *)nd_option; /* Target Link-layer Address option is found.*/
            }
            /* else, silently ignore any options they do not recognize
             * and continue processing the message.
             */
    				    
            nd_option_offset += (nd_option->length << 3);
        }
        
        /* Get Target Address Info, according to Target Address of NA message.
        */
        target_if_addr_info = fnet_netif_get_ip6_addr_info(netif, &na_packet->target_addr);
        if(target_if_addr_info != FNET_NULL)
        {
            /* Duplicated address!!!!! */
            if(target_if_addr_info->state == FNET_NETIF_IP6_ADDR_STATE_TENTATIVE) 
                fnet_nd6_dad_failed(netif, target_if_addr_info); /* => DAD is failed. */
            
            goto DROP;
        }

        /* A Neighbor Advertisements that passes the validity checks is called a
         * "valid advertisement".
         */
        
        /* Extract flag values.*/
        is_router = (na_packet->flag & FNET_ND6_NA_FLAG_ROUTER)? 1:0;
        is_override = na_packet->flag & FNET_ND6_NA_FLAG_OVERRIDE;
        
        
        /************************************************************
         * Handle NA message.
         ************************************************************/
        
        /* RFC4861 7.2.5: Neighbor Cache is searched for the target�s entry.
         */
        neighbor_cache_entry = fnet_nd6_neighbor_cache_get(netif, &na_packet->target_addr); 
        if(neighbor_cache_entry == FNET_NULL)
        {
            /* If no entry exists, the advertisement SHOULD be silently discarded.
             */
            goto DROP;
        }
        
        /* If the target�s Neighbor Cache entry is in the INCOMPLETE state.*/
        if(neighbor_cache_entry->state == FNET_ND6_NEIGHBOR_STATE_INCOMPLETE)
        {
            /* If the link layer has addresses and no Target Link-Layer Address option is
             * included, the receiving node SHOULD silently discard the received
             * advertisement.*/
            if(nd_option_tlla == FNET_NULL)
            {
                goto DROP;
            }
            /* Otherwise, the receiving node performs the following
             * steps:
             * - It records the link-layer address in the Neighbor Cache entry.
             */
            FNET_ND6_LL_ADDR_COPY(nd_option_tlla->addr, neighbor_cache_entry->ll_addr, netif->api->hw_addr_size);
              
            /* - If the advertisement�s Solicited flag is set, the state of the
             *   entry is set to REACHABLE; otherwise, it is set to STALE.
             */
            if (is_solicited)
            {
                neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_REACHABLE;
                /* Reset Reachable Timer. */
                neighbor_cache_entry->state_time = fnet_timer_ms();
            }
            else
            {
                neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;            
            }
              
            /* - It sets the IsRouter flag in the cache entry based on the Router
             *   flag in the received advertisement.
             */
            neighbor_cache_entry->is_router = is_router;
             
            /* - It sends any packets queued for the neighbor awaiting address
             *   resolution.
             */ 
             fnet_nd6_neighbor_send_waiting_netbuf(netif, neighbor_cache_entry);
        }
        else
        {
            /* If the target�s Neighbor Cache entry is in any state other than
             * INCOMPLETE.
             */
             if( nd_option_tlla )
             {
                /* If supplied link-layer address differs. */
                is_ll_addr_changed = !FNET_ND6_LL_ADDR_ARE_EQUAL(nd_option_tlla->addr, neighbor_cache_entry->ll_addr, netif->api->hw_addr_size);
             }
             else
             {
                is_ll_addr_changed = 0;  
             }
             
             /* I. If the Override flag is clear and the supplied link-layer address
              *     differs from that in the cache.
              */
             if((is_override == 0) && is_ll_addr_changed)
             {
                /* a. If the state of the entry is REACHABLE, set it to STALE, but
                 *    do not update the entry in any other way.
                 */
                 if(neighbor_cache_entry->state == FNET_ND6_NEIGHBOR_STATE_REACHABLE) 
                 {
                    neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
                 }
                /* b. Otherwise, the received advertisement should be ignored and
                 *    MUST NOT update the cache.
                 */
                 goto DROP;
             }
             /* II. If the Override flag is set, or the supplied link-layer address
              *     is the same as that in the cache, or no Target Link-Layer Address
              *     option was supplied, the received advertisement MUST update the
              *     Neighbor Cache entry as follows:
              */
             else
             {
                /* - The link-layer address in the Target Link-Layer Address option
                 *   MUST be inserted in the cache (if one is supplied and differs
                 *   from the already recorded address).
                 */
                if(nd_option_tlla)
                    FNET_ND6_LL_ADDR_COPY(nd_option_tlla->addr, neighbor_cache_entry->ll_addr, netif->api->hw_addr_size);
                
                /* - If the Solicited flag is set, the state of the entry MUST be
                 *   set to REACHABLE. 
                 */
                if(is_solicited) 
                {
                    neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_REACHABLE;
                    /* Reset Reachable Timer.*/
                    neighbor_cache_entry->state_time = fnet_timer_ms();
                }
                /* If the Solicited flag is zero and the linklayer
                 *   address was updated with a different address, the state
                 *   MUST be set to STALE. Otherwise, the entry�s state remains
                 *   unchanged.
                 */
                else if(is_ll_addr_changed)
                {
                    neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
                }
                /* - The IsRouter flag in the cache entry MUST be set based on the
                 *   Router flag in the received advertisement. In those cases
                 *   where the IsRouter flag changes from TRUE to FALSE as a result
                 *   of this update, the node MUST remove that router from the
                 *   Default Router List and update the Destination Cache entries
                 *   for all destinations using that neighbor as a router as
                 *   specified in Section 7.3.3.
                 */
                if((neighbor_cache_entry->is_router) && (is_router == 0))
                {
                    fnet_nd6_router_list_del(neighbor_cache_entry);
                }                 
            }
        }
    }
    
DROP:    
    fnet_netbuf_free_chain(ip6_nb);
    fnet_netbuf_free_chain(nb);   
}

/************************************************************************
* NAME: nd6_router_advertisement_receive
* DESCRIPTION: Handles received Router Advertisement message.
*************************************************************************/
void fnet_nd6_router_advertisement_receive(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb)
{
    fnet_icmp6_header_t             *icmp6_packet = nb->data_ptr;
    unsigned long                   icmp6_packet_size = nb->total_length;
    fnet_nd6_ra_header_t            *ra_packet = (fnet_nd6_ra_header_t *)icmp6_packet;
    fnet_nd6_option_lla_header_t    *nd_option_slla = FNET_NULL;
    fnet_nd6_option_mtu_header_t    *nd_option_mtu = FNET_NULL;
    fnet_nd6_option_prefix_header_t *nd_option_prefix[FNET_CFG_ND6_PREFIX_LIST_SIZE];
    int                             prefix_index;
    unsigned long                   nd_option_offset;
    fnet_ip6_header_t               *ip6_packet = (fnet_ip6_header_t *)ip6_nb->data_ptr;
    int                             i;
    fnet_nd6_neighbor_entry_t       *neighbor_cache_entry;

    
    FNET_COMP_UNUSED_ARG(dest_ip);
	
    /************************************************************
    * Validation of Router Advertisement Message.
    *************************************************************/	
    if(
        (icmp6_packet_size < sizeof(fnet_nd6_ra_header_t))  
        /* Validation RFC4861 (6.1.2).*/
        ||(ip6_packet->hop_limit != FNET_ND6_HOP_LIMIT)     /* The IP Hop Limit field has a value of 255.*/
        ||(icmp6_packet->code != 0)                         /* ICMP Code is 0.*/ 
        || !FNET_IP6_ADDR_IS_LINKLOCAL(src_ip)              /* MUST be the link-local address.*/  
        )
    { 		
        goto DROP;
    }
    
    {   
        /************************************************************
         * Handle posible options.
         ************************************************************
         * The contents of any defined options that are not specified
         * to be used with Router Advertisement messages MUST be
         * ignored and the packet processed as normal. The only defined 
         * options that may appear are the Source Link-Layer Address, 
         * Prefix Information and MTU options.
         ************************************************************/
        nd_option_offset = sizeof(fnet_nd6_ra_header_t);
        prefix_index = 0;
        while(icmp6_packet_size > nd_option_offset + sizeof(fnet_nd6_option_header_t)) 
        {
            fnet_nd6_option_header_t *nd_option;
    				    
            nd_option =  (fnet_nd6_option_header_t *) ((unsigned char *)icmp6_packet + nd_option_offset) ;
            /* Validation RFC4861 (6.1.2). All included options have a length that is greater than zero.
             */
            if(nd_option->length == 0)
            { 		
                goto DROP;
            }
            
            /* The advertisement is scanned for valid options.
             */
            
            /* Source Link-Layer Address option.*/
            if( (nd_option->type == FNET_ND6_OPTION_SOURCE_LLA)
    			   && ( ((nd_option->length << 3) - sizeof(fnet_nd6_option_header_t)) >= netif->api->hw_addr_size) )
            {
                nd_option_slla = (fnet_nd6_option_lla_header_t *)nd_option; /* Target Link-layer Address option is found.*/
            }
            /* MTU option */
            else if( (nd_option->type == FNET_ND6_OPTION_MTU)
    			   && ((nd_option->length << 3) >= sizeof(fnet_nd6_option_mtu_header_t)) )
            {
                 nd_option_mtu = (fnet_nd6_option_mtu_header_t *)nd_option; /* MTU option is found.*/
            }
            /* Prefix option */
            else if( (nd_option->type == FNET_ND6_OPTION_PREFIX)
    			   && ((nd_option->length << 3) >= sizeof(fnet_nd6_option_prefix_header_t)) )
            {
                 /* Note that there can be multiple "Prefix information" options included in a router advertisement.*/ 
                 if(prefix_index < FNET_CFG_ND6_PREFIX_LIST_SIZE)
                 {
                    nd_option_prefix[prefix_index] = (fnet_nd6_option_prefix_header_t *)nd_option; /* Prefix information.*/
                    prefix_index++;
                 }
            }
            /* else, silently ignore any options they do not recognize
             * and continue processing the message.
             */
    				    
            nd_option_offset += (nd_option->length << 3);
        }
        
        /************************************************************
         * Set parameters.
         ************************************************************/
        
        /* RFC4861 6.3.4: If the received Cur Hop Limit value is non-zero, the host SHOULD set
         * its CurHopLimit variable to the received value.
         */
        if(ra_packet->cur_hop_limit != 0)
        {
            netif->nd6_if_ptr->cur_hop_limit = ra_packet->cur_hop_limit;
        }
        
        /* RFC4861 6.3.4: If the received Reachable Time value is non-zero, the host SHOULD set
         * its BaseReachableTime variable to the received value.
         */
        if(ra_packet->reachable_time != 0)
        {
            netif->nd6_if_ptr->reachable_time = fnet_ntohl(ra_packet->reachable_time);
        }        
        
        /* RFC4861 6.3.4:The RetransTimer variable SHOULD be copied from the Retrans Timer
         * field, if the received value is non-zero.
         */
        if(fnet_ntohl(ra_packet->retrans_timer) != 0)
        {
            netif->nd6_if_ptr->retrans_timer = fnet_ntohl(ra_packet->retrans_timer);
        } 

        /* RFC4861: Hosts SHOULD copy the option�s value
         * into LinkMTU so long as the value is greater than or equal to the
         * minimum link MTU [IPv6] and does not exceed the maximum LinkMTU value
         * specified in the link-type-specific document.
         */
        if(nd_option_mtu) 
        {
            unsigned long mtu = fnet_ntohl(nd_option_mtu->mtu);
            
            if(mtu < netif->mtu)
            { 
                if(mtu < FNET_IP6_DEFAULT_MTU)
                    netif->nd6_if_ptr->mtu = FNET_IP6_DEFAULT_MTU;
                else
                    netif->nd6_if_ptr->mtu =  mtu; 
                      
            #if FNET_CFG_IP6_PMTU_DISCOVERY 
                if(netif->pmtu /* If PMTU is enabled.*/ &&
                   (netif->pmtu > netif->nd6_if_ptr->mtu))
                {
                    fnet_netif_set_pmtu(netif, netif->nd6_if_ptr->mtu);
                }
            #endif
            }                
        }
        
        /* RFC4861: If the advertisement contains a Source Link-Layer Address
         * option, the link-layer address SHOULD be recorded in the Neighbor
         * Cache entry for the router (creating an entry if necessary) and the
         * IsRouter flag in the Neighbor Cache entry MUST be set to TRUE. */
        neighbor_cache_entry = fnet_nd6_neighbor_cache_get(netif, src_ip);
        if(nd_option_slla)
        {
            if(neighbor_cache_entry == FNET_NULL)
            /* Creating an entry if necessary */
            {
                neighbor_cache_entry = fnet_nd6_neighbor_cache_add(netif, src_ip, nd_option_slla->addr, FNET_ND6_NEIGHBOR_STATE_STALE);
            }
            else
            {
                /* If a cache entry already exists and is
                 * updated with a different link-layer address, the reachability state
                 * MUST also be set to STALE.*/
                if( !FNET_ND6_LL_ADDR_ARE_EQUAL(nd_option_slla->addr, neighbor_cache_entry->ll_addr, netif->api->hw_addr_size) )
                {
                    FNET_ND6_LL_ADDR_COPY(nd_option_slla->addr, neighbor_cache_entry->ll_addr, netif->api->hw_addr_size);
                    neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
                }
            }
            
            /* Sends any packets queued for the neighbor awaiting address resolution.
             */ 
            fnet_nd6_neighbor_send_waiting_netbuf(netif, neighbor_cache_entry);
        }
        else
        {
            if(neighbor_cache_entry == FNET_NULL)
            {
                neighbor_cache_entry = fnet_nd6_neighbor_cache_add(netif, src_ip, FNET_NULL, FNET_ND6_NEIGHBOR_STATE_STALE);
            }
        }
        
        /*
         * RFC4861: If the address is already present in the host�s Default Router
         * List as a result of a previously received advertisement, reset
         * its invalidation timer to the Router Lifetime value in the newly
         * received advertisement.
         * If the address is already present in the host�s Default Router
         * List and the received Router Lifetime value is zero, immediately
         * time-out the entry.
         */
        fnet_nd6_router_list_add( neighbor_cache_entry, fnet_ntohs(ra_packet->router_lifetime));
        
        /*************************************************************
         * Prcess Prefix options.
         *************************************************************/
        if(prefix_index > 0)
        {
            for(i=0; i<prefix_index; i++)
            {
                fnet_nd6_prefix_entry_t *prefix_entry;

            #if FNET_CFG_DEBUG_IP6
                char numaddr[FNET_IP6_ADDR_STR_SIZE]={0};
                fnet_inet_ntop(AF_INET6, (char*)&nd_option_prefix[i]->prefix, numaddr, sizeof(numaddr));  
                
                fnet_println("Prefix[%d]= %s \n", i, numaddr);
            #endif /* FNET_CFG_DEBUG_IP6 */

                 
                /* RFC4861: A router SHOULD NOT send a prefix
                 * option for the link-local prefix and a host SHOULD
                 * ignore such a prefix option.*/
                if( !FNET_IP6_ADDR_IS_LINKLOCAL(&nd_option_prefix[i]->prefix)
                /* RFC4861: The value of Prefered Lifetime field MUST NOT exceed
                 * the Valid Lifetime field to avoid preferring
                 * addresses that are no longer valid.*/
                 && (fnet_ntohl(nd_option_prefix[i]->prefered_lifetime) <= fnet_ntohl(nd_option_prefix[i]->valid_lifetime)) )
                {
                    /*************************************************************
                     * Prefix Information option with the on-link flag set. 
                     *************************************************************/
                    if( (nd_option_prefix[i]->flag & FNET_ND6_OPTION_FLAG_L) == FNET_ND6_OPTION_FLAG_L )
                    {
                        prefix_entry = fnet_nd6_prefix_list_get(netif, &nd_option_prefix[i]->prefix);
                        
                        /* RFC4861: If the prefix is not already present in the Prefix List, and the
                         * Prefix Information option�s Valid Lifetime field is non-zero,
                         * create a new entry for the prefix and initialize its
                         * invalidation timer to the Valid Lifetime value in the Prefix
                         * Information option.*/
                        if(prefix_entry == FNET_NULL)
                        {
                            if(fnet_ntohl(nd_option_prefix[i]->valid_lifetime) != 0)
                            {
                                /* Create a new entry for the prefix.*/
                                fnet_nd6_prefix_list_add(netif, &nd_option_prefix[i]->prefix, 
                                                             nd_option_prefix[i]->prefix_length, 
                                                             fnet_ntohl(nd_option_prefix[i]->valid_lifetime));
                            }
                        }
                        else
                        {
                            /* RFC4861: If the prefix is already present in the host�s Prefix List as
                             * the result of a previously received advertisement, reset its
                             * invalidation timer to the Valid Lifetime value in the Prefix
                             * Information option. If the new Lifetime value is zero, time-out
                             * the prefix immediately.*/
                            if(fnet_ntohl(nd_option_prefix[i]->valid_lifetime) != 0)
                            {
                                /* Reset Timer. */
                                prefix_entry->lifetime = fnet_ntohl(nd_option_prefix[i]->valid_lifetime);
                                prefix_entry->creation_time = fnet_timer_seconds();
                            }
                            else
                            {   
                                /* Time-out the prefix immediately. */
                                fnet_nd6_prefix_list_del(prefix_entry);
                            }
                        }
                    }
                    
                    /*************************************************************
                     * Stateless Address Autconfiguration.
                     *************************************************************/
                     
                    /* For each Prefix-Information option in the Router Advertisement:*/
                    if( ((nd_option_prefix[i]->flag & FNET_ND6_OPTION_FLAG_A) == FNET_ND6_OPTION_FLAG_A )
                        && (fnet_ntohl(nd_option_prefix[i]->valid_lifetime) != 0) )
                    {
                        int j;
                        fnet_netif_ip6_addr_t *addr_info = FNET_NULL;
                        
                        /* RFC4862 5.5.3:If the advertised prefix is equal to the prefix of an address
                         * configured by stateless autoconfiguration in the list, the
                         * preferred lifetime of the address is reset to the Preferred
                         * Lifetime in the received advertisement. */
                         
                        /* Lookup the address */
                        for(j = 0; j < FNET_NETIF_IP6_ADDR_MAX; j++)
                        {
                            if((netif->ip6_addr[j].state != FNET_NETIF_IP6_ADDR_STATE_NOT_USED)
                                && (netif->ip6_addr[j].type == FNET_NETIF_IP6_ADDR_TYPE_AUTOCONFIGURABLE)
                                && (fnet_ip6_addr_pefix_cmp(&nd_option_prefix[i]->prefix, &netif->ip6_addr[j].address, nd_option_prefix[i]->prefix_length) == FNET_TRUE) )
                            {
                                addr_info = &netif->ip6_addr[j];
                                break;
                            }    
                        }
                        
                        if( addr_info != FNET_NULL)
                        {
                            /* RFC4862 5.5.3: The specific action to
                             * perform for the valid lifetime of the address depends on the Valid
                             * Lifetime in the received advertisement and the remaining time to
                             * the valid lifetime expiration of the previously autoconfigured
                             * address. We call the remaining time "RemainingLifetime" in the
                             * following discussion:*/
                            if( (fnet_ntohl(nd_option_prefix[i]->valid_lifetime) > (60*60*2) /* 2 hours */)
                               ||( fnet_ntohl(nd_option_prefix[i]->valid_lifetime /*sec*/) >  ((addr_info->creation_time + addr_info->lifetime /*sec*/) - fnet_timer_seconds() ) ) 
                              )
                            {
                               /* 1. If the received Valid Lifetime is greater than 2 hours or
                                *    greater than RemainingLifetime, set the valid lifetime of the
                                *    corresponding address to the advertised Valid Lifetime. */
                                addr_info->lifetime = fnet_ntohl(nd_option_prefix[i]->valid_lifetime);
                            
                            }
                            else
                            {
                               /* 2. If RemainingLifetime is less than or equal to 2 hours, ignore
                                *    the Prefix Information option with regards to the valid
                                *    lifetime, unless the Router Advertisement from which this
                                *    option was obtained has been authenticated (e.g., via Secure
                                *    Neighbor Discovery [RFC3971]). If the Router Advertisement
                                *    was authenticated, the valid lifetime of the corresponding
                                *    address should be set to the Valid Lifetime in the received
                                *    option.
                                * 3. Otherwise, reset the valid lifetime of the corresponding
                                *    address to 2 hours. */
                                addr_info->lifetime = (60*60*2) /* 2 hours */;
                            }
                            addr_info->creation_time = fnet_timer_seconds();
                        }
                        else
                        {
                        	/* RFC4862 5.5.3: If the prefix advertised is not equal to the prefix of an
                             * address configured by stateless autoconfiguration already in the
                             * list of addresses associated with the interface (where "equal"
                             * means the two prefix lengths are the same and the first prefixlength
                             * bits of the prefixes are identical), and if the Valid
                             * Lifetime is not 0, form an address (and add it to the list) by
                             * combining the advertised prefix with an interface identifier. */
                        	fnet_netif_bind_ip6_addr_prv (netif, &nd_option_prefix[i]->prefix, FNET_NETIF_IP6_ADDR_TYPE_AUTOCONFIGURABLE, 
                        	                                fnet_ntohl(nd_option_prefix[i]->valid_lifetime), nd_option_prefix[i]->prefix_length);
                        }
                    }
                    /* else. RFC4862: If the Autonomous flag is not set, silently ignore the Prefix Information option.*/
                }
            } /* for() end. */
        } /* End processing prefix options.*/

    }

    
DROP:    
    fnet_netbuf_free_chain(ip6_nb);
    fnet_netbuf_free_chain(nb);    
}

/************************************************************************
* NAME: fnet_nd6_redirect_receive
* DESCRIPTION: Handles received Redirect message.
*************************************************************************/
void fnet_nd6_redirect_receive(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb)
{
    fnet_icmp6_header_t             *icmp6_packet = nb->data_ptr;
    unsigned long                   icmp6_packet_size = nb->total_length;
    fnet_nd6_rd_header_t            *rd_packet = (fnet_nd6_rd_header_t *)icmp6_packet;
    fnet_nd6_option_lla_header_t    *nd_option_tlla = FNET_NULL;
    unsigned long                   nd_option_offset;
    fnet_ip6_header_t               *ip6_packet = (fnet_ip6_header_t *)ip6_nb->data_ptr;
    fnet_nd6_neighbor_entry_t       *neighbor_cache_entry;
    fnet_ip6_addr_t *               target_addr = &rd_packet->target_addr;
    fnet_ip6_addr_t *               destination_addr = &rd_packet->destination_addr;

    
    FNET_COMP_UNUSED_ARG(dest_ip);
	
    /************************************************************
    * Validation of Redirect Message RFC4861 (8.1).
    *************************************************************/	
    if(
        (icmp6_packet_size < sizeof(fnet_nd6_rd_header_t))  
        ||(ip6_packet->hop_limit != FNET_ND6_HOP_LIMIT)     /* The IP Hop Limit field has a value of 255.*/
        ||(icmp6_packet->code != 0)                         /* ICMP Code is 0.*/ 
        ||!FNET_IP6_ADDR_IS_LINKLOCAL(src_ip)               /* IP Source Address is a link-local address.*/ 
        /* The ICMP Destination Address field in the redirect message does
         * not contain a multicast address.*/
        ||(FNET_IP6_ADDR_IS_MULTICAST(destination_addr))   
        /* The ICMP Target Address is either a link-local address (when
         * redirected to a router) or the same as the ICMP Destination
         * Address (when redirected to the on-link destination). */        
        ||(!FNET_IP6_ADDR_IS_LINKLOCAL(target_addr) 
           && !FNET_IP6_ADDR_EQUAL(destination_addr, target_addr))
        /* The IP source address of the Redirect is the same as the current
         * first-hop router for the specified ICMP Destination Address.*/   
        ||(fnet_nd6_is_firsthop_router(netif, src_ip) == FNET_FALSE)   
        )
    { 		
        goto DROP;
    }

  
    /************************************************************
     * Handle posible options.
     ************************************************************
     * The contents of any defined options that are not specified
     * to be used with Redirect messages MUST be
     * ignored and the packet processed as normal. The only defined 
     * options that may appear are the Target Link-Layer Address, 
     * Prefix Information and MTU options.
     ************************************************************/
    nd_option_offset = sizeof(fnet_nd6_rd_header_t);
    while(icmp6_packet_size > nd_option_offset + sizeof(fnet_nd6_option_header_t)) 
    {
        fnet_nd6_option_header_t *nd_option;
    				    
        nd_option =  (fnet_nd6_option_header_t *) ((unsigned char *)icmp6_packet + nd_option_offset) ;
        /* Validation RFC4861 (8.1). All included options have a length that is greater than zero.
         */
        if(nd_option->length == 0)
        { 		
            goto DROP;
        }
            
        /* The advertisement is scanned for valid options.
         */
            
        /* Target Link-Layer Address option.*/
        if( (nd_option->type == FNET_ND6_OPTION_TARGET_LLA)
            && ( ((nd_option->length << 3) - sizeof(fnet_nd6_option_header_t)) >= netif->api->hw_addr_size) )
        {
            nd_option_tlla = (fnet_nd6_option_lla_header_t *)nd_option; /* Target Link-layer Address option is found.*/
        }
        /* else, silently ignore any options they do not recognize
         * and continue processing the message.
         */
    				    
        nd_option_offset += (nd_option->length << 3);
    }
        
    /* RFC4861: If the redirect contains a Target Link-Layer Address option, the host
     * either creates or updates the Neighbor Cache entry for the target. */
    neighbor_cache_entry = fnet_nd6_neighbor_cache_get(netif, target_addr);
    if(nd_option_tlla)
    {
        if(neighbor_cache_entry == FNET_NULL)
        {
            /*  If a Neighbor Cache entry is
             * created for the target, its reachability state MUST be set to STALE
             * as specified in Section 7.3.3. */
            neighbor_cache_entry = fnet_nd6_neighbor_cache_add(netif, target_addr, nd_option_tlla->addr, FNET_ND6_NEIGHBOR_STATE_STALE);
        }
        else
        {
            /* If a cache entry already existed and
             * it is updated with a different link-layer address, its reachability
             * state MUST also be set to STALE. */
            if( !FNET_ND6_LL_ADDR_ARE_EQUAL(nd_option_tlla->addr, neighbor_cache_entry->ll_addr, netif->api->hw_addr_size) )
            {
                FNET_ND6_LL_ADDR_COPY(nd_option_tlla->addr, neighbor_cache_entry->ll_addr, netif->api->hw_addr_size);
                neighbor_cache_entry->state = FNET_ND6_NEIGHBOR_STATE_STALE;
            }
            /* else. If the link-layer address is the
             * same as that already in the cache, the cache entry�s state remains
             * unchanged. */
        }
            
        /* Sends any packets queued for the neighbor awaiting address resolution.
         */ 
        fnet_nd6_neighbor_send_waiting_netbuf(netif, neighbor_cache_entry);
    }
    else
    {
        if(neighbor_cache_entry == FNET_NULL)
        {
            /*  If a Neighbor Cache entry is
             * created for the target, its reachability state MUST be set to STALE
             * as specified in Section 7.3.3. */                
            neighbor_cache_entry = fnet_nd6_neighbor_cache_add(netif, target_addr, FNET_NULL, FNET_ND6_NEIGHBOR_STATE_STALE);
        }
    }

    /* If the Target Address is not the same
     * as the Destination Address, the host MUST set IsRouter to TRUE for
     * the target.*/
    if( !FNET_IP6_ADDR_EQUAL(destination_addr, target_addr) )
    {
        neighbor_cache_entry->is_router = FNET_TRUE;
            
        /* Add to redirect table.*/
        fnet_nd6_redirect_table_add(netif, destination_addr, target_addr);
    }

    
DROP:    
    fnet_netbuf_free_chain(ip6_nb);
    fnet_netbuf_free_chain(nb);    
}

/************************************************************************
* NAME: fnet_nd6_rd_start
* DESCRIPTION: Start the Router Discovery for the interface.
*************************************************************************/
void fnet_nd6_rd_start(fnet_netif_t *netif)
{
    netif->nd6_if_ptr->rd_transmit_counter = FNET_ND6_MAX_RTR_SOLICITATIONS-1;
    netif->nd6_if_ptr->rd_time = fnet_timer_ms();  /* Save send time.*/
    fnet_nd6_router_solicitation_send(netif);
    
    //TBD Randomise first send.
}

/************************************************************************
* NAME: fnet_nd6_rd_timer
* DESCRIPTION: Timer routine used by Router Discovery.
*************************************************************************/
static void fnet_nd6_rd_timer(fnet_netif_t *netif)
{
    if(netif->nd6_if_ptr->rd_transmit_counter > 0)
    {
        if(fnet_nd6_default_router_get(netif) == FNET_NULL)
        /* Router is not found yet.*/
        {
            if( fnet_timer_get_interval( netif->nd6_if_ptr->rd_time, fnet_timer_ms()) > FNET_ND6_RTR_SOLICITATION_INTERVAL)
            {
                netif->nd6_if_ptr->rd_transmit_counter--;
                netif->nd6_if_ptr->rd_time = fnet_timer_ms();  /* Save send time.*/
                fnet_nd6_router_solicitation_send(netif);
            }
        }
        else
        /* Router is found.*/
        {
        #if FNET_CFG_DEBUG_IP6
            fnet_println("RD: ROUTER is FOUND!!\n");
        #endif /* FNET_CFG_DEBUG_IP6 */
            netif->nd6_if_ptr->rd_transmit_counter = 0; /* Stop timer.*/
        }
    }
}

/************************************************************************
* NAME: fnet_nd6_dad_start
* RETURS: None.
* DESCRIPTION: Start the Duplicate Address Detection for the address.
*************************************************************************/
void fnet_nd6_dad_start(fnet_netif_t *netif , fnet_netif_ip6_addr_t *addr_info)
{
#if FNET_CFG_ND6_DAD_TRANSMITS > 0 
    if(addr_info && (addr_info->state == FNET_NETIF_IP6_ADDR_STATE_TENTATIVE))
    {
        /* To check an address, a node sends RTCSCFG_ND6_DAD_TRANSMITS Neighbor
         * Solicitations, each separated by 1 second(TBD)..
         */
        addr_info->dad_transmit_counter = FNET_CFG_ND6_DAD_TRANSMITS; 
        addr_info->state_time = fnet_timer_ms();  /* Save state time.*/
        fnet_nd6_neighbor_solicitation_send(netif, FNET_NULL, FNET_NULL, &addr_info->address);
    }
#endif /* FNET_CFG_ND6_DAD_TRANSMITS */
}

/************************************************************************
* NAME: fnet_nd6_dad_timer
* RETURS: None.
* DESCRIPTION: Timer routine used by Duplicate Address Detection.
*************************************************************************/
static void fnet_nd6_dad_timer(fnet_netif_t *netif )
{
#if FNET_CFG_ND6_DAD_TRANSMITS > 0 
    int i;
    fnet_netif_ip6_addr_t *addr_info;
    
    for(i=0; i<FNET_NETIF_IP6_ADDR_MAX; i++)
    {
        addr_info = &netif->ip6_addr[i];
       
        if(addr_info->state == FNET_NETIF_IP6_ADDR_STATE_TENTATIVE)
        {
            if( fnet_timer_get_interval(addr_info->state_time, fnet_timer_ms()) > netif->nd6_if_ptr->retrans_timer )
            {
                addr_info->dad_transmit_counter--;
                if(addr_info->dad_transmit_counter == 0)
                {
                    /* DAD succeeded, for this address.
                     * Once an address is determined to be unique,
                     * it may be assigned to an interface.*/
                    addr_info->state = FNET_NETIF_IP6_ADDR_STATE_PREFERRED;
                    
                #if FNET_CFG_DEBUG_IP6
                    {
                        char numaddr[FNET_IP6_ADDR_STR_SIZE]={0};
                        fnet_inet_ntop(AF_INET6, (char*)&addr_info->address, numaddr, sizeof(numaddr));
                        fnet_println("%s is PREFERED NOW\n", numaddr);
                    }
                #endif /* FNET_CFG_DEBUG_IP6 */
                }
                else
                {
                    addr_info->state_time = fnet_timer_ms();
                    fnet_nd6_neighbor_solicitation_send(netif, FNET_NULL, FNET_NULL, &addr_info->address);
                }
            }
        }
    }
#endif /* RTCSCFG_ND6_DAD_TRANSMITS */    
}

/************************************************************************
* NAME: nd6_dad_failed
* RETURS: None.
* DESCRIPTION: Called when DAD is failed.
*************************************************************************/
static void fnet_nd6_dad_failed(fnet_netif_t *netif , fnet_netif_ip6_addr_t *addr_info)
{
    fnet_ip6_addr_t             if_ip6_address;

#if FNET_CFG_DEBUG_IP6
    {
        char numaddr[FNET_IP6_ADDR_STR_SIZE]={0};
        fnet_inet_ntop(AF_INET6, (char*)&addr_info->address, numaddr, sizeof(numaddr));  
        fnet_println("%s DAD is FAILED!!!!\n", numaddr);
    } 
#endif /* FNET_CFG_DEBUG_IP6 */
    /* RFC 4862: */
    /* 5.4.5. When Duplicate Address Detection Fails */
    /* Just remove address, or TBD mark it as dupicate.*/
    fnet_netif_unbind_ip6_addr_prv ( netif, addr_info);

    /* If the address is a link-local address formed from an interface
     * identifier based on the hardware address, which is supposed to be
     * uniquely assigned (e.g., EUI-64 for an Ethernet interface), IP
     * operation on the interface SHOULD be disabled.
     * In this case, the IP address duplication probably means duplicate
     * hardware addresses are in use, and trying to recover from it by
     * configuring another IP address will not result in a usable network.*/
    fnet_memset_zero(&if_ip6_address.addr[0], sizeof(fnet_ip6_addr_t));
    if_ip6_address.addr[0] = 0xFE;
    if_ip6_address.addr[1] = 0x80;
    fnet_netif_set_ip6_addr_autoconf(netif, &if_ip6_address);
        
    if(FNET_IP6_ADDR_EQUAL(&if_ip6_address, &addr_info->address))
        netif->nd6_if_ptr->ip6_disabled = FNET_TRUE;
}



//////////////// For DEBUG needs only. //////////////////////////////////

/************************************************************************
* NAME: fnet_nd6_debug_print_prefix_list
* RETURS: None.
* DESCRIPTION: Prints prefix list. For DEBUG needs only.
*************************************************************************/
void fnet_nd6_debug_print_prefix_list( fnet_netif_t *netif )
{
    fnet_nd6_if_t           *nd6_if;
    int                     i;
    fnet_nd6_prefix_entry_t *entry ;
    
    if(netif == FNET_NULL)
        netif = fnet_netif_get_by_number(0); /* Get the first one.*/
    
    nd6_if = netif->nd6_if_ptr;
    
    if (nd6_if)
    {
        fnet_println("Prefix List:");
        for(i=0; i<FNET_ND6_PREFIX_LIST_SIZE; i++)
        {
            if(nd6_if->prefix_list[i].state != FNET_ND6_PREFIX_STATE_NOTUSED)
            {
                char numaddr[FNET_IP6_ADDR_STR_SIZE]={0};
                entry = &nd6_if->prefix_list[i];
                
                fnet_inet_ntop(AF_INET6, (char*)entry->prefix.addr, numaddr, sizeof(numaddr)); 
                
                /* Print entry.*/
                fnet_println("[%d] %s /%d State(%d) LifeTime(%d)[%d]", i, numaddr, entry->prefix_length, entry->state,  entry->lifetime, entry->creation_time); 
            }
        }

    }
}

#endif /* FNET_CFG_IP6 */

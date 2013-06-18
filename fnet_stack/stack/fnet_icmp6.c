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
* @file fnet_icmp6.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.22.0
*
* @brief ICMP protocol implementation.
*
***************************************************************************/
#include "fnet_config.h"

#if FNET_CFG_IP6  

#include "fnet_icmp6.h"
#include "fnet_loop.h"
#include "fnet_timer.h"
#include "fnet_prot.h"
#include "fnet_socket.h"
#include "fnet_socket_prv.h"
#include "fnet_checksum.h"


/************************************************************************
*     Function Prototypes
*************************************************************************/
void fnet_icmp6_output( fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, unsigned char hop_limit, fnet_netbuf_t *nb );
static void fnet_icmp6_input(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb);

/************************************************************************
* Protocol API structure.
************************************************************************/
fnet_prot_if_t fnet_icmp6_prot_if =
{
    0,                      /* Pointer to the head of the protocol's socket list.*/
    AF_INET6,               /* Address domain family.*/
    SOCK_UNSPEC,            /* Socket type used for.*/
    FNET_IP_PROTOCOL_ICMP6, /* Protocol number.*/   
    0,                      /* Protocol initialization function.*/
    0,                      /* Protocol release function.*/
#if FNET_CFG_IP4    
    0,                      /* Protocol input function, from IPv4.*/
#endif    
    0,                      /* Protocol input control function.*/     
    0,                      /* protocol drain function.*/
    fnet_icmp6_input,       /* Protocol input function, from IPv6.*/
    0                       /* Socket API */
};

/************************************************************************
* NAME: fnet_icmp6_input
*
* DESCRIPTION: ICMPv6 input function.
*************************************************************************/
static void fnet_icmp6_input(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb)
{
    fnet_icmp6_header_t     *hdr;
    fnet_netbuf_t           *tmp_nb;
    unsigned short          sum;


    if((netif != 0) && (nb != 0))
    {
        /* The header must reside in contiguous area of memory. */
        if((tmp_nb = fnet_netbuf_pullup(nb, sizeof(fnet_icmp6_header_t))) == 0) 
        {
            goto DISCARD;
        }

        nb = tmp_nb;

        hdr = nb->data_ptr;

    /* Drop Multicast loopback.*/
    #if FNET_CFG_LOOPBACK     
        if ((netif == FNET_LOOP_IF) && FNET_IP6_ADDR_IS_MULTICAST(dest_ip))
        {
            goto DISCARD;
        }
    #endif /* FNET_CFG_LOOPBACK */
   
        /* Verify the checksum. */
        sum = fnet_checksum_pseudo_start( nb, FNET_HTONS((unsigned short)FNET_IP_PROTOCOL_ICMP6), (unsigned short)nb->total_length );
        sum = fnet_checksum_pseudo_end( sum, (char *)src_ip, (char *)dest_ip, sizeof(fnet_ip6_addr_t) );
        if(sum)
            goto DISCARD;

    	/************************************************************
    	* Process incoming ICMPv6 packets.
    	*************************************************************/
    	switch (hdr->type) 
    	{
            /**************************
             * Neighbor Solicitation.
             **************************/
            case FNET_ICMP6_TYPE_NEIGHBOR_SOLICITATION:
                fnet_nd6_neighbor_solicitation_receive(netif, src_ip, dest_ip, nb, ip6_nb);
                break;
            /**************************
             * Neighbor Advertisemnt.
             **************************/            
            case FNET_ICMP6_TYPE_NEIGHBOR_ADVERTISEMENT:
                fnet_nd6_neighbor_advertisement_receive(netif, src_ip, dest_ip, nb, ip6_nb);
                break;                
            /**************************
             * Router Advertisemnt.
             **************************/
            case FNET_ICMP6_TYPE_ROUTER_ADVERTISEMENT:
                fnet_nd6_router_advertisement_receive(netif, src_ip, dest_ip, nb, ip6_nb);
                break; 
            /**************************
             * Router Advertisemnt.
             **************************/
            case FNET_ICMP6_TYPE_REDIRECT:
                fnet_nd6_redirect_receive(netif, src_ip, dest_ip, nb, ip6_nb);
                break; 
            /**************************
             * Echo Request.
             * RFC4443 4.1: Every node MUST implement an ICMPv6 Echo responder function that
             * receives Echo Requests and originates corresponding Echo Replies.             
             **************************/
            case FNET_ICMP6_TYPE_ECHO_REQ:
                hdr->type = FNET_ICMP6_TYPE_ECHO_REPLY;
                
                /* RFC4443: the source address of the reply MUST be a unicast 
                 * address belonging to the interface on which 
                 * the Echo Request message was received.*/
                if(FNET_IP6_ADDR_IS_MULTICAST(dest_ip))
                     dest_ip = FNET_NULL;
                
                fnet_icmp6_output(netif, dest_ip/*ipsrc*/, src_ip/*ipdest*/, 0, nb);   //MD
                fnet_netbuf_free_chain(ip6_nb);
                break;   
      #if FNET_CFG_IP6_PMTU_DISCOVERY 
           /**************************
            * Packet Too Big Message.
            **************************/
            case FNET_ICMP6_TYPE_PACKET_TOOBIG:     
                if(netif->pmtu) /* If PMTU is enabled for the interface.*/
                {
                    unsigned long           pmtu;
                    fnet_icmp6_err_header_t *icmp6_err = nb->data_ptr;
                    
                    /* The header must reside in contiguous area of memory. */
                    if((tmp_nb = fnet_netbuf_pullup(nb, sizeof(fnet_icmp6_err_header_t))) == 0) 
                    {
                        goto DISCARD;
                    }

                    nb = tmp_nb;
                    
                    /* RFC 1981.Upon receipt of such a
                     * message, the source node reduces its assumed PMTU for the path based
                     * on the MTU of the constricting hop as reported in the Packet Too Big
                     * message.*/
                    pmtu = fnet_ntohl(icmp6_err->data); 

                    /* A node MUST NOT increase its estimate of the Path MTU in response to
                     * the contents of a Packet Too Big message. */
                    if(netif->pmtu > pmtu)        
                    {
                        fnet_netif_set_pmtu(netif, pmtu);
                    }                
                }
                goto DISCARD;
      #endif                       
            default:
                goto DISCARD;
        }                
    }
    else
    {
DISCARD:
        fnet_netbuf_free_chain(ip6_nb);
        fnet_netbuf_free_chain(nb);
    }
}

/************************************************************************
* NAME: fnet_icmp6_output
*
* DESCRIPTION: ICMPv6 output function.
*************************************************************************/
void fnet_icmp6_output( fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, unsigned char hop_limit, fnet_netbuf_t *nb )
{
    fnet_icmp6_header_t                     *hdr = nb->data_ptr;
    FNET_COMP_PACKED_VAR unsigned short     *checksum_p;
    
    /* Checksum calculation.*/
    hdr->checksum = 0;

    hdr->checksum = fnet_checksum_pseudo_start(nb, FNET_HTONS((unsigned short)FNET_IP_PROTOCOL_ICMP6), (unsigned short)nb->total_length);
    checksum_p = &hdr->checksum;

    fnet_ip6_output(netif, src_ip, dest_ip, FNET_IP_PROTOCOL_ICMP6, hop_limit, nb, checksum_p);
}

/************************************************************************
* NAME: fnet_icmp6_error
*
* DESCRIPTION: Sends ICMPv6 error message.
*************************************************************************/
void fnet_icmp6_error( fnet_netif_t *netif, unsigned char type, unsigned char code, unsigned long param, fnet_netbuf_t *origin_nb )
{
    fnet_ip6_header_t       *ip6_header;
    fnet_icmp6_err_header_t *icmp6_err_header;
    fnet_ip6_addr_t         *src_ip; 
    fnet_ip6_addr_t         *dest_ip; 
    fnet_netbuf_t           *nb_header;   
    
    if(origin_nb)
    {
        /* Limit to FNET_IP6_DEFAULT_MTU. */
        if(origin_nb->total_length > (FNET_IP6_DEFAULT_MTU - (sizeof(fnet_icmp6_err_header_t) + sizeof(fnet_ip6_header_t))  ))
            fnet_netbuf_trim(&origin_nb, (int)(FNET_IP6_DEFAULT_MTU - (sizeof(fnet_icmp6_err_header_t) + sizeof(fnet_ip6_header_t)) - origin_nb->total_length));
        
        ip6_header = origin_nb->data_ptr;
        
        src_ip = &ip6_header->source_addr;
        dest_ip = &ip6_header->destination_addr;
        
        /*******************************************************************
         * RFC 4443:
         * (e) An ICMPv6 error message MUST NOT be originated as a result of
         * receiving the following:
         *******************************************************************/
        /* (e.1) An ICMPv6 error message. */ 
        /* (e.2) An ICMPv6 REDIRECT message [IPv6-DISC].*/
        if (ip6_header->next_header == FNET_IP_PROTOCOL_ICMP6) //TBD Extension header case.
        {
            /* Make sure the packet has at least a 'TYPE' field */
            if (ip6_header->length == 0)
            {
                goto FREE_NB;
            }
            
            icmp6_err_header = (fnet_icmp6_err_header_t *)((unsigned char *)ip6_header + sizeof(fnet_ip6_header_t));
            if (FNET_ICMP6_TYPE_IS_ERROR(icmp6_err_header->icmp6_header.type) || (icmp6_err_header->icmp6_header.type == FNET_ICMP6_TYPE_REDIRECT ) )
            {
                goto FREE_NB;
            }
        }

        /*
         * (e.3) A packet destined to an IPv6 multicast address. (There are
         * two exceptions to this rule: (1) the Packet Too Big Message
         * (Section 3.2) to allow Path MTU discovery to work for IPv6
         * multicast, and (2) the Parameter Problem Message, Code 2
         * (Section 3.4) reporting an unrecognized IPv6 option (see
         * Section 4.2 of [IPv6]) that has the Option Type highestorder
         * two bits set to 10).
         * (e.4) A packet sent as a link-layer multicast (the exceptions
         * from e.3 apply to this case, too).     
         */
         if((FNET_IP6_ADDR_IS_MULTICAST(dest_ip) /*||(orig_pcb->TYPE & RTCSPCB_TYPE_MULTICAST)*/) 
            && !( (type == FNET_ICMP6_TYPE_PACKET_TOOBIG) 
                || ((type == FNET_ICMP6_TYPE_PARAM_PROB) && (code == FNET_ICMP6_CODE_PP_OPTION))) )
         {
            goto FREE_NB;
         }
         else
         {
            if(FNET_IP6_ADDR_IS_MULTICAST(dest_ip))
                /* We may not use multicast address as source. Get real source address. */
                dest_ip = (fnet_ip6_addr_t *)fnet_ip6_select_src_addr(netif, src_ip /*dest*/); 
         
         }    
             
        /*
         * (e.5) A packet sent as a link-layer broadcast (the exceptions
         *  from e.3 apply to this case, too).
         */
        //if (orig_pcb->TYPE & RTCSPCB_TYPE_BROADCAST)
        //{
        //    return;
        //}         

        /*
         * (e.6) A packet whose source address does not uniquely identify a
         * single node -- e.g., the IPv6 Unspecified Address, an IPv6
         * multicast address, or an address known by the ICMP message
         * originator to be an IPv6 anycast address.
         */
        if(FNET_IP6_ADDR_IS_MULTICAST(src_ip) || FNET_IP6_ADDR_EQUAL(&fnet_ip6_addr_any, src_ip))
        {
            goto FREE_NB;
        }
     
        /* Construct ICMPv6 error header.*/
        if((nb_header = fnet_netbuf_new((sizeof(fnet_icmp6_err_header_t)), FNET_FALSE)) == 0)
            goto FREE_NB;     
        
        icmp6_err_header = nb_header->data_ptr;
        
        icmp6_err_header->icmp6_header.type = type;
        icmp6_err_header->icmp6_header.code = code;        
        icmp6_err_header->data = fnet_htonl(param); 
        
        origin_nb = fnet_netbuf_concat(nb_header, origin_nb);
        
        fnet_icmp6_output( netif, dest_ip/*ipsrc*/, src_ip/*ipdest*/, 0, origin_nb);        
            
        return;

FREE_NB:
        fnet_netbuf_free_chain(origin_nb);        
    
    }

}
#endif /* FNET_CFG_IP6 */                    


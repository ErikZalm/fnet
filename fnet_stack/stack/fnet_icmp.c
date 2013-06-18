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
* @file fnet_icmp.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.36.0
*
* @brief ICMP protocol implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_IP4 

#include "fnet_icmp.h"
#include "fnet_timer.h"
#include "fnet_prot.h"
#include "fnet_socket.h"
#include "fnet_socket_prv.h"
#include "fnet_checksum.h"


/************************************************************************
*     Function Prototypes
*************************************************************************/
static void fnet_icmp_input( fnet_netif_t *netif, fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip4_nb);
static void fnet_icmp_output( fnet_netif_t *netif, fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip, fnet_netbuf_t *nb );
                        
#if FNET_CFG_DEBUG_TRACE_ICMP
void fnet_icmp_trace(char *str, fnet_icmp_header_t *icmpp_hdr);
#else
#define fnet_icmp_trace(str, icmp_hdr)
#endif


/************************************************************************
* Protocol API structure.
************************************************************************/
fnet_prot_if_t fnet_icmp_prot_if =
{
    0,                      /* Pointer to the head of the protocol's socket list.*/
    AF_INET,                /* Address domain family.*/
    SOCK_UNSPEC,            /* Socket type used for.*/
    FNET_IP_PROTOCOL_ICMP,  /* Protocol number.*/   
    0,                      /* Protocol initialization function.*/
    0,                      /* Protocol release function.*/
    fnet_icmp_input,        /* Protocol input function.*/
    0,                      /* Protocol input control function.*/     
    0,                      /* protocol drain function.*/
#if FNET_CFG_IP6    
    0,                      /* Protocol IPv6 input function.*/
#endif /* FNET_CFG_IP6 */      
    0                       /* Socket API */
};
                        

/************************************************************************
* NAME: fnet_icmp_input
*
* DESCRIPTION: ICMP input function.
*************************************************************************/
static void fnet_icmp_input( fnet_netif_t *netif, fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip4_nb )
{
    fnet_icmp_header_t      *hdr;
    fnet_icmp_err_header_t  *hdr_err;
    fnet_prot_notify_t      prot_cmd;
    fnet_prot_if_t          *protocol;
    fnet_netbuf_t           *tmp_nb;
    
    fnet_netbuf_free_chain(ip4_nb);
    
    if((netif != 0) && (nb != 0) )
    {
        if((tmp_nb = fnet_netbuf_pullup(nb, sizeof(fnet_icmp_header_t))) == 0) /* The header must reside in contiguous area of memory. */
        {
            goto DISCARD;
        }

        nb = tmp_nb;

        hdr = nb->data_ptr;

        if(
        #if FNET_CFG_CPU_ETH_HW_RX_PROTOCOL_CHECKSUM || FNET_CFG_CPU_ETH_HW_TX_PROTOCOL_CHECKSUM
            ((nb->flags & FNET_NETBUF_FLAG_HW_PROTOCOL_CHECKSUM) == 0) &&
        #endif
            fnet_checksum(nb, (int)nb->total_length) 
            || fnet_ip_addr_is_broadcast(src_ip, netif)
            || FNET_IP4_ADDR_IS_MULTICAST(src_ip))
        {
            goto DISCARD;
        }
        
        fnet_icmp_trace("RX", hdr); 
        
        switch(hdr->type)
        {
            /**************************
             * ICMP Request Processing
             **************************/
            case FNET_ICMP_ECHO:
                if((nb->total_length < sizeof(fnet_icmp_echo_header_t)) ||
                /* An ICMP Echo Request destined to an IP broadcast or IP
                * multicast address MAY be silently discarded.(RFC1122)*/
                fnet_ip_addr_is_broadcast(dest_ip, netif) || FNET_IP4_ADDR_IS_MULTICAST(dest_ip))
                {
                    goto DISCARD;
                }

                hdr->type = FNET_ICMP_ECHOREPLY;

                fnet_icmp_output(netif, dest_ip, src_ip, nb);
                break;
#if 0 /* Optional functionality.*/                
            /************************
             * Time Stamp Query 
             ************************/
            case FNET_ICMP_TSTAMP:
 
                /* The header must reside in contiguous area of memory. */
                if((tmp_nb = fnet_netbuf_pullup(nb, sizeof(fnet_icmp_timestamp_header_t))) == 0)
                {
                    goto DISCARD;
                }

                nb = tmp_nb;

                hdr = nb->data_ptr;

                hdr->type = FNET_ICMP_TSTAMPREPLY;
                /* All times are in milliseconds since the stack timer start modulo 1 day. 
                * The high-order bit is set as the time value is recorded in nonstandard units. */
                ((fnet_icmp_timestamp_header_t *)hdr)->receive_timestamp
                    = fnet_htonl(((fnet_timer_ticks() * FNET_TIMER_PERIOD_MS) % (24 * 60 * 60 * 1000)) | (0x80000000));

                dest_ip = netif->ip4_addr.address;

                fnet_icmp_output(netif, dest_ip, src_ip, nb);
                break;
            /************************
             * Address Mask Query
             ************************/
            case FNET_ICMP_MASKREQ:
                /* The header must reside in contiguous area of memory*/
                if((tmp_nb = fnet_netbuf_pullup(nb, sizeof(fnet_icmp_mask_header_t))) == 0) 
                {
                    goto DISCARD;
                }

                nb = tmp_nb;

                hdr = nb->data_ptr;

                hdr->type = FNET_ICMP_MASKREPLY;

                ((fnet_icmp_mask_header_t *)hdr)->mask = netif->ip4_addr.subnetmask;

                dest_ip = netif->ip4_addr.address;

                fnet_icmp_output(netif, dest_ip, src_ip, nb);
                break;
#endif                
            /**************************
             * ICMP Error Processing
             **************************/
            case FNET_ICMP_UNREACHABLE:
                switch(hdr->code)
                {
                    case FNET_ICMP_UNREACHABLE_NET:           /* net unreachable */
                    case FNET_ICMP_UNREACHABLE_NET_UNKNOWN:   /* unknown net */
                    case FNET_ICMP_UNREACHABLE_NET_PROHIB:    /* prohibited access */
                    case FNET_ICMP_UNREACHABLE_TOSNET:        /* bad tos for net */
                        prot_cmd = FNET_PROT_NOTIFY_UNREACH_NET;
                        break;

                    case FNET_ICMP_UNREACHABLE_HOST:          /* host unreachable */
                    case FNET_ICMP_UNREACHABLE_HOST_UNKNOWN:  /* unknown host */
                    case FNET_ICMP_UNREACHABLE_ISOLATED:      /* src host isolated */
                    case FNET_ICMP_UNREACHABLE_HOST_PROHIB:   /* ditto */
                    case FNET_ICMP_UNREACHABLE_TOSHOST:       /* bad tos for host */
                        prot_cmd = FNET_PROT_NOTIFY_UNREACH_HOST;
                        break;

                    case FNET_ICMP_UNREACHABLE_PROTOCOL:      /* protocol unreachable */
                        prot_cmd = FNET_PROT_NOTIFY_UNREACH_PROTOCOL;
                        break;

                    case FNET_ICMP_UNREACHABLE_PORT:          /* port unreachable */
                        prot_cmd = FNET_PROT_NOTIFY_UNREACH_PORT;
                        break;

                    case FNET_ICMP_UNREACHABLE_SRCFAIL:       /* source route failed */
                        prot_cmd = FNET_PROT_NOTIFY_UNREACH_SRCFAIL;
                        break;

                    case FNET_ICMP_UNREACHABLE_NEEDFRAG:      /* fragmentation needed and DF set*/
                        prot_cmd = FNET_PROT_NOTIFY_MSGSIZE;
                        break;

                    default:
                        goto DISCARD;
                }
                goto NOTIFY_PROT;

            case FNET_ICMP_TIMXCEED:
                switch(hdr->code)
                {
                    case FNET_ICMP_TIMXCEED_INTRANS:          /* time to live exceeded in transit (ttl==0)*/
                        prot_cmd = FNET_PROT_NOTIFY_TIMXCEED_INTRANS;
                        break;

                    case FNET_ICMP_TIMXCEED_REASS:            /* fragment reassembly time exceeded (ttl==0)*/
                        prot_cmd = FNET_PROT_NOTIFY_TIMXCEED_REASS;
                        break;

                    default:
                        goto DISCARD;
                }

                goto NOTIFY_PROT;

            case FNET_ICMP_PARAMPROB:                       /* Parameter Problem Message.*/
                if(hdr->code > 1)
                    goto DISCARD;

                prot_cmd = FNET_PROT_NOTIFY_PARAMPROB;
                goto NOTIFY_PROT;

            case FNET_ICMP_SOURCEQUENCH:                    /* Source Quench Message; packet lost, slow down.*/
                if(hdr->code)
                    goto DISCARD;

                prot_cmd = FNET_PROT_NOTIFY_QUENCH;

    NOTIFY_PROT: /* Protocol notification.*/
                if(nb->total_length < (sizeof(fnet_icmp_err_header_t) + 8))
                    goto DISCARD;

                if(nb->total_length > (sizeof(fnet_icmp_err_header_t) + 8))
                    fnet_netbuf_trim(&nb, (int)((sizeof(fnet_icmp_err_header_t) + 8) - nb->total_length));

                if((tmp_nb = fnet_netbuf_pullup(nb, (int)nb->total_length)) == 0) /* The header must reside in contiguous area of memory.*/
                {
                    goto DISCARD;
                }

                nb = tmp_nb;

                hdr_err = (fnet_icmp_err_header_t *)nb->data_ptr;
              

                if((protocol = fnet_prot_find(AF_INET, SOCK_UNSPEC, hdr_err->ip.protocol)) != 0)
                {
                    if(protocol->prot_control_input)
                        protocol->prot_control_input(prot_cmd, (void *)(&hdr_err->ip));
                }
                goto DISCARD;

            /************************
             * Ignore others
             ************************/
            /*
            case FNET_ICMP_REDIRECT:
            case FNET_ICMP_ECHOREPLY:
            case FNET_ICMP_ROUTERADVERT:
            case FNET_ICMP_ROUTERSOLICIT:
            case FNET_ICMP_TSTAMPREPLY:
            case FNET_ICMP_IREQREPLY:
            case FNET_ICMP_MASKREPLY:*/
            default:
                goto DISCARD;
                
        }/* switch(hdr->type) */
    }
    else
    {
DISCARD:
        fnet_netbuf_free_chain(nb);
    }
}

/************************************************************************
* NAME: fnet_icmp_output
*
* DESCRIPTION: ICMP output function.
*************************************************************************/
static void fnet_icmp_output( fnet_netif_t *netif, fnet_ip4_addr_t src_ip, 
                                fnet_ip4_addr_t dest_ip, fnet_netbuf_t *nb )
{
    fnet_icmp_header_t *hdr = nb->data_ptr;

    hdr->checksum = 0;
#if FNET_CFG_CPU_ETH_HW_TX_PROTOCOL_CHECKSUM 
    if( netif 
            && (netif->features & FNET_NETIF_FEATURE_HW_TX_PROTOCOL_CHECKSUM)
            && (fnet_ip_will_fragment(netif, nb->total_length) == FNET_FALSE) /* Fragmented packets are not inspected.*/  ) 
    {
        nb->flags |= FNET_NETBUF_FLAG_HW_PROTOCOL_CHECKSUM;
    }
    else
#endif
        hdr->checksum = fnet_checksum(nb, (int)nb->total_length);

    fnet_ip_output(netif, src_ip, dest_ip, FNET_IP_PROTOCOL_ICMP, FNET_ICMP_TOS, FNET_ICMP_TTL, nb, 0, 0, 0);
}

/************************************************************************
* NAME: fnet_icmp_error
*
* DESCRIPTION: Sends ICMP error message.
*************************************************************************/
void fnet_icmp_error( fnet_netif_t *netif, unsigned char type, 
                        unsigned char code, fnet_netbuf_t *nb )
{
    fnet_ip_header_t        *ipheader;
    fnet_netbuf_t           *nb_header;
    fnet_icmp_err_header_t  *icmpheader;
    fnet_ip4_addr_t          source_addr;
    fnet_ip4_addr_t          destination_addr;
    

    if(nb)
    {
        ipheader = nb->data_ptr;
        
        source_addr = ipheader->source_addr;
        destination_addr = ipheader->desination_addr;

        /* Do not send error if not the first fragment of message (RFC1122)*/
        if((FNET_IP_HEADER_GET_OFFSET(ipheader)) ||
            /* Do not send error on ICMP error messages*/
            ((ipheader->protocol == FNET_IP_PROTOCOL_ICMP)
                 && (!FNET_ICMP_IS_QUERY_TYPE(((fnet_icmp_header_t *)((unsigned char *)nb->data_ptr
                      + (FNET_IP_HEADER_GET_HEADER_LENGTH(ipheader) << 2)))->type))) 
            /* Do not send error on a datagram whose source address does not define a single
             * host -- e.g., a zero address, a loopback address, a
             * broadcast address, a multicast address, or a Class E
             * address.*/
            ||(fnet_ip_addr_is_broadcast(source_addr, netif)
            || FNET_IP4_ADDR_IS_MULTICAST(source_addr) 
            || FNET_IP4_CLASS_E(source_addr))
            /* Do not send error on a datagram destined to an IP broadcast or IP multicast address*/
            ||(fnet_ip_addr_is_broadcast(destination_addr, netif)
            || FNET_IP4_ADDR_IS_MULTICAST(destination_addr)
             /* Do not send error on datagram sent as a link-layer broadcast or multicast.*/
            ||(nb->flags & FNET_NETBUF_FLAG_BROADCAST) || (nb->flags & FNET_NETBUF_FLAG_MULTICAST))
        )
        {
            goto FREE_NB;
        }

        /* Construct ICMP error header*/
        if((nb_header = fnet_netbuf_new((sizeof(fnet_icmp_err_header_t)
                                             - sizeof(fnet_ip_header_t)), FNET_FALSE)) == 0)
            goto FREE_NB;

        icmpheader = nb_header->data_ptr;
        icmpheader->fields.unused = 0;

        if(type == FNET_ICMP_PARAMPROB)
        {
            icmpheader->fields.ptr = fnet_htons((unsigned short)code);
            code = 0;
        }
        else if((type == FNET_ICMP_PARAMPROB) && (code == FNET_ICMP_UNREACHABLE_NEEDFRAG) && netif)
            icmpheader->fields.mtu = fnet_htons((unsigned short)netif->mtu);

        icmpheader->header.type = type;
        icmpheader->header.code = code;

        if(((FNET_IP_HEADER_GET_HEADER_LENGTH(ipheader) << 2) + 8) < nb->total_length)
            fnet_netbuf_trim(&nb, (int)(((FNET_IP_HEADER_GET_HEADER_LENGTH(ipheader) << 2) + 8) - nb->total_length));

        nb = fnet_netbuf_concat(nb_header, nb);

        fnet_icmp_output(netif, destination_addr, source_addr, nb);

        return;

FREE_NB:
        fnet_netbuf_free_chain(nb);
    }
}

/************************************************************************
* NAME: fnet_icmp_trace
*
* DESCRIPTION: Prints an ICMP header. For debug needs only.
*************************************************************************/
#if FNET_CFG_DEBUG_TRACE_ICMP
void fnet_icmp_trace(char *str, fnet_icmp_header_t *icmp_hdr)
{

    fnet_printf(FNET_SERIAL_ESC_FG_GREEN"%s", str); /* Print app-specific header.*/
    fnet_println("[ICMP header]"FNET_SERIAL_ESC_FG_BLACK);
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(Type)     "FNET_SERIAL_ESC_FG_BLUE"%3u"FNET_SERIAL_ESC_FG_BLACK" |(Code)     %3u |(Cheksum)               0x%04x |",
                    icmp_hdr->type,
                    icmp_hdr->code,
                    fnet_ntohs(icmp_hdr->checksum));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    
}
#endif /* FNET_CFG_DEBUG_TRACE_ICMP */

#endif /* FNET_CFG_IP4 */

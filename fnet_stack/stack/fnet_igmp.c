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
* @file fnet_igmp.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.23.0
*
* @brief IGMPv1 protocol implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_IGMP & FNET_CFG_IP4

#include "fnet_igmp.h"
#include "fnet_timer.h"
#include "fnet_prot.h"
#include "fnet_checksum.h"

/* TBD Random delay timers */

/************************************************************************
*     Definitions
*************************************************************************/
#define FNET_IGMP_TTL                   (1)             /* IP time-to-live is 1.*/
#define FNET_IGMP_TOS                   (IP_TOS_NORMAL) /* The TOS of IGMP messages.*/

/************************************************************************
*     Function Prototypes
*************************************************************************/
static void fnet_igmp_input( fnet_netif_t *netif, fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip4_nb);
                        
#if FNET_CFG_DEBUG_TRACE_IGMP
    static void fnet_igmp_trace(char *str, fnet_igmp_header_t *icmpp_hdr);
#else
    #define fnet_igmp_trace(str, icmp_hdr)
#endif


/************************************************************************
* Protocol API structure.
************************************************************************/
fnet_prot_if_t fnet_igmp_prot_if =
{
    0,                      /* Pointer to the head of the protocol's socket list.*/
    AF_INET,                /* Address domain family.*/
    SOCK_UNSPEC,            /* Socket type used for.*/
    FNET_IP_PROTOCOL_IGMP,  /* Protocol number.*/   
    0,                      /* Protocol initialization function.*/
    0,                      /* Protocol release function.*/
    fnet_igmp_input,        /* Protocol input function.*/
    0,                      /* Protocol input control function.*/     
    0,                      /* protocol drain function.*/
#if FNET_CFG_IP6    
    0,                      /* Protocol IPv6 input function.*/
#endif /* FNET_CFG_IP6 */    
    0                       /* Socket API */
};

/************************************************************************
* NAME: fnet_igmp_input
*
* DESCRIPTION: IGMP input function.
*************************************************************************/
static void fnet_igmp_input( fnet_netif_t *netif, fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip4_nb)
{
    fnet_igmp_header_t *hdr;
    fnet_netbuf_t *tmp_nb;
    int i;
    
    FNET_COMP_UNUSED_ARG(dest_ip);
    FNET_COMP_UNUSED_ARG(src_ip);
    
    fnet_netbuf_free_chain(ip4_nb);

    if((netif != 0) && (nb != 0) )
    {
        /* The header must reside in contiguous area of memory. */
        if((tmp_nb = fnet_netbuf_pullup(nb, sizeof(fnet_igmp_header_t))) == 0) 
        {
            goto DISCARD;
        }

        nb = tmp_nb;

        hdr = nb->data_ptr;
        
        /* RFC2236 To be valid, the Query message
         * must be at least 8 octets long, have a correct IGMP
         * checksum.
         */
        if(fnet_checksum(nb, (int)nb->total_length)  )
        {
            goto DISCARD;
        }
        
        fnet_igmp_trace("RX", hdr); 

        /**************************
        * IGMP QUERY Processing
        **************************/     
        if(hdr->type == IGMP_HEADER_TYPE_QUERY)
        {
            /* RFC2236: The group address in the IGMP header must either be zero (a General
             * Query) or a valid multicast group address (a Group-Specific Query).
             * A General Query applies to all memberships on the interface from
             * which the Query is received. A Group-Specific Query applies to
             * membership in a single group on the interface from which the Query
             * is received. Queries are ignored for memberships in the Non-Member
             * state.
             */
            if(hdr->group_addr == 0)
            /* General Query */
            {
                 /* Find all joined-groups for this interface.*/
                for(i=0; i < FNET_CFG_MULTICAST_MAX; i++)
                {
                    if((fnet_ip_multicast_list[i].user_counter > 0) && (fnet_ip_multicast_list[i].netif == netif))
                    {
                        /* Send report.*/
                        fnet_igmp_join(netif, fnet_ip_multicast_list[i].group_addr );
                    }
                }
            }
        #if FNET_CFG_IGMP_VERSION == 2                    
            else if(FNET_IP4_ADDR_IS_MULTICAST(hdr->group_addr))
            /* A Group-Specific Query.*/ 
            {
                /* Find specific group.*/
                for(i=0; i < FNET_CFG_MULTICAST_MAX; i++)
                {
                    if((fnet_ip_multicast_list[i].user_counter > 0) && (fnet_ip_multicast_list[i].netif == netif) && (fnet_ip_multicast_list[i].group_addr == hdr->group_addr))
                    {
                        /* Send report.*/
                        fnet_igmp_join(netif, fnet_ip_multicast_list[i].group_addr );
                        break;
                    }
                }
            }
        #endif /* FNET_CFG_IGMP_VERSION */                
            /* esle wrong */
        }
        /************************
         * Ignore others
         ************************/
    }

DISCARD:
    fnet_netbuf_free_chain(nb);
}

/************************************************************************
* NAME: fnet_igmp_join
*
* DESCRIPTION: Sends Host Membership Reports.
*************************************************************************/
void fnet_igmp_join( fnet_netif_t *netif, fnet_ip4_addr_t  group_addr )
{
    fnet_netbuf_t *nb_header;
    fnet_igmp_header_t *igmp_header;
    
    /* Construct IGMP header*/
    if((nb_header = fnet_netbuf_new(sizeof(fnet_igmp_header_t), FNET_FALSE)) != 0)
    {
        igmp_header = nb_header->data_ptr;
        /* Type.*/ 
#if FNET_CFG_IGMP_VERSION == 1        
        igmp_header->type = IGMP_HEADER_TYPE_REPORT_V1;
#else /* FNET_CFG_IGMP_VERSION == 2 */
        igmp_header->type = IGMP_HEADER_TYPE_REPORT_V2;
#endif           
        igmp_header->max_resp_time = 0;             /* Unused field, zeroed when sent, ignored when received.*/
        igmp_header->group_addr = group_addr ;      /* Group address field.*/   
        
        
        igmp_header->checksum = 0;
        igmp_header->checksum = fnet_checksum(nb_header, (int)nb_header->total_length);

        /* RFC 1112: A Report is sent with an IP destination address equal to the
         * host group address being reported, and with an IP time-to-live of 1.
         */
        fnet_ip_output(netif, INADDR_ANY, group_addr /*dest_addr*/, FNET_IP_PROTOCOL_IGMP, FNET_IGMP_TOS, FNET_IGMP_TTL, nb_header, 0, 0, 0);
    }
}


/************************************************************************
* NAME: fnet_igmp_leave
*
* DESCRIPTION: Sends a Leave Group message.
*************************************************************************/
void fnet_igmp_leave( fnet_netif_t *netif, fnet_ip4_addr_t  group_addr )
{
#if FNET_CFG_IGMP_VERSION == 2 
    fnet_netbuf_t *nb_header;
    fnet_igmp_header_t *igmp_header;
    fnet_ip4_addr_t dest_ip = FNET_IP4_ADDR_INIT(224, 0, 0, 2); /* All-routers multicast group.*/
    
    /* Construct IGMP header*/
    if((nb_header = fnet_netbuf_new(sizeof(fnet_igmp_header_t), FNET_FALSE)) != 0)
    {
        /*
         * When a host leaves a multicast group, if it was the last host to
         * reply to a Query with a Membership Report for that group, it SHOULD
         * send a Leave Group message to the all-routers multicast group (224.0.0.2).
         */

        igmp_header = nb_header->data_ptr;
        
        igmp_header->type = IGMP_HEADER_TYPE_LEAVE_GROUP; /* Type.*/ 
        igmp_header->max_resp_time = 0;             /* Unused field, zeroed when sent, ignored when received.*/
        igmp_header->group_addr = group_addr ;      /* Group address field.*/   
        
        
        igmp_header->checksum = 0;
        igmp_header->checksum = fnet_checksum(nb_header, (int)nb_header->total_length);

        /* RFC 1112: A Report is sent with an IP destination address equal to the
         * host group address being reported, and with an IP time-to-live of 1.
         */
        
        fnet_ip_output(netif, INADDR_ANY, dest_ip /*dest_addr*/, FNET_IP_PROTOCOL_IGMP, FNET_IGMP_TOS, FNET_IGMP_TTL, nb_header, 0, 0, 0);
    }
#endif /* FNET_CFG_IGMP_VERSION */
}

/************************************************************************
* NAME: fnet_igmp_trace
*
* DESCRIPTION: Prints an IGMP header. For debug needs only.
*************************************************************************/
#if FNET_CFG_DEBUG_TRACE_IGMP
static void fnet_igmp_trace(char *str, fnet_igmp_header_t *igmp_hdr)
{
    char ip_str[FNET_IP4_ADDR_STR_SIZE];

    fnet_printf(FNET_SERIAL_ESC_FG_GREEN"%s", str); /* Print app-specific header.*/
    fnet_println("[IGMP header]"FNET_SERIAL_ESC_FG_BLACK);
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(Type)    "FNET_SERIAL_ESC_FG_BLUE"0x%2x"FNET_SERIAL_ESC_FG_BLACK" |(Res Time) %3u |(Cheksum)               0x%04x |",
                    igmp_hdr->type,
                    igmp_hdr->max_resp_time,
                    fnet_ntohs(igmp_hdr->checksum));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");                    
    fnet_println("|(Group Addr)                                   "FNET_SERIAL_ESC_FG_BLUE"%15s"FNET_SERIAL_ESC_FG_BLACK" |",
                    fnet_inet_ntoa(*(struct in_addr *)(&igmp_hdr->group_addr), ip_str));                        
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    
}
#endif /* FNET_CFG_DEBUG_TRACE_IGMP */

#endif /* FNET_CFG_IGMP */

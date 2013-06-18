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
* @file fnet_ip6.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.34.0
*
* @brief IPv6 protocol implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_IP6

#include "fnet_ip6_prv.h"
#include "fnet_ip_prv.h"
#include "fnet_icmp.h"
#include "fnet_checksum.h"
#include "fnet_timer_prv.h"
#include "fnet_socket.h"
#include "fnet_socket_prv.h"
#include "fnet_isr.h"
#include "fnet_netbuf.h"
#include "fnet_netif_prv.h"
#include "fnet_prot.h"
#include "fnet_stdlib.h"
#include "fnet_loop.h"
#include "fnet_igmp.h"
#include "fnet_prot.h"
#include "fnet_raw.h"
    
/******************************************************************
* Ext. header handler results.
*******************************************************************/
typedef enum{
    FNET_IP6_EXT_HEADER_HANDLER_RESULT_EXIT,      /* Stop processing.*/ 
    FNET_IP6_EXT_HEADER_HANDLER_RESULT_NEXT,      /* Go to the next Ext. Header processing.*/
    FNET_IP6_EXT_HEADER_HANDLER_RESULT_TRANSPORT  /* Go to Transport Layer processing.*/
} fnet_ip6_ext_header_handler_result_t;

/******************************************************************
* Extension Header Handler structure
*******************************************************************/
typedef struct fnet_ip6_ext_header
{
    unsigned long type;                                                     /* Identifies the type of header. */
    fnet_ip6_ext_header_handler_result_t (*handler)(fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb); /* Extension header handler.*/
    
} fnet_ip6_ext_header_t;
 
/******************************************************************
* Function Prototypes
*******************************************************************/
static void fnet_ip6_netif_output(struct fnet_netif *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t* nb);
static int fnet_ip6_ext_header_process(fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb);
static fnet_ip6_ext_header_handler_result_t fnet_ip6_ext_header_handler_fragment_header(fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb);
static fnet_ip6_ext_header_handler_result_t fnet_ip6_ext_header_handler_routing_header(fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb);
static fnet_ip6_ext_header_handler_result_t fnet_ip6_ext_header_handler_options (fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb);
static fnet_ip6_ext_header_handler_result_t fnet_ip6_ext_header_handler_no_next_header (fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb);

#if FNET_CFG_IP6_FRAGMENTATION
    static void fnet_ip6_frag_list_add( fnet_ip6_frag_list_t ** head, fnet_ip6_frag_list_t *fl );
    static void fnet_ip6_frag_list_del( fnet_ip6_frag_list_t ** head, fnet_ip6_frag_list_t *fl );
    static void fnet_ip6_frag_add( fnet_ip6_frag_header_t ** head, fnet_ip6_frag_header_t *frag,  fnet_ip6_frag_header_t *frag_prev );
    static void fnet_ip6_frag_del( fnet_ip6_frag_header_t ** head, fnet_ip6_frag_header_t *frag );
    static void fnet_ip6_frag_list_free( fnet_ip6_frag_list_t *list );
    static fnet_netbuf_t *fnet_ip6_reassembly(fnet_netif_t *netif, fnet_netbuf_t ** nb_ptr, fnet_netbuf_t *ip6_nb, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip );
    static void fnet_ip6_timer(void *cookie);
#endif

/******************************************************************
* Extension Header Handler List
*******************************************************************/
fnet_ip6_ext_header_t fnet_ip6_ext_header_list[] = 
{
    {FNET_IP6_TYPE_NO_NEXT_HEADER,          fnet_ip6_ext_header_handler_no_next_header}
    ,{FNET_IP6_TYPE_HOP_BY_HOP_OPTIONS,     fnet_ip6_ext_header_handler_options}
    ,{FNET_IP6_TYPE_DESTINATION_OPTIONS,    fnet_ip6_ext_header_handler_options}
    ,{FNET_IP6_TYPE_ROUTING_HEADER,         fnet_ip6_ext_header_handler_routing_header}
//#if FNET_CFG_IP6_FRAGMENTATION    
    ,{FNET_IP6_TYPE_FRAGMENT_HEADER,        fnet_ip6_ext_header_handler_fragment_header}
//#endif    
    
    /* ADD YOUR EXTENSION HEADER HANDLERS HERE.*/
};

#define FNET_IP6_EXT_HEADER_LIST_SIZE    (sizeof(fnet_ip6_ext_header_list)/sizeof(fnet_ip6_ext_header_t))

/************************************************************************
*     Policy Table (RFC3484)
*************************************************************************/
typedef struct fnet_ip6_if_policy_entry
{
    fnet_ip6_addr_t prefix;             /* Prefix of an IP address. */
    unsigned long   prefix_length;      /* Prefix length (in bits). The number of leading bits
                                         * in the Prefix that are valid. */
    unsigned long   precedence;         /* Precedence value used for sorting destination addresses.*/
    unsigned long   label;              /* The label value Label(A) allows for policies that prefer a particular
                                         * source address prefix for use with a destination address prefix.*/         
} fnet_ip6_if_policy_entry_t;

/* RFC3484 Default policy table:*/
const fnet_ip6_if_policy_entry_t fnet_ip6_if_policy_table[] =
{
    {FNET_IP6_ADDR_INIT(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1),          128,    50, 0},
    {FNET_IP6_ADDR_INIT(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),          0,      40, 1},
    {FNET_IP6_ADDR_INIT(0x20,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,0),    16,     30, 2},
    {FNET_IP6_ADDR_INIT(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),          96,     20, 3},
    {FNET_IP6_ADDR_INIT(0,0,0,0,0,0,0,0,0,0,0xFF,0xFF,0,0,0,0),    96,     10, 4}    
};
#define FNET_IP6_IF_POLICY_TABLE_SIZE    (sizeof(fnet_ip6_if_policy_table)/sizeof(fnet_ip6_if_policy_entry_t))


static fnet_ip_queue_t ip6_queue;
static fnet_event_desc_t ip6_event;

#if FNET_CFG_IP6_FRAGMENTATION
    static fnet_ip6_frag_list_t *ip6_frag_list_head;
    static fnet_timer_desc_t ip6_timer_ptr;
#endif


/******************************************************************
* Definitions of some costant IP6 addresses (BSD-like).
*******************************************************************/ 
const fnet_ip6_addr_t fnet_ip6_addr_any = FNET_IP6_ADDR_ANY_INIT;
const fnet_ip6_addr_t fnet_ip6_addr_loopback = FNET_IP6_ADDR_LOOPBACK_INIT;
const fnet_ip6_addr_t fnet_ip6_addr_nodelocal_allnodes = FNET_IP6_ADDR_NODELOCAL_ALLNODES_INIT;
const fnet_ip6_addr_t fnet_ip6_addr_linklocal_allnodes = FNET_IP6_ADDR_LINKLOCAL_ALLNODES_INIT;
const fnet_ip6_addr_t fnet_ip6_addr_linklocal_allrouters = FNET_IP6_ADDR_LINKLOCAL_ALLROUTERS_INIT;
const fnet_ip6_addr_t fnet_ip6_addr_linklocal_allv2routers = FNET_IP6_ADDR_LINKLOCAL_ALLV2ROUTERS_INIT;
const fnet_ip6_addr_t fnet_ip6_addr_linklocal_prefix = FNET_IP6_ADDR_LINKLOCAL_PREFIX_INIT;


static void fnet_ip6_input_low( void *cookie );


/************************************************************************
* NAME: fnet_ip6_init
*
* DESCRIPTION: This function makes initialization of the IPv6 layer. 
*************************************************************************/
int fnet_ip6_init( void )
{
    int result = FNET_ERR;

#if FNET_CFG_IP6_FRAGMENTATION

    ip6_frag_list_head = 0;
    
    ip6_timer_ptr = fnet_timer_new((FNET_IP6_TIMER_PERIOD / FNET_TIMER_PERIOD_MS), fnet_ip6_timer, 0);

    if(ip6_timer_ptr)
    {
#endif
        /* Install IPv6 event handler. */
    	ip6_event = fnet_event_init(fnet_ip6_input_low, 0);
    	
    	if(ip6_event != FNET_ERR)
    		result = FNET_OK;
        
#if FNET_CFG_IP6_FRAGMENTATION
    }
#endif    
    
    return result;
}

/************************************************************************
* NAME: fnet_ip6_release
*
* DESCRIPTION: This function makes release of the all resources 
*              allocated for IPv6 layer module.
*************************************************************************/
void fnet_ip6_release( void )
{
    fnet_ip6_drain();
#if FNET_CFG_IP6_FRAGMENTATION
    fnet_timer_free(ip6_timer_ptr);
    ip6_timer_ptr = 0;
#endif
}

/************************************************************************
* NAME: fnet_ip6_ext_header_process
* 
* DESCRIPTION: Process IPv6 Extension Headers.
* 
* RETURNS: FNET_OK - continue processing by transport layer.
*          FNET_ERR - stop processing.
*************************************************************************/
static int fnet_ip6_ext_header_process(fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb)
{
    int                     try_upper_layer = FNET_FALSE;
    unsigned char           next_header;
    int                     ext_header_counter = 0;
    fnet_ip6_ext_header_t   *ext_header;
    
    /* RFC 2460 4:
     * Therefore, extension headers must
     * be processed strictly in the order they appear in the packet; a
     * receiver must not, for example, scan through a packet looking for a
     * particular kind of extension header and process that header prior to
     * processing all preceding ones.
     */
    
    /* Process headers.*/
    do
    {
        int j;

        next_header = **next_header_p;
        ext_header = FNET_NULL;

        /* IPv6 nodes must accept and attempt to process extension headers in
         * any order and occurring any number of times in the same packet,
         * except for the Hop-by-Hop Options header which is restricted to
         * appear immediately after an IPv6 header only.*/
        //TBD try to put it to main handler 
        if( (next_header == FNET_IP6_TYPE_HOP_BY_HOP_OPTIONS) && ext_header_counter)
        {
            fnet_netbuf_free_chain(*nb_p);
            fnet_icmp6_error( netif, FNET_ICMP6_TYPE_PARAM_PROB, FNET_ICMP6_CODE_PP_NEXT_HEADER, 
                                    (unsigned long)(*next_header_p) - (unsigned long)(ip6_nb->data_ptr), ip6_nb );
            return FNET_ERR;
        }

        for(j = 0; j < FNET_IP6_EXT_HEADER_LIST_SIZE; j++)
        {
            if(next_header == fnet_ip6_ext_header_list[j].type)
            {
                ext_header = &fnet_ip6_ext_header_list[j];
                break;
            }
        }
        
        if(ext_header)
        {
            fnet_ip6_ext_header_handler_result_t handler_result = ext_header->handler(netif, next_header_p, src_ip, dest_ip, nb_p, ip6_nb);
                    
            switch(handler_result)
            {
                case FNET_IP6_EXT_HEADER_HANDLER_RESULT_EXIT:
                    return FNET_ERR;
                case FNET_IP6_EXT_HEADER_HANDLER_RESULT_NEXT:
                    break;
                case FNET_IP6_EXT_HEADER_HANDLER_RESULT_TRANSPORT:
                    try_upper_layer = FNET_TRUE;
                    break;
            };  
        }
        else
        {
            try_upper_layer = FNET_TRUE;
        }

        ext_header_counter++;
    } 
    while((try_upper_layer == FNET_FALSE) || (ext_header != FNET_NULL));
    
    return FNET_OK;          
}

/************************************************************************
* NAME: fnet_ip6_ext_header_handler_no_next_header
*
* DESCRIPTION: Process No Next Header
*************************************************************************/
static fnet_ip6_ext_header_handler_result_t fnet_ip6_ext_header_handler_no_next_header (fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb)
{
     FNET_COMP_UNUSED_ARG(netif);
     FNET_COMP_UNUSED_ARG(src_ip);
     FNET_COMP_UNUSED_ARG(dest_ip);
     FNET_COMP_UNUSED_ARG(next_header_p);
    
    /* RFC 2460: The value 59 in the Next Header field of an IPv6 header or any
     * extension header indicates that there is nothing following that
     * header. If the Payload Length field of the IPv6 header indicates the
     * presence of octets past the end of a header whose Next Header field
     * contains 59, those octets must be ignored.*/        
    fnet_netbuf_free_chain(ip6_nb);
    fnet_netbuf_free_chain(*nb_p);
    return FNET_IP6_EXT_HEADER_HANDLER_RESULT_EXIT;
}

/************************************************************************
* NAME: fnet_ip6_ext_header_handler_options
*
* DESCRIPTION: Process Hop by Hop Options and Destimation Options
*************************************************************************/
static fnet_ip6_ext_header_handler_result_t fnet_ip6_ext_header_handler_options (fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb)
{
    fnet_netbuf_t               *nb = *nb_p;
    fnet_ip6_options_header_t   *options_h = nb->data_ptr;
    fnet_ip6_option_header_t    *option;
   
   
    unsigned long               offset;
    unsigned long               length = (unsigned long)((options_h->hdr_ext_length<<3) + (8-2));
   
    FNET_COMP_UNUSED_ARG(src_ip);
   
    for(offset = 0; offset < length; )
    {
        option = (fnet_ip6_option_header_t *)&options_h->options[offset];
        
        /* The RFC2460 supports only PAD0 and PADN options.*/
        switch(option->type)
        {        
            /* Pad1 option */
            case FNET_IP6_OPTION_TYPE_PAD1:
                offset++;
                break;
            /* Pad1 option */
            case FNET_IP6_OPTION_TYPE_PADN:
                offset += sizeof(fnet_ip6_option_header_t) + option->data_length;
                break;
            /* Unrecognized Options.*/
            default:     
                /* The Option Type identifiers are internally encoded such that their
                 * highest-order two bits specify the action that must be taken if the
                 * processing IPv6 node does not recognize the Option Type.*/
                switch(option->type & FNET_IP6_OPTION_TYPE_UNRECOGNIZED_MASK)
                {
                    /* 00 - skip over this option and continue processing the header.*/
                    case FNET_IP6_OPTION_TYPE_UNRECOGNIZED_SKIP:      
                        break;
                    /* 01 - discard the packet. */    
                    case FNET_IP6_OPTION_TYPE_UNRECOGNIZED_DISCARD:
                        fnet_netbuf_free_chain(nb);  
                        fnet_netbuf_free_chain(ip6_nb);   
                        return FNET_IP6_EXT_HEADER_HANDLER_RESULT_EXIT;
                    /* 10 - discard the packet and, regardless of whether or not the
                     *      packet’s Destination Address was a multicast address, send an
                     *      ICMP Parameter Problem, Code 2, message to the packet’s
                     *      Source Address, pointing to the unrecognized Option Type.*/    
                    case FNET_IP6_OPTION_TYPE_UNRECOGNIZED_DISCARD_ICMP:
                        fnet_netbuf_free_chain(nb);
                        fnet_icmp6_error( netif, FNET_ICMP6_TYPE_PARAM_PROB, FNET_ICMP6_CODE_PP_OPTION, 
                                    (unsigned long)(&option->type) - (unsigned long)(ip6_nb->data_ptr), ip6_nb ); //TBD not tested.
                        return FNET_IP6_EXT_HEADER_HANDLER_RESULT_EXIT; 
                    /* 11 - discard the packet and, only if the packet’s Destination
                     *      Address was not a multicast address, send an ICMP Parameter
                     *      Problem, Code 2, message to the packet’s Source Address,
                     *      pointing to the unrecognized Option Type.*/                                                 
                    case FNET_IP6_OPTION_TYPE_UNRECOGNIZED_DISCARD_UICMP:
                        fnet_netbuf_free_chain(nb);
                        if(!FNET_IP6_ADDR_IS_MULTICAST(dest_ip))
                            fnet_icmp6_error( netif, FNET_ICMP6_TYPE_PARAM_PROB, FNET_ICMP6_CODE_PP_OPTION, 
                                    (unsigned long)(&option->type) - (unsigned long)(ip6_nb->data_ptr), ip6_nb ); //TBD not tested.
                        else
                            fnet_netbuf_free_chain(ip6_nb); 
                                                                  
                        return FNET_IP6_EXT_HEADER_HANDLER_RESULT_EXIT;
                }
                
                offset += sizeof(fnet_ip6_option_header_t) + option->data_length;
                break;
        }
    }
    
    *next_header_p = &options_h->next_header;
    fnet_netbuf_trim(nb_p, (int)(length+2));
    
   
    return FNET_IP6_EXT_HEADER_HANDLER_RESULT_NEXT;
}


/************************************************************************
* NAME: fnet_ip6_ext_header_handler_routing_header
* 
* DESCRIPTION: Process Routing header.
*************************************************************************/
static fnet_ip6_ext_header_handler_result_t fnet_ip6_ext_header_handler_routing_header(fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb)
{
    fnet_ip6_ext_header_handler_result_t    result;
    fnet_netbuf_t                           *nb = *nb_p;
    fnet_ip6_routing_header_t   *routing_h;
     
    FNET_COMP_UNUSED_ARG(src_ip); 
    FNET_COMP_UNUSED_ARG(dest_ip);                   
                    
    routing_h    = nb->data_ptr;
                    
    /* RFC 2460 4.4: If, while processing a received packet, a node encounters a Routing
    * header with an unrecognized Routing Type value, the required behavior
    * of the node depends on the value of the Segments Left field, as
    * follows:
    * -If Segments Left is non-zero, the node must discard the packet and
    *  send an ICMP Parameter Problem, Code 0, message to the packet’s
    *  Source Address, pointing to the unrecognized Routing Type.
    */
    if( routing_h->segments_left > 0)
    {
        fnet_netbuf_free_chain(nb);
        fnet_icmp6_error( netif, FNET_ICMP6_TYPE_PARAM_PROB, FNET_ICMP6_CODE_PP_HEADER, 
                            (unsigned long)(&routing_h->routing_type) - (unsigned long)(ip6_nb->data_ptr), ip6_nb ); //TBD not tested.
        result = FNET_IP6_EXT_HEADER_HANDLER_RESULT_EXIT;
    }
    else
    {
        /* -If Segments Left is zero, the node must ignore the Routing header
         *  and proceed to process the next header in the packet, whose type
         *  is identified by the Next Header field in the Routing header.*/ 
                        

        
        *next_header_p = &routing_h->next_header;
        fnet_netbuf_trim(nb_p, (routing_h->hdr_ext_length<<3)+(8));  
        
        result = FNET_IP6_EXT_HEADER_HANDLER_RESULT_NEXT;
    }

    return result;             
}

/************************************************************************
* NAME: fnet_ip6_ext_header_handler_fragment_header
* 
* DESCRIPTION: Process IPv6 Fragment header.
*************************************************************************/
#if FNET_CFG_IP6_FRAGMENTATION
static fnet_ip6_ext_header_handler_result_t fnet_ip6_ext_header_handler_fragment_header(fnet_netif_t *netif, unsigned char **next_header_p, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t **nb_p, fnet_netbuf_t *ip6_nb)
{
    fnet_ip6_ext_header_handler_result_t result;
    
    if( (*nb_p = fnet_ip6_reassembly(netif, nb_p, ip6_nb, src_ip, dest_ip )) != FNET_NULL)
    {
        fnet_ip6_header_t  *ip6_header = ip6_nb->data_ptr;
        *next_header_p = &ip6_header->next_header;
        
        result = FNET_IP6_EXT_HEADER_HANDLER_RESULT_TRANSPORT;
    }
    else
    {
        result = FNET_IP6_EXT_HEADER_HANDLER_RESULT_EXIT;    
    }    
    
    return result;
}
#endif

/************************************************************************
* NAME: fnet_ip6_input
*
* DESCRIPTION: IPv6 input function.
*************************************************************************/
void fnet_ip6_input( fnet_netif_t *netif, fnet_netbuf_t *nb )
{
    if(netif && nb)
    {
        if(fnet_ip_queue_append(&ip6_queue, netif, nb) != FNET_OK)
        {
            fnet_netbuf_free_chain(nb);
            return;
        }

        /* Raise IPv6 event.*/
        fnet_event_raise(ip6_event);
    }    
}

/************************************************************************
* NAME: fnet_ip6_input_low
*
* DESCRIPTION: This function performs handling of incoming IPv6 datagrams.
*************************************************************************/
static void fnet_ip6_input_low( void *cookie )
{
    fnet_ip6_header_t   *hdr;
    fnet_netif_t        *netif;
    fnet_netbuf_t       *nb;
    fnet_netbuf_t       *tmp_nb;
    fnet_prot_if_t      *protocol;
    fnet_ip6_addr_t     *source_addr;
    fnet_ip6_addr_t     *destination_addr;
    unsigned short      payload_length;    

    FNET_COMP_UNUSED_ARG(cookie);    
    
    fnet_isr_lock();
 
    while((nb = fnet_ip_queue_read(&ip6_queue, &netif)) != 0)
    {
        fnet_netbuf_t   *ip6_nb = FNET_NULL;
        
        nb->next_chain = 0;

        /* RFC 4862: By disabling IP operation, 
         * silently drop any IP packets received on the interface.*/
        if(netif->nd6_if_ptr && netif->nd6_if_ptr->ip6_disabled)
        {
            goto DROP;
        }  

        /* The header must reside in contiguous area of memory. */
        if((tmp_nb = fnet_netbuf_pullup(nb, sizeof(fnet_ip6_header_t))) == 0) 
        {
            goto DROP;
        }
        nb = tmp_nb; 

        hdr = nb->data_ptr;
        source_addr = &hdr->source_addr; //TBD Save copy or do ICMP copy
        destination_addr = &hdr->destination_addr;        
        
        
        payload_length = fnet_ntohs(hdr->length);

        if(nb->total_length > (sizeof(fnet_ip6_header_t)+ payload_length))
        {
            /* Logical size and the physical size of the packet should be the same.*/
            fnet_netbuf_trim(&nb, (int)sizeof(fnet_ip6_header_t) + (int)payload_length - (int)nb->total_length ); 
        }
 
        /*******************************************************************
         * Start IPv6 header  processing.
         *******************************************************************/
        if( (nb->total_length >= sizeof(fnet_ip6_header_t))                 /* Check Size of the packet. */
            && (nb->total_length >= (sizeof(fnet_ip6_header_t) + payload_length)) 
            && (FNET_IP6_HEADER_GET_VERSION(hdr) == 6)                      /* Check the IP Version. */
            && (!FNET_IP6_ADDR_IS_MULTICAST(&hdr->source_addr))             /* Validate source address. */
            && (fnet_netif_is_my_ip6_addr(netif, &hdr->destination_addr)    /* Validate destination address. */
                || fnet_netif_is_my_ip6_solicited_multicast_addr(netif, &hdr->destination_addr)
                || FNET_IP6_ADDR_EQUAL(&fnet_ip6_addr_linklocal_allnodes, &hdr->destination_addr) )
          )
        { 
            unsigned char   *next_header = &hdr->next_header;
            fnet_netbuf_t   *ip6_nb;
            
            ip6_nb = fnet_netbuf_copy(nb, 0, ((nb->total_length > FNET_IP6_DEFAULT_MTU) ? FNET_IP6_DEFAULT_MTU : FNET_NETBUF_COPYALL), 
                                        0); /* Used mainly for ICMP errors .*/
            if(ip6_nb == FNET_NULL)
                goto DROP;
    

    #if FNET_CFG_CPU_ETH_HW_RX_PROTOCOL_CHECKSUM
            if((netif->features | FNET_NETIF_FEATURE_HW_RX_PROTOCOL_CHECKSUM) &&
                ((hdr->next_header == FNET_IP_PROTOCOL_ICMP) ||
                (hdr->next_header == FNET_IP_PROTOCOL_UDP) ||
                (hdr->next_header == FNET_IP_PROTOCOL_TCP)) )
            {
                nb->flags |= FNET_NETBUF_FLAG_HW_PROTOCOL_CHECKSUM;
            }
    #endif
       
            fnet_netbuf_trim(&nb, (int)sizeof(fnet_ip6_header_t)); 
            
            /********************************************
             * Extension headers processing.
             *********************************************/
            if(fnet_ip6_ext_header_process(netif, &next_header, source_addr, destination_addr, &nb, ip6_nb) == FNET_ERR)
                continue;    
           
#if FNET_CFG_RAW
            /* RAW Sockets input.*/
            fnet_raw_input_ip6(netif, source_addr, destination_addr, nb, ip6_nb);                         
#endif              
           
            /* Note: (http://www.cisco.com/web/about/ac123/ac147/archived_issues/ipj_9-3/ipv6_internals.html)
             * Note that there is no standard extension header format, meaning that when a host 
             * encounters a header that it does not recognize in the protocol chain, the only thing 
             * it can do is discard the packet. Worse, firewalls and routers configured to filter IPv6 
             * have the same problem: as soon as they encounter an unknown extension header, 
             * they must decide to allow or disallow the packet, even though another header deeper 
             * inside the packet would possibly trigger the opposite behavior. In other words, an IPv6 
             * packet with a TCP payload that would normally be allowed through could be blocked if 
             * there is an unknown extension header between the IPv6 and TCP headers.
             */          
             
             
            /********************************************
             * Transport layer processing (ICMP/TCP/UDP).
             *********************************************/
            /* Find transport protocol.*/
            if((protocol = fnet_prot_find(AF_INET6, SOCK_UNSPEC, *next_header)) != FNET_NULL)
            {
                protocol->prot_input_ip6(netif, source_addr, destination_addr, nb, ip6_nb);
                /* After that nb may point to wrong place. Do not use it.*/
            }
            else 
            /* No protocol found.*/                                  
            {
                fnet_netbuf_free_chain(nb);
                /* RFC 2460 4:If, as a result of processing a header, a node is required to proceed
                 * to the next header but the Next Header value in the current header is
                 * unrecognized by the node, it should discard the packet and send an
                 * ICMP Parameter Problem message to the source of the packet, with an
                 * ICMP Code value of 1 ("unrecognized Next Header type encountered")
                 * and the ICMP Pointer field containing the offset of the unrecognized
                 * value within the original packet. The same action should be taken if
                 * a node encounters a Next Header value of zero in any header other
                 * than an IPv6 header.*/ 
                fnet_icmp6_error( netif, FNET_ICMP6_TYPE_PARAM_PROB, FNET_ICMP6_CODE_PP_NEXT_HEADER, 
                                    (unsigned long)(next_header) - (unsigned long)(ip6_nb->data_ptr), ip6_nb ); //TBD not tested.
            }
        }
        else
        {
    DROP:   
            fnet_netbuf_free_chain(ip6_nb);   
            fnet_netbuf_free_chain(nb);
        }                        
        
           
    } /* while end */
   
    fnet_isr_unlock();    
}

/************************************************************************
* NAME: fnet_ip6_addr_scope
*
* DESCRIPTION: Returns scope of the IPv6 address (Node-local, 
* link-local, site-local or global.).
*************************************************************************/
int fnet_ip6_addr_scope(fnet_ip6_addr_t *ip_addr)
{
    int result = FNET_IP6_ADDR_SCOPE_GLOBAL;
    
    /* Local Host. */
    if(FNET_IP6_ADDR_IS_LINKLOCAL(ip_addr))
    {
        result = FNET_IP6_ADDR_SCOPE_LINKLOCAL;
    }    
    else if(FNET_IP6_ADDR_IS_SITELOCAL(ip_addr))    
    {
        result = FNET_IP6_ADDR_SCOPE_SITELOCAL;
    }
    else if(FNET_IP6_ADDR_IS_MULTICAST(ip_addr))/* Multicast. */
    {
        int scope = FNET_IP6_ADDR_MULTICAST_SCOPE(ip_addr);
        
        switch(scope)
        {
            case FNET_IP6_ADDR_SCOPE_INTFACELOCAL:
                result = FNET_IP6_ADDR_SCOPE_INTFACELOCAL;
            case FNET_IP6_ADDR_SCOPE_LINKLOCAL:    
                result = FNET_IP6_ADDR_SCOPE_LINKLOCAL;
            case FNET_IP6_ADDR_SCOPE_SITELOCAL:    
                result = FNET_IP6_ADDR_SCOPE_SITELOCAL;                
        }
    }
    else if(FNET_IP6_ADDR_EQUAL(ip_addr, &fnet_ip6_addr_loopback)) /* Loopback interface - special case.*/
    {
        result = FNET_IP6_ADDR_SCOPE_INTFACELOCAL;    
    }
    
    return result;
}

/************************************************************************
* NAME: fnet_ip6_common_prefix_length
*
* DESCRIPTION: RFC3484 2.2. Common Prefix Length
*  We define the common prefix length CommonPrefixLen(A, B) of two
*  addresses A and B as the length of the longest prefix (looking at the
*  most significant, or leftmost, bits) that the two addresses have in
*  common. It ranges from 0 to 128.
*************************************************************************/
int fnet_ip6_common_prefix_length(fnet_ip6_addr_t *ip_addr_1, fnet_ip6_addr_t *ip_addr_2)
{
    int             length = 0;
    int             i;
    int             bit_i;
    unsigned char   byte_xor;
    
    for(i=0; i < sizeof(fnet_ip6_addr_t); i++)
    {
        /* XOR */
        byte_xor = (unsigned char)(ip_addr_1->addr[i] ^ ip_addr_2->addr[i]);
                        
        for(bit_i = 0; bit_i<8; bit_i++)
        {
            if((byte_xor & 0x80) == 0x00)
            {
                length++;
                byte_xor<<=1; /* Shift to the next bit*/
            }
            else
            {
                break;
            }
        }
    } 
    
    return length;
}

/************************************************************************
* NAME: fnet_ip6_policy_label
*
* DESCRIPTION:  Returns label value from policy table.
*************************************************************************/
unsigned long fnet_ip6_policy_label( fnet_ip6_addr_t *addr ) 
{ 
    int             i;
    unsigned long   label = 0;
    int             biggest_prefix_length = -1;
    
    /* Find the entry in the list. */
    for(i=0; i < FNET_IP6_IF_POLICY_TABLE_SIZE; i++)
    {
        int prefix_length = (int)(fnet_ip6_if_policy_table[i].prefix_length);
           
        if(fnet_ip6_addr_pefix_cmp((fnet_ip6_addr_t*)&fnet_ip6_if_policy_table[i].prefix, addr, (unsigned long)prefix_length) == FNET_TRUE)
        {
            if(prefix_length > biggest_prefix_length)
            {
                biggest_prefix_length = prefix_length;
                label = fnet_ip6_if_policy_table[i].label;
            }
        
        }
    }
    return label;
} 

/************************************************************************
* NAME: fnet_ip6_addr_pefix_cmp
*
* DESCRIPTION: Compares first "prefix_length" bits of the addresses.
*************************************************************************/
int fnet_ip6_addr_pefix_cmp(fnet_ip6_addr_t *addr_1, fnet_ip6_addr_t *addr_2, unsigned long prefix_length)
{
    int             result;
    unsigned long   prefix_length_bytes = prefix_length>>3;
    unsigned long   prefix_length_bits_mask = ((1<<(prefix_length%8))-1) << (8-(prefix_length%8));
    
    if((prefix_length <= 128) 
        && (fnet_memcmp(addr_1, addr_2, (int)prefix_length_bytes) == 0) 
        && ((addr_1->addr[prefix_length_bytes] & prefix_length_bits_mask) == (addr_2->addr[prefix_length_bytes] & prefix_length_bits_mask)) )
    {
        result = FNET_TRUE;
    }
    else
    {
        result = FNET_FALSE;
    }

    return result;
}

/************************************************************************
* NAME: fnet_ip6_select_src_addr
*
* DESCRIPTION:  Selects the best source address to use with a 
*               destination address, Based on RFC3484.
*************************************************************************/ 
const fnet_ip6_addr_t *fnet_ip6_select_src_addr(fnet_netif_t *netif /* Optional.*/, fnet_ip6_addr_t *dest_addr)
{
    int             i;
    fnet_ip6_addr_t *best_addr = FNET_NULL;
    int             best_scope;
    int             dest_scope;  
    int             new_scope; 
    int             best_prefix_length; 
    int             new_prefix_length; 
    unsigned long   dest_label;
    unsigned long   best_label;
    unsigned long   new_label;
    fnet_netif_t    *if_dest_cur;
    fnet_netif_t    *if_dest_best = FNET_NULL;
    
     /* Just take the first/last interface.*/
    if(dest_addr) 
    {
            for(if_dest_cur = fnet_netif_list; if_dest_cur != FNET_NULL ; if_dest_cur = if_dest_cur->next)
            {
                dest_scope = fnet_ip6_addr_scope(dest_addr);
                dest_label = fnet_ip6_policy_label(dest_addr);
                
                
                /* Just continue the first loop.*/
                for(i=0; i<FNET_NETIF_IP6_ADDR_MAX; i++)
                { 
                    /* Skip not used enries. */
                    if(if_dest_cur->ip6_addr[i].state == FNET_NETIF_IP6_ADDR_STATE_NOT_USED)
                    {
                        continue;
                    }
                    else if(best_addr == FNET_NULL)
                    {
                        /* Just take the first used address, by default.=> Link-local address.*/
                        best_addr = &if_dest_cur->ip6_addr[i].address;
                        if_dest_best = if_dest_cur;
                    }
                    
                    /* RFC3484 Source Address Selection.*/
                    /* Rule 1:  Prefer same address.*/
                    if(FNET_IP6_ADDR_EQUAL(dest_addr, &if_dest_cur->ip6_addr[i].address))
                    {
                        best_addr = &if_dest_cur->ip6_addr[i].address;
                        if_dest_best = if_dest_cur;
                        break;
                    }
                    
                    /* Rule 2:  Prefer appropriate scope.*/
                    /* If Scope(SA) < Scope(SB): If Scope(SA) < Scope(D), then prefer SB
                     * and otherwise prefer SA.  Similarly, if Scope(SB) < Scope(SA): If
                     * Scope(SB) < Scope(D), then prefer SA and otherwise prefer SB.
                     */
                    best_scope = fnet_ip6_addr_scope(best_addr);
                    new_scope = fnet_ip6_addr_scope(&if_dest_cur->ip6_addr[i].address);
                     
                    if(best_scope < new_scope)
                    {
                        if(best_scope < dest_scope)
                        {
                            best_addr = &if_dest_cur->ip6_addr[i].address; //PFI take pointer at the begining.
                            if_dest_best = if_dest_cur;
                        }
                        continue;
                    }
                    else if( new_scope < best_scope)
                    {
                        if( new_scope >= dest_scope)
                        {
                            best_addr = &if_dest_cur->ip6_addr[i].address;
                            if_dest_best = if_dest_cur;
                        }
                        continue;
                    }
                      
                     
                    /* Rule 3:  Avoid deprecated addresses.
                     * XXX: Not implemented - we do not store depricated addresses."*/
                     
                    /* Rule 4:  Prefer home addresses.
                     * XXX: Not implemented - we do nit have Mobile IPv6.*/
                      
                    /* Rule 5:  Prefer outgoing interface.
                     */
                     if((if_dest_best == netif ) && (if_dest_cur != netif))
                     {
                        continue;
                     }
                     if((if_dest_best != netif ) && (if_dest_cur == netif))
                     {
                        best_addr = &if_dest_cur->ip6_addr[i].address;
                        if_dest_best = if_dest_cur;
                        continue;
                     }
                     
                   
                    /* Rule 6:  Prefer matching label.
                     * If Label(SA) = Label(D) and Label(SB) <> Label(D), then prefer SA.
                     * Similarly, if Label(SB) = Label(D) and Label(SA) <> Label(D), then
                     * prefer SB.     
                     */
                     best_label = fnet_ip6_policy_label(best_addr);
                     new_label = fnet_ip6_policy_label(&if_dest_cur->ip6_addr[i].address);
                     
                     if(best_label == dest_label)
                     {
                        if(new_label != dest_label)
                            continue; /* Prefer SA.*/
                     }
                     if(new_label == dest_label)
                     {
                        if(new_label != dest_label)
                        {
                            best_addr = &if_dest_cur->ip6_addr[i].address;
                            if_dest_best = if_dest_cur;
                            continue;
                        }
                     }
                     
                    /* Rule 7:  Prefer public addresses.
                     * If SA is a public address and SB is a temporary address, then prefer
                     * SA.  Similarly, if SB is a public address and SA is a temporary
                     * address, then prefer SB.
                     * XXX: We do not support "temporary"/"random" addresses.*/
                     
                    /* Rule 8:  Use longest matching prefix.
                     * If CommonPrefixLen(SA, D) > CommonPrefixLen(SB, D), then prefer SA.
                     * Similarly, if CommonPrefixLen(SB, D) > CommonPrefixLen(SA, D), then
                     * prefer SB.
                     */
                    best_prefix_length = fnet_ip6_common_prefix_length(best_addr, dest_addr);
                    new_prefix_length = fnet_ip6_common_prefix_length(&if_dest_cur->ip6_addr[i].address, dest_addr);
                       
                         
                    if(new_prefix_length > best_prefix_length)
                    /* Found better one.*/
                    {
                        best_addr = &if_dest_cur->ip6_addr[i].address;
                        if_dest_best = if_dest_cur;
                    }
                } /* for(IP6_IF_ADDRESSES_MAX) */
            
            }/* for(if_dest_cur) */
    } /* if(dest_addr) */

    return best_addr;
}

/************************************************************************
* NAME: fnet_ip6_mtu
*
* DESCRIPTION: Returns MTU (based on ND and PMTU).
*
* RETURNS: MTU
*************************************************************************/
unsigned long fnet_ip6_mtu( fnet_netif_t *netif)
{
    unsigned long mtu;

    if(netif)
    {
    #if FNET_CFG_IP6_PMTU_DISCOVERY 
        if(netif->pmtu) /* If PMTU is enabled for the interface.*/
        {
            mtu = netif->pmtu;
        }
        else
    #endif
        {
            mtu = netif->nd6_if_ptr  ? netif->nd6_if_ptr->mtu : netif->mtu;
        }
        
        if(mtu < FNET_IP6_DEFAULT_MTU)
            mtu = FNET_IP6_DEFAULT_MTU;
        
    }
    else
        mtu = 0;
    
    return mtu;
}

/************************************************************************
* NAME: fnet_ip6_route
*
* DESCRIPTION: This function performs IPv6 routing 
*              on an outgoing IP packet. 
*************************************************************************/
fnet_netif_t *fnet_ip6_route(fnet_ip6_addr_t *src_ip /*optional*/, fnet_ip6_addr_t *dest_ip)
{
    fnet_netif_t        *netif = FNET_NULL;
  

    /* Validate destination address. */
    /* RFC3513: The unspecified address must not be used as the destination address
     * of IPv6 packets or in IPv6 Routing Headers.*/ 
    if(!(dest_ip == FNET_NULL) || !FNET_IP6_ADDR_IS_UNSPECIFIED(dest_ip))
    {
        /* 
         * The specified source address may
         * influence the candidate set, by affecting the choice of outgoing
         * interface.  If the application or upper-layer specifies a source
         * address that is in the candidate set for the destination, then the
         * network layer MUST respect that choice.  If the application or
         * upper-layer does not specify a source address, then the network layer
         * uses the source address selection algorithm
         */
        if((src_ip == FNET_NULL) || FNET_IP6_ADDR_IS_UNSPECIFIED(src_ip))
        /* Determine a source address. */
        {
            src_ip = (fnet_ip6_addr_t *)fnet_ip6_select_src_addr(FNET_NULL, dest_ip);
        }

        if(src_ip != FNET_NULL)
        {
            /* Determine an output interface. */
            netif = fnet_netif_get_by_ip6_addr(src_ip);
        } 
    }

    return netif;
}

/************************************************************************
* NAME: fnet_ip6_will_fragment
*
* DESCRIPTION: This function returns FNET_TRUE if the protocol message 
*              will be fragmented by IPv6, and FNET_FALSE otherwise.
*************************************************************************/
int fnet_ip6_will_fragment( fnet_netif_t *netif, unsigned long protocol_message_size)
{
    int res;

   if(
#if FNET_CFG_IP6_PMTU_DISCOVERY
    /*
     * In response to an IPv6 packet that is sent to an IPv4 destination
     * (i.e., a packet that undergoes translation from IPv6 to IPv4), the
     * originating IPv6 node may receive an ICMP Packet Too Big message
     * reporting a Next-Hop MTU less than 1280.  In that case, the IPv6 node
     * is not required to reduce the size of subsequent packets to less than
     * 1280, but must include a Fragment header in those packets so that the
     * IPv6-to-IPv4 translating router can obtain a suitable Identification
     * value to use in resulting IPv4 fragments.  Note that this means the
     * payload may have to be reduced to 1232 octets (1280 minus 40 for the
     * IPv6 header and 8 for the Fragment header), and smaller still if
     * additional extension headers are used.
     */
      
        (netif->pmtu /* If PMTU is enabled.*/ &&  ((protocol_message_size + sizeof(fnet_ip6_header_t)) > netif->pmtu)) ||
         !netif->pmtu &&
#endif   
        ((protocol_message_size + sizeof(fnet_ip6_header_t)) > fnet_ip6_mtu(netif)) ) /* IP Fragmentation. */ 
        res = FNET_TRUE;
    else
        res = FNET_FALSE;

    return res;
}

/************************************************************************
* NAME: fnet_ip6_output
*
* DESCRIPTION: IPv6 output function.
*
* RETURNS: FNET_OK=OK
*          FNET_ERR_NETUNREACH=No route
*          FNET_ERR_MSGSIZE=Size error
*          FNET_ERR_NOMEM=No memory
*************************************************************************/
int fnet_ip6_output(fnet_netif_t *netif /*optional*/, fnet_ip6_addr_t *src_ip /*optional*/, fnet_ip6_addr_t *dest_ip,
                    unsigned char protocol, unsigned char hop_limit /*optional*/, fnet_netbuf_t *nb, FNET_COMP_PACKED_VAR unsigned short *checksum)
{
    int                 error_code;
    fnet_netbuf_t       *nb_header;
    fnet_ip6_header_t   *ip6_header;
    unsigned long       mtu;


    /* Check maximum packet size. */
    if((nb->total_length + sizeof(fnet_ip6_header_t)) > FNET_IP6_MAX_PACKET)
    {
        error_code = FNET_ERR_MSGSIZE;
        goto DROP;
    }    
    
    /* Validate destination address. */
    /* RFC3513: The unspecified address must not be used as the destination address
     * of IPv6 packets or in IPv6 Routing Headers.*/ 
    if((dest_ip == FNET_NULL) || FNET_IP6_ADDR_IS_UNSPECIFIED(dest_ip))
    {
        error_code = FNET_ERR_DESTADDRREQ;   
        goto DROP;        
    }
    

    /* 
     * The specified source address may
     * influence the candidate set, by affecting the choice of outgoing
     * interface.  If the application or upper-layer specifies a source
     * address that is in the candidate set for the destination, then the
     * network layer MUST respect that choice.  If the application or
     * upper-layer does not specify a source address, then the network layer
     * uses the source address selection algorithm
     */
    if(src_ip == FNET_NULL) /* It may be any address.*/
    /* Determine a source address. */
    {
        src_ip = (fnet_ip6_addr_t *)fnet_ip6_select_src_addr(netif, dest_ip);
            
        if(src_ip == FNET_NULL)
        {
            error_code = FNET_ERR_NETUNREACH;
            goto DROP;
        } 
    }

    if(netif == FNET_NULL)
    /* Determine an output interface. */
    {
        netif = fnet_netif_get_by_ip6_addr(src_ip);

        if(netif == FNET_NULL) /* Ther is no any initializaed IF.*/
        {
            error_code = FNET_ERR_NETUNREACH;
            goto DROP;
        }
    } 
    
    /* RFC 4862: By disabling IP operation, the node will then not 
     * send any IP packets from the interface.*/
    if(netif->nd6_if_ptr && netif->nd6_if_ptr->ip6_disabled)
    {
        error_code = FNET_ERR_IPDISABLED;
        goto DROP;
    }      

    /* Pseudo checksum. */
    if(checksum)
        *checksum = fnet_checksum_pseudo_end( *checksum, (char *)src_ip, (char *)dest_ip, sizeof(fnet_ip6_addr_t) );    
    
    /****** Construct IP header. ******/
    if((nb_header = fnet_netbuf_new(sizeof(fnet_ip6_header_t), FNET_TRUE)) == 0)
    {
        error_code = FNET_ERR_NOMEM;   
        goto DROP;
    }
    
    ip6_header = nb_header->data_ptr;
    
    ip6_header->version__tclass = FNET_IP6_VERSION<<4;
    ip6_header->tclass__flowl = 0;
    ip6_header->flowl = 0;
    ip6_header->length = fnet_htons((unsigned short)nb->total_length);
    ip6_header->next_header = protocol;
    
    /* Set Hop Limit.*/
    if(hop_limit == 0)
        hop_limit = netif->nd6_if_ptr->cur_hop_limit; /* Defined by ND.*/
    ip6_header->hop_limit = hop_limit;
    

    FNET_IP6_ADDR_COPY(src_ip, &ip6_header->source_addr);
    FNET_IP6_ADDR_COPY(dest_ip, &ip6_header->destination_addr);
    
    mtu = fnet_ip6_mtu(netif); 

    if(
#if FNET_CFG_IP6_PMTU_DISCOVERY
    /*
     * In response to an IPv6 packet that is sent to an IPv4 destination
     * (i.e., a packet that undergoes translation from IPv6 to IPv4), the
     * originating IPv6 node may receive an ICMP Packet Too Big message
     * reporting a Next-Hop MTU less than 1280.  In that case, the IPv6 node
     * is not required to reduce the size of subsequent packets to less than
     * 1280, but must include a Fragment header in those packets so that the
     * IPv6-to-IPv4 translating router can obtain a suitable Identification
     * value to use in resulting IPv4 fragments.  Note that this means the
     * payload may have to be reduced to 1232 octets (1280 minus 40 for the
     * IPv6 header and 8 for the Fragment header), and smaller still if
     * additional extension headers are used.
     */
      
        (netif->pmtu /* If PMTU is enabled.*/ &&  ((nb->total_length + nb_header->total_length) > netif->pmtu)) ||
         !netif->pmtu &&
#endif   
        ((nb->total_length + nb_header->total_length) > mtu) ) /* IP Fragmentation. */ 
    {

#if FNET_CFG_IP6_FRAGMENTATION

        int                         first_frag_length;
        int                         frag_length; /* The number of data in each fragment. */
        int                         offset;
        int                         error = 0;
        fnet_netbuf_t               *tmp_nb;
        fnet_netbuf_t               *nb_prev;
        fnet_netbuf_t               ** nb_next_ptr;
        int                         header_length = sizeof(fnet_ip6_header_t) + sizeof(fnet_ip6_fragment_header_t);
        fnet_ip6_header_t           *ip6_header_new;
        fnet_ip6_fragment_header_t  *ip6_fragment_header;
        fnet_ip6_fragment_header_t  *ip6_fragment_header_new;
        fnet_netbuf_t               *nb_frag_header;
        unsigned long               total_length;
        static unsigned long        ip6_id = 0;
        
        

        frag_length = (int)(mtu - header_length) & ~7; /* Rounded down to an 8-byte boundary.*/
        first_frag_length = frag_length;

        if(frag_length < 8)             /* The MTU is too small.*/
        {
            error_code = FNET_ERR_MSGSIZE; 
            fnet_netbuf_free_chain(nb_header);            
            goto DROP; 
        }
        
        if((nb_frag_header = fnet_netbuf_new(sizeof(fnet_ip6_fragment_header_t), FNET_TRUE)) == 0)
        {
            error_code = FNET_ERR_NOMEM; 
            fnet_netbuf_free_chain(nb_header);   
            goto DROP;
        }
        
        
        nb = fnet_netbuf_concat(nb_frag_header, nb);
        nb = fnet_netbuf_concat(nb_header, nb);
        
        nb_next_ptr = &nb->next_chain;

        /* The header (and options) must reside in contiguous area of memory.*/
        if((tmp_nb = fnet_netbuf_pullup(nb,  header_length)) == 0)
        {
            error_code = FNET_ERR_NOMEM;   
            goto DROP;
        }

        nb = tmp_nb;

        ip6_header = nb->data_ptr;
        ip6_fragment_header = (fnet_ip6_fragment_header_t*)((unsigned long)ip6_header + sizeof(fnet_ip6_header_t));
        
        nb_prev = nb;
        
        total_length = nb->total_length; 
        
        ip6_id++;
        
        
        ip6_header->next_header = FNET_IP6_TYPE_FRAGMENT_HEADER;
        
        ip6_fragment_header->id = fnet_htonl(ip6_id);
        ip6_fragment_header->_reserved = 0;
        ip6_fragment_header->next_header = protocol;
        ip6_fragment_header->offset_more = FNET_HTONS(FNET_IP6_FRAGMENT_MF_MASK);
        

        /* Go through the whole data segment after first fragment.*/
        for (offset = (header_length + frag_length); offset < total_length; offset += frag_length)
        {
            fnet_netbuf_t *nb_tmp;

            nb = fnet_netbuf_new(header_length, FNET_FALSE); /* Allocate a new header.*/

            if(nb == 0)
            {
                error++;
                goto FRAG_END;
            }


            ip6_header_new = nb->data_ptr;
            ip6_fragment_header_new = (fnet_ip6_fragment_header_t *)((unsigned long)ip6_header_new + sizeof(fnet_ip6_header_t));
            
            fnet_memcpy(ip6_header_new, ip6_header, (unsigned int)header_length); /* Copy IPv6 header.*/
             
            
            ip6_fragment_header_new->offset_more = fnet_htons((unsigned short)(offset - header_length) );

            if(offset + frag_length >= total_length)  
                frag_length = (int)(total_length - offset); 
            else
                ip6_fragment_header_new->offset_more |= FNET_HTONS(FNET_IP6_FRAGMENT_MF_MASK);

            /* Copy the data from the original packet into the fragment.*/
            if((nb_tmp = fnet_netbuf_copy(nb_prev, offset, frag_length, 0)) == 0)
            {
                error++;
                fnet_netbuf_free_chain(nb);
                goto FRAG_END;
            }

            nb = fnet_netbuf_concat(nb, nb_tmp);
            

            ip6_header_new->length = fnet_htons((unsigned short)(nb->total_length - sizeof(fnet_ip6_header_t)) );

            *nb_next_ptr = nb;
            nb_next_ptr = &nb->next_chain;
        }

        /* Update the first fragment.*/
        nb = nb_prev;
        fnet_netbuf_trim(&nb, /*header_length +*/ first_frag_length -  fnet_ntohs(ip6_header->length) /*- sizeof(fnet_ip6_header_t)*/ );
        
        ip6_header->length = fnet_htons((unsigned short)(nb->total_length - sizeof(fnet_ip6_header_t)) );
        
FRAG_END:
        for (nb = nb_prev; nb; nb = nb_prev)    /* Send each fragment.*/
        {
            nb_prev = nb->next_chain;
            nb->next_chain = 0;

            if(error == 0)
            {
                fnet_ip6_netif_output(netif, src_ip, dest_ip, nb);
            }
            else
                fnet_netbuf_free_chain(nb);
        }

#else

        error_code = FNET_ERR_MSGSIZE;   /* Discard datagram.*/
        goto DROP; 

#endif  /* FNET_CFG_IP6_FRAGMENTATION */

    }
    else
    {
        nb = fnet_netbuf_concat(nb_header, nb);
        fnet_ip6_netif_output(netif, src_ip, dest_ip, nb);
    }
    

    return (FNET_OK);

DROP:
    fnet_netbuf_free_chain(nb);           /* Discard datagram */ 
           
    return (error_code);
}

/************************************************************************
* NAME: fnet_ip6_netif_output
*
* DESCRIPTION:
*************************************************************************/
static void fnet_ip6_netif_output(struct fnet_netif *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t* nb)
{
    
#if FNET_CFG_LOOPBACK    
    /***********************************************
     * Handle possible loopback 
     ***********************************************/
   
    /* Anything sent to one of the host's own IP address is sent to the loopback interface.*/
    if(fnet_netif_is_my_ip6_addr(netif, dest_ip) == FNET_TRUE)
    {
        netif = FNET_LOOP_IF;
    }
    else
#endif /* FNET_CFG_LOOPBACK */
    { 
#if  FNET_CFG_LOOPBACK && FNET_CFG_LOOPBACK_MULTICAST  
        /* Send Multicast packets also to the loopback.*/        
        if((netif != FNET_LOOP_IF) && FNET_IP6_ADDR_IS_MULTICAST(dest_ip))
        {
            fnet_netbuf_t* nb_loop; 
            
            /* Datagrams sent to amulticast address are copied to the loopback interface.*/
            if((nb_loop=fnet_netbuf_copy(nb, 0, FNET_NETBUF_COPYALL, FNET_TRUE))!=0) 
            {
                fnet_loop_output_ip6(netif, src_ip,  dest_ip, nb_loop);
            }
        } 
#endif /* FNET_CFG_LOOPBACK && FNET_CFG_LOOPBACK_MULTICAST */
    }
    netif->api->output_ip6(netif, src_ip,  dest_ip, nb); /* IPv6 Transmit function.*/
}



/************************************************************************
* NAME: fnet_ip6_get_solicited_multicast_addr
*
* DESCRIPTION: Get IPv6 solicited-node multicast address.
*        It has the prefix FF02:0:0:0:0:1:FF00:0000/104 concatenated 
*        with the 24 low-order bits of a corresponding IPv6 unicast 
*        or anycast address.
*************************************************************************/
void fnet_ip6_get_solicited_multicast_addr(fnet_ip6_addr_t *ip_addr, fnet_ip6_addr_t *solicited_multicast_addr)
{
    solicited_multicast_addr->addr[0]=0xFF;
    solicited_multicast_addr->addr[1]=0x02;
    solicited_multicast_addr->addr[2]=0x00;
    solicited_multicast_addr->addr[3]=0x00;
    solicited_multicast_addr->addr[4]=0x00;
    solicited_multicast_addr->addr[5]=0x00;
    solicited_multicast_addr->addr[6]=0x00;
    solicited_multicast_addr->addr[7]=0x00;
    solicited_multicast_addr->addr[8]=0x00;
    solicited_multicast_addr->addr[9]=0x00;
    solicited_multicast_addr->addr[10]=0x00;
    solicited_multicast_addr->addr[11]=0x01;
    solicited_multicast_addr->addr[12]=0xFF;
    solicited_multicast_addr->addr[13]=ip_addr->addr[13];
    solicited_multicast_addr->addr[14]=ip_addr->addr[14];
    solicited_multicast_addr->addr[15]=ip_addr->addr[15];
}


/************************************************************************
* NAME: fnet_ip6_frag_list_free
*
* DESCRIPTION: This function frees list of datagram fragments.
*************************************************************************/
#if FNET_CFG_IP6_FRAGMENTATION  //PFI create general library fo list, linked lists etc.
static void fnet_ip6_frag_list_free( fnet_ip6_frag_list_t *list )
{
    fnet_netbuf_t *nb;

    fnet_isr_lock();

    if(list)
    {
        while((volatile fnet_ip6_frag_header_t *)(list->frag_ptr) != 0)
        {
            nb = list->frag_ptr->nb;
            fnet_ip6_frag_del((fnet_ip6_frag_header_t **)(&list->frag_ptr), list->frag_ptr);
            fnet_netbuf_free_chain(nb);
        }

        fnet_ip6_frag_list_del(&ip6_frag_list_head, list);
        fnet_free(list);
    }

    fnet_isr_unlock();
}

#endif /* FNET_CFG_IP6_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip6_frag_list_add
*
* DESCRIPTION: Adds frag list to the general frag list.
*************************************************************************/
#if FNET_CFG_IP6_FRAGMENTATION
static void fnet_ip6_frag_list_add( fnet_ip6_frag_list_t ** head, fnet_ip6_frag_list_t *fl )
{
    fl->next = *head;

    if(fl->next != 0)
        fl->next->prev = fl;

    fl->prev = 0;
    *head = fl;
}

#endif /* FNET_CFG_IP6_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip6_frag_list_del
*
* DESCRIPTION: Deletes frag list from the general frag list.
*************************************************************************/
#if FNET_CFG_IP6_FRAGMENTATION
static void fnet_ip6_frag_list_del( fnet_ip6_frag_list_t ** head, fnet_ip6_frag_list_t *fl )
{
    if(fl->prev == 0)
        *head=fl->next;
    else
        fl->prev->next = fl->next;

    if(fl->next != 0)
        fl->next->prev = fl->prev;
}
#endif /* FNET_CFG_IP6_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip6_frag_add
*
* DESCRIPTION: Adds frag to the frag list.
*************************************************************************/
#if FNET_CFG_IP6_FRAGMENTATION
static void fnet_ip6_frag_add( fnet_ip6_frag_header_t ** head, fnet_ip6_frag_header_t *frag,
                       fnet_ip6_frag_header_t *frag_prev )
{
    if(frag_prev && ( *head))
    {
        frag->next = frag_prev->next;
        frag->prev = frag_prev;
        frag_prev->next->prev = frag;
        frag_prev->next = frag;

        if((*head)->offset > frag->offset)
        {
            *head = frag;
        }
    }
    else
    {
        frag->next = frag;
        frag->prev = frag;
        *head = frag;
    }
}
#endif /* FNET_CFG_IP6_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip6_frag_del
*
* DESCRIPTION: Deletes frag from the frag list.
*************************************************************************/
#if FNET_CFG_IP6_FRAGMENTATION
static void fnet_ip6_frag_del( fnet_ip6_frag_header_t ** head, fnet_ip6_frag_header_t *frag )
{
    if(frag->prev == frag)
        *head=0;
    else
    {
        frag->prev->next = frag->next;
        frag->next->prev = frag->prev;

        if(*head == frag)
            *head=frag->next;
    }
    
    fnet_free(frag);
}
#endif /* FNET_CFG_IP6_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip6_reassembly
*
* DESCRIPTION: This function attempts to assemble a complete datagram.
*************************************************************************/
#if FNET_CFG_IP6_FRAGMENTATION
static fnet_netbuf_t *fnet_ip6_reassembly(fnet_netif_t *netif, fnet_netbuf_t ** nb_p, fnet_netbuf_t *ip6_nb, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip )
{
    fnet_ip6_frag_list_t        *frag_list_ptr;
    fnet_ip6_frag_header_t      *frag_ptr;
    fnet_ip6_frag_header_t      *cur_frag_ptr;
    fnet_netbuf_t               *nb = *nb_p;
    fnet_netbuf_t               *tmp_nb;
    fnet_ip6_header_t           *iphdr = (fnet_ip6_header_t *)ip6_nb->data_ptr;
    int                         i;
    unsigned char               next_header;
    unsigned long               id;
    unsigned short              offset;
    unsigned char               mf;
    unsigned short              total_length;
    fnet_ip6_fragment_header_t  *ip6_fragment_header;
    
  
    /* For this algorithm the all datagram must reside in contiguous area of memory.*/
    if((tmp_nb = fnet_netbuf_pullup(nb, (int)nb->total_length)) == 0) 
    {
        goto DROP_FRAG_1;
    }

    *nb_p = tmp_nb;
    nb = tmp_nb;
    
    
    /* Process fragment header.*/
    ip6_fragment_header = nb->data_ptr;
    next_header = ip6_fragment_header->next_header;
    id = ip6_fragment_header->id;
    offset = (unsigned short)FNET_IP6_FRAGMENT_OFFSET(ip6_fragment_header->offset_more);
    mf = (unsigned char)FNET_IP6_FRAGMENT_MF(ip6_fragment_header->offset_more);
     
    
    fnet_netbuf_trim(nb_p, sizeof(fnet_ip6_fragment_header_t));
    nb = *nb_p;
    
    total_length = (unsigned short)nb->length;
    
    if(mf)
    {
        /* Fragments (except the last) must be multiples of 8 bytes */
        if ((total_length & 0x07) != 0)
        {
            /* If the length of a fragment, as derived from the fragment packet’s
             * Payload Length field, is not a multiple of 8 octets and the M flag
             * of that fragment is 1, then that fragment must be discarded and an
             * ICMP Parameter Problem, Code 0, message should be sent to the
             * source of the fragment, pointing to the Payload Length field of
             * the fragment packet. */
            fnet_icmp6_error( netif, FNET_ICMP6_TYPE_PARAM_PROB, FNET_ICMP6_CODE_PP_HEADER, 
                                    (unsigned long)((unsigned long)(&iphdr->length) - (unsigned long)ip6_nb->data_ptr), ip6_nb ); //TBD not tested.             
            goto DROP_FRAG_0;
        }
    }
    

    /* Create fragment header.*/
    if((cur_frag_ptr = fnet_malloc(sizeof(fnet_ip6_frag_header_t))) == 0) 
        goto DROP_FRAG_1;
    
    cur_frag_ptr->mf = mf;
    cur_frag_ptr->offset = offset; 
    cur_frag_ptr->nb = nb;
    cur_frag_ptr->total_length = total_length;


    /* Liner search of the list to locate the appropriate datagram for the current fragment.*/
    for (frag_list_ptr = ip6_frag_list_head; frag_list_ptr != 0; frag_list_ptr = frag_list_ptr->next)
    {
        if( (frag_list_ptr->id == id) 
            && (frag_list_ptr->next_header == next_header)
            && FNET_IP6_ADDR_EQUAL(&frag_list_ptr->source_addr, src_ip)
            && FNET_IP6_ADDR_EQUAL(&frag_list_ptr->destination_addr, dest_ip))
                break;
    }

    /* The first fragment of the new datagram.*/
    if(frag_list_ptr == 0)                                                  
    {
        /* Create list.*/
        if((frag_list_ptr = fnet_malloc(sizeof(fnet_ip6_frag_list_t))) == 0) 
            goto DROP_FRAG_2;

        fnet_ip6_frag_list_add(&ip6_frag_list_head, frag_list_ptr);

        frag_list_ptr->ttl = FNET_IP6_FRAG_TTL;
        frag_list_ptr->id = id;
        frag_list_ptr->next_header = next_header;
        FNET_IP6_ADDR_COPY(src_ip, &frag_list_ptr->source_addr);
        FNET_IP6_ADDR_COPY(dest_ip, &frag_list_ptr->destination_addr);
        frag_list_ptr->frag_ptr = 0;
        frag_ptr = 0;
    }
    else
    {
        /* Find position in reassembly list.*/
        frag_ptr = frag_list_ptr->frag_ptr;
        do
        {
            if(frag_ptr->offset > cur_frag_ptr->offset)
                break;

            frag_ptr = frag_ptr->next;
        } 
        while (frag_ptr != frag_list_ptr->frag_ptr);
        
        /* Trims or discards icoming fragments.*/
        if(frag_ptr != frag_list_ptr->frag_ptr)
        {
            if((i = frag_ptr->prev->offset + frag_ptr->prev->total_length - cur_frag_ptr->prev->offset) != 0)
            {
                if(i > cur_frag_ptr->total_length)
                    goto DROP_FRAG_2;

                fnet_netbuf_trim(nb_p, i);
                nb = *nb_p;
                cur_frag_ptr->total_length -= i;
                cur_frag_ptr->offset += i;
            }
        }
        
        /* Trims or discards existing fragments.*/
        while((frag_ptr != frag_list_ptr->frag_ptr)
                  && ((cur_frag_ptr->offset + cur_frag_ptr->total_length) > frag_ptr->offset))
        {
            i = (cur_frag_ptr->offset + cur_frag_ptr->total_length) - frag_ptr->offset;

            if(i < frag_ptr->total_length)
            {
                frag_ptr->total_length -= i;
                frag_ptr->offset += i;
                fnet_netbuf_trim((fnet_netbuf_t **)&frag_ptr->nb, i);
                break;
            }

            frag_ptr = frag_ptr->next;
            fnet_netbuf_free_chain(frag_ptr->prev->nb);
            fnet_ip6_frag_del((fnet_ip6_frag_header_t **)&frag_list_ptr->frag_ptr, frag_ptr->prev);
        }
    }
    
    if(offset == 0) /* First fragment */
    {
        frag_list_ptr->hdr_length = iphdr->length;
        frag_list_ptr->hdr_hop_limit = iphdr->hop_limit;
        frag_list_ptr->netif = netif;
    }

    /* Insert fragment to the list.*/
    fnet_ip6_frag_add((fnet_ip6_frag_header_t **)(&frag_list_ptr->frag_ptr), cur_frag_ptr, frag_ptr ? frag_ptr->prev : 0); 
    
    {
        int offset = 0;
        frag_ptr = frag_list_ptr->frag_ptr;

        do
        {
            if(frag_ptr->offset != offset)
                goto NEXT_FRAG;

            offset += frag_ptr->total_length;
            frag_ptr = frag_ptr->next;
        } while (frag_ptr != frag_list_ptr->frag_ptr);
    }

    if(frag_ptr->prev->mf & 1)
        goto NEXT_FRAG;//????

    /* Reconstruct datagram.*/
    frag_ptr = frag_list_ptr->frag_ptr;
    nb = frag_ptr->nb;
    frag_ptr = frag_ptr->next;

    while(frag_ptr != frag_list_ptr->frag_ptr)
    {
        nb = fnet_netbuf_concat(nb, frag_ptr->nb);

        frag_ptr = frag_ptr->next;
    }

    /* Reconstruct datagram header.*/
    iphdr = (fnet_ip6_header_t *)ip6_nb->data_ptr;
    iphdr->length = fnet_htons((unsigned short)nb->total_length);
    iphdr->next_header = frag_list_ptr->next_header;

    while(frag_list_ptr->frag_ptr != 0)
    {
       fnet_ip6_frag_del((fnet_ip6_frag_header_t **)(&frag_list_ptr->frag_ptr), frag_list_ptr->frag_ptr);
    }

    fnet_ip6_frag_list_del(&ip6_frag_list_head, frag_list_ptr);
    fnet_free(frag_list_ptr);

    return (nb);
    
DROP_FRAG_2:
    fnet_free(cur_frag_ptr);
DROP_FRAG_1:
    fnet_netbuf_free_chain(ip6_nb);
DROP_FRAG_0:    
    fnet_netbuf_free_chain(nb);
    return (FNET_NULL);

NEXT_FRAG:
    fnet_netbuf_free_chain(ip6_nb);
    return (FNET_NULL);    
}
#endif /* FNET_CFG_IP4_FRAGMENTATION */

/************************************************************************
* NAME: fnet_ip_timer
*
* DESCRIPTION: IP timer function.
*************************************************************************/
#if FNET_CFG_IP6_FRAGMENTATION
static void fnet_ip6_timer(void *cookie)
{
    fnet_ip6_frag_list_t *frag_list_ptr;
    fnet_ip6_frag_list_t *tmp_frag_list_ptr;

    FNET_COMP_UNUSED_ARG(cookie);
    
    fnet_isr_lock();
    frag_list_ptr = ip6_frag_list_head;

    while(frag_list_ptr != 0)
    {
        frag_list_ptr->ttl--;

        if(frag_list_ptr->ttl == 0)
        {
            /* If the first fragment (i.e., the one
             * with a Fragment Offset of zero) has been received, an ICMP Time
             * Exceeded -- Fragment Reassembly Time Exceeded message should be
             * sent to the source of that fragment.
             */
            if( frag_list_ptr->frag_ptr && (frag_list_ptr->frag_ptr->offset == 0) )
            {
                 fnet_netbuf_t              *nb_header;
                 fnet_netbuf_t              *nb;
                 fnet_ip6_header_t          *ip6_header;
                 fnet_ip6_fragment_header_t *ip6_fragment_header;
                /*************************************
                 * Reconstact PCB for ICMP error.
                 *************************************/         
                nb_header = fnet_netbuf_new(sizeof(fnet_ip6_header_t) + sizeof(fnet_ip6_fragment_header_t), FNET_FALSE); /* Allocate a new header.*/

                if(nb_header == FNET_NULL)
                {
                    goto FREE_LIST;
                }
                nb = fnet_netbuf_copy(frag_list_ptr->frag_ptr->nb, 0, FNET_NETBUF_COPYALL, 0);
                if(nb == FNET_NULL)
                {
                    fnet_netbuf_free(nb_header);
                    goto FREE_LIST;                
                }  
                
                nb = fnet_netbuf_concat(nb_header, nb); 
                
                ip6_header = nb->data_ptr;
                ip6_fragment_header = (fnet_ip6_fragment_header_t*)((unsigned long)ip6_header + sizeof(fnet_ip6_header_t));
                
                /* IPv6 header.*/
                ip6_header->version__tclass = FNET_IP6_VERSION<<4; //PFI copy/save header
                ip6_header->tclass__flowl = 0;
                ip6_header->flowl = 0;
                ip6_header->length = frag_list_ptr->hdr_length; 
                ip6_header->next_header = FNET_IP6_TYPE_FRAGMENT_HEADER;
                ip6_header->hop_limit = frag_list_ptr->hdr_hop_limit;
                FNET_IP6_ADDR_COPY(&frag_list_ptr->source_addr, &ip6_header->source_addr);
                FNET_IP6_ADDR_COPY(&frag_list_ptr->destination_addr, &ip6_header->destination_addr);
                
                /* Fragment header.*/
                
                ip6_fragment_header->next_header = frag_list_ptr->next_header;
                ip6_fragment_header->_reserved=0;   
                ip6_fragment_header->offset_more = fnet_htons(frag_list_ptr->frag_ptr->mf);
                ip6_fragment_header->id = frag_list_ptr->id;
                
                fnet_icmp6_error( frag_list_ptr->netif, FNET_ICMP6_TYPE_TIME_EXCEED, FNET_ICMP6_CODE_TE_FRG_REASSEMBLY, 
                            0, nb ); //TBD not tested.
            }
            
        FREE_LIST:     
            tmp_frag_list_ptr = frag_list_ptr->next;
            fnet_ip6_frag_list_free(frag_list_ptr);
            frag_list_ptr = tmp_frag_list_ptr;

        }
        else
            frag_list_ptr = frag_list_ptr->next;
    }

    fnet_isr_unlock();
}

#endif

/************************************************************************
* NAME: fnet_ip6_drain
*
* DESCRIPTION: This function tries to free not critical parts 
*              of memory occupied by the IPv6 module.
*************************************************************************/
void fnet_ip6_drain( void )
{
    fnet_isr_lock();

#if FNET_CFG_IP6_FRAGMENTATION

    while(((volatile fnet_ip6_frag_list_t *)ip6_frag_list_head) != 0)
    {
        fnet_ip6_frag_list_free(ip6_frag_list_head);
    }

#endif

    while(((volatile fnet_netbuf_t *)ip6_queue.head) != 0)
    {
        fnet_netbuf_del_chain(&ip6_queue.head, ip6_queue.head);
    }

    ip6_queue.count = 0;

    fnet_isr_unlock();
}



#endif /* FNET_CFG_IP6 */

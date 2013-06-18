/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
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
* @file fnet_ping.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.5.0
*
* @brief PING implementation.
*
***************************************************************************/


#include "fnet_config.h"

#if FNET_CFG_PING

#include "fnet_ping.h"
#include "fnet_checksum.h"
#include "fnet_icmp.h"
#include "fnet_ip6_prv.h"

#if FNET_CFG_DEBUG_PING    
    #define FNET_DEBUG_PING   FNET_DEBUG
#else
    #define FNET_DEBUG_PING(...)
#endif

/************************************************************************
*     Definitions
*************************************************************************/
#define FNET_PING_ERR_PARAMS            "ERROR: Wrong input parameters."
#define FNET_PING_ERR_SOCKET_CREATION   "ERROR: Socket creation error."
#define FNET_PING_ERR_SOCKET_CONNECT    "ERROR: Socket Error during connect."
#define FNET_PING_ERR_SERVICE           "ERROR: Service registration is failed."
#define FNET_PING_ERR_IS_INITIALIZED    "ERROR: PING is already initialized."
#define FNET_PING_ERR_GETSOCKNAME       "ERROR: Socket getsockname error."
 

#define FNET_PING_BUFFER_SIZE   (sizeof(fnet_icmp_echo_header_t) + FNET_CFG_PING_PACKET_MAX)

static void fnet_ping_state_machine(void *fnet_ping_if_p);



/************************************************************************
*    PING service interface structure.
*************************************************************************/
typedef struct
{
    SOCKET                  socket_foreign;     /* Foreign socket.*/
    fnet_address_family_t   family;
    unsigned short          sequence_number;
    fnet_poll_desc_t        service_descriptor;
    fnet_ping_state_t       state;              /* Current state. */
    fnet_ping_handler_t     handler;            /* Callback function. */
    long                    handler_cookie;                 /* Callback-handler specific parameter. */
    char                    buffer[FNET_PING_BUFFER_SIZE];  /* Message buffer. */
    unsigned long           timeout_clk;        /* Timeout value in clocks, that ping request waits for reply.*/
    unsigned long           send_time;          /* Last send time, used for timeout detection. */
    unsigned int            packet_count;       /* Number of packets to be sent.*/
    unsigned long           packet_size;
    unsigned char           pattern;
    struct sockaddr         target_addr; 
} 
fnet_ping_if_t;


/* PING interface structure */
static fnet_ping_if_t fnet_ping_if;

/************************************************************************
* NAME: fnet_ping_request
*
* DESCRIPTION: Initializes PING service.
************************************************************************/
int fnet_ping_request( struct fnet_ping_params *params )
{
    const unsigned long bufsize_option = FNET_PING_BUFFER_SIZE;

    /* Check input parameters. */
    if((params == 0) || (params->packet_count==0) || fnet_socket_addr_is_unspecified(&params->target_addr))
    {
        FNET_DEBUG_PING(FNET_PING_ERR_PARAMS);
        goto ERROR;
    }

    
    /* Check if PING service is free.*/
    if(fnet_ping_if.state != FNET_PING_STATE_DISABLED)
    {
        FNET_DEBUG_PING(FNET_PING_ERR_IS_INITIALIZED);
        goto ERROR;
    }
    
    /* Save input parmeters.*/
    fnet_ping_if.handler = params->handler;
    fnet_ping_if.handler_cookie = params->cookie;
    fnet_ping_if.timeout_clk = params->timeout/FNET_TIMER_PERIOD_MS;
    if(fnet_ping_if.timeout_clk == 0)
        fnet_ping_if.timeout_clk = 1;
    fnet_ping_if.family = params->target_addr.sa_family;
    fnet_ping_if.packet_count = params->packet_count;
    fnet_ping_if.pattern = params->pattern;
    fnet_ping_if.packet_size = params->packet_size;
    if(fnet_ping_if.packet_size > FNET_CFG_PING_PACKET_MAX)
        fnet_ping_if.packet_size = FNET_CFG_PING_PACKET_MAX;
    fnet_ping_if.target_addr  = params->target_addr; 
       
    /* Create socket */
    if((fnet_ping_if.socket_foreign = socket(fnet_ping_if.family, SOCK_RAW, (params->target_addr.sa_family == AF_INET) ? IPPROTO_ICMP : IPPROTO_ICMPV6)) == SOCKET_INVALID)
    {
        FNET_DEBUG_PING(FNET_PING_ERR_SOCKET_CREATION);
        goto ERROR;
    }
    
    /* Set Socket options. */
#if FNET_CFG_IP4    
    if(fnet_ping_if.family == AF_INET)    
        setsockopt(fnet_ping_if.socket_foreign, IPPROTO_IP, IP_TTL, (char *) &params->ttl, sizeof(params->ttl));
#endif

#if FNET_CFG_IP6        
    if(fnet_ping_if.family == AF_INET6)
        setsockopt(fnet_ping_if.socket_foreign, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (char *) &params->ttl, sizeof(params->ttl));
#endif        
            
    setsockopt(fnet_ping_if.socket_foreign, SOL_SOCKET, SO_RCVBUF, (char *) &bufsize_option, sizeof(bufsize_option));
    setsockopt(fnet_ping_if.socket_foreign, SOL_SOCKET, SO_SNDBUF, (char *) &bufsize_option, sizeof(bufsize_option));

    /* Register PING service. */
    fnet_ping_if.service_descriptor = fnet_poll_service_register(fnet_ping_state_machine, (void *) &fnet_ping_if);
    if(fnet_ping_if.service_descriptor == (fnet_poll_desc_t)FNET_ERR)
    {
        FNET_DEBUG_PING(FNET_PING_ERR_SERVICE);
        goto ERROR_1;
    }
    
    fnet_ping_if.state = FNET_PING_STATE_SENDING_REQUEST;
    
    return FNET_OK;
    
ERROR_1:
    closesocket(fnet_ping_if.socket_foreign);

ERROR:
    return FNET_ERR;
}

/************************************************************************
* NAME: fnet_ping_state_machine
*
* DESCRIPTION: PING service state machine.
************************************************************************/
static void fnet_ping_state_machine(void *fnet_ping_if_p)
{
    int                     received;    
    fnet_icmp_echo_header_t *hdr;
    fnet_ping_if_t          *ping_if = (fnet_ping_if_t *)fnet_ping_if_p;
    struct sockaddr         addr;
    int                     addr_len = sizeof(addr);

    switch(ping_if->state)
    {
        /*===================================*/ 
        case FNET_PING_STATE_SENDING_REQUEST:
            /* Build message.*/
            hdr = (fnet_icmp_echo_header_t *)&fnet_ping_if.buffer[0];

            /* Fill ICMP Echo request header.*/
            fnet_memset_zero(hdr, sizeof(*hdr));
            hdr->header.type = (unsigned char)((fnet_ping_if.family == AF_INET) ? FNET_ICMP_ECHO: FNET_ICMP6_TYPE_ECHO_REQ);
            hdr->identifier = FNET_CFG_PING_IDENTIFIER;
            fnet_ping_if.sequence_number++;
            hdr->sequence_number = fnet_htons(fnet_ping_if.sequence_number);
            
            /* Fill payload data by pattern.*/
            fnet_memset(&fnet_ping_if.buffer[sizeof(*hdr)], ping_if->pattern, ping_if->packet_size);
                
            /* Checksum.*/
#if FNET_CFG_IP4
            if(ping_if->family == AF_INET)
            {
                hdr->header.checksum = fnet_checksum_buf(&fnet_ping_if.buffer[0], (int)(sizeof(*hdr) + ping_if->packet_size));
            }
            else
#endif  
#if FNET_CFG_IP6
            if(ping_if->family == AF_INET6)
            {
                fnet_ip6_addr_t   *src_ip = (fnet_ip6_addr_t *)fnet_ip6_select_src_addr(FNET_NULL, (fnet_ip6_addr_t *)ping_if->target_addr.sa_data); //TBD //DM Check result.

                hdr->header.checksum = fnet_checksum_pseudo_buf(&fnet_ping_if.buffer[0], 
                                                                (unsigned short)(sizeof(*hdr) + ping_if->packet_size), 
                                                                FNET_HTONS(IPPROTO_ICMPV6), 
                                                                (char *)src_ip,
                                                                ping_if->target_addr.sa_data, 
                                                                sizeof(fnet_ip6_addr_t));
            }
            else
#endif 
            {};
            
            /* Send request.*/    
            sendto(fnet_ping_if.socket_foreign, (char*)(&fnet_ping_if.buffer[0]), (int)(sizeof(*hdr) + ping_if->packet_size), 0,  &ping_if->target_addr, sizeof(ping_if->target_addr));
            ping_if->packet_count--;
           
            fnet_ping_if.send_time = fnet_timer_ticks();        
            
            ping_if->state = FNET_PING_STATE_WAITING_REPLY;
            break;
        /*===================================*/    
        case  FNET_PING_STATE_WAITING_REPLY:
            /* Receive data */
            
            received = recvfrom(ping_if->socket_foreign, (char*)(&ping_if->buffer[0]), FNET_PING_BUFFER_SIZE, 0, &addr, &addr_len );
            
            if(received > 0 )
            {
                unsigned short  checksum = 0;
                
                hdr = (fnet_icmp_echo_header_t *)(ping_if->buffer);
                
                
                /* Check checksum.*/
#if FNET_CFG_IP4
                if(ping_if->family == AF_INET)
                {
                    checksum = fnet_checksum_buf(&fnet_ping_if.buffer[0], received);
                }
                else
#endif  
#if 0 /* #if FNET_CFG_IP6  */ // TBD case to receive from multicast address ff02::1
                if(ping_if->family == AF_INET6)
                {
                     checksum = fnet_checksum_pseudo_buf(&fnet_ping_if.buffer[0], 
                                                                (unsigned short)(received), 
                                                                IPPROTO_ICMPV6, 
                                                                ping_if->local_addr.sa_data,
                                                                ping_if->target_addr.sa_data, 
                                                                sizeof(fnet_ip6_addr_t));
                }
                else    
#endif                               
                {}; 
                /* Check header.*/
                if( checksum
                    ||(hdr->header.type != (addr.sa_family == AF_INET) ? FNET_ICMP_ECHOREPLY: FNET_ICMP6_TYPE_ECHO_REPLY)
                    ||(hdr->identifier != FNET_CFG_PING_IDENTIFIER)
                    ||(hdr->sequence_number != fnet_htons(ping_if->sequence_number)) )
                {
                    goto NO_DATA;
                }     
                
                /* Call handler.*/
                if(ping_if->handler)                 
                    ping_if->handler(FNET_OK, ping_if->packet_count, &addr, ping_if->handler_cookie);
                    
                if(ping_if->packet_count)
                    ping_if->state = FNET_PING_STATE_WAITING_TIMEOUT;
                else
                    fnet_ping_release();              
            }
            else if(received == SOCKET_ERROR)
            {
                /* Call handler.*/
                if(ping_if->handler)
                {
                    int     sock_err ;
                    int     option_len;
                    
                    /* Get socket error.*/
                    option_len = sizeof(sock_err); 
                    getsockopt(ping_if->socket_foreign, SOL_SOCKET, SO_ERROR, (char*)&sock_err, &option_len);
                                 
                    ping_if->handler(sock_err, ping_if->packet_count, FNET_NULL, ping_if->handler_cookie);
                }
                
                
                if(ping_if->packet_count)
                    ping_if->state = FNET_PING_STATE_WAITING_TIMEOUT;
                else
                    fnet_ping_release();
                    
            }
            else /* No data. Check timeout */
            {
NO_DATA:            
                if(fnet_timer_get_interval(fnet_ping_if.send_time, fnet_timer_ticks()) > fnet_ping_if.timeout_clk)
                {
                    /* Call handler.*/
                    if(ping_if->handler)                 
                        ping_if->handler(FNET_ERR_TIMEDOUT, ping_if->packet_count, FNET_NULL, ping_if->handler_cookie);
                        
                    if(ping_if->packet_count)
                        ping_if->state = FNET_PING_STATE_SENDING_REQUEST;
                    else
                        fnet_ping_release();    
                }
            }
            break;
        /*===================================*/             
        case FNET_PING_STATE_WAITING_TIMEOUT:
            if(fnet_timer_get_interval(fnet_ping_if.send_time, fnet_timer_ticks()) > fnet_ping_if.timeout_clk)
            {
                ping_if->state = FNET_PING_STATE_SENDING_REQUEST;
            }
            break;
    }
}

/************************************************************************
* NAME: fnet_ping_release
*
* DESCRIPTION: Releases the PING service.
************************************************************************/ 
void fnet_ping_release( void )
{
    if(fnet_ping_if.state != FNET_PING_STATE_DISABLED)
    {
        /* Close socket. */
        closesocket(fnet_ping_if.socket_foreign);
    
        /* Unregister the tftp service. */
        fnet_poll_service_unregister(fnet_ping_if.service_descriptor);
    
        fnet_ping_if.state = FNET_PING_STATE_DISABLED; 
    }
}

/************************************************************************
* NAME: fnet_ping_state
*
* DESCRIPTION: Retrieves the current state of the PING service 
*              (for debugging purposes).
************************************************************************/
fnet_ping_state_t fnet_ping_state( void )
{
    return fnet_ping_if.state;
}


#endif /* FNET_CFG_PING */

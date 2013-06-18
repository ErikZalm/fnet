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
* @file fnet_dns.c
*
* @author Andrey Butok
*
* @date Jan-16-2013
*
* @version 0.1.11.0
*
* @brief DNS Resolver implementation.
*
***************************************************************************/


#include "fnet_config.h"

#if FNET_CFG_DNS_RESOLVER

#include "fnet_dns.h"

#if FNET_CFG_DEBUG_DNS    
    #define FNET_DEBUG_DNS   FNET_DEBUG
#else
    #define FNET_DEBUG_DNS(...)
#endif

/************************************************************************
*     Definitions
*************************************************************************/

#define FNET_DNS_ERR_PARAMS            "ERROR: Wrong input parameters."
#define FNET_DNS_ERR_SOCKET_CREATION   "ERROR: Socket creation error."
#define FNET_DNS_ERR_SOCKET_CONNECT    "ERROR: Socket Error during connect."
#define FNET_DNS_ERR_SERVICE           "ERROR: Service registration is failed."
#define FNET_DNS_ERR_IS_INITIALIZED    "ERROR: DNS is already initialized."

/* Size limits. */
#define FNET_DNS_MAME_SIZE      (255)     /*
                                           * RFC1035:To simplify implementations, the total length of a domain name (i.e.,
                                           * label octets and label length octets) is restricted to 255 octets or less.
                                           */
#define FNET_DNS_MESSAGE_SIZE   (512)     /* Messages carried by UDP are restricted to 512 bytes (not counting the IP
                                           * or UDP headers).  
                                           * Longer messages (not supported) are truncated and the TC bit is set in
                                           * the header.
                                           */    


/************************************************************************
*    DNS header [RFC1035, 4.1.1.]
*************************************************************************
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/  
    
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned short id FNET_COMP_PACKED;      /* A 16 bit identifier assigned by the program that
                             * generates any kind of query. This identifier is copied
                             * the corresponding reply and can be used by the requester
                             * to match up replies to outstanding queries. */
    unsigned short flags FNET_COMP_PACKED;   /* Flags.*/
    unsigned short qdcount FNET_COMP_PACKED; /* An unsigned 16 bit integer specifying the number of
                             * entries in the question section.*/
    unsigned short ancount FNET_COMP_PACKED; /* An unsigned 16 bit integer specifying the number of
                             * resource records in the answer section.*/
    unsigned short nscount FNET_COMP_PACKED; /* an unsigned 16 bit integer specifying the number of name
                             * server resource records in the authority records
                             * section.*/
    unsigned short arcount FNET_COMP_PACKED; /* An unsigned 16 bit integer specifying the number of
                             * resource records in the additional records section.*/

} fnet_dns_header_t;
FNET_COMP_PACKED_END

#define FNET_DNS_HEADER_FLAGS_QR    (0x8000) /* Query (0), Response (1)*/
#define FNET_DNS_HEADER_FLAGS_AA    (0x0400) /* Authoritative Answer. */
#define FNET_DNS_HEADER_FLAGS_TC    (0x0200) /* TrunCation. */
#define FNET_DNS_HEADER_FLAGS_RD    (0x0100) /* Recursion Desired. */
#define FNET_DNS_HEADER_FLAGS_RA    (0x0080) /* Recursion Available. */

/************************************************************************
*    DNS Question section [RFC1035, 4.1.2.]
*************************************************************************
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/  
    
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned char zero_length FNET_COMP_PACKED;  /* The domain name terminates with the
                                 * zero length octet for the null label of the root. */
    unsigned short qtype FNET_COMP_PACKED;       /* Specifies the type of the query.*/
    unsigned short qclass FNET_COMP_PACKED;      /* Specifies the class of the query.*/

} fnet_dns_q_tail_t;
FNET_COMP_PACKED_END


/************************************************************************
*    DNS Resource Record header [RFC1035, 4.1.3.] with message compression
*************************************************************************
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                                               /
    /                      NAME                     /
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     CLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TTL                      |
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                   RDLENGTH                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RDATA                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/  
    
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned char name_ptr[2] FNET_COMP_PACKED; /* A domain name to which this resource record pertains.
                                * For compression, it is replaced with a pointer to a prior occurance
                                * of the same name */
    unsigned short type FNET_COMP_PACKED;    /* This field specifies the meaning of the data in the RDATA
                             * field.*/
    unsigned short class FNET_COMP_PACKED;   /* An unsigned 16 bit integer specifying the number of
                             * entries in the question section.*/
    unsigned long ttl FNET_COMP_PACKED;      /* Specifies the time interval (in seconds) that the 
                             * resource record may be
                             * cached before it should be discarded.*/
    unsigned short rdlength FNET_COMP_PACKED;/* Length in octets of the RDATA field.*/
    unsigned long rdata FNET_COMP_PACKED;   /* The format of this information varies
                             * according to the TYPE and CLASS of the resource record. 
                             * If the TYPE is A and the CLASS is IN,
                             * the RDATA field is a 4 octet ARPA Internet address.*/

} fnet_dns_rr_header_t;
FNET_COMP_PACKED_END

#define FNET_DNS_HEADER_TYPE_A          (0x01)  /* Host address.*/
#define FNET_DNS_HEADER_CLASS_IN        (0x01)  /* The Internet.*/
#define FNET_DNS_RR_HEADER_RDLENGTH_4   (0x04)  /* 32bit IPv4 address.*/

static void fnet_dns_state_machine(void *);

/************************************************************************
*    DNS-client interface structure.
*************************************************************************/
typedef struct
{
    SOCKET socket_cln;
    fnet_poll_desc_t service_descriptor;
    fnet_dns_state_t state;                /* Current state. */
    fnet_dns_handler_resolved_t handler;   /* Callback function. */
    long handler_cookie;                   /* Callback-handler specific parameter. */
    unsigned long last_time;               /* Last receive time, used for timeout detection. */
    int iteration;                         /* Current iteration number.*/
    char message[FNET_DNS_MESSAGE_SIZE];   /* Message buffer. */
    unsigned long message_size;            /* Size of the message.*/
    fnet_ip4_addr_t result;
    unsigned short id;
} 
fnet_dns_if_t;


/* DNS-client interface */
static fnet_dns_if_t fnet_dns_if;

/************************************************************************
* NAME: fnet_dns_init
*
* DESCRIPTION: Initializes DNS client service and starts the host 
*              name reolving.
************************************************************************/
int fnet_dns_init( struct fnet_dns_params *params )
{
    const unsigned long bufsize_option = FNET_DNS_MESSAGE_SIZE;
    int                 total_length;
    int                 label_length;
    unsigned long       host_name_length;
    struct sockaddr     local_addr;
    fnet_dns_header_t   *header;
    fnet_dns_q_tail_t   *q_tail;
    char                *strtok_pos = FNET_NULL;
  
    /* Check input parameters. */
    if((params == 0) || (params->dns_server == 0) || (params->handler == 0) ||
       /* Check length of host_name.*/
       ((host_name_length = fnet_strlen(params->host_name)) == 0) || (host_name_length >= FNET_DNS_MAME_SIZE))
    {
        FNET_DEBUG_DNS(FNET_DNS_ERR_PARAMS);
        goto ERROR;
    }
    
    /* Check if DNS service is free.*/
    if(fnet_dns_if.state != FNET_DNS_STATE_DISABLED)
    {
        FNET_DEBUG_DNS(FNET_DNS_ERR_IS_INITIALIZED);
        goto ERROR;
    }
    
    /* Save input parmeters.*/
    fnet_dns_if.handler = params->handler;
    fnet_dns_if.handler_cookie = params->cookie;
    
    
    fnet_dns_if.iteration = 0;  /* Reset iteration counter.*/
    fnet_dns_if.id++;           /* Change query ID.*/
   
    /* Create socket */
    if((fnet_dns_if.socket_cln = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_INVALID)
    {
        FNET_DEBUG_DNS(FNET_DNS_ERR_SOCKET_CREATION);
        goto ERROR;
    }
    
    /* Set socket options */
    setsockopt(fnet_dns_if.socket_cln, SOL_SOCKET, SO_RCVBUF, (char *) &bufsize_option, sizeof(bufsize_option));
    setsockopt(fnet_dns_if.socket_cln, SOL_SOCKET, SO_SNDBUF, (char *) &bufsize_option, sizeof(bufsize_option));
    
    /* Bind/connect to the server.*/
    FNET_DEBUG_DNS("Connecting to DNS Server.");
    fnet_memset_zero(&local_addr, sizeof(local_addr));
    ((struct sockaddr_in *)(&local_addr))->sin_addr.s_addr = params->dns_server;
    ((struct sockaddr_in *)(&local_addr))->sin_port = FNET_CFG_DNS_PORT;    
    ((struct sockaddr_in *)(&local_addr))->sin_family = AF_INET;
    
    if(connect(fnet_dns_if.socket_cln, (struct sockaddr *)(&local_addr), sizeof(local_addr))== FNET_ERR)
    {
        FNET_DEBUG_DNS(FNET_DNS_ERR_SOCKET_CONNECT);
        goto ERROR_1;
    }
    
    /* ==== Build message. ==== */
    fnet_memset_zero(fnet_dns_if.message, sizeof(fnet_dns_if.message)); /* Clear buffer.*/
     
    /* Set header fields:
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    */    
    
    header = (fnet_dns_header_t *)fnet_dns_if.message;
    
    header->id = fnet_dns_if.id;            /* Set ID. */
    
    header->flags = FNET_HTONS(FNET_DNS_HEADER_FLAGS_RD); /* Recursion Desired.*/
   
    header->qdcount = FNET_HTONS(1);        /* One Question. */
    
    /* No Answer (ANCOUNT).*/ /* No Authority (NSCOUNT). */ /* No Additional (ARCOUNT). */

    
    /* Set Question section :    
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    */ 


    /* QNAME */
    /* a domain name represented as a sequence of labels, where
    * each label consists of a length octet followed by that
    * number of octets. The domain name terminates with the
    * zero length octet for the null label of the root. Note
    * that this field may be an odd number of octets; no
    * padding is used.
    */
 
    /* Copy host_name string.*/
    fnet_strcpy(&fnet_dns_if.message[sizeof(fnet_dns_header_t)+1], params->host_name); 
    
    total_length = sizeof(fnet_dns_header_t);
 
    //TBD Place for improvement, use strtok_pos as pointer.
    
    /* Replace '.' by zero.*/
    fnet_strtok_r(&fnet_dns_if.message[sizeof(fnet_dns_header_t)+1], ".", &strtok_pos);

    while((label_length = (int)fnet_strlen(&fnet_dns_if.message[total_length]+1)) > 0)
    {
        fnet_dns_if.message[total_length] = (char)label_length; /* Set length before (previous) label.*/
        total_length += label_length + 1;
       
        fnet_strtok_r(FNET_NULL,".", &strtok_pos);
    }
    
    q_tail = (fnet_dns_q_tail_t *)&fnet_dns_if.message[total_length];
    
    /* Skip 1 byte (zero). End of string. */

    /* QTYPE */
    q_tail->qtype = FNET_HTONS(FNET_DNS_HEADER_TYPE_A);
    
    /* QCLASS */
    q_tail->qclass = FNET_HTONS(FNET_DNS_HEADER_CLASS_IN);
    
    
    fnet_dns_if.message_size = total_length + 1 + sizeof(fnet_dns_q_tail_t);
   

    /* Register DNS service. */
    fnet_dns_if.service_descriptor = fnet_poll_service_register(fnet_dns_state_machine, (void *) &fnet_dns_if);
    if(fnet_dns_if.service_descriptor == (fnet_poll_desc_t)FNET_ERR)
    {
        FNET_DEBUG_DNS(FNET_DNS_ERR_SERVICE);
        goto ERROR_1;
    }
    
    /* Check if the input string is IP address "x.x.x.x". */
    if( fnet_inet_aton(params->host_name, (struct in_addr *) &fnet_dns_if.result) == FNET_OK) /* TFTP server IP*/
    {
        fnet_dns_if.state = FNET_DNS_STATE_RELEASE;
    }
    else /* Send DNS request. */
    {
        fnet_dns_if.result = (fnet_ip4_addr_t)FNET_ERR;
        fnet_dns_if.state = FNET_DNS_STATE_TX; /* => Send request. */    
    }
    
    return FNET_OK;
ERROR_1:
    closesocket(fnet_dns_if.socket_cln);

ERROR:
    return FNET_ERR;
}

/************************************************************************
* NAME: fnet_dns_state_machine
*
* DESCRIPTION: DNS-client state machine.
************************************************************************/
static void fnet_dns_state_machine( void *fnet_dns_if_p )
{
    int sent_size;
    int received;    
    int i;
    fnet_dns_header_t *header;
    fnet_dns_rr_header_t *rr_header;
    fnet_dns_if_t *dns_if = (fnet_dns_if_t *)fnet_dns_if_p;

    switch(dns_if->state)
    {
        /*---- TX --------------------------------------------*/
        case FNET_DNS_STATE_TX:

            FNET_DEBUG_DNS("Sending query...");
            sent_size = (int)send(dns_if->socket_cln, dns_if->message, (int)dns_if->message_size, 0);
            
            if (sent_size != dns_if->message_size)
        	{
        		dns_if->state = FNET_DNS_STATE_RELEASE; /* ERROR */
        	}	
            else
            {
                dns_if->last_time = fnet_timer_ticks();
                dns_if->state = FNET_DNS_STATE_RX;
            }		
            break; 
        /*---- RX -----------------------------------------------*/
        case  FNET_DNS_STATE_RX:
            /* Receive data */
            
            received = recv(dns_if->socket_cln, dns_if->message, sizeof(dns_if->message), 0);
            
            if(received > 0 )
            {
                header = (fnet_dns_header_t *)fnet_dns_if.message;
                
                if((header->id == dns_if->id) && /* Check the ID.*/
                   (header->flags & FNET_DNS_HEADER_FLAGS_QR)) /* Is response.*/
                {
                    for (i=(sizeof(fnet_dns_header_t)-1); i < received; i++)
                    {
                        /* [RFC1035 4.1.4.] In order to reduce the size of messages, the domain system utilizes a
                        * compression scheme which eliminates the repetition of domain names in a
                        * message. In this scheme, an entire domain name or a list of labels at
                        * the end of a domain name is replaced with a pointer to a prior occurance
                        * of the same name.
                        * The pointer takes the form of a two octet sequence:
                        * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                        * | 1  1|                OFFSET                   |
                        * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
                        */
                        /* => Check for 0xC0. */
                        if ((unsigned char)dns_if->message[i] == 0xC0) //look for the beginnig of the response (Question Name == 192 (label compression))
                        {
                            rr_header = (fnet_dns_rr_header_t *)&dns_if->message[i]; 


                            /* Check Question Type, Class and Resource Data Lenght. */
                            if ( (rr_header->type == FNET_HTONS(FNET_DNS_HEADER_TYPE_A)) && 
                                 (rr_header->class == FNET_HTONS(FNET_DNS_HEADER_CLASS_IN)) && 
                                 (rr_header->rdlength == FNET_HTONS(FNET_DNS_RR_HEADER_RDLENGTH_4)) ) 
                            {
                                /* Resolved.*/
                                dns_if->result = rr_header->rdata; /* Save IP address.*/
                                
                                break; /* Current version takes the first provided IP address.*/
                            }
                        }
                    }
                }
                /* else = wrong message.*/
                
                dns_if->state = FNET_DNS_STATE_RELEASE;
            }
            else if(received == SOCKET_ERROR) /* Check error.*/
            {
                dns_if->state = FNET_DNS_STATE_RELEASE; /* ERROR */
            }
            else /* No data. Check timeout */
            if(fnet_timer_get_interval(dns_if->last_time, fnet_timer_ticks()) > ((FNET_CFG_DNS_RETRANSMISSION_TIMEOUT*1000)/FNET_TIMER_PERIOD_MS))
            {
                dns_if->iteration++;
                
                if(dns_if->iteration > FNET_CFG_DNS_RETRANSMISSION_MAX)
                {
                    dns_if->state = FNET_DNS_STATE_RELEASE; /* ERROR */
                }
                else
                {
                    dns_if->state = FNET_DNS_STATE_TX;
                }
            }
            break;
         /*---- RELEASE -------------------------------------------------*/    
        case FNET_DNS_STATE_RELEASE:
            fnet_dns_release(); 
            dns_if->handler(dns_if->result, dns_if->handler_cookie); /* User Callback.*/
            break;
        default:
            break;            
    }

}

/************************************************************************
* NAME: fnet_dns_release
*
* DESCRIPTION: This function aborts the resolving and releases 
* the DNS-client service.
************************************************************************/ 
void fnet_dns_release( void )
{
    if(fnet_dns_if.state != FNET_DNS_STATE_DISABLED)
    {
        /* Close socket. */
        closesocket(fnet_dns_if.socket_cln);
    
        /* Unregister the tftp service. */
        fnet_poll_service_unregister( fnet_dns_if.service_descriptor );
    
        fnet_dns_if.state = FNET_DNS_STATE_DISABLED; 
    }
}

/************************************************************************
* NAME: fnet_dns_state
*
* DESCRIPTION: This function returns a current state of the DNS client.
************************************************************************/
fnet_dns_state_t fnet_dns_state( void )
{
    return fnet_dns_if.state;
}


#endif /* FNET_CFG_DNS_RESOLVER */

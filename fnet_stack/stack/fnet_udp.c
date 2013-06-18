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
* @file fnet_udp.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.62.0
*
* @brief UDP protocol implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_udp.h"
#include "fnet_ip_prv.h"
#include "fnet_timer.h"
#include "fnet_stdlib.h"

#include "fnet_isr.h"
#include "fnet_checksum.h"
#include "fnet_prot.h"
#include "fnet_icmp.h"

#if FNET_CFG_UDP

/************************************************************************
*     Function Prototypes
*************************************************************************/
static int fnet_udp_attach( fnet_socket_t *sk );
static int fnet_udp_detach( fnet_socket_t *sk );
static int fnet_udp_connect( fnet_socket_t *sk, struct sockaddr *foreign_addr);
static int fnet_udp_snd( fnet_socket_t *sk, char *buf, int len, int flags, const struct sockaddr *foreign_addr);
static int fnet_udp_rcv( fnet_socket_t *sk, char *buf, int len, int flags, struct sockaddr *foreign_addr);
static void fnet_udp_control_input( fnet_prot_notify_t command, fnet_ip_header_t *ip_hdr );
static int fnet_udp_shutdown( fnet_socket_t *sk, int how );
static void fnet_udp_input( fnet_netif_t *netif, struct sockaddr *foreign_addr,  struct sockaddr *local_addr, fnet_netbuf_t *nb, fnet_netbuf_t *ip_nb);

#if FNET_CFG_DEBUG_TRACE_UDP
    void fnet_udp_trace(char *str, fnet_udp_header_t *udp_hdr);
#else
    #define fnet_udp_trace(str, udp_hdr)
#endif

/************************************************************************
*     Global Data Structures
*************************************************************************/

/************************************************************************
 * Protocol API structures.
 ************************************************************************/
static const fnet_socket_prot_if_t fnet_udp_socket_api =
{
    0,                      /* Flag that protocol is connection oriented.*/
    fnet_udp_attach,        /* Protocol "attach" function.*/
    fnet_udp_detach,        /* Protocol "detach" function.*/
    fnet_udp_connect,       /* Protocol "connect" function.*/
    0,                      /* Protocol "accept" function.*/
    fnet_udp_rcv,           /* Protocol "receive" function.*/
    fnet_udp_snd,           /* Protocol "send" function.*/
    fnet_udp_shutdown,      /* Protocol "shutdown" function.*/
    fnet_ip_setsockopt,     /* Protocol "setsockopt" function.*/
    fnet_ip_getsockopt,     /* Protocol "getsockopt" function.*/
    0                       /* Protocol "listen" function.*/
};

fnet_prot_if_t fnet_udp_prot_if =
{
    0,                      /* Pointer to the head of the protocol's socket list.*/
    AF_SUPPORTED,           /* Address domain family.*/
    SOCK_DGRAM,             /* Socket type used for.*/
    FNET_IP_PROTOCOL_UDP,   /* Protocol number.*/   
    0,                      /* Protocol initialization function.*/
    fnet_udp_release,       /* Protocol release function.*/
#if FNET_CFG_IP4     
    fnet_udp_input_ip4,     /* Protocol input function, from IPv4.*/
#endif    
    fnet_udp_control_input, /* Protocol input control function.*/     
    0,                      /* protocol drain function.*/
#if FNET_CFG_IP6    
    fnet_udp_input_ip6,     /* Protocol input function, from IPv6.*/
#endif /* FNET_CFG_IP6 */      
    &fnet_udp_socket_api    /* Socket API */
};

/************************************************************************
* NAME: fnet_udp_release
*
* DESCRIPTION: This function releases all sockets associated 
*              with UDP protocol. 
*************************************************************************/
static void fnet_udp_release( void )
{
    while(fnet_udp_prot_if.head)
      fnet_socket_release(&fnet_udp_prot_if.head, fnet_udp_prot_if.head);
}

/************************************************************************
* NAME: fnet_udp_output
*
* DESCRIPTION: UDP output function
*************************************************************************/
static int fnet_udp_output(  struct sockaddr *src_addr, const struct sockaddr *dest_addr,
                             fnet_socket_option_t *sockoption, fnet_netbuf_t *nb )                            
{
    fnet_netbuf_t                           *nb_header;
    fnet_udp_header_t                       *udp_header;
    int                                     error =  FNET_OK;
    FNET_COMP_PACKED_VAR unsigned short     *checksum_p;
    fnet_netif_t                            *netif;

#if FNET_CFG_IP6    
    if(dest_addr->sa_family == AF_INET6)
    {
        /* Check Scope ID.*/
        netif = fnet_netif_get_by_scope_id( ((struct sockaddr_in6 *)dest_addr)->sin6_scope_id );
    }
    else
#endif
        netif = FNET_NULL;  

    /* Construct UDP header.*/
    if((nb_header = fnet_netbuf_new(sizeof(fnet_udp_header_t), FNET_TRUE)) == 0)
    {
        fnet_netbuf_free_chain(nb); 
        return (FNET_ERR_NOMEM);
    }

    udp_header = nb_header->data_ptr;

    udp_header->source_port = src_addr->sa_port;             /* Source port number.*/
    udp_header->destination_port = dest_addr->sa_port;       /* Destination port number.*/
    nb = fnet_netbuf_concat(nb_header, nb);
    udp_header->length = fnet_htons((unsigned short)nb->total_length);  /* Length.*/

    /* Checksum calculation.*/
    udp_header->checksum = 0;  

#if FNET_CFG_UDP_CHECKSUM

#if FNET_CFG_CPU_ETH_HW_TX_IP_CHECKSUM
    if( 0 
#if FNET_CFG_IP4
        ||( (dest_addr->sa_family == AF_INET) 
        && ((netif = fnet_ip_route(((struct sockaddr_in *)(dest_addr))->sin_addr.s_addr))!= FNET_NULL)
        && (netif->features & FNET_NETIF_FEATURE_HW_TX_PROTOCOL_CHECKSUM)
        && (fnet_ip_will_fragment(netif, nb->total_length) == FNET_FALSE) /* Fragmented packets are not inspected.*/  ) 
#endif
#if FNET_CFG_IP6
        ||( (dest_addr->sa_family == AF_INET6) 
        && (netif || (((netif = fnet_ip6_route(&((struct sockaddr_in6 *)(src_addr))->sin6_addr.s6_addr, &((struct sockaddr_in6 *)(dest_addr))->sin6_addr.s6_addr)))!= FNET_NULL) )
        && (netif->features & FNET_NETIF_FEATURE_HW_TX_PROTOCOL_CHECKSUM)
        && (fnet_ip6_will_fragment(netif, nb->total_length) == FNET_FALSE) /* Fragmented packets are not inspected.*/  ) 
#endif
    )
    {
        nb->flags |= FNET_NETBUF_FLAG_HW_PROTOCOL_CHECKSUM;
        checksum_p = 0;
    }
    else
#endif /* FNET_CFG_CPU_ETH_HW_TX_IP_CHECKSUM */
    {
        udp_header->checksum = fnet_checksum_pseudo_start( nb, FNET_HTONS((unsigned short)FNET_IP_PROTOCOL_UDP), (unsigned short)nb->total_length );
        checksum_p = &udp_header->checksum;
    }
#endif /* FNET_CFG_UDP_CHECKSUM */

#if FNET_CFG_IP4
    if(dest_addr->sa_family == AF_INET)
    {
        error = fnet_ip_output(netif, 
                                ((struct sockaddr_in *)(src_addr))->sin_addr.s_addr, 
                                ((struct sockaddr_in *)(dest_addr))->sin_addr.s_addr, 
                                FNET_IP_PROTOCOL_UDP, sockoption->ip_opt.tos,
                            #if FNET_CFG_MULTICAST
                                (unsigned char)((FNET_IP4_ADDR_IS_MULTICAST(((struct sockaddr_in *)(dest_addr))->sin_addr.s_addr)?sockoption->ip_opt.ttl_multicast:sockoption->ip_opt.ttl)),                               
                            #else
                                sockoption->ip_opt.ttl, 
                            #endif /* FNET_CFG_MULTICAST */                               
                                nb, FNET_UDP_DF, ((sockoption->flags & SO_DONTROUTE) > 0),
                                checksum_p
                                );
    }
    else
#endif
#if FNET_CFG_IP6    
    if(dest_addr->sa_family == AF_INET6)
    {
        error = fnet_ip6_output( netif, 
                                fnet_socket_addr_is_unspecified(src_addr)? FNET_NULL : &((struct sockaddr_in6 *)(src_addr))->sin6_addr.s6_addr, 
                                &((struct sockaddr_in6 *)(dest_addr))->sin6_addr.s6_addr, 
                                FNET_IP_PROTOCOL_UDP, sockoption->ip6_opt.unicast_hops, nb, 
                                checksum_p
                                );
    }
    else
#endif                               
    {};

    return (error);
}

/************************************************************************
* NAME: fnet_udp_input_ip4
*
* DESCRIPTION: UDP input function, from IPv4.
*************************************************************************/
#if FNET_CFG_IP4 
static void fnet_udp_input_ip4( fnet_netif_t *netif, fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip,
                           fnet_netbuf_t *nb, fnet_netbuf_t *ip4_nb)
{
    struct sockaddr     src_addr;
    struct sockaddr     dest_addr;
    
    fnet_memset_zero(&src_addr, sizeof(struct sockaddr));
     src_addr.sa_family = AF_INET;
    ((struct sockaddr_in*)(&src_addr))->sin_addr.s_addr = src_ip;
    
    fnet_memset_zero(&dest_addr, sizeof(struct sockaddr));
    dest_addr.sa_family = AF_INET;
    ((struct sockaddr_in*)(&dest_addr))->sin_addr.s_addr = dest_ip;

    fnet_udp_input(netif, &src_addr,  &dest_addr, nb, ip4_nb);    
}
#endif /* FNET_CFG_IP4 */

/************************************************************************
* NAME: fnet_udp_input_ip6
*
* DESCRIPTION: UDP input function, from IPv6.
*************************************************************************/
#if FNET_CFG_IP6 
static void fnet_udp_input_ip6(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb)
{
    struct sockaddr     src_addr;
    struct sockaddr     dest_addr;
    
    fnet_memset_zero(&src_addr, sizeof(struct sockaddr));
    src_addr.sa_family = AF_INET6;
    FNET_IP6_ADDR_COPY(src_ip, &((struct sockaddr_in6 *)(&src_addr))->sin6_addr.s6_addr);
    ((struct sockaddr_in6 *)(&src_addr))->sin6_scope_id = netif->scope_id;
    
    
    fnet_memset_zero(&dest_addr, sizeof(struct sockaddr));
    dest_addr.sa_family = AF_INET6;
    FNET_IP6_ADDR_COPY(dest_ip, &((struct sockaddr_in6 *)(&dest_addr))->sin6_addr.s6_addr);
    ((struct sockaddr_in6 *)(&dest_addr))->sin6_scope_id = netif->scope_id;

    fnet_udp_input(netif, &src_addr,  &dest_addr, nb, ip6_nb);    
}
#endif /* FNET_CFG_IP6*/

/************************************************************************
* NAME: fnet_udp_input
*
* DESCRIPTION: UDP input function.
*************************************************************************/
static void fnet_udp_input(fnet_netif_t *netif, struct sockaddr *foreign_addr,  struct sockaddr *local_addr, fnet_netbuf_t *nb, fnet_netbuf_t *ip_nb)
{
    fnet_udp_header_t   *udp_header = nb->data_ptr;
    fnet_socket_t       *sock;
    fnet_socket_t       *last;
    fnet_netbuf_t       *nb_tmp;
    unsigned long       udp_length;

    if((netif != 0) && (nb != 0))
    {
        /* The header must reside in contiguous area of memory.*/
        if((nb_tmp = fnet_netbuf_pullup(nb, sizeof(fnet_udp_header_t))) == 0) 
        {
            goto BAD;
        }
        
        nb = nb_tmp;
        
        udp_length = fnet_ntohs(udp_header->length);

        if(nb->total_length >= udp_length)  /* Check the amount of data.*/
        {
            if(nb->total_length > udp_length)
            {
                /* Logical size and the physical size of the packet should be the same.*/
                fnet_netbuf_trim(&nb, (int)(udp_length - nb->total_length)); 
            }

#if FNET_CFG_UDP_CHECKSUM
            if((udp_header->checksum != 0)
        #if FNET_CFG_CPU_ETH_HW_RX_PROTOCOL_CHECKSUM || FNET_CFG_CPU_ETH_HW_TX_PROTOCOL_CHECKSUM
            && ((nb->flags & FNET_NETBUF_FLAG_HW_PROTOCOL_CHECKSUM) == 0)
        #endif
            )
            {
                unsigned short sum;
                
                sum = fnet_checksum_pseudo_start( nb, FNET_HTONS((unsigned short)FNET_IP_PROTOCOL_UDP), (unsigned short)udp_length );
                sum = fnet_checksum_pseudo_end( sum, (char *)foreign_addr->sa_data, (char *)local_addr->sa_data, 
                            (local_addr->sa_family == AF_INET) ? sizeof(fnet_ip4_addr_t) : sizeof(fnet_ip6_addr_t));

                if(sum)
                    goto BAD;
            }
#endif
            fnet_udp_trace("RX", udp_header); /* Trace UDP header.*/

            local_addr->sa_port = udp_header->destination_port;
            foreign_addr->sa_port = udp_header->source_port;

            fnet_netbuf_trim(&nb, sizeof(fnet_udp_header_t));

            /* Demultiplex broadcast & multicast datagrams.*/
            if(fnet_socket_addr_is_broadcast(local_addr, netif) || fnet_socket_addr_is_multicast(local_addr)) 
            {
                last = 0;

                for (sock = fnet_udp_prot_if.head; sock != 0; sock = sock->next)
                {
                    /* Compare local port number.*/
                    if(sock->local_addr.sa_port != local_addr->sa_port)
                        continue; /* => ignore.*/

#if FNET_CFG_MULTICAST
                    if(fnet_socket_addr_is_multicast(local_addr) && (local_addr->sa_family == AF_INET)) //TBD add support for IPv6
                    {
                        int m;
                        int for_us = FNET_FALSE;
                        
                        for(m=0; m < FNET_CFG_MULTICAST_SOCKET_MAX; m++)
                        {
                            if(sock->multicast_entry[m])
                            {
                                if((sock->multicast_entry[m]->group_addr == ((struct sockaddr_in *)(local_addr))->sin_addr.s_addr) &&  (sock->multicast_entry[m]->netif == netif )) 
                                    for_us = FNET_TRUE;       
                            }
                        }
                        
                        if(for_us == FNET_FALSE)
                            continue;
                    }
                    else
#endif /* FNET_CFG_MULTICAST */                   
                    {

                        /* Compare local address.*/
                        if(!fnet_socket_addr_is_unspecified(&sock->local_addr))
                        {
                            if(!fnet_socket_addr_are_equal(&sock->local_addr, local_addr))
                                continue;
                        }

                        /* Compare foreign address and port number.*/
                        if(!fnet_socket_addr_is_unspecified(&sock->foreign_addr))
                        {
                            if((!fnet_socket_addr_are_equal(&sock->foreign_addr, foreign_addr)) || (sock->foreign_addr.sa_port != foreign_addr->sa_port))
                                continue;
                        }
                    
                    }

                    if((last != 0) && (last->receive_buffer.is_shutdown == 0))
                    {
                        if((nb_tmp = fnet_netbuf_copy(nb, 0, FNET_NETBUF_COPYALL, 0)) != 0)
                        {
                            if(fnet_socket_buffer_append_address(&(last->receive_buffer), nb_tmp, foreign_addr) == FNET_ERR)
                            {
                                fnet_netbuf_free_chain(nb_tmp);
                            }
                        }
                    }
                    last = sock;
                }

                if(last == 0)
                    goto BAD;

                if(last->receive_buffer.is_shutdown) /* Is shutdown.*/
                    goto BAD;

                if(fnet_socket_buffer_append_address(&(last->receive_buffer), nb, foreign_addr) == FNET_ERR)
                    goto BAD;
                
                fnet_netbuf_free_chain(ip_nb);                  
            }
            else /* For unicast datagram.*/
            {
                sock = fnet_socket_lookup(fnet_udp_prot_if.head, local_addr, foreign_addr, FNET_IP_PROTOCOL_UDP);

                if(sock)
                {
                    if(sock->receive_buffer.is_shutdown) /* Is shutdown.*/
                        goto BAD;

                    if(fnet_socket_buffer_append_address(&(sock->receive_buffer), nb, foreign_addr) == FNET_ERR)
                        goto BAD;
                    
                    fnet_netbuf_free_chain(ip_nb);
                }
                else
                {
                    fnet_netbuf_free_chain(nb); /* No match was found, send ICMP destination port unreachable.*/
                #if FNET_CFG_IP4                    
                    if(local_addr->sa_family == AF_INET)
                        fnet_icmp_error(netif, FNET_ICMP_UNREACHABLE, FNET_ICMP_UNREACHABLE_PORT, ip_nb);
                #endif
                #if FNET_CFG_IP6                        
                    if(local_addr->sa_family == AF_INET6)
                        fnet_icmp6_error(netif, FNET_ICMP6_TYPE_DEST_UNREACH, FNET_ICMP6_CODE_DU_PORT_UNREACH, 0, ip_nb );
                #endif                        
                }
            }
        }
        else
            goto BAD;
    }
    else
    {
BAD:
        fnet_netbuf_free_chain(ip_nb);
        fnet_netbuf_free_chain(nb);
    }
}

/************************************************************************
* NAME: fnet_udp_attach
*
* DESCRIPTION: UDP attach function. 
*************************************************************************/
static int fnet_udp_attach( fnet_socket_t *sk )
{
#if FNET_CFG_IP4
    sk->options.ip_opt.ttl = FNET_UDP_TTL;
#if FNET_CFG_MULTICAST 
    sk->options.ip_opt.ttl_multicast = FNET_UDP_TTL_MULTICAST;
#endif /* FNET_CFG_MULTICAST */     
    sk->options.ip_opt.tos = 0;
#endif    
    
#if FNET_CFG_IP6
    sk->options.ip6_opt.unicast_hops = 0; /* Defined by ND.*/ 
#endif    

    sk->send_buffer.count_max = FNET_UDP_TX_BUF_MAX;
    sk->receive_buffer.count_max = FNET_UDP_RX_BUF_MAX;
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_udp_detach
*
* DESCRIPTION: UDP close function.
*************************************************************************/
static int fnet_udp_detach( fnet_socket_t *sk )
{
    fnet_isr_lock();
    fnet_socket_release(&fnet_udp_prot_if.head, sk);
    fnet_isr_unlock();
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_udp_shutdown
*
* DESCRIPTION:  UDP shutdown function.
*************************************************************************/
static int fnet_udp_shutdown( fnet_socket_t *sk, int how )
{
    fnet_isr_lock();

    if(how & SD_READ)
    {
        sk->receive_buffer.is_shutdown = 1;
        fnet_socket_buffer_release(&sk->receive_buffer);
    }

    if(how & SD_WRITE)
    {
        sk->send_buffer.is_shutdown = 1;
    }

    fnet_isr_unlock();

    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_udp_connect
*
* DESCRIPTION: UDP connect function.
*************************************************************************/
static int fnet_udp_connect( fnet_socket_t *sk, struct sockaddr *foreign_addr)
{
    fnet_isr_lock();
    
    sk->foreign_addr = *foreign_addr;
    
    sk->state = SS_CONNECTED;
    fnet_socket_buffer_release(&sk->receive_buffer);
    fnet_isr_unlock();
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_udp_snd
*
* DESCRIPTION: UDP send function.
*************************************************************************/
static int fnet_udp_snd( fnet_socket_t *sk, char *buf, int len, int flags, const struct sockaddr *addr)
{
    fnet_netbuf_t           *nb;
    int                     error = FNET_OK;
    const struct sockaddr   *foreign_addr;
    int                     flags_save = 0;

#if FNET_CFG_TCP_URGENT
    if(flags & MSG_OOB)
    {
        error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
        goto ERROR;
    }
#endif /* FNET_CFG_TCP_URGENT */

    if(len > sk->send_buffer.count_max)
    {
        error = FNET_ERR_MSGSIZE;   /* Message too long. */
        goto ERROR;
    }

    if(addr)
    {
        foreign_addr = addr;
    }
    else
    {
        foreign_addr = &sk->foreign_addr;
    }

    if((nb = fnet_netbuf_from_buf(buf, len, FNET_FALSE)) == 0)
    {
        error = FNET_ERR_NOMEM;     /* Cannot allocate memory.*/
        goto ERROR;
    }

    if(sk->local_addr.sa_port == 0)
    {
        sk->local_addr.sa_port = fnet_socket_get_uniqueport(sk->protocol_interface->head, &sk->local_addr); /* Get ephemeral port.*/
    }

    if(flags & MSG_DONTROUTE) /* Save */
    {
        flags_save = sk->options.flags;
        sk->options.flags |= SO_DONTROUTE;
    }

    error = fnet_udp_output(&sk->local_addr, foreign_addr, &(sk->options), nb);

    if(flags & MSG_DONTROUTE) /* Restore.*/
    {
        sk->options.flags = flags_save;
    }

    if((error == FNET_OK) && (sk->options.local_error == FNET_OK)) /* We get UDP or ICMP error.*/
    {
        return (len);
    }

ERROR:
    fnet_socket_set_error(sk, error);
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: fnet_udp_rcv
*
* DESCRIPTION :UDP receive function.
*************************************************************************/
static int fnet_udp_rcv(fnet_socket_t *sk, char *buf, int len, int flags, struct sockaddr *addr)
{
    int             error = FNET_OK;
    int             length;
    struct sockaddr foreign_addr;
    
    fnet_memset_zero ((void *)&foreign_addr, sizeof(foreign_addr));
    
#if FNET_CFG_TCP_URGENT
    if(flags & MSG_OOB)
    {
        error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
        goto ERROR;
    }
#endif /* FNET_CFG_TCP_URGENT */
    
    if((length = fnet_socket_buffer_read_address(&(sk->receive_buffer), buf,
            len, &foreign_addr, ((flags &MSG_PEEK)== 0))) == FNET_ERR)
    {
        /* The message was too large to fit into the specified buffer and was truncated.*/
        error = FNET_ERR_MSGSIZE;
        goto ERROR;
    }

    if(sk->options.local_error == FNET_OK) 
    {
        if(addr)
        {
            fnet_socket_addr_copy(&foreign_addr, addr);
        }
        
        return (length);
    }
    else /* We get UDP or ICMP error.*/
    {
        error = sk->options.local_error;
    }

ERROR:
    fnet_socket_set_error(sk, error);
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: fnet_udp_control_input
*
* DESCRIPTION: This function processes the ICMP error.
*************************************************************************/
static void fnet_udp_control_input( fnet_prot_notify_t command, fnet_ip_header_t *ip_header )
{
    fnet_udp_header_t   *udp_header;
    fnet_ip4_addr_t     foreign_addr; /* Foreign IP address.*/
    unsigned short      foreign_port; /* Foreign port.*/
    fnet_ip4_addr_t     local_addr;   /* Local IP address.*/
    unsigned short      local_port;   /* Local port.*/
    int                 error;
    fnet_socket_t       *sock;

    if(ip_header)
    {
        udp_header = (fnet_udp_header_t *)((char *)ip_header + (FNET_IP_HEADER_GET_HEADER_LENGTH(ip_header) << 2));
        foreign_addr = ip_header->desination_addr;
        foreign_port = udp_header->destination_port;
        local_addr = ip_header->source_addr;
        local_port = udp_header->source_port;

        if(foreign_addr == INADDR_ANY)
            return;

        switch(command)
        {
            case FNET_PROT_NOTIFY_MSGSIZE:          /* Message size forced drop.*/
              error = FNET_ERR_MSGSIZE;
              break;

            case FNET_PROT_NOTIFY_UNREACH_HOST:     /* No route to host.*/
            case FNET_PROT_NOTIFY_UNREACH_NET:      /* No route to network.*/
            case FNET_PROT_NOTIFY_UNREACH_SRCFAIL:  /* Source route failed.*/
              error = FNET_ERR_HOSTUNREACH;
              break;

            case FNET_PROT_NOTIFY_UNREACH_PROTOCOL: /* Dst says bad protocol.*/
            case FNET_PROT_NOTIFY_UNREACH_PORT:     /* Bad port #.*/
              error = FNET_ERR_CONNRESET;
              break;

            case FNET_PROT_NOTIFY_PARAMPROB:        /* Header incorrect.*/
              error = FNET_ERR_NOPROTOOPT;          /* Bad protocol option.*/
              break;

            default:
              return;
        }

        for (sock = fnet_udp_prot_if.head; sock != 0; sock = sock->next)
        {
            if((((struct sockaddr_in *)(&sock->foreign_addr))->sin_addr.s_addr != foreign_addr) || (sock->foreign_addr.sa_port != foreign_port)
                   || (sock->local_addr.sa_port != local_port) || (((struct sockaddr_in *)(&sock->local_addr))->sin_addr.s_addr != local_addr))
                continue;

            sock->options.local_error = error;
        }
    }
}

/************************************************************************
* NAME: fnet_udp_trace
*
* DESCRIPTION: Prints UDP header. For debugging purposes.
*************************************************************************/
#if FNET_CFG_DEBUG_TRACE_UDP
void fnet_udp_trace(char *str, fnet_udp_header_t *udp_hdr)
{

    fnet_printf(FNET_SERIAL_ESC_FG_GREEN"%s", str); /* Print app-specific header.*/
    fnet_println("[UDP header]"FNET_SERIAL_ESC_FG_BLACK);
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(SrcPort)                  "FNET_SERIAL_ESC_FG_BLUE"%3u"FNET_SERIAL_ESC_FG_BLACK" |(DestPort)                 "FNET_SERIAL_ESC_FG_BLUE"%3u"FNET_SERIAL_ESC_FG_BLACK" |",
                    fnet_ntohs(udp_hdr->source_port),
                    fnet_ntohs(udp_hdr->destination_port));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(Lenghth)                  "FNET_SERIAL_ESC_FG_BLUE"%3u"FNET_SERIAL_ESC_FG_BLACK" |(Checksum)              0x%04X |",
                    fnet_ntohs(udp_hdr->length),
                    fnet_ntohs(udp_hdr->checksum));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");    
}
#endif /* FNET_CFG_DEBUG_TRACE_UDP */

#endif 

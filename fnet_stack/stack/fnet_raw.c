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
* @file fnet_raw.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.9.0
*
* @brief RAW socket implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_raw.h"
#include "fnet_ip_prv.h"
#include "fnet_timer.h"
#include "fnet_stdlib.h"

#include "fnet_isr.h"
#include "fnet_checksum.h"
#include "fnet_prot.h"

#if FNET_CFG_RAW

/************************************************************************
*     Function Prototypes
*************************************************************************/
static int fnet_raw_attach( fnet_socket_t *sk );
static int fnet_raw_detach( fnet_socket_t *sk );
static int fnet_raw_connect( fnet_socket_t *sk, struct sockaddr *foreign_addr);
static int fnet_raw_snd( fnet_socket_t *sk, char *buf, int len, int flags, const struct sockaddr *foreign_addr);
static int fnet_raw_rcv( fnet_socket_t *sk, char *buf, int len, int flags, struct sockaddr *foreign_addr);
static int fnet_raw_shutdown( fnet_socket_t *sk, int how );
static void fnet_raw_input( fnet_netif_t *netif, struct sockaddr *foreign_addr,  struct sockaddr *local_addr, fnet_netbuf_t *nb, int protocol_number);


/************************************************************************
*     Global Data Structures
*************************************************************************/

/************************************************************************
 * Protocol API structures.
 ************************************************************************/
static const fnet_socket_prot_if_t fnet_raw_socket_api =
{
    0,                      /* Flag that protocol is connection oriented.*/
    fnet_raw_attach,        /* Protocol "attach" function.*/
    fnet_raw_detach,        /* Protocol "detach" function.*/
    fnet_raw_connect,       /* Protocol "connect" function.*/
    0,                      /* Protocol "accept" function.*/
    fnet_raw_rcv,           /* Protocol "receive" function.*/
    fnet_raw_snd,           /* Protocol "send" function.*/
    fnet_raw_shutdown,      /* Protocol "shutdown" function.*/
    fnet_ip_setsockopt,     /* Protocol "setsockopt" function.*/
    fnet_ip_getsockopt,     /* Protocol "getsockopt" function.*/
    0,                      /* Protocol "listen" function.*/
};

fnet_prot_if_t fnet_raw_prot_if =
{
    0,                      /* Pointer to the head of the protocol's socket list.*/
    AF_SUPPORTED,           /* Address domain family.*/
    SOCK_RAW,               /* Socket type used for.*/
    0,                      /* Protocol number.*/   
    0,                      /* Protocol initialization function.*/
    fnet_raw_release,       /* Protocol release function.*/
#if FNET_CFG_IP4     
    fnet_raw_input_ip4,     /* Protocol input function, from IPv4.*/
#endif    
    0,                      /* Protocol input control function.*/     
    0,                      /* protocol drain function.*/
#if FNET_CFG_IP6    
    fnet_raw_input_ip6,      /* Protocol input function, from IPv6.*/
#endif /* FNET_CFG_IP6 */      
    &fnet_raw_socket_api    /* Socket API */
};

/************************************************************************
* NAME: fnet_raw_release
*
* DESCRIPTION: This function releases all sockets associated 
*              with RAW protocol. 
*************************************************************************/
static void fnet_raw_release( void )
{
    while(fnet_raw_prot_if.head)
      fnet_socket_release(&fnet_raw_prot_if.head, fnet_raw_prot_if.head);
}

/************************************************************************
* NAME: fnet_raw_output
*
* DESCRIPTION: RAW output function
*************************************************************************/
static int fnet_raw_output(  struct sockaddr *src_addr, const struct sockaddr *dest_addr, unsigned char protocol_number,
                             fnet_socket_option_t *sockoption, fnet_netbuf_t *nb )                            
{
    int error =  FNET_OK;
    
    fnet_netif_t *netif = FNET_NULL;


#if FNET_CFG_IP4
    if(dest_addr->sa_family == AF_INET)
    {
        error = fnet_ip_output(netif, ((struct sockaddr_in *)(src_addr))->sin_addr.s_addr, 
                                        ((struct sockaddr_in *)(dest_addr))->sin_addr.s_addr, 
                                        protocol_number, 
                                        sockoption->ip_opt.tos,
                                #if FNET_CFG_MULTICAST
                                    (unsigned char)((FNET_IP4_ADDR_IS_MULTICAST(((struct sockaddr_in *)(dest_addr))->sin_addr.s_addr)?sockoption->ip_opt.ttl_multicast:sockoption->ip_opt.ttl)),                               
                                #else
                                    sockoption->ip_opt.ttl, 
                                #endif /* FNET_CFG_MULTICAST */                               
                                   nb, 0, ((sockoption->flags & SO_DONTROUTE) > 0),
                                   0
                                   );
    }
#endif
   
#if FNET_CFG_IP6    
    if(dest_addr->sa_family == AF_INET6)
    {
        /* Check Scope ID.*/
        netif = fnet_netif_get_by_scope_id( ((struct sockaddr_in6 *)dest_addr)->sin6_scope_id );
        
        error = fnet_ip6_output( netif, 
                                fnet_socket_addr_is_unspecified(src_addr)? FNET_NULL : &((struct sockaddr_in6 *)(src_addr))->sin6_addr.s6_addr, 
                                &((struct sockaddr_in6 *)(dest_addr))->sin6_addr.s6_addr, 
                                protocol_number, 
                                sockoption->ip6_opt.unicast_hops, nb,0);
    }
#endif                               

    return (error);
}

/************************************************************************
* NAME: fnet_raw_input_ip4
*
* DESCRIPTION: RAW input function, from IPv4.
*************************************************************************/
#if FNET_CFG_IP4 
void fnet_raw_input_ip4( fnet_netif_t *netif, fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip,
                           fnet_netbuf_t *nb, fnet_netbuf_t *ip4_nb)
{
    struct sockaddr     src_addr;
    struct sockaddr     dest_addr;
    fnet_ip_header_t    *hdr = ip4_nb->data_ptr;
    
    fnet_memset_zero(&src_addr, sizeof(struct sockaddr));
     src_addr.sa_family = AF_INET;
    ((struct sockaddr_in*)(&src_addr))->sin_addr.s_addr = src_ip;
    
    fnet_memset_zero(&dest_addr, sizeof(struct sockaddr));
    dest_addr.sa_family = AF_INET;
    ((struct sockaddr_in*)(&dest_addr))->sin_addr.s_addr = dest_ip;

    fnet_raw_input(netif, &src_addr,  &dest_addr, nb, hdr->protocol);    
}
#endif /* FNET_CFG_IP4 */

/************************************************************************
* NAME: fnet_raw_input_ip6
*
* DESCRIPTION: RAW input function, from IPv6.
*************************************************************************/
#if FNET_CFG_IP6 
void fnet_raw_input_ip6(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb)
{
    struct sockaddr     src_addr;
    struct sockaddr     dest_addr;
    fnet_ip6_header_t   *hdr = ip6_nb->data_ptr;
    
    fnet_memset_zero(&src_addr, sizeof(struct sockaddr));
    src_addr.sa_family = AF_INET6;
    FNET_IP6_ADDR_COPY(src_ip, &((struct sockaddr_in6 *)(&src_addr))->sin6_addr.s6_addr);
    ((struct sockaddr_in6 *)(&src_addr))->sin6_scope_id = netif->scope_id;
    
    
    fnet_memset_zero(&dest_addr, sizeof(struct sockaddr));
    dest_addr.sa_family = AF_INET6;
    FNET_IP6_ADDR_COPY(dest_ip, &((struct sockaddr_in6 *)(&dest_addr))->sin6_addr.s6_addr);
    ((struct sockaddr_in6 *)(&dest_addr))->sin6_scope_id = netif->scope_id;

    fnet_raw_input(netif, &src_addr,  &dest_addr, nb, hdr->next_header);    
}
#endif /* FNET_CFG_IP6*/

/************************************************************************
* NAME: fnet_raw_input
*
* DESCRIPTION: RAW input function.
*************************************************************************/
static void fnet_raw_input(fnet_netif_t *netif, struct sockaddr *foreign_addr,  struct sockaddr *local_addr, fnet_netbuf_t *nb, int protocol_number)
{
    fnet_socket_t       *sock;
    fnet_socket_t       *last;
    fnet_netbuf_t       *nb_tmp;

    if(netif && nb && nb->total_length)
    {
        /* Demultiplex broadcast & multicast datagrams.*/
        if(fnet_socket_addr_is_broadcast(local_addr, netif) || fnet_socket_addr_is_multicast(local_addr)) 
        {
            last = 0;

            for (sock = fnet_raw_prot_if.head; sock != 0; sock = sock->next)
            {
                /* Ignore local port number.*/

                /* Check protocol number.*/
                if(sock->protocol_number != protocol_number)
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
                        if((!fnet_socket_addr_are_equal(&sock->foreign_addr, foreign_addr)) ) 
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
                
            /* Copy buffer.*/
            if((nb_tmp = fnet_netbuf_copy(nb, 0, FNET_NETBUF_COPYALL, 0)) != 0)
            {
                if(fnet_socket_buffer_append_address(&(last->receive_buffer), nb_tmp, foreign_addr) == FNET_ERR)
                {
                    fnet_netbuf_free_chain(nb_tmp);
                    goto BAD;
                }
            }
            else
                goto BAD;
        }
        else /* For unicast datagram.*/
        {
            sock = fnet_socket_lookup(fnet_raw_prot_if.head, local_addr, foreign_addr, protocol_number);

            if(sock)
            {
                if(sock->receive_buffer.is_shutdown) /* Is shutdown.*/
                    goto BAD;
                    
                /* Copy buffer.*/
                if((nb_tmp = fnet_netbuf_copy(nb, 0, FNET_NETBUF_COPYALL, 0)) != 0)
                {
                    if(fnet_socket_buffer_append_address(&(sock->receive_buffer), nb_tmp, foreign_addr) == FNET_ERR)
                    {
                        fnet_netbuf_free_chain(nb_tmp);
                        goto BAD;
                    }
                }
                else
                    goto BAD;
            }
        }
    }
BAD:;
}

/************************************************************************
* NAME: fnet_raw_attach
*
* DESCRIPTION: RAW attach function. 
*************************************************************************/
static int fnet_raw_attach( fnet_socket_t *sk )
{
#if FNET_CFG_IP4
    sk->options.ip_opt.ttl = FNET_RAW_TTL;
    sk->options.ip_opt.tos = 0;
#endif    

#if FNET_CFG_IP6    
    sk->options.ip6_opt.unicast_hops = 0; /* Defined by ND.*/ 
#endif    
    
    sk->send_buffer.count_max = FNET_RAW_TX_BUF_MAX;
    sk->receive_buffer.count_max = FNET_RAW_RX_BUF_MAX;
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_raw_detach
*
* DESCRIPTION: RAW close function.
*************************************************************************/
static int fnet_raw_detach( fnet_socket_t *sk )
{
    fnet_isr_lock();
    fnet_socket_release(&fnet_raw_prot_if.head, sk);
    fnet_isr_unlock();
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_raw_shutdown
*
* DESCRIPTION:  RAW shutdown function.
*************************************************************************/
static int fnet_raw_shutdown( fnet_socket_t *sk, int how )
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
* NAME: fnet_raw_connect
*
* DESCRIPTION: RAW connect function.
*************************************************************************/
static int fnet_raw_connect( fnet_socket_t *sk, struct sockaddr *foreign_addr)
{
    fnet_isr_lock();
    
    sk->foreign_addr = *foreign_addr;
    sk->local_addr.sa_port = 0;
    sk->foreign_addr.sa_port = 0;
    sk->state = SS_CONNECTED;
    fnet_socket_buffer_release(&sk->receive_buffer);
    fnet_isr_unlock();
    return (FNET_OK);
}

/************************************************************************
* NAME: fnet_raw_snd
*
* DESCRIPTION: RAW send function.
*************************************************************************/
static int fnet_raw_snd( fnet_socket_t *sk, char *buf, int len, int flags, const struct sockaddr *addr)
{
    fnet_netbuf_t           *nb;
    int                     error = FNET_OK;
    const struct sockaddr   *foreign_addr;
    int                     flags_save = 0;

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

    if(flags & MSG_DONTROUTE) /* Save */
    {
        flags_save = sk->options.flags;
        sk->options.flags |= SO_DONTROUTE;
    }

    error = fnet_raw_output(&sk->local_addr, foreign_addr, (unsigned char)sk->protocol_number, &(sk->options), nb);

    if(flags & MSG_DONTROUTE) /* Restore.*/
    {
        sk->options.flags = flags_save;
    }

    if((error == FNET_OK) && (sk->options.local_error == FNET_OK)) /* We get RAW or ICMP error.*/
    {
        return (len);
    }

ERROR:
    fnet_socket_set_error(sk, error);
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: fnet_raw_rcv
*
* DESCRIPTION :RAW receive function.
*************************************************************************/
static int fnet_raw_rcv(fnet_socket_t *sk, char *buf, int len, int flags, struct sockaddr *addr)
{
    int error = FNET_OK;
    int length;
    struct sockaddr foreign_addr;

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


    if((error == FNET_OK) && (sk->options.local_error == FNET_OK)) /* We get RAW or ICMP error.*/
    {
        if(addr)
        {
            fnet_socket_addr_copy(&foreign_addr, addr);
        }
        
        return (length);
    }

ERROR:
    fnet_socket_set_error(sk, error);
    return (SOCKET_ERROR);
}


#endif  /* FNET_CFG_RAW */

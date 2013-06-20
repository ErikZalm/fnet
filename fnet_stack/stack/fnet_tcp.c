/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
* Copyright 2003 by Alexey Shervashidze. Motorola SPS.
*
***************************************************************************
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License Version 3 
* or later (the "LGPL").
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
* @file fnet_tcp.c
*
* @date Mar-25-2013
*
* @author Olexandr Servasidze, Andrey Butok.
*
* @version 0.1.76.0
*
* @brief TCP protocol implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_TCP

#include "fnet.h"
#include "fnet_error.h"
#include "fnet_socket.h"
#include "fnet_socket_prv.h"
#include "fnet_timer_prv.h"
#include "fnet_tcp.h"
#include "fnet_isr.h"
#include "fnet_checksum.h"
#include "fnet_prot.h"
#include "fnet_stdlib.h"
#include "fnet_debug.h"

/************************************************************************
*     Definitions
*************************************************************************/
struct fnet_tcp_segment
{
    fnet_socket_option_t    *sockoption; 
    struct sockaddr         src_addr;
    struct sockaddr         dest_addr;
    unsigned long           seq;
    unsigned long           ack;
    unsigned char           flags;
    unsigned short          wnd;
    unsigned short          urgpointer;
    void                    *options;
    char                    optlen;
    fnet_netbuf_t           *data;
};

/************************************************************************
*     Function Prototypes
*************************************************************************/
static void fnet_tcp_slowtimo( void *cookie );
static void fnet_tcp_fasttimo( void *cookie );
static void fnet_tcp_slowtimosk( fnet_socket_t *sk );
static void fnet_tcp_fasttimosk( fnet_socket_t *sk );
static int fnet_tcp_inputsk( fnet_socket_t *sk, fnet_netbuf_t *insegment, struct sockaddr *src_addr,  struct sockaddr *dest_addr);
static void fnet_tcp_initconnection( fnet_socket_t *sk );
static int fnet_tcp_dataprocess( fnet_socket_t *sk, fnet_netbuf_t *insegment, int *ackparam );
static int fnet_tcp_sendheadseg( fnet_socket_t *sk, unsigned char flags, void *options, char optlen );
static int fnet_tcp_senddataseg( fnet_socket_t *sk, void *options, char optlen, unsigned long datasize );
static unsigned long fnet_tcp_getrcvwnd( fnet_socket_t *sk );
static int fnet_tcp_sendseg( struct fnet_tcp_segment *segment);                      
static void fnet_tcp_sendrst( fnet_socket_option_t *sockoption, fnet_netbuf_t *insegment, struct sockaddr *src_addr,  struct sockaddr *dest_addr);
static void fnet_tcp_sendrstsk( fnet_socket_t *sk );
static void fnet_tcp_sendack( fnet_socket_t *sk );
static void fnet_tcp_abortsk( fnet_socket_t *sk );
static void fnet_tcp_setsynopt( fnet_socket_t *sk, char *options, char *optionlen );
static void fnet_tcp_getsynopt( fnet_socket_t *sk );
static int fnet_tcp_addopt( fnet_netbuf_t *segment, unsigned char len, void *data );
static void fnet_tcp_getopt( fnet_socket_t *sk, fnet_netbuf_t *segment );
static unsigned long fnet_tcp_getsize( unsigned long pos1, unsigned long pos2 );
static void fnet_tcp_rtimeo( fnet_socket_t *sk );
static void fnet_tcp_ktimeo( fnet_socket_t *sk );
static void fnet_tcp_ptimeo( fnet_socket_t *sk );
static int fnet_tcp_hit( unsigned long startpos, unsigned long endpos, unsigned long pos );
static int fnet_tcp_addinpbuf( fnet_socket_t *sk, fnet_netbuf_t *insegment, int *ackparam );
static fnet_socket_t *fnet_tcp_findsk( struct sockaddr *src_addr,  struct sockaddr *dest_addr );
static void fnet_tcp_addpartialsk( fnet_socket_t *mainsk, fnet_socket_t *partialsk );
static void fnet_tcp_movesk2incominglist( fnet_socket_t *sk );
static void fnet_tcp_closesk( fnet_socket_t *sk );
static void fnet_tcp_delpartialsk( fnet_socket_t *sk );
static void fnet_tcp_delincomingsk( fnet_socket_t *sk );
static void fnet_tcp_delcb( fnet_tcp_control_t *cb );
#if !FNET_CFG_TCP_DISCARD_OUT_OF_ORDER
    static void fnet_tcp_deletetmpbuf( fnet_tcp_control_t *cb );
#endif
static void fnet_tcp_delsk( fnet_socket_t ** head, fnet_socket_t *sk );
static int fnet_tcp_sendanydata( fnet_socket_t *sk, int oneexec );
#if FNET_CFG_TCP_URGENT
    static void fnet_tcp_urgprocessing( fnet_socket_t *sk, fnet_netbuf_t ** segment, unsigned long repdatasize, int *ackparam );
#endif
static void fnet_tcp_finprocessing( fnet_socket_t *sk, unsigned long ack );
static int fnet_tcp_init( void );
static void fnet_tcp_release( void );
static void fnet_tcp_input_ip4( fnet_netif_t *netif, fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip4_nb);
static void fnet_tcp_input_ip6(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb);
static void fnet_tcp_input(fnet_netif_t *netif, struct sockaddr *src_addr,  struct sockaddr *dest_addr, fnet_netbuf_t *nb, fnet_netbuf_t *ip_nb);
static void fnet_tcp_ctrlinput( fnet_prot_notify_t command, fnet_ip_header_t *ip_hdr );
static int fnet_tcp_attach( fnet_socket_t *sk );
static int fnet_tcp_close( fnet_socket_t *sk );
static int fnet_tcp_connect( fnet_socket_t *sk, struct sockaddr *foreign_addr);
static fnet_socket_t *fnet_tcp_accept( fnet_socket_t *listensk );
static int fnet_tcp_rcv( fnet_socket_t *sk, char *buf, int len, int flags, struct sockaddr *foreign_addr);
static int fnet_tcp_snd( fnet_socket_t *sk, char *buf, int len, int flags, const struct sockaddr *foreign_addr);
static int fnet_tcp_shutdown( fnet_socket_t *sk, int how );
static int fnet_tcp_setsockopt( fnet_socket_t *sk, int level, int optname, char *optval, int optlen );
static int fnet_tcp_getsockopt( fnet_socket_t *sk, int level, int optname, char *optval, int *optlen );
static int fnet_tcp_listen( fnet_socket_t *sk, int backlog );
static void fnet_tcp_drain( void );

#if FNET_CFG_DEBUG_TRACE_TCP
    void fnet_tcp_trace(char *str, fnet_tcp_header_t *tcp_hdr);
#else
    #define fnet_tcp_trace(str, tcp_hdr)
#endif


#if 0 /* For Debug needs.*/
    int FNET_DEBUG_check_send_buffer(fnet_socket_t *sk);
#endif

/************************************************************************
*     Global Variables
*************************************************************************/
/* Initial Sequence Number
 * tcpcb_isntime is changed by STEPISN every 0.5 sec. 
 * Additionaly, each time a connection is established,
 * tcpcb_isntime is also incremented by FNET_TCP_STEPISN */
static unsigned long fnet_tcp_isntime = 1;

/* Timers.*/
static fnet_timer_desc_t fnet_tcp_fasttimer;
static fnet_timer_desc_t fnet_tcp_slowtimer;


/*****************************************************************************
 * Protocol API structure.
 ******************************************************************************/
static const fnet_socket_prot_if_t fnet_tcp_socket_api =
{
    1,                    /* TRUE = connection required by protocol.*/
    fnet_tcp_attach,      /* A new socket has been created.*/
    fnet_tcp_close,      
    fnet_tcp_connect,
    fnet_tcp_accept,     
    fnet_tcp_rcv,
    fnet_tcp_snd,        
    fnet_tcp_shutdown,
    fnet_tcp_setsockopt, 
    fnet_tcp_getsockopt,
    fnet_tcp_listen   
};

/* Protocol structure.*/
fnet_prot_if_t fnet_tcp_prot_if =
{
    0, 
    AF_SUPPORTED,      /* Protocol domain.*/
    SOCK_STREAM,            /* Socket type.*/
    FNET_IP_PROTOCOL_TCP,   /* Protocol number.*/ 
    fnet_tcp_init,       
    fnet_tcp_release,
#if FNET_CFG_IP4     
    fnet_tcp_input_ip4,     /* Input to protocol (from below).*/
#endif    
    fnet_tcp_ctrlinput,     /* Control input (from below).*/
    fnet_tcp_drain, 
#if FNET_CFG_IP6    
    fnet_tcp_input_ip6,     /* Protocol IPv6 input function.*/
#endif /* FNET_CFG_IP6 */                              
    &fnet_tcp_socket_api    /* Socket API */
};

/************************************************************************
* NAME: fnet_tcp_init
*
* DESCRIPTION: This function performs a protocol initialization.
*
* RETURNS: If no error occurs, this function returns ERR_OK. Otherwise
*          it returns FNET_ERR.
*************************************************************************/
static int fnet_tcp_init( void )
{

    /* Create the slow timer.*/
    fnet_tcp_fasttimer = fnet_timer_new(FNET_TCP_FASTTIMO / FNET_TIMER_PERIOD_MS, fnet_tcp_fasttimo, 0);

    if(!fnet_tcp_fasttimer)
        return FNET_ERR;

    /* Create the fast timer.*/
    fnet_tcp_slowtimer = fnet_timer_new(FNET_TCP_SLOWTIMO / FNET_TIMER_PERIOD_MS, fnet_tcp_slowtimo, 0);

    if(!fnet_tcp_slowtimer)
    {
        fnet_timer_free(fnet_tcp_fasttimer);
        fnet_tcp_fasttimer = 0;
        return FNET_ERR;
    }

    return FNET_OK;
}

/************************************************************************
* NAME: fnet_tcp_release
*
* DESCRIPTION: This function resets and deletes sockets and releases timers.
*
* RETURNS: None.          
*************************************************************************/
static void fnet_tcp_release( void )
{
    fnet_tcp_control_t *cb; 

    fnet_isr_lock();

    /* Release sockets.*/
    while(fnet_tcp_prot_if.head)
    {
        cb = (fnet_tcp_control_t *)fnet_tcp_prot_if.head->protocol_control;
        cb->tcpcb_flags |= FNET_TCP_CBF_CLOSE;
        fnet_tcp_abortsk(fnet_tcp_prot_if.head);
    }

    /* Free timers.*/
    fnet_timer_free(fnet_tcp_fasttimer);
    fnet_timer_free(fnet_tcp_slowtimer);

    fnet_tcp_fasttimer = 0;
    fnet_tcp_slowtimer = 0;

    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_tcp_input_ip4
*
* DESCRIPTION: TCP input function, from IPv4.
*************************************************************************/
#if FNET_CFG_IP4 
static void fnet_tcp_input_ip4( fnet_netif_t *netif, fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip4_nb)
{
    struct sockaddr     src_addr;
    struct sockaddr     dest_addr;;
    
    fnet_memset_zero(&src_addr, sizeof(struct sockaddr));
     src_addr.sa_family = AF_INET;
    ((struct sockaddr_in*)(&src_addr))->sin_addr.s_addr = src_ip;
    
    fnet_memset_zero(&dest_addr, sizeof(struct sockaddr));
    dest_addr.sa_family = AF_INET;
    ((struct sockaddr_in*)(&dest_addr))->sin_addr.s_addr = dest_ip;

    fnet_tcp_input(netif, &src_addr,  &dest_addr, nb, ip4_nb);  
}
#endif /* FNET_CFG_IP4 */

/************************************************************************
* NAME: fnet_tcp_input_ip6
*
* DESCRIPTION: TCP input function, from IPv6.
*************************************************************************/
#if FNET_CFG_IP6 
static void fnet_tcp_input_ip6(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb)
{
    struct sockaddr     src_addr;
    struct sockaddr     dest_addr;;
    
    fnet_memset_zero(&src_addr, sizeof(struct sockaddr));
    src_addr.sa_family = AF_INET6;
    FNET_IP6_ADDR_COPY(src_ip, &((struct sockaddr_in6 *)(&src_addr))->sin6_addr.s6_addr);
    ((struct sockaddr_in6 *)(&src_addr))->sin6_scope_id = netif->scope_id;
    
    
    fnet_memset_zero(&dest_addr, sizeof(struct sockaddr));
    dest_addr.sa_family = AF_INET6;
    FNET_IP6_ADDR_COPY(dest_ip, &((struct sockaddr_in6 *)(&dest_addr))->sin6_addr.s6_addr);
    ((struct sockaddr_in6 *)(&dest_addr))->sin6_scope_id = netif->scope_id;

    fnet_tcp_input(netif, &src_addr,  &dest_addr, nb, ip6_nb);    
}
#endif /* FNET_CFG_IP4 */

/************************************************************************
* NAME: fnet_tcp_input
*
* DESCRIPTION: This function receives, checks and routes input segments.
*
* RETURNS: FNET_OK.
*************************************************************************/
static void fnet_tcp_input(fnet_netif_t *netif, struct sockaddr *src_addr,  struct sockaddr *dest_addr, fnet_netbuf_t *nb, fnet_netbuf_t *ip_nb)
{

    fnet_socket_t   *sk;       
    fnet_netbuf_t   *buf;
    unsigned short  checksum; 
    unsigned long   tcp_length;
    
    tcp_length = (unsigned long)FNET_TCP_LENGTH(nb);
    
    fnet_netbuf_free_chain(ip_nb);
 
    /* The header must reside in contiguous area of the memory.*/
    buf = fnet_netbuf_pullup(nb, (int)tcp_length);

    if(!buf)
    {
        goto DROP;
    }
    
    nb = buf;
    
    /*Checksum.*/
#if FNET_CFG_CPU_ETH_HW_RX_PROTOCOL_CHECKSUM || FNET_CFG_CPU_ETH_HW_TX_PROTOCOL_CHECKSUM
    if(nb->flags & FNET_NETBUF_FLAG_HW_PROTOCOL_CHECKSUM)
    {
        checksum = 0;
    }
    else
#endif
    {
        checksum = fnet_checksum_pseudo_start( nb, FNET_HTONS((unsigned short)FNET_IP_PROTOCOL_TCP), (unsigned short)nb->total_length );
        checksum = fnet_checksum_pseudo_end( checksum, (char *)src_addr->sa_data, (char *)dest_addr->sa_data, 
                                            (dest_addr->sa_family == AF_INET) ? sizeof(fnet_ip4_addr_t) : sizeof(fnet_ip6_addr_t) );
    }
   
    if(checksum
        || (fnet_socket_addr_is_broadcast(src_addr, netif) 
        || fnet_socket_addr_is_multicast(src_addr))
        || (nb->total_length < tcp_length) /* Check the length.*/)
    {
        goto DROP;
    }

    /* Set port numbers.*/
    src_addr->sa_port = FNET_TCP_SPORT(nb);
    dest_addr->sa_port = FNET_TCP_DPORT(nb);

    if(fnet_socket_addr_is_broadcast(dest_addr, netif) || fnet_socket_addr_is_multicast(dest_addr))
    {
        /* Send RST.*/
        fnet_tcp_sendrst(0, nb, dest_addr, src_addr);
        goto DROP;
    }
    
    fnet_tcp_trace("RX", buf->data_ptr); /* TCP trace.*/        
    
    sk = fnet_tcp_findsk(src_addr,  dest_addr);

    if(sk)
    {
        if(sk->state == SS_LISTENING)
        {
            sk->foreign_addr = *src_addr;
        }

        nb->next_chain = 0;

        /* Process  the segment.*/
        if(fnet_tcp_inputsk(sk, nb, src_addr, dest_addr) == FNET_TRUE)
            goto DROP;
    }
    else
    {
        if(!(FNET_TCP_FLAGS(nb) & FNET_TCP_SGT_RST))
            fnet_tcp_sendrst(0, nb, dest_addr, src_addr);

        goto DROP;
    }


	/* Wake-up user application.*/
 	fnet_os_event_raise(); 

    return;
    
DROP:
	/* Wake-up user application.*/
 	fnet_os_event_raise(); 
 
 	/* Delete the segment.*/
    fnet_netbuf_free_chain(nb);
}


/************************************************************************
* NAME: fnet_tcp_ctrlinput
*
* DESCRIPTION: This function process ICMP errors.
*
* RETURNS: None.          
*************************************************************************/
static void fnet_tcp_ctrlinput( fnet_prot_notify_t command, fnet_ip_header_t *ip_header )
{
    fnet_tcp_header_t   *tcp_header;    /* Pointer to the TCP header.*/
    fnet_socket_t       *sk;            /* Pointer to the socket.*/
    fnet_tcp_control_t  *cb;
    struct sockaddr     src_addr;
    struct sockaddr     dest_addr;

    if(ip_header)
    {
        /* Find the corresponding socket.*/
        tcp_header = (fnet_tcp_header_t *)((char *)ip_header + (FNET_IP_HEADER_GET_HEADER_LENGTH(ip_header) << 2));
     
        /* Foreign addr.*/
        fnet_memset_zero(&src_addr, sizeof(struct sockaddr));
        src_addr.sa_family = AF_INET;
        src_addr.sa_port = tcp_header->destination_port;
        ((struct sockaddr_in*)(&src_addr))->sin_addr.s_addr = ip_header->desination_addr;
        
        /* Local addr.*/
        fnet_memset_zero(&dest_addr, sizeof(struct sockaddr));
        dest_addr.sa_family = AF_INET;
        dest_addr.sa_port = tcp_header->source_port;
        ((struct sockaddr_in*)(&dest_addr))->sin_addr.s_addr = ip_header->source_addr;
         
        sk = fnet_tcp_findsk(&src_addr, &dest_addr);
 
        if(sk)
        {
            /* Initialize the pointer of the control block.*/
            cb = sk->protocol_control;

            switch(command)
            {
                case FNET_PROT_NOTIFY_QUENCH: /* Someone said to slow down.*/
                  /* Begin the Slow Start algorithm.*/
                  cb->tcpcb_cwnd = cb->tcpcb_sndmss;
                  break;

                case FNET_PROT_NOTIFY_MSGSIZE: /* Message size forced drop.*/
                  sk->options.local_error = FNET_ERR_MSGSIZE;
                  break;

                case FNET_PROT_NOTIFY_UNREACH_HOST:    /* No route to host.*/
                case FNET_PROT_NOTIFY_UNREACH_NET:     /* No route to network.*/
                case FNET_PROT_NOTIFY_UNREACH_SRCFAIL: /* Source route failed.*/
                  sk->options.local_error = FNET_ERR_HOSTUNREACH;
                  break;

                case FNET_PROT_NOTIFY_PARAMPROB:                 /* Header incorrect.*/
                  sk->options.local_error = FNET_ERR_NOPROTOOPT; /* Bad protocol option.*/
                  break;

                case FNET_PROT_NOTIFY_UNREACH_PORT:              /* bad port #.*/
                case FNET_PROT_NOTIFY_UNREACH_PROTOCOL:
                  sk->options.local_error = FNET_ERR_CONNRESET;
                  fnet_tcp_closesk(sk);
                  break;

                default:
                  break;
            }
        }
    }
}

/************************************************************************
* NAME: fnet_tcp_attach
*
* DESCRIPTION: This function performs the initialization of the  
*              socket's options and creates the structure of the control blok.
*
* RETURNS: If no error occurs, this function returns FNET_OK. Otherwise
*          it returns FNET_ERR.
*************************************************************************/
static int fnet_tcp_attach( fnet_socket_t *sk )
{
    struct tcpcb *cb; 

    /* Create the control block.*/
    cb = (struct tcpcb *)fnet_malloc(sizeof(fnet_tcp_control_t));

    /* Check the memory allocation.*/
    if(!cb)
    {
        fnet_socket_set_error(sk, FNET_ERR_NOMEM);
        return FNET_ERR;
    }

    fnet_memset_zero(cb, sizeof(fnet_tcp_control_t));

    sk->protocol_control = (void *)cb;

    /* Set the maximal segment size option.*/
    sk->options.tcp_opt.mss = FNET_CFG_SOCKET_TCP_MSS;

    /* Default setting of the flags.*/
    sk->options.tcp_opt.flags = 
        #if FNET_CFG_TCP_URGENT
            TCP_BSD | 
        #endif    
            TCP_NODELAY;
    sk->options.tcp_opt.keep_idle = FNET_TCP_KEEPIDLE_DEFAULT;      /* TCP_KEEPIDLE option. */
    sk->options.tcp_opt.keep_intvl = FNET_TCP_KEEPINTVL_DEFAULT;    /* TCP_KEEPINTVL option. */
    sk->options.tcp_opt.keep_cnt = FNET_TCP_KEEPCNT_DEFAULT;        /* TCP_KEEPCNT option. */
    
    sk->options.flags = SO_KEEPALIVE;

    /* Set the IP options.*/
#if FNET_CFG_IP4    
    sk->options.ip_opt.ttl = FNET_TCP_TTL_DEFAULT;
    sk->options.ip_opt.tos = 0;
#endif

#if FNET_CFG_IP6    
    sk->options.ip6_opt.unicast_hops = 0; /* Defined by ND.*/ 
#endif
    

    /* Set the buffer sizes.*/
    sk->send_buffer.count_max = FNET_TCP_TX_BUF_MAX;
    sk->receive_buffer.count_max = FNET_TCP_RX_BUF_MAX;

    return FNET_OK;
}

/************************************************************************
* NAME: fnet_tcp_close
*
* DESCRIPTION: This function performs the connection termination.
*
* RETURNS: If no error occurs, this function returns FNET_OK. Otherwise
*          it returns FNET_ERR.
*************************************************************************/
static int fnet_tcp_close( fnet_socket_t *sk )
{
    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control; 
    
    /* If the connection is closed, free the memory.*/
    if(sk->state == SS_UNCONNECTED)
    {
        cb->tcpcb_flags |= FNET_TCP_CBF_CLOSE;
        fnet_tcp_closesk(sk);
        return FNET_OK;
    }

    /* If SO_LINGER option is present.*/
    if(sk->options.flags & SO_LINGER)
    {
        
        if(sk->options.linger == 0)
        /* Linger is 0 so close the socket immediately. */
        {
            /* Hard reset.*/
            fnet_isr_lock();
            cb->tcpcb_flags |= FNET_TCP_CBF_CLOSE;
            fnet_tcp_abortsk(sk);
            fnet_isr_unlock();
            return FNET_OK;
        }
    }

    fnet_isr_lock();

    if(sk->state != SS_CONNECTED)
    {
        cb->tcpcb_flags |= FNET_TCP_CBF_CLOSE;
        fnet_tcp_abortsk(sk);

        /* Unlock interrupts.*/
        fnet_isr_unlock();
        return FNET_OK;
    }

    /* If the socket is not unlocked, try to send the data.*/
    if(!sk->send_buffer.is_shutdown)
    {
        sk->send_buffer.is_shutdown = 1;
        fnet_tcp_sendanydata(sk, 1);
    }

    fnet_isr_unlock();



    fnet_isr_lock();

    /* After this moment the unconnecetd socket must be deleted.*/
    cb->tcpcb_flags |= FNET_TCP_CBF_CLOSE;

    /* If the socket is already unconnected, close the socket.*/
    if(sk->state == SS_UNCONNECTED)
    {
        fnet_tcp_closesk(sk);
    }
    else
    {
        if(cb->tcpcb_connection_state != FNET_TCP_CS_TIME_WAIT)
        {
            if((sk->options.flags & SO_LINGER) && sk->options.linger)
                cb->tcpcb_timers.connection = ((sk->options.linger * FNET_TIMER_PERIOD_MS)
                                                                 / FNET_TCP_SLOWTIMO);
            else
                cb->tcpcb_timers.connection = FNET_TCP_ABORT_INTERVAL;

            sk->receive_buffer.is_shutdown = 1;
        }

        if(sk->receive_buffer.count)
            fnet_socket_buffer_release(&sk->receive_buffer);
    }
  
    fnet_isr_unlock();

    return FNET_OK;
}

/************************************************************************
* NAME: fnet_tcp_connect
*
* DESCRIPTION: This function performs the connection establishment.
*   
* RETURNS: If no error occurs, this function returns FNET_OK. Otherwise
*          it returns FNET_ERR.
*************************************************************************/
static int fnet_tcp_connect( fnet_socket_t *sk, struct sockaddr *foreign_addr)
{
    fnet_tcp_control_t  *cb;              
    char                options[FNET_TCP_MAX_OPT_SIZE]; 
    char                optionlen;                    
    int                 error;                 
    fnet_netif_t        *netif;              
    
    if((netif = fnet_socket_addr_route(foreign_addr)) == FNET_NULL)
    {
        error = FNET_ERR_NETUNREACH;
        goto ERROR;
    }

    /* TCP doesn't support broadcasting and multicasting.*/
    if(fnet_socket_addr_is_broadcast(foreign_addr, netif) || fnet_socket_addr_is_multicast(foreign_addr))
    {
        error = FNET_ERR_ADDRNOTAVAIL;
        goto ERROR;
    }

    /* Initialize the pointer to the control block.*/
    cb = (fnet_tcp_control_t *)sk->protocol_control;

    /* Initialize the control block.*/
    fnet_tcp_initconnection(sk);

    /* Set synchronized options.*/
    fnet_tcp_setsynopt(sk, options, &optionlen);

    /* Initialize sequnece number parameters.*/
    cb->tcpcb_sndseq = fnet_tcp_isntime;
    cb->tcpcb_maxrcvack = fnet_tcp_isntime + 1;
#if FNET_CFG_TCP_URGENT      
    cb->tcpcb_sndurgseq = cb->tcpcb_sndseq - 1;
#endif /* FNET_CFG_TCP_URGENT */

    /* Set the foreign address.*/
    sk->foreign_addr = *foreign_addr; 

    fnet_isr_lock();

    /* Send SYN segment.*/
    error = fnet_tcp_sendheadseg(sk, FNET_TCP_SGT_SYN, options, optionlen);

    /* Check the result.*/
    if(error)
    {
        fnet_isr_unlock();
        goto ERROR;
    }

    /* Change the states.*/
    cb->tcpcb_connection_state = FNET_TCP_CS_SYN_SENT;
    sk->state = SS_CONNECTING;

    /* Increase Initial Sequence Number.*/
    fnet_tcp_isntime += FNET_TCP_STEPISN;

    /* Initialize Abort Timer.*/
    cb->tcpcb_timers.retransmission = cb->tcpcb_rto;
    cb->tcpcb_timers.connection = FNET_TCP_ABORT_INTERVAL_CON;

    fnet_isr_unlock();

    return FNET_OK;

ERROR:
    fnet_socket_set_error(sk, error);
    return FNET_ERR; 
}

/************************************************************************
* NAME: fnet_tcp_accept
*
* DESCRIPTION: This function removes the created socket
*              from the incoming queue of the listening socket, 
*              and adds this socket to the main list.
*
* RETURNS: If the incoming socket is present, this function returns 
*          the first created incoming socket. Otherwise, it returns 0.
*************************************************************************/
static fnet_socket_t *fnet_tcp_accept( fnet_socket_t *listensk )
{
    fnet_socket_t *sk= FNET_NULL;

    /* If the incoming socket is not present, return.*/
    if(listensk->incoming_con)
    {
        fnet_isr_lock();
        /* Find the first incoming socket.*/
        sk = listensk->incoming_con;

        while(sk->next)
          sk = sk->next;

        /* Delete the incoming socket from the list.*/
        sk->head_con->incoming_con_len--;
        fnet_socket_list_del(&sk->head_con->incoming_con, sk);

        fnet_isr_unlock();
        sk->head_con = 0;
    }
    
    return sk;
}

/************************************************************************
* NAME: fnet_tcp_rcv
*
* DESCRIPTION: This function receives the data and sends the acknowledgment
*              (This acknowledgment segment informs about the new free size
*               in the input buffer).
* 
* RETURNS: If no error occurs, this function returns the length
*          of the received data. Otherwise, it returns FNET_ERR.
*************************************************************************/
static int fnet_tcp_rcv( fnet_socket_t *sk, char *buf, int len, int flags, struct sockaddr *foreign_addr)
{
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control;
    int                 remove; /* Remove flag. 1 means that the data must be deleted
                                 * from the input buffer after the reading.*/
    int                 error_code;                 
    
    /* Receive the flags.*/
    remove = !(flags & MSG_PEEK);

#if FNET_CFG_TCP_URGENT
    /* Reading of the OOB data.*/
    if(flags & MSG_OOB)
    {
        /* If the OOB data can't be read, return.*/
        if(sk->options.flags & SO_OOBINLINE)
        {
            error_code = FNET_ERR_INVAL;
            goto ERROR;
        }

        /* If the OOB data is present, read */
        if(cb->tcpcb_flags & FNET_TCP_CBF_RCVURGENT)
        {
            fnet_isr_lock();

            if(remove)
            {
                cb->tcpcb_rcvurgmark = FNET_TCP_NOT_USED;
                cb->tcpcb_flags &= ~FNET_TCP_CBF_RCVURGENT;
            }

            *buf = cb->tcpcb_iobc;
            fnet_isr_unlock();
            
            return FNET_TCP_URGENT_DATA_SIZE;
        }
        else
        {
            /* If the socket is not connected , return with the error. */
            if(sk->state != SS_CONNECTED)
            {
                error_code = FNET_ERR_NOTCONN;
                goto ERROR;                
            }

            return 0;
        }
    }
#endif /* FNET_CFG_TCP_URGENT */

    fnet_isr_lock();

#if FNET_CFG_TCP_URGENT
    /* Calculate the length of the data that can be received.*/
    if(cb->tcpcb_rcvurgmark > 0 && len >= cb->tcpcb_rcvurgmark)
    {
        len = cb->tcpcb_rcvurgmark;

        if(remove)
            cb->tcpcb_rcvurgmark = FNET_TCP_NOT_USED;
    }
    else 
#endif /* FNET_CFG_TCP_URGENT */
    if(sk->receive_buffer.count < len)
    {
        len = (int)sk->receive_buffer.count;
    }

    /* Copy the data to the buffer.*/
    len = fnet_socket_buffer_read_record(&sk->receive_buffer, buf, len, remove); 

    /* Remove the data from input buffer.*/
    if(remove)
    {
        /* Recalculate the new free size in the input buffer.*/
        cb->tcpcb_newfreercvsize += len;

        /* If the window is opened, send acknowledgment.*/
        if((cb->tcpcb_newfreercvsize >= (cb->tcpcb_rcvmss << 1)
                || cb->tcpcb_newfreercvsize >= (cb->tcpcb_rcvcountmax >> 1) /* More than half of RX buffer.*/
                || (!cb->tcpcb_rcvwnd && cb->tcpcb_newfreercvsize)) && (sk->state == SS_CONNECTED))
            fnet_tcp_sendack(sk);
    }

    /* If the socket is not connected and the data are not received, return with error.*/
    if(len == 0 && sk->state != SS_CONNECTED)
    {
        error_code = FNET_ERR_NOTCONN;
        goto ERROR_UNLOCK;
    }
    else
    {
        /* Set the foreign address and port.*/
        if(foreign_addr)
            *foreign_addr = sk->foreign_addr;
         
        /* If the socket is closed by peer and no data.*/
        if((len == 0) && (cb->tcpcb_flags & FNET_TCP_CBF_FIN_RCVD))
        {
            error_code = FNET_ERR_CONNCLOSED;
            goto ERROR_UNLOCK;
        }
    }
    
    fnet_isr_unlock();
    return len;
    
ERROR_UNLOCK:
    fnet_isr_unlock();
#if FNET_CFG_TCP_URGENT    
ERROR:
#endif    
    fnet_socket_set_error(sk, error_code);
    return FNET_ERR;
}

/************************************************************************
* NAME: fnet_tcp_snd
*
* DESCRIPTION: This function adds the data to the output buffer and
*              sends the data that can be sent.
*
* RETURNS: If no error occurs, this function returns the length
*          of the data that is added to the output buffer.
*          Otherwise, it returns FNET_ERR.
*************************************************************************/
static int fnet_tcp_snd( fnet_socket_t *sk, char *buf, int len, int flags, const struct sockaddr *foreign_addr)
{
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control; 
    fnet_netbuf_t       *netbuf;                                             
    long                sendlength = len;   /* Size of the data that must be sent.*/
    long                sentlength = 0;     /* Length of the sent data.*/
    long                freespace;          /* Free space in the output buffer.*/
    long                currentlen;         /* Current length.*/
    int                 dontroute = 0;      /* Routing flag.*/
    unsigned long       malloc_max;
    int                 error_code;

    FNET_COMP_UNUSED_ARG(foreign_addr);

    /* If the size of the data greater than the maximal size of the output buffer, return*/
    if(sendlength > FNET_TCP_MAX_BUFFER)
    {
        error_code = FNET_ERR_INVAL;
        goto ERROR;
    }

    /* If the socket is not connected, return*/
    if(sk->state != SS_CONNECTED)
    {
        error_code = FNET_ERR_NOTCONN;
        goto ERROR;
    }
    
    
    if(sendlength)
    {
        fnet_isr_lock();
        
        /* Caclulate a free space in the output buffer.*/
        freespace = (long)(sk->send_buffer.count_max - sk->send_buffer.count);

        /* Check maximum allocated memory chunck */
        malloc_max = fnet_malloc_max_netbuf();

        if(malloc_max < (long)(FNET_CFG_CPU_ETH0_MTU*1.5)) //TBD I do not like it ????
        {
            freespace = 0;     
        }
        else if(freespace > malloc_max)
        {
           freespace = (long)malloc_max;
        }
       
      
        /* If the function is nonblocking and the data length greater than the freespace, recalculate the size of the data*/
        if(freespace < sendlength)
        {
            /* If the data can't be added to the output buffer, return*/
            if(freespace <= 0)
            {
                fnet_isr_unlock();
                return 0;
            }
            else
            {
                /* Recalculate the data size.*/
                sendlength = freespace;
            #if FNET_CFG_TCP_URGENT            
                flags &= ~MSG_OOB;
            #endif /* FNET_CFG_TCP_URGENT */
            }
        }

    #if FNET_CFG_TCP_URGENT
        /* Process the OOB data.*/
        if(flags & MSG_OOB)
        {
            /* If the urgent data are already present, the urgent byte will not be added.*/
            if(FNET_TCP_COMP_GE(cb->tcpcb_sndurgseq, cb->tcpcb_rcvack))
            {
                sendlength -= 1;

                /* If the  data size is 0, return.*/
                if(!sendlength)
                {
                    fnet_isr_unlock();
                    return 0;
                }
            }
            else
            {
                /* Calculate the new sequence number of the urgent data.*/
                cb->tcpcb_sndurgseq = cb->tcpcb_rcvack + sk->send_buffer.count + sendlength - 1;
            }

        }
    #endif /* FNET_CFG_TCP_URGENT */

        /* If the routing tables should be bypassed for this message only, set dontroute flag.*/
        if((flags & MSG_DONTROUTE) && !(sk->options.flags & SO_DONTROUTE))
        {
            dontroute = 1;
            sk->options.flags |= SO_DONTROUTE;
        }

       
        /* Receive the freespace value.*/
    //      freespace = (long)(sk->send_buffer.count_max - sk->send_buffer.count);

        /* Try to add the data.*/
        if(freespace > 0)
        {
            cb->tcpcb_flags |= FNET_TCP_CBF_INSND;

            /* Calculate the data size that can be added.*/
            if(sendlength > freespace)
                currentlen = freespace;
            else
                currentlen = sendlength;

            netbuf = fnet_netbuf_from_buf(&buf[sentlength], currentlen, FNET_TRUE);

            /* Check the memory allocation.*/
            if(netbuf) 
            {
                sendlength -= currentlen;
                sentlength += currentlen;

                if(fnet_socket_buffer_append_record(&sk->send_buffer, netbuf) == FNET_OK)
                {

                    /* If the window of another side is closed, set the persist timer.*/
                    if(!cb->tcpcb_sndwnd)
                    {
                        if(cb->tcpcb_timers.persist == FNET_TCP_TIMER_OFF)
                        {
                            cb->tcpcb_cprto = cb->tcpcb_rto;
                            cb->tcpcb_timers.persist = cb->tcpcb_cprto;
                        }
                    }
                    else
                    {
                        /* Try to send the data.*/
                        while(1)
                        {
                            /* If the connection is not established, delete the data. Otherwise try to send the data*/
                            if(sk->state == SS_CONNECTED)
                            {
                                if(!fnet_tcp_sendanydata(sk, 1))
                                {
                                    cb->tcpcb_flags &= ~FNET_TCP_CBF_INSND;
                                    break;
                                }
                            }
                            else
                            {
                                /* If socket is not connected, delete the output buffer.*/
                                fnet_socket_buffer_release(&sk->send_buffer);
                                cb->tcpcb_flags &= ~FNET_TCP_CBF_INSND;
                                break;
                            }
                        }
                    }                    
                }
                else /* Not able to add to the socket send buffer.*/
                {
                    fnet_netbuf_free( netbuf );
                    fnet_isr_unlock();
                    return 0;
                }
            }
        }
            
        /* Receive the freespace value.*/
        freespace = (long)(sk->send_buffer.count_max - sk->send_buffer.count);

    #if FNET_CFG_TCP_URGENT  
        if(FNET_TCP_COMP_GE(cb->tcpcb_sndurgseq, cb->tcpcb_rcvack + sk->send_buffer.count))
            cb->tcpcb_sndurgseq = cb->tcpcb_rcvack - 1;
    #endif /* FNET_CFG_TCP_URGENT */

        
        /* Remove the dontroute flag.*/
        if(dontroute)
            sk->options.flags &= ~SO_DONTROUTE;

        
        fnet_isr_unlock();
    }
    return sentlength;

ERROR:
    fnet_socket_set_error(sk, error_code);
    return FNET_ERR;
}

/************************************************************************
* NAME: fnet_tcp_shutdown
*
* DESCRIPTION: This function closes a write-half, read-half, or 
*              both halves of the connection.
*
* RETURNS: If no error occurs, this function returns FNET_OK. Otherwise
*          it returns FNET_ERR. 
*************************************************************************/
static int fnet_tcp_shutdown( fnet_socket_t *sk, int how )
{
    /* If the socket is not connected, return.*/
    if(sk->state != SS_CONNECTED)
    {
        fnet_socket_set_error(sk, FNET_ERR_NOTCONN);
        return FNET_ERR;
    }
    else
    {
        fnet_isr_lock();

        /* Shutdown the writing.*/
        if(how & SD_WRITE && !sk->send_buffer.is_shutdown)
        {
            /* Set the flag of the buffer.*/
            sk->send_buffer.is_shutdown = 1;

            /* Send the data that is in the output buffer.*/
            fnet_tcp_sendanydata(sk, 1);
        }

        /* Shutdown the reading.*/
        if(how & SD_READ && !sk->receive_buffer.is_shutdown)
        {
            fnet_socket_buffer_release(&sk->receive_buffer);

            /* Set the flag of the buffer (Data can't be read).*/
            sk->receive_buffer.is_shutdown = 1;
        }
        
        fnet_isr_unlock();

        return FNET_OK;
    }
}

/************************************************************************
* NAME: fnet_tcp_setsockopt
*
* DESCRIPTION: This function sets a TCP option.
*
* RETURNS: If no error occurs, this function returns FNET_OK. Otherwise
*          it returns FNET_ERR.
*************************************************************************/
static int fnet_tcp_setsockopt( fnet_socket_t *sk, int level, int optname, char *optval, int optlen )
{
    int error_code;
    
    /* If the level is not IPPROTO_TCP, go to IP processing.*/
    if(level == IPPROTO_TCP)
    {
        /* Check the option size.*/
        switch(optname)
        {
            case TCP_MSS:
                if(optlen != sizeof(unsigned short))
                {
                    error_code = FNET_ERR_INVAL;
                    goto ERROR;
                }
                break;
            case TCP_KEEPCNT:  
            case TCP_KEEPINTVL:
            case TCP_KEEPIDLE:
            case TCP_NODELAY:
        #if FNET_CFG_TCP_URGENT            
            case TCP_BSD:
        #endif            
                if(optlen != sizeof(int))
                {
                    error_code = FNET_ERR_INVAL;
                    goto ERROR;
                }
                break;
            default:
                /* The option is not supported.*/
                error_code = FNET_ERR_NOPROTOOPT;
                goto ERROR;                
        }

        /* Process the option.*/
        switch(optname)
        {
            /* Maximal segment size option.*/
            case TCP_MSS:
                if(!(*((unsigned short *)(optval))))
                {
                    error_code = FNET_ERR_INVAL;
                    goto ERROR;
                }

                sk->options.tcp_opt.mss = *((unsigned short *)(optval));
                break;
            /* Keepalive probe retransmit limit.*/
            case TCP_KEEPCNT:
                if(!(*((unsigned int *)(optval))))
                {
                    error_code = FNET_ERR_INVAL;
                    goto ERROR;                  
                }

                sk->options.tcp_opt.keep_cnt = *((int *)(optval));
                break;
            /* Keepalive retransmit interval.*/
            case TCP_KEEPINTVL:
                if(!(*((unsigned int *)(optval))))
                {
                    error_code = FNET_ERR_INVAL;
                    goto ERROR;
                }

                sk->options.tcp_opt.keep_intvl = *((int *)(optval))*(1000/FNET_TCP_SLOWTIMO);
                break;            
            /* Time between keepalive probes.*/
            case TCP_KEEPIDLE:
                if(!(*((unsigned int *)(optval))))
                {
                    error_code = FNET_ERR_INVAL;
                    goto ERROR;
                }

                sk->options.tcp_opt.keep_idle = *((int *)(optval))*(1000/FNET_TCP_SLOWTIMO);
                break;
        #if FNET_CFG_TCP_URGENT                            
            /* BSD interpretation of the urgent pointer.*/
            case TCP_BSD:
        #endif            
            /* TCP_NO_DELAY option.*/
            case TCP_NODELAY:
                if(*((int *)(optval)))
                    sk->options.tcp_opt.flags |= optname;
                else
                    sk->options.tcp_opt.flags &= ~optname;

                break;
        }
        
        return FNET_OK;
    }
    else
    {
        /* IP level option processing.*/
        return fnet_ip_setsockopt(sk, level, optname, optval, optlen);
    }

ERROR:
    fnet_socket_set_error(sk, error_code);
    return FNET_ERR;    
}

/************************************************************************
* NAME: fnet_tcp_getsockopt
*
* DESCRIPTION: This function receives a TCP option.
*
* RETURNS: If no error occurs, this function returns FNET_OK. Otherwise
*          it returns FNET_ERR.
*************************************************************************/
static int fnet_tcp_getsockopt( fnet_socket_t *sk, int level, int optname, char *optval, int *optlen )
{
  
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control;
    
   
    if(level == IPPROTO_TCP)
    {
        switch(optname)
        {
            case TCP_KEEPCNT:
                *((int *)(optval)) = sk->options.tcp_opt.keep_cnt;
                break;
            case TCP_KEEPINTVL:
                *((int *)(optval)) = sk->options.tcp_opt.keep_intvl/(1000/FNET_TCP_SLOWTIMO);
                break;
            case TCP_KEEPIDLE:
                *((int *)(optval)) = sk->options.tcp_opt.keep_idle/(1000/FNET_TCP_SLOWTIMO);
                break;
        #if FNET_CFG_TCP_URGENT                
            case TCP_BSD:
        #endif            
            case TCP_NODELAY:
                if(sk->options.tcp_opt.flags & optname)
                    *((int *)(optval)) = 1;
                else
                    *((int *)(optval)) = 0;
                break;
            case TCP_FINRCVD:
                if(cb->tcpcb_flags & FNET_TCP_CBF_FIN_RCVD)
                    *((int *)(optval)) = 1;
                else
                    *((int *)(optval)) = 0;
                break;
    #if FNET_CFG_TCP_URGENT               
            case TCP_URGRCVD:
                if(cb->tcpcb_flags & FNET_TCP_CBF_RCVURGENT)
                    *((int *)(optval)) = 1;
                else
                    *((int *)(optval)) = 0;
                break;
    #endif                
            case TCP_MSS:
                *((unsigned short *)(optval)) = sk->options.tcp_opt.mss;
                *optlen = sizeof(unsigned short);
                return FNET_OK;                
            default:
                fnet_socket_set_error(sk, FNET_ERR_NOPROTOOPT);
                return FNET_ERR;
        }

        *optlen = sizeof(int);

        return FNET_OK;
    }
    else
    {
        /* IP level option processing.*/
        return fnet_ip_getsockopt(sk, level, optname, optval, optlen);
    }
}

/************************************************************************
* NAME: fnet_tcp_listen
*
* DESCRIPTION: This function changes
*              the state of the socket to the listening state.
*
* RETURNS: FNET_OK.
*************************************************************************/
static int fnet_tcp_listen( fnet_socket_t *sk, int backlog )
{
    /* Initializen the pointer to the control block.*/
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control;
    
    
    if(sk->state == SS_LISTENING)
    {
        /* If the socket number of the listening socket is greater than the new backlog,
         * delete the sockets.*/
        fnet_isr_lock();

        while(backlog < sk->partial_con_len + sk->incoming_con_len && sk->partial_con_len)
          fnet_tcp_abortsk(sk->partial_con);

        while(backlog < sk->incoming_con_len)
          fnet_tcp_abortsk(sk->incoming_con);

        sk->con_limit = backlog;
        fnet_isr_unlock();
    }
    else
    {
        fnet_tcp_initconnection(sk);

        /* Foreign address must be any.*/
        ((struct sockaddr_in *)(&sk->foreign_addr))->sin_addr.s_addr = INADDR_ANY;
        sk->foreign_addr.sa_port = 0;
        sk->foreign_addr.sa_family = AF_INET;
        sk->con_limit = backlog;

        /* Change the state.*/
        cb->tcpcb_connection_state = FNET_TCP_CS_LISTENING;
        sk->state = SS_LISTENING;
    }
    
    return FNET_OK;
}

/************************************************************************
* NAME: fnet_tcp_drain
*
* DESCRIPTION: fnet_tcp_drain removes the temporary data.
*
* RETURNS: None.          
*************************************************************************/
static void fnet_tcp_drain( void )
{
    fnet_socket_t       *sk; 
    fnet_socket_t       *delsk;
    fnet_tcp_control_t  *cb;    


    fnet_isr_lock();

    /* Receive the pointer to the first socket.*/
    sk = fnet_tcp_prot_if.head;

    while(sk)
    {
        cb = (fnet_tcp_control_t *)sk->protocol_control;
        delsk = sk;
        sk = sk->next;

        /* if((cb->tcpcb_connection_state == FNET_TCP_CS_TIME_WAIT) && (cb->tcpcb_flags & FNET_TCP_CBF_CLOSE))*/
        if((cb->tcpcb_flags & FNET_TCP_CBF_CLOSE))
        {
            /* Remove the socket that to be closed.  */
            fnet_tcp_closesk(delsk);
        }
        /* Delete all partial and incoming connections */
        else if(delsk->state == SS_LISTENING)
        {
            while(delsk->partial_con)
              fnet_tcp_abortsk(delsk->partial_con);
        /* while (delsk->incoming_con)
           fnet_tcp_abortsk(delsk->incoming_con); */
        }
#if !FNET_CFG_TCP_DISCARD_OUT_OF_ORDER        
        else
            /* Remove the temporary data.*/
            fnet_tcp_deletetmpbuf(cb);
#endif            
    }


    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_tcp_initconnection
*
* DESCRIPTION: This function creates and initializes the control block, 
*              and initializes the socket.
* 
* RETURNS: None.
*************************************************************************/
static void fnet_tcp_initconnection( fnet_socket_t *sk )
{
    fnet_tcp_control_t  *cb;

    cb = sk->protocol_control;

    fnet_memset_zero(cb, sizeof(fnet_tcp_control_t));

    /* Set the default maximal segment size value.*/
    cb->tcpcb_sndmss = FNET_TCP_DEFAULT_MSS;
    cb->tcpcb_rcvmss = sk->options.tcp_opt.mss;
    
    /* Set the length of the socket buffers.*/
    cb->tcpcb_rcvcountmax = sk->receive_buffer.count_max;

    /* The input buffer can't be greater than the FNET_TCP_MAX_BUFFER value.*/
    if(cb->tcpcb_rcvcountmax > FNET_TCP_MAX_BUFFER)
        cb->tcpcb_rcvcountmax = FNET_TCP_MAX_BUFFER;

    /* If a segment size greater than the buffer length, recalculate the segment size.*/
    if(cb->tcpcb_rcvcountmax < cb->tcpcb_rcvmss)
        cb->tcpcb_rcvmss = (unsigned short)cb->tcpcb_rcvcountmax;

    /* Receive a scale of the input window.*/
    while(FNET_TCP_MAXWIN << cb->tcpcb_recvscale < cb->tcpcb_rcvcountmax)
      cb->tcpcb_recvscale++;

    /* Stop all timers.*/
    cb->tcpcb_timers.retransmission = FNET_TCP_TIMER_OFF;
    cb->tcpcb_timers.connection = FNET_TCP_TIMER_OFF;
    cb->tcpcb_timers.abort = FNET_TCP_TIMER_OFF;
    cb->tcpcb_timers.round_trip = FNET_TCP_TIMER_OFF;
    cb->tcpcb_timers.persist = FNET_TCP_TIMER_OFF;
    cb->tcpcb_timers.keepalive = FNET_TCP_TIMER_OFF;
    cb->tcpcb_timers.delayed_ack = FNET_TCP_TIMER_OFF;

    /* Initialize the retransmission timeout.*/
    cb->tcpcb_rto = cb->tcpcb_crto = FNET_TCP_TIMERS_INIT;

#if FNET_CFG_TCP_URGENT
    /* Initialize the receive urgent mark.*/
    cb->tcpcb_rcvurgmark = FNET_TCP_NOT_USED;
#endif /* FNET_CFG_TCP_URGENT */

    /* Initialize Slow Start Threshold.*/
    cb->tcpcb_ssthresh = FNET_TCP_MAX_BUFFER;

    /* Clear the input buffer.*/
    if(sk->receive_buffer.count)
        fnet_socket_buffer_release(&sk->receive_buffer);
 
}

/************************************************************************
* NAME: fnet_tcp_inputsk
*
* DESCRIPTION: This function processes the input segments of 
*              the corresponding socket.
*
* RETURNS: TRUE if the input segment must be deleted. Otherwise
*          this function returns FALSE.          
*************************************************************************/
static int fnet_tcp_inputsk( fnet_socket_t *sk, fnet_netbuf_t *insegment, struct sockaddr *src_addr,  struct sockaddr *dest_addr)
{
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control; 
    fnet_tcp_control_t  *pcb;                   /* Pointer to the partial control block.*/
    unsigned char       sgmtype;                /* Flags of the segment.*/
    fnet_socket_t       *psk;                   /* Pointer to the partial socket.*/
    int                 result = FNET_TRUE;                         
    char                options[FNET_TCP_MAX_OPT_SIZE];    
    char                optionlen;                         
    unsigned long       repsize;                /* Size of repeated data.*/
    int                 ackparam = 0;           /* Acknowledgment parameter.*/
    unsigned long       tcp_seq = fnet_ntohl(FNET_TCP_SEQ(insegment));
    unsigned long       tcp_length = (unsigned long)FNET_TCP_LENGTH(insegment);
    unsigned long       tcp_ack = fnet_ntohl(FNET_TCP_ACK(insegment));

    /* Get the flags.*/
    sgmtype = (unsigned char)(FNET_TCP_FLAGS(insegment));
    
    /* Check the sequence number.*/
    switch(cb->tcpcb_connection_state)
    {
        case FNET_TCP_CS_SYN_SENT:
        case FNET_TCP_CS_LISTENING:
            break;

        case FNET_TCP_CS_SYN_RCVD:
            if(cb->tcpcb_prev_connection_state == FNET_TCP_CS_SYN_SENT && (sgmtype & FNET_TCP_SGT_SYN))
            {
                /* Check the sequence number for simultaneouos open.*/
                if(tcp_seq == cb->tcpcb_sndack - 1)
                    break;
            }
        default:
            if(FNET_TCP_COMP_G(cb->tcpcb_sndack, tcp_seq)) 
            {
                if(FNET_TCP_COMP_G(tcp_seq + insegment->total_length - tcp_length, cb->tcpcb_sndack))
                {
                    /* Delete the left repeated part.*/
                    repsize = fnet_tcp_getsize(tcp_seq, cb->tcpcb_sndack);
                    fnet_netbuf_cut_center(&insegment, (int)tcp_length, (int)repsize);

                    /* If urgent  flag is present, recalculate of the urgent pointer.*/
                    if(sgmtype & FNET_TCP_SGT_URG)
                    {
                        if((int)fnet_ntohs(FNET_TCP_URG(insegment)) - (int)repsize >= 0)               
                        {
                            FNET_TCP_URG(insegment) = fnet_htons((unsigned short)(fnet_ntohs(FNET_TCP_URG(insegment)) - repsize)); 
                        }
                        else
                        {
                            sgmtype &= ~FNET_TCP_SGT_URG;
                            FNET_TCP_SET_FLAGS(insegment) = sgmtype;
                        }
                    }

                    /* Set the sequence number.*/
                    tcp_seq = cb->tcpcb_sndack;
                    FNET_TCP_SEQ(insegment) = fnet_htonl(tcp_seq);

                    /* Acknowledgment must be sent immediatelly.*/
                    ackparam |= FNET_TCP_AP_SEND_IMMEDIATELLY;
                }
                else
                {
                    /* Segment is repeated */
                    /* Send the acknowledgment */           
                    fnet_tcp_sendack(sk);
                    return FNET_TRUE;
                }
            }

            if(FNET_TCP_COMP_G(tcp_seq, cb->tcpcb_sndack + cb->tcpcb_rcvwnd))
            {
                /* Segment is not in the window*/
                /* Send the acknowledgment */
                fnet_tcp_sendack(sk);
                return FNET_TRUE;
            }
            else
            {
                if(FNET_TCP_COMP_G(tcp_seq + insegment->total_length - tcp_length, cb->tcpcb_sndack + cb->tcpcb_rcvwnd))
                {
                    /* Delete the right part that is not in the window.*/
                    fnet_netbuf_trim(&insegment, -(int)fnet_tcp_getsize(cb->tcpcb_sndack + cb->tcpcb_rcvwnd,
                                               (unsigned long)(tcp_seq + insegment->total_length - tcp_length)));
                    /* Acknowledgment must be sent immediatelly.*/
                    ackparam |= FNET_TCP_AP_SEND_IMMEDIATELLY;
                }
            }
    }

    /* Process the reset segment with acknowledgment.*/
    if((sgmtype &(FNET_TCP_SGT_RST | FNET_TCP_SGT_ACK)) == (FNET_TCP_SGT_RST | FNET_TCP_SGT_ACK))
    {
        if(cb->tcpcb_connection_state == FNET_TCP_CS_SYN_SENT)
        {
              if(tcp_ack == cb->tcpcb_sndseq)
                  /* Close the socket (connecting is failed).*/
                  sk->options.local_error = FNET_ERR_CONNRESET;

              fnet_tcp_closesk(sk);
        }

        return FNET_TRUE;
    }

    /* Process the reset segment without acknowledgment.*/
    if(sgmtype & FNET_TCP_SGT_RST)
    {
        switch(cb->tcpcb_connection_state)
        {
            case FNET_TCP_CS_LISTENING:
            case FNET_TCP_CS_SYN_SENT:
              return FNET_TRUE;

            default:
              /* Close the socket.*/
              sk->options.local_error = FNET_ERR_CONNRESET;
              fnet_tcp_closesk(sk);
        }

        return FNET_TRUE;
    }


    /* Process the SYN segment.*/
    if(sgmtype & FNET_TCP_SGT_SYN)
    {
        switch(cb->tcpcb_connection_state)
        {
            case FNET_TCP_CS_SYN_SENT:
            case FNET_TCP_CS_LISTENING:
              break;

            case FNET_TCP_CS_SYN_RCVD:
              if((cb->tcpcb_prev_connection_state == FNET_TCP_CS_SYN_SENT) && (tcp_seq == (cb->tcpcb_sndack - 1))) 
                  break;

            default:
              /* Close the socket and send the reset segment.*/
              fnet_tcp_sendrst(&sk->options, insegment, dest_addr, src_addr);
              fnet_tcp_closesk(sk);
              return FNET_TRUE;
        }
    }
    else
    {
        /* Process the segment without SYN flag.*/
        switch(cb->tcpcb_connection_state)
        {
            case FNET_TCP_CS_LISTENING:
              if(sgmtype & FNET_TCP_SGT_ACK)
                  /* Send the reset segment.*/
                  fnet_tcp_sendrst(&sk->options, insegment, dest_addr, src_addr);

              return FNET_TRUE;

            case FNET_TCP_CS_SYN_SENT:
              if(sgmtype & FNET_TCP_SGT_ACK)
                  /* Send the reset segment.*/
                  fnet_tcp_sendrst(&sk->options, insegment, dest_addr, src_addr);

              return FNET_TRUE;
        }
    }


    /* Process the segment with acknowledgment.*/
    if(sgmtype & FNET_TCP_SGT_ACK)
    {
        switch(cb->tcpcb_connection_state)
        {
            case FNET_TCP_CS_SYN_SENT:
            case FNET_TCP_CS_SYN_RCVD:
              if(tcp_ack != cb->tcpcb_sndseq) 
              {
                  /* Send the reset segment.*/
                  fnet_tcp_sendrst(&sk->options, insegment, dest_addr, src_addr);
                  return FNET_TRUE;
              }

              break;

            case FNET_TCP_CS_LISTENING:
              /* Send the reset segment.*/
              fnet_tcp_sendrst(&sk->options, insegment, dest_addr, src_addr);
              return FNET_TRUE;
              
            default:
              if(!fnet_tcp_hit(cb->tcpcb_rcvack, cb->tcpcb_maxrcvack, tcp_ack))
              {
                  if(FNET_TCP_COMP_G(tcp_ack, cb->tcpcb_maxrcvack))
                      /* Send the acknowledgment.*/
                      fnet_tcp_sendack(sk);

                  return FNET_TRUE;
              }
        }
    }
    else
    {
        /* Process the segment without acknowledgment.*/
        switch(cb->tcpcb_connection_state)
        {
            case FNET_TCP_CS_SYN_SENT:
            case FNET_TCP_CS_LISTENING:
              break;

            case FNET_TCP_CS_SYN_RCVD:
              fnet_tcp_sendack(sk);

            default:
              return FNET_TRUE;
        }
    }

    /* Set the window size (of another side).*/
    if(sgmtype & FNET_TCP_SGT_SYN)
        cb->tcpcb_sndwnd = fnet_ntohs(FNET_TCP_WND(insegment));
    else
        cb->tcpcb_sndwnd = (unsigned long)(fnet_ntohs(FNET_TCP_WND(insegment)) << cb->tcpcb_sendscale);

    if(cb->tcpcb_maxwnd < cb->tcpcb_sndwnd)
        cb->tcpcb_maxwnd = cb->tcpcb_sndwnd;

    /* Main processing.*/
    switch(cb->tcpcb_connection_state)
    {
        case FNET_TCP_CS_SYN_SENT:
          cb->tcpcb_sndack = tcp_seq + 1; 

          /* Process the second segment of the open.*/
          if(sgmtype & FNET_TCP_SGT_ACK)
          {
              cb->tcpcb_rcvack = tcp_ack;

#if FNET_CFG_TCP_URGENT
              /* Initialize the urgent sequence number.*/
              cb->tcpcb_rcvurgseq = tcp_seq;
#endif /* FNET_CFG_TCP_URGENT */

              /* Receive the options.*/
              fnet_tcp_getopt(sk, insegment);
              fnet_tcp_getsynopt(sk);

              /* If MSS of another side 0, return.*/
              if(!cb->tcpcb_sndmss)
              {
                  fnet_tcp_sendrst(&sk->options, insegment, dest_addr, src_addr);
                  fnet_tcp_closesk(sk);
                  return FNET_TRUE;
              }

              /* Stop the timers.*/
              cb->tcpcb_timers.retransmission = FNET_TCP_TIMER_OFF;
              cb->tcpcb_timers.connection = FNET_TCP_TIMER_OFF;

              /* Send the acknowledgment (third segment of the open).*/
              fnet_tcp_sendack(sk);

              /* Change the states.*/
              cb->tcpcb_connection_state = FNET_TCP_CS_ESTABLISHED;
              sk->state = SS_CONNECTED;

              /* Initialize the keepalive timer.*/
              if(sk->options.flags & SO_KEEPALIVE)
                cb->tcpcb_timers.keepalive = sk->options.tcp_opt.keep_idle;

              break;
          }
          else
          /* Process the simultaneous open.*/
          {
              /* Reinitialize the retrasmission timer.*/
              cb->tcpcb_timers.retransmission = cb->tcpcb_rto;

              /* Receive the options.*/
              fnet_tcp_getopt(sk, insegment);
              fnet_tcp_getsynopt(sk);

              /* If MSS of another side 0, return.*/
              if(!cb->tcpcb_sndmss)
              {
                  fnet_tcp_sendrst(&sk->options, insegment, dest_addr, src_addr);
                  fnet_tcp_closesk(sk);
                  return FNET_TRUE;
              }

#if FNET_CFG_TCP_URGENT
              /* Initialize the urgent sequence number.*/
              cb->tcpcb_rcvurgseq = tcp_seq;
#endif /* FNET_CFG_TCP_URGENT */

              /* Change the states.*/
              cb->tcpcb_connection_state = FNET_TCP_CS_SYN_RCVD;
              cb->tcpcb_prev_connection_state = FNET_TCP_CS_SYN_SENT;

              /* Send acknowledgment.*/
              fnet_tcp_sendack(sk);
              break;
          }

        /* Process the first segment.*/       
        case FNET_TCP_CS_LISTENING:

            /* If socket can't be created, return.*/
            if(sk->partial_con_len + sk->incoming_con_len >= sk->con_limit)
            {
                fnet_memset_zero(&sk->foreign_addr, sizeof(sk->foreign_addr));
                cb->tcpcb_sndwnd = 0;
                cb->tcpcb_maxwnd = 0;
                break;
            }

            /* Create the socket.*/
            psk = fnet_socket_copy(sk);

            fnet_memset_zero(&sk->foreign_addr, sizeof(sk->foreign_addr));         

            /* Check the memory allocation.*/
            if(!psk)
            {
                cb->tcpcb_sndwnd = 0;
                cb->tcpcb_maxwnd = 0;
                break;
            }

            /* Set the local address.*/
            psk->local_addr = *dest_addr;

            /* Create the control block.*/
            pcb = (fnet_tcp_control_t *)fnet_malloc(sizeof(fnet_tcp_control_t));

            /* Check the memory allocation.*/
            if(!pcb)
            {
                fnet_free(psk);
                cb->tcpcb_sndwnd = 0;
                cb->tcpcb_maxwnd = 0;
                break;
            }

            /* Initialize the pointer.*/
            psk->protocol_control = (void *)pcb;
            fnet_tcp_initconnection(psk);

            /* Copy the control block parameters.*/
            pcb->tcpcb_sndwnd = cb->tcpcb_sndwnd;
            pcb->tcpcb_maxwnd = cb->tcpcb_maxwnd;
            cb->tcpcb_sndwnd = 0;
            cb->tcpcb_maxwnd = 0;

            /* Add the new socket to the partial list.*/
            fnet_tcp_addpartialsk(sk, psk);

            /* Initialize the parameters of the control block.*/
            pcb->tcpcb_sndack = tcp_seq + 1;
            pcb->tcpcb_sndseq = fnet_tcp_isntime;
            pcb->tcpcb_maxrcvack = fnet_tcp_isntime + 1;
          

#if FNET_CFG_TCP_URGENT  
            pcb->tcpcb_sndurgseq = pcb->tcpcb_sndseq;        
            pcb->tcpcb_rcvurgseq = tcp_seq;
#endif /* FNET_CFG_TCP_URGENT */

            /* Change the states.*/
            psk->state = SS_CONNECTING;
            pcb->tcpcb_prev_connection_state = FNET_TCP_CS_LISTENING;
            pcb->tcpcb_connection_state = FNET_TCP_CS_SYN_RCVD;

            /* Receive the options.*/
            fnet_tcp_getopt(psk, insegment);
            fnet_tcp_getsynopt(psk);

            /* If MSS of another side 0, return.*/
            if(!pcb->tcpcb_sndmss)
            {
                fnet_tcp_sendrst(&sk->options, insegment, dest_addr, src_addr);
                fnet_tcp_closesk(psk);
                break;
            }

            /* Set the options.*/
            fnet_tcp_setsynopt(psk, options, &optionlen);

            /* Send SYN segment.*/
            fnet_tcp_sendheadseg(psk, FNET_TCP_SGT_SYN | FNET_TCP_SGT_ACK, options, optionlen);

            /* Increase ISN (Initial Sequence Number).*/
            fnet_tcp_isntime += FNET_TCP_STEPISN;

            /* Initialization the connection timer.*/
            pcb->tcpcb_timers.connection = FNET_TCP_ABORT_INTERVAL_CON;
            pcb->tcpcb_timers.retransmission = pcb->tcpcb_rto;
            break;

        case FNET_TCP_CS_SYN_RCVD:

          /* Change the states.*/
          cb->tcpcb_connection_state = FNET_TCP_CS_ESTABLISHED;
          sk->state = SS_CONNECTED;

          /* Stop the connection and retransmission timers.*/
          cb->tcpcb_timers.connection = FNET_TCP_TIMER_OFF;
          cb->tcpcb_timers.retransmission = FNET_TCP_TIMER_OFF;

          cb->tcpcb_rcvack = tcp_ack;

          /* If previous state is FNET_TCP_CS_LISTENING, process the acknowledgment (third segment of the open)
           * Otherwise, process the SYN segment.*/
          if(cb->tcpcb_prev_connection_state == FNET_TCP_CS_LISTENING)
          {
              /* Add the partial socket to the list of incoming socket.*/
              fnet_tcp_movesk2incominglist(sk);

              /* Proceed the processing.*/
              result = fnet_tcp_dataprocess(sk, insegment, &ackparam);
              break;
          }
          else
          {
              if(!(sgmtype & FNET_TCP_SGT_SYN))
              {
                  /* Proseed the processing.*/
                  result = fnet_tcp_dataprocess(sk, insegment, &ackparam);
              }
              else
              {
                  /* Initialize the keepalive timer.*/
                  if(sk->options.flags & SO_KEEPALIVE)
                      cb->tcpcb_timers.keepalive = sk->options.tcp_opt.keep_idle;//FNET_TCP_KEEP_ALIVE_TIMEO;
              }

              break;
          }

        case FNET_TCP_CS_FIN_WAIT_2:
        case FNET_TCP_CS_CLOSE_WAIT:
        case FNET_TCP_CS_ESTABLISHED:

          /* Proseed the processing.*/
          result = fnet_tcp_dataprocess(sk, insegment, &ackparam);
          break;

        case FNET_TCP_CS_FIN_WAIT_1:

          /* Proseed the processing.*/
          result = fnet_tcp_dataprocess(sk, insegment, &ackparam);

          if(cb->tcpcb_sndseq == cb->tcpcb_rcvack && cb->tcpcb_connection_state == FNET_TCP_CS_FIN_WAIT_1)
              /* Change the state.*/
              cb->tcpcb_connection_state = FNET_TCP_CS_FIN_WAIT_2;

          break;

        case FNET_TCP_CS_LAST_ACK:
          if(tcp_ack == cb->tcpcb_sndseq) 
              /* Close the socket.*/
              fnet_tcp_closesk(sk);

          return FNET_TRUE;

        case FNET_TCP_CS_CLOSING:
          if(tcp_ack == cb->tcpcb_sndseq) 
          {
              cb->tcpcb_connection_state = FNET_TCP_CS_TIME_WAIT;
              /* Set the  timeout of the TIME_WAIT state.*/
              cb->tcpcb_timers.connection = FNET_TCP_TIME_WAIT;
              cb->tcpcb_timers.retransmission = FNET_TCP_TIMER_OFF;
              cb->tcpcb_timers.keepalive = FNET_TCP_TIMER_OFF;
          }

          break;

        case FNET_TCP_CS_TIME_WAIT:
          fnet_tcp_sendrst(&sk->options, insegment, dest_addr, src_addr);
          break;
    }

    /* If the input buffer is closed, delete the input data.*/
    if(sk->receive_buffer.is_shutdown && sk->receive_buffer.count)
        fnet_socket_buffer_release(&sk->receive_buffer);
    
    return result;
}

/************************************************************************
* NAME: fnet_tcp_dataprocess
*
* DESCRIPTION: This function proceeds the processing of the input segemnt.
*               
* RETURNS: TRUE if the input segment must be deleted. Otherwise
*          this function returns FALSE.             
*************************************************************************/
static int fnet_tcp_dataprocess( fnet_socket_t *sk, fnet_netbuf_t *insegment, int *ackparam )
{
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control;       
    long                size;                                     
    short               err;
    int                 delflag = 1;
    unsigned long       seq;
    unsigned long       tcp_ack = fnet_ntohl(FNET_TCP_ACK(insegment));

    /* Reinitialize the keepalive timer.*/
    if(sk->options.flags & SO_KEEPALIVE)
        cb->tcpcb_timers.keepalive = sk->options.tcp_opt.keep_idle;
    else
        cb->tcpcb_timers.keepalive = FNET_TCP_TIMER_OFF;

    /* Reset the abort timer.*/
    cb->tcpcb_timers.abort = FNET_TCP_TIMER_OFF;

    /* If acknowledgment is repeated.*/
    if(cb->tcpcb_rcvack == tcp_ack)
    {
        /* If unacknowledged data is present.*/
        if(cb->tcpcb_sndseq != cb->tcpcb_rcvack && sk->send_buffer.count)
        {
            /* Increase the timer of rpeated acknowledgments.*/
            cb->tcpcb_fastretrcounter++;

            /* If the number of repeated acknowledgments is FNET_TCP_NUMBER_FOR_FAST_RET,
             * process the fast retransmission.*/
            if(cb->tcpcb_fastretrcounter == FNET_TCP_NUMBER_FOR_FAST_RET)
            {
                /* Increase the timer of rpeated acknowledgments.*/  
                cb->tcpcb_fastretrcounter++;

                /* Recalculate the congestion window and slow start threshold values.*/
                if(cb->tcpcb_cwnd > cb->tcpcb_sndwnd)
                    cb->tcpcb_ssthresh = cb->tcpcb_sndwnd >> 1;
                else
                    cb->tcpcb_ssthresh = cb->tcpcb_cwnd >> 1;

                if(cb->tcpcb_ssthresh < (cb->tcpcb_sndmss << 1))
                    cb->tcpcb_ssthresh = (unsigned long)(cb->tcpcb_sndmss << 1);

                cb->tcpcb_cwnd = cb->tcpcb_ssthresh;

                /* Retransmit the segment.*/
                seq = cb->tcpcb_sndseq;
                cb->tcpcb_sndseq = cb->tcpcb_rcvack;
                fnet_tcp_senddataseg(sk, 0, 0, cb->tcpcb_sndmss);
                cb->tcpcb_sndseq = seq;

                /* Acknowledgment is sent in retransmited segment.*/
                *ackparam |= FNET_TCP_AP_NO_SENDING;

                /* Round trip time can't be measured in this case.*/
                cb->tcpcb_timers.round_trip = FNET_TCP_TIMER_OFF;
                cb->tcpcb_timing_state = TCP_TS_SEGMENT_LOST;
            }
        }
    }
    else
    {
        /* Reset the counter of repeated acknowledgments.*/
        cb->tcpcb_fastretrcounter = 0;

        /* Recalculate the congestion window and slow start threshold values.*/
        size = (long)fnet_tcp_getsize(cb->tcpcb_rcvack, tcp_ack);

        if(size > sk->send_buffer.count)
            size = (long)sk->send_buffer.count;

        if(cb->tcpcb_cwnd < FNET_TCP_MAX_BUFFER)
        {
            if(cb->tcpcb_cwnd > cb->tcpcb_ssthresh)
            {
                /* Congestion avoidance mode.*/
                cb->tcpcb_pcount += size;
            }
            else
            {
                /* Slow start mode.*/
                if(cb->tcpcb_cwnd + size > cb->tcpcb_ssthresh)
                {
                    cb->tcpcb_pcount = cb->tcpcb_pcount + cb->tcpcb_cwnd + size - cb->tcpcb_ssthresh;
                    cb->tcpcb_cwnd = cb->tcpcb_ssthresh;
                }
                else
                {
                    cb->tcpcb_cwnd += size;
                }
            }

            if(cb->tcpcb_pcount >= cb->tcpcb_cwnd)
            {
                cb->tcpcb_pcount -= cb->tcpcb_cwnd;
                cb->tcpcb_cwnd += cb->tcpcb_sndmss;
            }
        }

        /* Delete the acknowledged data.*/
        fnet_netbuf_trim(&sk->send_buffer.net_buf_chain, size);
        sk->send_buffer.count -= size;

        /* Save the acknowledgment number.*/
        cb->tcpcb_rcvack = tcp_ack;

        if(FNET_TCP_COMP_G(cb->tcpcb_rcvack, cb->tcpcb_sndseq))
            cb->tcpcb_sndseq = cb->tcpcb_rcvack;

        /* Calculate the retransmission timeout ( using Jacobson method ).*/
        if(FNET_TCP_COMP_GE(cb->tcpcb_rcvack, cb->tcpcb_timingack) && cb->tcpcb_timing_state == TCP_TS_SEGMENT_SENT)
        {
            if(cb->tcpcb_srtt)
            {
                err = (short)(cb->tcpcb_timers.round_trip - (cb->tcpcb_srtt >> FNET_TCP_RTT_SHIFT));

                if((cb->tcpcb_srtt += err) <= 0)
                    cb->tcpcb_srtt = 1;

                if(err < 0)
                    err = (short)(-err);

                err -= (cb->tcpcb_rttvar >> FNET_TCP_RTTVAR_SHIFT);

                if((cb->tcpcb_rttvar += err) <= 0)
                    cb->tcpcb_rttvar = 1;
            }
            else
            {
                /* Initial calculation of the retransmission variables.*/
                cb->tcpcb_srtt = (unsigned short)((cb->tcpcb_timers.round_trip + 1) << FNET_TCP_RTT_SHIFT);
                cb->tcpcb_rttvar = (unsigned short)((cb->tcpcb_timers.round_trip + 1)
                                                        << (FNET_TCP_RTTVAR_SHIFT - 1));
            }

            cb->tcpcb_timing_state = TCP_TS_ACK_RECEIVED;
            cb->tcpcb_rto = (cb->tcpcb_srtt >> FNET_TCP_RTT_SHIFT) + cb->tcpcb_rttvar;
            cb->tcpcb_timers.round_trip = FNET_TCP_TIMER_OFF;
        }
    }

    /* If the final segment is not received, add the data to the input buffer.*/
    if(!(cb->tcpcb_flags & FNET_TCP_CBF_FIN_RCVD))
    {
        delflag = !fnet_tcp_addinpbuf(sk, insegment, ackparam);

    }

    else if((insegment->total_length - FNET_TCP_LENGTH(insegment)) > 0)
            *ackparam |= FNET_TCP_AP_SEND_IMMEDIATELLY;

    /* Acknowledgment of the final segment must be send immediatelly.*/
    if((*ackparam & FNET_TCP_AP_FIN_ACK)
         ||(FNET_TCP_FLAGS(insegment) & FNET_TCP_SGT_PSH)) //TBD
    {
           fnet_tcp_sendack(sk);
    }

    /* Try to sent the data.*/
    if(fnet_tcp_sendanydata(sk, (int)(cb->tcpcb_flags & FNET_TCP_CBF_INSND)))
      *ackparam |= FNET_TCP_AP_NO_SENDING;

    /* If the window of another side is closed, turn on the persist timer.*/
    if(!cb->tcpcb_sndwnd && sk->send_buffer.count)
    {
        if(cb->tcpcb_timers.persist == FNET_TCP_TIMER_OFF)
            cb->tcpcb_cprto = cb->tcpcb_rto;

        cb->tcpcb_timers.persist = cb->tcpcb_cprto;
    }
    else
    {
        if(!(cb->tcpcb_flags & FNET_TCP_CBF_SEND_TIMEOUT))
            cb->tcpcb_timers.persist = FNET_TCP_TIMER_OFF;
    }

    /* If the sent data is acknowledged, turn of the retransmission timer.*/
    if(cb->tcpcb_rcvack == cb->tcpcb_sndseq)
        cb->tcpcb_timers.retransmission = FNET_TCP_TIMER_OFF;
    else
        cb->tcpcb_timers.retransmission = cb->tcpcb_rto;

    /* If the acknowledgment is sent, return
     * If the acnkowledgment must be sent immediatelly, send it
     * If the acnkowledgment must be sent after delay, turn on the acknowledgment timer.*/
    if((*ackparam & FNET_TCP_AP_NO_SENDING) || (*ackparam & FNET_TCP_AP_FIN_ACK))
        return delflag;

    if(*ackparam & FNET_TCP_AP_SEND_IMMEDIATELLY)
    {
        fnet_tcp_sendack(sk);
        return delflag;
    }

    if(*ackparam & FNET_TCP_AP_SEND_WITH_DELAY)
        cb->tcpcb_timers.delayed_ack = 1; /* Delay 200 ms*/

    return delflag;
}

/***********************************************************************
* NAME: fnet_tcp_addinpbuf
*
* DESCRIPTION: This function adds the data to the input buffer and
*              returns the acknowledgement parameter.
*
* RETURNS: TRUE if the input segment is added to the buffer. Otherwise
*          this function returns FALSE.
*************************************************************************/
static int fnet_tcp_addinpbuf( fnet_socket_t *sk, fnet_netbuf_t *insegment, int *ackparam )
{
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control;
    unsigned long       tcp_length = (unsigned long)FNET_TCP_LENGTH(insegment);
    unsigned long       tcp_flags = (unsigned long)FNET_TCP_FLAGS(insegment); 
    int                 result;                   
    
    
    /* If segment doesn't include the data and the FIN or URG flag,
     * return from the function.*/
    if(!(insegment->total_length - tcp_length)         
            && !(tcp_flags & (FNET_TCP_SGT_URG | FNET_TCP_SGT_FIN))) 
        return 0; /* The data isn't added.*/

    
    /* Process the segment that came in order.*/
    if(fnet_ntohl(FNET_TCP_SEQ(insegment)) == cb->tcpcb_sndack 
        #if !FNET_CFG_TCP_DISCARD_OUT_OF_ORDER 
            && !cb->tcpcb_count
        #endif
        )
    {
    #if FNET_CFG_TCP_URGENT    
        if(tcp_flags & FNET_TCP_SGT_URG)
        {
            /* Process the urgent data.*/
            fnet_tcp_urgprocessing(sk, &insegment, 0, ackparam);
        }
        else
        {
            /* Pull  the receive urgent pointer
             * along with the receive window */
            cb->tcpcb_rcvurgseq = cb->tcpcb_sndack - 1;
        }
    #endif /* FNET_CFG_TCP_URGENT */        

        /* Process the final segment. */
        if(tcp_flags & FNET_TCP_SGT_FIN)
        {
            fnet_tcp_finprocessing(sk, fnet_ntohl(FNET_TCP_ACK(insegment)));
            cb->tcpcb_sndack++;
            *ackparam |= FNET_TCP_AP_FIN_ACK;
        }

        /* Delete the header.*/
        fnet_netbuf_trim(&insegment, (int)tcp_length);


        /* Add the data.*/
        if(insegment)
        {
            cb->tcpcb_sndack += insegment->total_length;
            sk->receive_buffer.net_buf_chain = fnet_netbuf_concat(sk->receive_buffer.net_buf_chain,
                                                                  insegment);
            sk->receive_buffer.count += insegment->total_length;
           
            *ackparam |= FNET_TCP_AP_SEND_WITH_DELAY;
        }
        
        return FNET_TRUE;
    }

    result = 0; /* The data is not added to the buffer.*/

#if !FNET_CFG_TCP_DISCARD_OUT_OF_ORDER 
    {
        fnet_netbuf_t *buf, *prevbuf; 
        unsigned long seq, size;
    
    
        /* Add to the temporary input buffer.*/
        if(cb->tcpcb_count + insegment->total_length <= cb->tcpcb_rcvcountmax
               || fnet_ntohl(FNET_TCP_SEQ(insegment)) == cb->tcpcb_sndack)
        {
            /* Acknowledgement must be sent immediately.*/
            *ackparam |= FNET_TCP_AP_SEND_IMMEDIATELLY;

            /* Data is added to the buffer.*/
            result = 1;

            /* Add the segment to the temporary buffer (with sorting).*/
            if(cb->tcpcb_rcvchain)
            {
                buf = cb->tcpcb_rcvchain;
                prevbuf = 0;

                while(1)
                {
                    if(FNET_TCP_COMP_G(fnet_ntohl(FNET_TCP_SEQ(buf)), fnet_ntohl(FNET_TCP_SEQ(insegment))))
                    {
                        if(prevbuf)
                            prevbuf->next_chain = insegment;
                        else
                            cb->tcpcb_rcvchain = insegment;

                        insegment->next_chain = buf;
                        break;
                    }

                    if(!buf->next_chain)
                    {
                        buf->next_chain = insegment;
                        break;
                    }

                    prevbuf = buf;
                    buf = buf->next_chain;
                }
            }
            else
            {
                cb->tcpcb_rcvchain = insegment;
            }

            cb->tcpcb_count += insegment->total_length;
        }

        /* If the temporary buffer received the lost segment
         * move the data from the temporary buffer to the input buffer of the socket.*/
        if(fnet_ntohl(FNET_TCP_SEQ(insegment)) == cb->tcpcb_sndack) 
        {
            seq = cb->tcpcb_sndack;

            while(cb->tcpcb_rcvchain)
            {
                if(FNET_TCP_COMP_GE(seq, fnet_ntohl(FNET_TCP_SEQ(cb->tcpcb_rcvchain))))
                {
                    if(FNET_TCP_COMP_GE(fnet_ntohl(FNET_TCP_SEQ(cb->tcpcb_rcvchain)) + cb->tcpcb_rcvchain->total_length 
                                            - FNET_TCP_LENGTH(cb->tcpcb_rcvchain),
                                        seq))
                    {

                        /* Receive the size of the repeated segment.*/
                        size = fnet_tcp_getsize(fnet_ntohl(FNET_TCP_SEQ(cb->tcpcb_rcvchain)), seq);

                        /* Receive the new sequnce number.*/
                        seq = fnet_ntohl(FNET_TCP_SEQ(cb->tcpcb_rcvchain)) + cb->tcpcb_rcvchain->total_length 
                                  - FNET_TCP_LENGTH(cb->tcpcb_rcvchain);

                        /* Set the size of the temporary buffer.*/
                        cb->tcpcb_count -= cb->tcpcb_rcvchain->total_length;

                        buf = cb->tcpcb_rcvchain;
                        cb->tcpcb_rcvchain = cb->tcpcb_rcvchain->next_chain;
                        buf->next_chain = 0;

                    #if FNET_CFG_TCP_URGENT
                        if(FNET_TCP_FLAGS(buf) & FNET_TCP_SGT_URG)
                        {
                            /* Process the urgent data.*/
                            fnet_tcp_urgprocessing(sk, &buf, size, ackparam);
                        }
                        else
                                           
                        {
                            /* Pull the receive urgent pointer
                             * along with the receive window */
                            cb->tcpcb_rcvurgseq = cb->tcpcb_sndack - 1;
                        }
                    #endif      


                        /* Process the final segment.*/
                        if(FNET_TCP_FLAGS(buf) & FNET_TCP_SGT_FIN)
                        {
                            fnet_tcp_finprocessing(sk, fnet_ntohl(FNET_TCP_ACK(buf)));
                            *ackparam |= FNET_TCP_AP_FIN_ACK;
                            seq++;
                        }


                        /* Delete the header and repeated part.*/
                        fnet_netbuf_trim(&buf, (int)(FNET_TCP_LENGTH(buf) + size));

                        /* Add the data.*/
                        if(buf)
                        {
                            sk->receive_buffer.count += buf->total_length;
                            sk->receive_buffer.net_buf_chain =
                                fnet_netbuf_concat(sk->receive_buffer.net_buf_chain,
                                                   buf);
                        }

                        /* Set the  new acknowledgment number.*/
                        cb->tcpcb_sndack = seq;
                    }
                    else
                    {
                        /* Delete the repeated segment.*/
                        buf = cb->tcpcb_rcvchain;
                        cb->tcpcb_rcvchain = cb->tcpcb_rcvchain->next_chain;

                        /* Set the new size of the temporary buffer.*/
                        cb->tcpcb_count -= cb->tcpcb_rcvchain->total_length;
                        fnet_netbuf_free_chain(buf);
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
#endif 

    return result;
}

/***********************************************************************
* NAME: fnet_tcp_sendanydata
*
* DESCRIPTION: This function sends the data that is allowed by the windows
*              (The congestion window and the window of another side).
*
* RETURNS: TRUE if some data are sent. Otherwise
*          this function returns FALSE.       
*************************************************************************/
static int fnet_tcp_sendanydata( fnet_socket_t *sk, int oneexec )
{
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control;
    long                sndwnd;     /* Size of the data that can be sent.*/
    unsigned long       cwnd;       /* Congestion window.*/
    unsigned long       sntdata;    /* Size of data that is sent.*/
    long                wnd;        /* Windows */
    long                datasize;   /* Size of the data that can be sent from the output buffer.*/
    int                 result = 0;
    
    /* Reset the silly window avoidance flag.*/
    cb->tcpcb_flags &= ~FNET_TCP_CBF_SEND_TIMEOUT;

    /* Receive the size of the data that is sent.*/
    sntdata = fnet_tcp_getsize(cb->tcpcb_rcvack, cb->tcpcb_sndseq);

    /* Receive the size of the data that is allowed by the window.*/
    wnd = (long)(cb->tcpcb_sndwnd - sntdata);

    if(wnd < 0)
    {
        /* The window is shrinked.*/
        cb->tcpcb_timing_state = TCP_TS_SEGMENT_LOST;
        cb->tcpcb_sndseq += wnd;
        sntdata += wnd;
        wnd = 0;
    }

    /* The size of the data in the output buffer that can be sent.*/
    datasize = (long)(sk->send_buffer.count - sntdata);

    /* Congestion window.*/
    cwnd = cb->tcpcb_cwnd - sntdata;
    cwnd = ((unsigned long)(cwnd / cb->tcpcb_sndmss)) * cb->tcpcb_sndmss;

    /* Calculate sndwnd (size of the data that will be sent).*/
    if(wnd > cwnd)
        sndwnd = (long)cwnd;
    else
        sndwnd = wnd;

    if(sndwnd > datasize)
        sndwnd = datasize;

    /* If the data can't be sent.*/
    if(sndwnd <= 0)
    {
        /* Send the final segment without data.*/
        if(sk->send_buffer.is_shutdown && !datasize && !(cb->tcpcb_flags & FNET_TCP_CBF_FIN_SENT))
        {
            fnet_tcp_sendheadseg(sk, FNET_TCP_SGT_FIN | FNET_TCP_SGT_ACK, 0, 0);

            /* Reinitialize the retransmission timer.*/
            if(cb->tcpcb_timers.retransmission == FNET_TCP_TIMER_OFF)
                cb->tcpcb_timers.retransmission = cb->tcpcb_rto;

            result = 1;
        }

        return result;
    }


    /* The sntdata variable will include the size of the data that is sent.*/
    sntdata = 0;

    while(sndwnd > 0)
    {
        /* Sending of the maximal size segment.*/
        if(cb->tcpcb_sndmss <= sndwnd)
        {
            /* Send the full sized segment.*/
            fnet_tcp_senddataseg(sk, 0, 0, cb->tcpcb_sndmss);

            sndwnd -= cb->tcpcb_sndmss;
            sntdata += cb->tcpcb_sndmss;
        }
        else
        {
            /*The sending of the partial segments is occured in cases:
             * If the data must be sent in any cases
             * If the urgent data is present
             * If the size of the input buffer of another side greater than the half of its maximal size
             * If the Nagle algorithm is blocked and all data can be sent in this segment
             * If all sent data is acknowledged and all data can be sent in this segment.*/
            if(sndwnd > (cb->tcpcb_maxwnd >> 1) 
                || (cb->tcpcb_flags & FNET_TCP_CBF_FORCE_SEND)
            #if FNET_CFG_TCP_URGENT                      
                || FNET_TCP_COMP_GE(cb->tcpcb_sndurgseq, cb->tcpcb_sndseq)
            #endif /* FNET_CFG_TCP_URGENT */
              )
            {
                /* Send the partial segment.*/
                fnet_tcp_senddataseg(sk, 0, 0, (unsigned long)sndwnd);
                sntdata += sndwnd;
            }
            else
            {
                if(cb->tcpcb_rcvack == cb->tcpcb_sndseq || (sk->options.tcp_opt.flags & TCP_NODELAY))
                {
                    if(sntdata + sndwnd == datasize)
                    {
                        /* Send the partial segment.*/
                        fnet_tcp_senddataseg(sk, 0, 0, (unsigned long)sndwnd);
                        sntdata += sndwnd;
                    }
                    else
                    {
                        /* Set the silly window avoidance flag.*/
                        if(cb->tcpcb_timers.persist == FNET_TCP_TIMER_OFF
                               || cb->tcpcb_timers.persist > cb->tcpcb_rto)
                        {
                            cb->tcpcb_cprto = cb->tcpcb_rto;
                            cb->tcpcb_timers.persist = cb->tcpcb_cprto;
                        }

                        cb->tcpcb_flags |= FNET_TCP_CBF_SEND_TIMEOUT;
                    }
                }
            }

            sndwnd = 0;
        }

        /* If the execution of the function must be without delay, return.*/
        if(oneexec)
            break;
    }

    /* If the data is sent.*/
    if(sntdata > 0)
    {

        /* Process the states of round trip time measurement.*/
        if(cb->tcpcb_timing_state == TCP_TS_ACK_RECEIVED || (cb->tcpcb_timing_state == TCP_TS_SEGMENT_LOST
               && FNET_TCP_COMP_G(cb->tcpcb_sndseq, cb->tcpcb_timingack)))
        {
            cb->tcpcb_timingack = cb->tcpcb_sndseq;
            cb->tcpcb_timers.round_trip = FNET_TCP_TIMER_ON_INCREMENT;

            cb->tcpcb_timing_state = TCP_TS_SEGMENT_SENT;
        }

        result = 1;
        cb->tcpcb_timers.persist = FNET_TCP_TIMER_OFF;
    }

    /* Reinitialize the retransmission timer.*/
    if(sntdata > 0 && cb->tcpcb_timers.retransmission == FNET_TCP_TIMER_OFF)
        cb->tcpcb_timers.retransmission = cb->tcpcb_rto;

    return result;
}

#if FNET_CFG_TCP_URGENT
/***********************************************************************
* NAME: fnet_tcp_urgprocessing
*
* DESCRIPTION: This function processes the urgent data.
*
* RETURNS: None.              
*************************************************************************/
static void fnet_tcp_urgprocessing( fnet_socket_t *sk, fnet_netbuf_t ** segment, unsigned long repdatasize,
                                    int *ackparam )
{
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control;
    unsigned short      upointer; 
    unsigned short      uoffset;  
    fnet_netbuf_t       *buf;
    unsigned long       tcp_seq;
    

    buf = (fnet_netbuf_t *) *segment;
    

    /* Receive the urgent pointer.*/
    upointer = fnet_ntohs(FNET_TCP_URG(buf));

    /* Different interpretation of the urgent pointer in the TCP header.*/
    if(sk->options.tcp_opt.flags & TCP_BSD)
    {
        /* BSD interpretation.*/
        if(!upointer)
            return;

        upointer--;
    }

    /*  Only  the new data must be processed.*/
    if(upointer < repdatasize || (repdatasize && (buf->total_length - FNET_TCP_LENGTH(buf) <= repdatasize)))
        return;
    
    tcp_seq = fnet_ntohl(FNET_TCP_SEQ(buf));
    
    /* Receive the new urgent pointer.*/
    if(FNET_TCP_COMP_G(tcp_seq + upointer, cb->tcpcb_rcvurgseq)) 
        cb->tcpcb_rcvurgseq = tcp_seq + upointer;
    else
        upointer = (unsigned short)(cb->tcpcb_rcvurgseq - tcp_seq);

    /* If the urgent byte is not in this segment.*/
    if(upointer >= buf->total_length - FNET_TCP_LENGTH(buf))
        return;

    /* Set the number of bytes before the urgent character.*/
    cb->tcpcb_rcvurgmark = (long)(sk->receive_buffer.count + upointer);

    /* If the urgent data must be in the stream, return from this function. */
    if(sk->options.flags & SO_OOBINLINE)
        return;

    /* Find and read the urgent byte.*/
    uoffset = (unsigned short)(upointer + FNET_TCP_LENGTH(buf));

    while(uoffset >= buf->length)
    {
        uoffset -= buf->length;
        buf = buf->next;
    }

    cb->tcpcb_flags |= FNET_TCP_CBF_RCVURGENT;
    cb->tcpcb_iobc = (char)FNET_TCP_GETUCHAR(buf->data_ptr, uoffset);
    cb->tcpcb_sndack++;

    /* Delete the urgent byte from the stream.*/
    fnet_netbuf_cut_center(segment, upointer + FNET_TCP_LENGTH((fnet_netbuf_t *)(*segment)), FNET_TRUE );

    /* Acknowledgment must be sent immediately.*/
    *ackparam |= FNET_TCP_AP_SEND_IMMEDIATELLY;

}
#endif /* FNET_CFG_TCP_URGENT */

/***********************************************************************
* NAME: fnet_tcp_finprocessing
*
* DESCRIPTION: This function processes the final segment.
*     
* RETURNS: None.         
*************************************************************************/
static void fnet_tcp_finprocessing( fnet_socket_t *sk, unsigned long ack )
{

    /* Initialize the pointer to the control block.*/
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control;
    
    /* Process the final segment depend on state.*/
    switch(cb->tcpcb_connection_state)
    {
        case FNET_TCP_CS_ESTABLISHED:
            cb->tcpcb_connection_state = FNET_TCP_CS_CLOSE_WAIT;
        /*RStevens: The connection can remain in this state forever. 
         * The other end is still in the CLOSE_WAIT state, and can remain 
         * there forever, until the application decides to issue its close.*/ 
            break;

        case FNET_TCP_CS_FIN_WAIT_1:
            if(ack != cb->tcpcb_sndseq)
            {
                /* Simultaneous close.*/
                cb->tcpcb_connection_state = FNET_TCP_CS_CLOSING;
                break;
            }

        case FNET_TCP_CS_FIN_WAIT_2:
            cb->tcpcb_connection_state = FNET_TCP_CS_TIME_WAIT;
            /* Set timewait timeout.*/
            if(cb->tcpcb_timers.connection <= 0) /* If it was already set before by other state. */          
                cb->tcpcb_timers.connection = FNET_TCP_TIME_WAIT;
          
            cb->tcpcb_timers.keepalive = FNET_TCP_TIMER_OFF;
            break;

        default:
            return;
    }

    cb->tcpcb_flags |= FNET_TCP_CBF_FIN_RCVD;

}

/***********************************************************************
* NAME: tcp_rcvwnd
*
* DESCRIPTION: This function receives a new window size. 
*   
* RETURNS: The new window.              
*************************************************************************/
static unsigned long fnet_tcp_getrcvwnd( fnet_socket_t *sk )
{
    long wnd; 
    unsigned long rcvwnd;
    
    /* Initialize the pointer to the control block.*/
    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control;

    rcvwnd = cb->tcpcb_rcvwnd;

    /* Set the receive window size.*/
    wnd = (long)(cb->tcpcb_rcvcountmax - sk->receive_buffer.count);

    if(wnd < 0)
        wnd = 0;

    if(rcvwnd < wnd)
    {
        /* Window can be opened only on condition that*/
        if(wnd - rcvwnd >= cb->tcpcb_rcvmss || wnd - rcvwnd >= (cb->tcpcb_rcvcountmax >> 1) || !rcvwnd)
            rcvwnd = (unsigned long)wnd;
    }
    else
    {
        rcvwnd = (unsigned long)wnd;
    }

    return rcvwnd;
}

/************************************************************************
* NAME: fnet_tcp_slowtimo
*
* DESCRIPTION: This function processes the timeouts. 
*              (fnet_tcp_slowtimo is performed every 500 ms).
*
* RETURNS: None. 
*************************************************************************/
static void fnet_tcp_slowtimo(void *cookie)
{
    fnet_socket_t *sk;               
    fnet_socket_t *addedsk; 
    fnet_socket_t *nextsk;

    FNET_COMP_UNUSED_ARG(cookie);      

    fnet_isr_lock();
    sk = fnet_tcp_prot_if.head;

    while(sk)
    {
        addedsk = sk->partial_con;

        while(addedsk)
        {
            nextsk = addedsk->next;
            /* Process the partial sockets.*/
            fnet_tcp_slowtimosk(addedsk);
            addedsk = nextsk;
        }

        addedsk = sk->incoming_con;

        while(addedsk)
        {
            nextsk = addedsk->next;
            /* Process the incoming sockets.*/
            fnet_tcp_slowtimosk(addedsk);
            addedsk = nextsk;
        }

        /* Processg the main socket.*/
        nextsk = sk->next;
        fnet_tcp_slowtimosk(sk);
        sk = nextsk;
    }

    fnet_tcp_isntime += FNET_TCP_STEPISN;

    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_tcp_fasttimo
*
* DESCRIPTION: This function processes the timeouts 
*              (fnet_tcp_fasttimo is performed every 200 ms).
*
* RETURNS: None. 
*************************************************************************/
static void fnet_tcp_fasttimo( void *cookie )
{
    fnet_socket_t *sk; 
    fnet_socket_t *addedsk;
    
    FNET_COMP_UNUSED_ARG(cookie);
    
    fnet_isr_lock();
    sk = fnet_tcp_prot_if.head;

    while(sk)
    {
        fnet_tcp_fasttimosk(sk);
        addedsk = sk->incoming_con;

        while(addedsk)
        {
            fnet_tcp_fasttimosk(addedsk);
            addedsk = addedsk->next;
        }

        sk = sk->next;
    }

    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_tcp_slowtimosk
*
* DESCRIPTION: This function processes the timers of the socket 
*              (fnet_tcp_slowtimosk is performed every 500 ms).
*
* RETURNS: None. 
*************************************************************************/
static void fnet_tcp_slowtimosk( fnet_socket_t *sk )
{

    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control;

    
    /* If the socket is not connected, return.*/
    if(sk->state != SS_UNCONNECTED)
    {
        /* Check the abort timer.*/
        if(cb->tcpcb_timers.abort != FNET_TCP_TIMER_OFF)
        {
            cb->tcpcb_timers.abort--;

            if(!cb->tcpcb_timers.abort)
            {
                cb->tcpcb_timers.abort = FNET_TCP_TIMER_OFF;

                if(sk->options.local_error != FNET_ERR_HOSTUNREACH)
                    sk->options.local_error = FNET_ERR_CONNABORTED;

                fnet_tcp_closesk(sk);
                return;
            }
        }

        /* Check the connection timer.*/
        if(cb->tcpcb_timers.connection != FNET_TCP_TIMER_OFF)
        {
            cb->tcpcb_timers.connection--;

            if(!cb->tcpcb_timers.connection)
            {
                cb->tcpcb_timers.connection = FNET_TCP_TIMER_OFF;

                if(cb->tcpcb_flags & FNET_TCP_CBF_CLOSE)
                {
                    if(cb->tcpcb_connection_state == FNET_TCP_CS_TIME_WAIT)
                        fnet_tcp_closesk(sk);
                    else
                        fnet_tcp_abortsk(sk);
                }
                else
                {
                    if(sk->options.local_error != FNET_ERR_HOSTUNREACH
                           && sk->options.local_error != FNET_ERR_NOPROTOOPT)
                        sk->options.local_error = FNET_ERR_TIMEDOUT;

                    fnet_tcp_closesk(sk);
                }
                return;
            }
        }

        /* Check the retransmission timer.*/
        if(cb->tcpcb_timers.retransmission != FNET_TCP_TIMER_OFF)
        {
            cb->tcpcb_timers.retransmission--;

            if(!cb->tcpcb_timers.retransmission)
            {
                cb->tcpcb_timers.retransmission = FNET_TCP_TIMER_OFF;
                fnet_tcp_rtimeo(sk);
            }
        }

        /* Check the keepalive timer.*/
        if(cb->tcpcb_timers.keepalive != FNET_TCP_TIMER_OFF)
        {
            cb->tcpcb_timers.keepalive--;

            if(!cb->tcpcb_timers.keepalive)
            {
                cb->tcpcb_timers.keepalive = FNET_TCP_TIMER_OFF;
                fnet_tcp_ktimeo(sk);
            }
        }

        /* Check the persist timer.*/
        if(cb->tcpcb_timers.persist != FNET_TCP_TIMER_OFF)
        {
            cb->tcpcb_timers.persist--;

            if(!cb->tcpcb_timers.persist)
            {
                cb->tcpcb_timers.persist = FNET_TCP_TIMER_OFF;
                fnet_tcp_ptimeo(sk);
            }
        }

        /* If the round trip timer is turned on, recalculate it.*/
        if(cb->tcpcb_timers.round_trip != FNET_TCP_TIMER_OFF)
            cb->tcpcb_timers.round_trip++;
    }
    
}

/************************************************************************
* NAME: fnet_tcp_fasttimosk
*
* DESCRIPTION: This function processes the delayed acknowledgment timer 
*              of the socket (fnet_tcp_fasttimo is performed every 200 ms).
*
* RETURNS: None. 
*************************************************************************/
static void fnet_tcp_fasttimosk( fnet_socket_t *sk )
{
    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control;
    
   
    if(sk->state != SS_UNCONNECTED)
    {
        /* Check the delayed acknowledgment timer.*/
        if(cb->tcpcb_timers.delayed_ack != FNET_TCP_TIMER_OFF)
        {
            cb->tcpcb_timers.delayed_ack--;

            if(!cb->tcpcb_timers.delayed_ack)
            {
                cb->tcpcb_timers.delayed_ack = FNET_TCP_TIMER_OFF;
                fnet_tcp_sendack(sk);
                return;
            }
        }
    }
    
}

/************************************************************************
* NAME: fnet_tcp_rtimeo
*
* DESCRIPTION: This function processes the timeout of the retransmission 
*              timer.
*
* RETURNS: None.                
*************************************************************************/
static void fnet_tcp_rtimeo( fnet_socket_t *sk )
{
    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control;
    char                options[FNET_TCP_MAX_OPT_SIZE]; 
    char                optionlen;                      
    
    switch(cb->tcpcb_connection_state)
    {
        case FNET_TCP_CS_SYN_RCVD:

          /* Reinitialize of the sequnce number.*/
          cb->tcpcb_sndseq--;
          fnet_tcp_setsynopt(sk, options, &optionlen);

          /* Create and send the SYN segment.*/
          fnet_tcp_sendheadseg(sk, FNET_TCP_SGT_SYN | FNET_TCP_SGT_ACK, options, optionlen);

          break;

        case FNET_TCP_CS_SYN_SENT:
          /* Retransmission of the first segment.*/

          /* Recalculate the sequence number.*/
          cb->tcpcb_sndseq--;

          fnet_tcp_setsynopt(sk, options, &optionlen);

          /* Send the SYN segment.*/
          fnet_tcp_sendheadseg(sk, FNET_TCP_SGT_SYN, options, optionlen);

          break;

        default:

          /* If FIN segment is sent, it must be retransmited.*/
          cb->tcpcb_flags &= ~FNET_TCP_CBF_FIN_SENT;

          /* Initialize of the abort timer.*/
          if(cb->tcpcb_timers.abort == FNET_TCP_TIMER_OFF)
              cb->tcpcb_timers.abort = FNET_TCP_ABORT_INTERVAL;

          /* Recalculate the sequence number.*/
          cb->tcpcb_sndseq = cb->tcpcb_rcvack;

          /* Recalculate the congestion window and slow start threshold values (for case of  retransmission).*/
          if(cb->tcpcb_cwnd > cb->tcpcb_sndwnd)
              cb->tcpcb_ssthresh = cb->tcpcb_sndwnd >> 1;
          else
              cb->tcpcb_ssthresh = cb->tcpcb_cwnd >> 1;

          if(cb->tcpcb_ssthresh < (cb->tcpcb_sndmss << 1))
              cb->tcpcb_ssthresh = (unsigned long)(cb->tcpcb_sndmss << 1);

          cb->tcpcb_cwnd = cb->tcpcb_sndmss;

          /* Round trip time can't be measured in this case.*/
          cb->tcpcb_timers.round_trip = FNET_TCP_TIMER_OFF;
          cb->tcpcb_timing_state = TCP_TS_SEGMENT_LOST;

          cb->tcpcb_flags |= FNET_TCP_CBF_FORCE_SEND;
          fnet_tcp_sendanydata(sk, 0);
          cb->tcpcb_flags &= ~FNET_TCP_CBF_FORCE_SEND;
    }

    /* If the first timeout is occured, initialize the retransmission timer.*/
    if(cb->tcpcb_retrseq != cb->tcpcb_rcvack)
    {
        cb->tcpcb_crto = cb->tcpcb_rto;
        cb->tcpcb_retrseq = cb->tcpcb_rcvack;
    }

    /* Recalculate the retransission timer.*/
    if(cb->tcpcb_crto != FNET_TCP_TIMERS_LIMIT)
    {
        if((cb->tcpcb_crto << 1) > FNET_TCP_TIMERS_LIMIT)
            /* Timeout must be less or equal than TIMER_LIMIT.*/
            cb->tcpcb_crto = FNET_TCP_TIMERS_LIMIT;
        else
            /* Increase the timeout (*2).*/
            cb->tcpcb_crto <<= 1;
    }

    cb->tcpcb_timers.retransmission = cb->tcpcb_crto;
    
}

/************************************************************************
* NAME: fnet_tcp_ktimeo
*
* DESCRIPTION: This function processes the timeout of the keepalive 
*              timer.
*
* RETURNS: None.                
*************************************************************************/
static void fnet_tcp_ktimeo( fnet_socket_t *sk )
{
    fnet_netbuf_t           *data;   
    unsigned short          rcvwnd; 
    fnet_tcp_control_t      *cb = (fnet_tcp_control_t *)sk->protocol_control;
    struct fnet_tcp_segment segment;
    
    /* Create the keepalive segment.*/
    data = fnet_netbuf_new(1, FNET_FALSE);

    /* Check the memory allocation.*/
    if(!data)
        return;

    /* Receive the window size.*/
    rcvwnd = (unsigned short)(fnet_tcp_getrcvwnd(sk) >> cb->tcpcb_recvscale);

    /* Send the segment.*/
    segment.sockoption = &sk->options; 
    segment.src_addr = sk->local_addr;
    segment.dest_addr = sk->foreign_addr;
    segment.seq = cb->tcpcb_rcvack - 1;
    segment.ack = cb->tcpcb_sndack;
    segment.flags = FNET_TCP_SGT_ACK;
    segment.wnd = rcvwnd;
    segment.urgpointer = 0;
    segment.options = 0;
    segment.optlen = 0;
    segment.data = data;
    
    fnet_tcp_sendseg(&segment);    //TBD res check       

    /* Set the timers.*/
    cb->tcpcb_timers.keepalive = sk->options.tcp_opt.keep_intvl;

    if((cb->tcpcb_timers.abort == FNET_TCP_TIMER_OFF) || (cb->tcpcb_timers.abort > (sk->options.tcp_opt.keep_cnt * sk->options.tcp_opt.keep_intvl) ))
    {
        cb->tcpcb_timers.abort = sk->options.tcp_opt.keep_cnt * sk->options.tcp_opt.keep_intvl;
    }
}

/************************************************************************
* NAME: fnet_tcp_ptimeo
*
* DESCRIPTION: This function processes the timeout of the persist 
*              timer.
*
* RETURNS: None.                
*************************************************************************/
static void fnet_tcp_ptimeo( fnet_socket_t *sk )
{
    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control;

    
    /* If the window is closed, send the probe segment
     * Otherwise, try to send any data.*/
    if(!cb->tcpcb_sndwnd)
    {
        fnet_tcp_senddataseg(sk, 0, 0, 1);
        cb->tcpcb_sndseq--;
    }
    else
    {
        cb->tcpcb_flags |= FNET_TCP_CBF_FORCE_SEND;
        fnet_tcp_sendanydata(sk, 0);
        cb->tcpcb_flags &= ~FNET_TCP_CBF_FORCE_SEND;
        return;
    }

    /* Reinitialize the persist timer.*/
    if(cb->tcpcb_cprto != FNET_TCP_TIMERS_LIMIT)
    {
        if((cb->tcpcb_cprto << 1) > FNET_TCP_TIMERS_LIMIT)
            /* Timeout must be less or equal than TIMER_LIMIT.*/
            cb->tcpcb_cprto = FNET_TCP_TIMERS_LIMIT;
        else
            /* Increase the timeout (*2).*/
            cb->tcpcb_cprto <<= 1;
    }

    cb->tcpcb_timers.persist = cb->tcpcb_cprto;

    /* Initialize the abort timer.*/
    if(cb->tcpcb_timers.abort == FNET_TCP_TIMER_OFF)
        cb->tcpcb_timers.abort = FNET_TCP_ABORT_INTERVAL;

    
}

/************************************************************************
* NAME: fnet_tcp_sendseg
*
* DESCRIPTION: This function sends the segment.
*
* RETURNS: FNET_OK if the segment is sent. Otherwise
*          this function returns the error code.                 
*************************************************************************/
static int fnet_tcp_sendseg(struct fnet_tcp_segment *segment)
{
    fnet_netbuf_t                           *nb;
    int                                     error = FNET_OK;
    fnet_netif_t                            *netif;
    FNET_COMP_PACKED_VAR unsigned short     *checksum_p;

#if FNET_CFG_IP6    
    if(segment->dest_addr.sa_family == AF_INET6)
    {
        /* Check Scope ID.*/
        netif = fnet_netif_get_by_scope_id( ((struct sockaddr_in6 *)(&segment->dest_addr))->sin6_scope_id );
    }
    else
#endif
        netif = FNET_NULL;  

    /* Create the header.*/
    nb = fnet_netbuf_new(FNET_TCP_SIZE_HEADER, FNET_FALSE);

    if(!nb)
    {
        if(segment->data)
            fnet_netbuf_free_chain(segment->data);

        return FNET_ERR_NOMEM;
    }

    fnet_memset_zero(nb->data_ptr, FNET_TCP_SIZE_HEADER);

    /* Add TCP options.*/
    if(segment->options && segment->optlen)
    {
        error = (unsigned char)fnet_tcp_addopt(nb, (unsigned char)segment->optlen, segment->options);

        if(error)
        {
            fnet_netbuf_free_chain(nb);

            if(segment->data)
                fnet_netbuf_free_chain(segment->data);

            return error;
        }
    }
    else
    {
        FNET_TCP_SET_LENGTH(nb) = FNET_TCP_SIZE_HEADER << 2;  /* (FNET_TCP_SIZE_HEADER/4 + opt_len/4) */
    }

    /* Initialization of the header.*/
    FNET_TCP_SPORT(nb) = segment->src_addr.sa_port;
    FNET_TCP_DPORT(nb) = segment->dest_addr.sa_port;
    FNET_TCP_SEQ(nb) = fnet_htonl(segment->seq);
    FNET_TCP_ACK(nb) = fnet_htonl(segment->ack);
    FNET_TCP_SET_FLAGS(nb) = segment->flags;
    FNET_TCP_WND(nb) = fnet_htons(segment->wnd);

    /* Add the data.*/
    nb = fnet_netbuf_concat(nb, segment->data);

    /* Set the pointer to the urgent data.*/
    FNET_TCP_URG(nb) = fnet_htons(segment->urgpointer);

    /* Checksum calculation.*/
    FNET_TCP_CHECKSUM(nb) = 0; 

#if FNET_CFG_UDP_CHECKSUM

#if FNET_CFG_CPU_ETH_HW_TX_IP_CHECKSUM
    if( 0 
#if FNET_CFG_IP4
        ||( (segment->dest_addr.sa_family == AF_INET) 
        && ((netif = fnet_ip_route(((struct sockaddr_in *)(&segment->dest_addr))->sin_addr.s_addr))!= FNET_NULL)
        && (netif->features & FNET_NETIF_FEATURE_HW_TX_PROTOCOL_CHECKSUM)
        && (fnet_ip_will_fragment(netif, nb->total_length) == FNET_FALSE) /* Fragmented packets are not inspected.*/  ) 
#endif
#if FNET_CFG_IP6
        ||( (segment->dest_addr.sa_family == AF_INET6) 
        && (netif || ((netif = fnet_ip6_route(&((struct sockaddr_in6 *)(&segment->src_addr))->sin6_addr.s6_addr, &((struct sockaddr_in6 *)(&segment->dest_addr))->sin6_addr.s6_addr))!= FNET_NULL ) )
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
        FNET_TCP_CHECKSUM(nb) = fnet_checksum_pseudo_start(nb, FNET_HTONS((unsigned short)FNET_IP_PROTOCOL_TCP), (unsigned short)nb->total_length);
        checksum_p = &FNET_TCP_CHECKSUM(nb);
    }
#endif /* FNET_CFG_UDP_CHECKSUM */
                                   


#if FNET_CFG_IP4
    if(segment->dest_addr.sa_family == AF_INET)
    {
        error = fnet_ip_output(netif, ((struct sockaddr_in *)(&segment->src_addr))->sin_addr.s_addr, 
                                ((struct sockaddr_in *)(&segment->dest_addr))->sin_addr.s_addr, 
                                FNET_IP_PROTOCOL_TCP, segment->sockoption->ip_opt.tos,
                                (unsigned char)(segment->sockoption ? segment->sockoption->ip_opt.ttl : FNET_TCP_TTL_DEFAULT),
                                nb, 0, 
                                segment->sockoption ? ((segment->sockoption->flags & SO_DONTROUTE) > 0) : 0,
                                checksum_p);
    }
    else
#endif    
#if FNET_CFG_IP6    
    if(segment->dest_addr.sa_family == AF_INET6)
    {
        error = fnet_ip6_output( netif, 
                                &((struct sockaddr_in6 *)(&segment->src_addr))->sin6_addr.s6_addr, 
                                &((struct sockaddr_in6 *)(&segment->dest_addr))->sin6_addr.s6_addr, 
                                FNET_IP_PROTOCOL_TCP, 
                                (unsigned char)(segment->sockoption ? segment->sockoption->ip6_opt.unicast_hops : 0 /*default*/),
                                nb, 
                                checksum_p );
    }
    else
#endif    
    {};
                      
    return error;
}


/************************************************************************
* NAME: fnet_tcp_sendheadseg
*
* DESCRIPTION: This function sends the segment wihout data.
*
* RETURNS: TRUE if no error occurs. Otherwise
*          this function returns the error code.                 
*************************************************************************/
static int fnet_tcp_sendheadseg( fnet_socket_t *sk, unsigned char flags, void *options, char optlen )
{
    unsigned long   seq;             
    unsigned short  wnd;           
    int             error = FNET_OK;           
    unsigned long   ack = 0;         
    unsigned short  urgpointer = 0;
    struct fnet_tcp_segment segment;

    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control;

    
    /* Get the sequence number.*/
    seq = cb->tcpcb_sndseq;

    /* Get the window.*/
    cb->tcpcb_rcvwnd = fnet_tcp_getrcvwnd(sk);

    /* Process the SYN flag.*/
    if(flags & FNET_TCP_SGT_SYN)
    {
        seq++;

        if(cb->tcpcb_rcvwnd > FNET_TCP_MAXWIN)
            wnd = FNET_TCP_MAXWIN;
        else
            wnd = (unsigned short)cb->tcpcb_rcvwnd;
    }
    else
    {
        wnd = (unsigned short)(cb->tcpcb_rcvwnd >> cb->tcpcb_recvscale);
    }

    /* Process the FIN flag.*/
    if(flags & FNET_TCP_SGT_FIN)
    {
        seq++;

        /* Change the state after sending of the final segment*/
        if(cb->tcpcb_connection_state == FNET_TCP_CS_ESTABLISHED)
            cb->tcpcb_connection_state = FNET_TCP_CS_FIN_WAIT_1;
        else if(cb->tcpcb_connection_state == FNET_TCP_CS_CLOSE_WAIT)
            cb->tcpcb_connection_state = FNET_TCP_CS_LAST_ACK;

        cb->tcpcb_flags |= FNET_TCP_CBF_FIN_SENT;
    }

#if FNET_CFG_TCP_URGENT
    /* Process the URG flag.*/
    if((flags & FNET_TCP_SGT_URG)
           && fnet_tcp_getsize(cb->tcpcb_sndseq, cb->tcpcb_sndurgseq) < FNET_TCP_MAXWIN)
    {
        urgpointer = (unsigned short)fnet_tcp_getsize(cb->tcpcb_sndseq, cb->tcpcb_sndurgseq);

        if(sk->options.tcp_opt.flags & TCP_BSD)
            urgpointer++;
    }
#endif /* FNET_CFG_TCP_URGENT */

    /* Process the ACK flag.*/
    if(flags & FNET_TCP_SGT_ACK)
        ack = cb->tcpcb_sndack;

    /* Send the segment.*/ 
    segment.sockoption = &sk->options; 
    segment.src_addr = sk->local_addr;
    segment.dest_addr = sk->foreign_addr;
    segment.seq = cb->tcpcb_sndseq;
    segment.ack = ack;
    segment.flags = flags;
    segment.wnd = wnd;
    segment.urgpointer = urgpointer;
    segment.options = options;
    segment.optlen = optlen;
    segment.data = 0;
    
    error = fnet_tcp_sendseg(&segment);
    

    /* Turn off the delayed acknowledgment timer.*/
    cb->tcpcb_timers.delayed_ack = FNET_TCP_TIMER_OFF;

    cb->tcpcb_newfreercvsize = 0;

    /* Set the new sequence number.*/
    cb->tcpcb_sndseq = seq;

    if(FNET_TCP_COMP_G(cb->tcpcb_sndseq, cb->tcpcb_maxrcvack))
        cb->tcpcb_maxrcvack = cb->tcpcb_sndseq;

    return error;
}

/************************************************************************
* NAME: fnet_tcp_senddataseg
*
* DESCRIPTION: This function sends the data segment.
*
* RETURNS: TRUE if no error occurs. Otherwise
*          this function returns the error code.                 
*************************************************************************/
static int fnet_tcp_senddataseg( fnet_socket_t *sk, void *options, char optlen, unsigned long datasize )
{
    unsigned char           flags;          
    long                    newdatasize;    /* Datasize that can be sent.*/
    unsigned long           urgpointer = 0; /* Pointer of the urgent data.*/
    fnet_netbuf_t           *data = 0;      /* Data pointer.*/
    unsigned long           seq;            /* Sequence number.*/
    int                     error;                   
    unsigned long           tmp;
    struct fnet_tcp_segment segment;
    fnet_tcp_control_t      *cb = (fnet_tcp_control_t *)sk->protocol_control;
    
    /* Receive the sequence number.*/
    seq = cb->tcpcb_sndseq;

    /* Reset the flags.*/
    flags = 0;

    /* Data that can be sent.*/
    newdatasize = (long)(sk->send_buffer.count - fnet_tcp_getsize(cb->tcpcb_rcvack, cb->tcpcb_sndseq));

    /* Check the data size.*/
    if(datasize > newdatasize)
        datasize = (unsigned long)newdatasize;

#if FNET_CFG_IP4
    tmp = fnet_ip_maximum_packet(((struct sockaddr_in *)(&sk->foreign_addr))->sin_addr.s_addr);
#else //TBD
    tmp = 0;
#endif    

    if((datasize + FNET_TCP_SIZE_HEADER) > tmp)
        datasize = (tmp - FNET_TCP_SIZE_HEADER);

    /* Create the flags.*/
    flags |= FNET_TCP_SGT_ACK;

    /* If the data of the segment is the last in the output buffer,*/
    /* Set PSH flag.*/
    if(newdatasize == datasize && datasize)
        flags |= FNET_TCP_SGT_PSH;

#if FNET_CFG_TCP_URGENT
    /* Process the urgent request.*/
    if(FNET_TCP_COMP_GE(cb->tcpcb_sndurgseq, cb->tcpcb_sndseq))
    {
        if(fnet_tcp_getsize(cb->tcpcb_sndseq, cb->tcpcb_sndurgseq) < FNET_TCP_MAX_URG_POINTER)
        {
            /* Set the urgent pointer.*/
            urgpointer = fnet_tcp_getsize(cb->tcpcb_sndseq, cb->tcpcb_sndurgseq);
            flags |= FNET_TCP_SGT_URG;

            /* If the BSD interpretation, increase the urgent pointer.*/
            if(sk->options.tcp_opt.flags & TCP_BSD)
                urgpointer++;
        }
    }
    else
    {
        /* Pull the send urgent pointer along with the send window.*/
        if(FNET_TCP_COMP_G(cb->tcpcb_rcvack - 1, cb->tcpcb_sndurgseq))
            cb->tcpcb_sndurgseq = cb->tcpcb_rcvack - 1;
    }
#endif /* FNET_CFG_TCP_URGENT */

    /* If final segment must be sent.*/
    if(sk->send_buffer.is_shutdown)
    {
        if(newdatasize == datasize)
        {
            /* End of the data sending.*/
            flags |= FNET_TCP_SGT_FIN;
            seq++;

            /* Change the state after the sending of the final segment.*/
            if(cb->tcpcb_connection_state == FNET_TCP_CS_ESTABLISHED)
                cb->tcpcb_connection_state = FNET_TCP_CS_FIN_WAIT_1;

            else if(cb->tcpcb_connection_state == FNET_TCP_CS_CLOSE_WAIT)
                cb->tcpcb_connection_state = FNET_TCP_CS_LAST_ACK;

            cb->tcpcb_flags |= FNET_TCP_CBF_FIN_SENT;
        }
    }

    /* Receive the window size.*/
    cb->tcpcb_rcvwnd = fnet_tcp_getrcvwnd(sk);

    /* If the data is present, add it.*/
    if(datasize)
    {
        data = fnet_netbuf_copy(sk->send_buffer.net_buf_chain, (int)(sk->send_buffer.count - newdatasize),
                                (int)datasize,                 0);

        /* Check the memory allocation.*/
        if(!data)
        {
            cb->tcpcb_sndseq = seq + datasize;
            return FNET_ERR_NOMEM;
        }
    }

    /* Send the segment.*/
    segment.sockoption = &sk->options; 
    segment.src_addr = sk->local_addr;
    segment.dest_addr = sk->foreign_addr;    
    segment.seq = cb->tcpcb_sndseq;
    segment.ack = cb->tcpcb_sndack;
    segment.flags = flags;
    segment.wnd = (unsigned short)(cb->tcpcb_rcvwnd >> cb->tcpcb_recvscale);
    segment.urgpointer = (unsigned short)urgpointer;
    segment.options = options;
    segment.optlen = optlen;
    segment.data = data;
    
    error = fnet_tcp_sendseg(&segment);

    /* Turn off the delayed acknowledgment timer.*/
    cb->tcpcb_timers.delayed_ack = FNET_TCP_TIMER_OFF;
    cb->tcpcb_newfreercvsize = 0;

    /* Set the new sequence number.*/
    cb->tcpcb_sndseq = seq + datasize;

    if(FNET_TCP_COMP_G(cb->tcpcb_sndseq, cb->tcpcb_maxrcvack))
        cb->tcpcb_maxrcvack = cb->tcpcb_sndseq;

    return error;
}

/************************************************************************
* NAME: fnet_tcp_sendack
*
* DESCRIPTION: This function sends the acknowledgement segment.
*
* RETURNS: None.                 
*************************************************************************/
static void fnet_tcp_sendack( fnet_socket_t *sk )
{
    char                options[FNET_TCP_MAX_OPT_SIZE]; 
    char                optionlen;                     

    fnet_tcp_control_t  *cb = (fnet_tcp_control_t *)sk->protocol_control;
    
    switch(cb->tcpcb_connection_state)
    {
        /* In the SYN_RCVD state acknowledgement must be with the SYN flag.*/ 
        case FNET_TCP_CS_SYN_RCVD:

          /* Reinitialize of the sequnce number.*/
          cb->tcpcb_sndseq--;

          fnet_tcp_setsynopt(sk, options, &optionlen);

          /* Create and send the SYN segment.*/      
          fnet_tcp_sendheadseg(sk, FNET_TCP_SGT_SYN | FNET_TCP_SGT_ACK, options, optionlen);
          break;

        /* In the FIN_WAIT_1 and LAST_ACK states acknowledgement must be with the FIN flag.*/
        case FNET_TCP_CS_FIN_WAIT_1:
        case FNET_TCP_CS_LAST_ACK:

          /* Reinitialize the sequnce number.*/
          cb->tcpcb_sndseq--;

          /* Create and send the FIN segment.*/
          fnet_tcp_sendheadseg(sk, FNET_TCP_SGT_FIN | FNET_TCP_SGT_ACK, 0, 0);
          break;

        default:
    #if FNET_CFG_TCP_URGENT        
          /* If the urgent data is present, set the urgent flag.*/
          if(FNET_TCP_COMP_GE(cb->tcpcb_sndurgseq, cb->tcpcb_sndseq))
              fnet_tcp_sendheadseg(sk, FNET_TCP_SGT_ACK | FNET_TCP_SGT_URG, 0, 0);
          else
    #endif /* FNET_CFG_TCP_URGENT */
              fnet_tcp_sendheadseg(sk, FNET_TCP_SGT_ACK, 0, 0);

          break;
    }

    /* Turn of the delayed acknowledgment timer.*/
    cb->tcpcb_timers.delayed_ack = FNET_TCP_TIMER_OFF;
    

}

/************************************************************************
* NAME: fnet_tcp_sendrst
*
* DESCRIPTION: This function sends the reset segment.
*
* RETURNS: None.                  
*************************************************************************/
static void fnet_tcp_sendrst( fnet_socket_option_t *sockoption, fnet_netbuf_t *insegment,
                              struct sockaddr *src_addr,  struct sockaddr *dest_addr)
{
    struct fnet_tcp_segment segment;
    
    if(FNET_TCP_FLAGS(insegment) & FNET_TCP_SGT_ACK)
    {
        /* Send the reset segment without acknowledgment*/
        segment.seq = fnet_ntohl(FNET_TCP_ACK(insegment));
        segment.ack = 0;
        segment.flags = FNET_TCP_SGT_RST;
    }                         
    else
    {
        /* Send the reset segment with acknowledgment*/
        segment.seq = 0;
        segment.ack = fnet_ntohl(FNET_TCP_SEQ(insegment)) + insegment->total_length - FNET_TCP_LENGTH(insegment)
                      + (FNET_TCP_SGT_FIN & FNET_TCP_FLAGS(insegment)) + ((FNET_TCP_SGT_SYN
                       & FNET_TCP_FLAGS(insegment)) >> 1);
        segment.flags = FNET_TCP_SGT_RST | FNET_TCP_SGT_ACK;
    } 
    
    segment.sockoption = sockoption; 
    segment.src_addr = *src_addr;
    segment.dest_addr = *dest_addr;
    segment.wnd = 0;
    segment.urgpointer = 0;
    segment.options = 0;
    segment.optlen = 0;
    segment.data = 0;  
        
    fnet_tcp_sendseg(&segment);                      
}

/************************************************************************
* NAME: fnet_tcp_sendrstsk
*
* DESCRIPTION: This function sends the reset segment.
*
* RETURNS: None.                  
*************************************************************************/
static void fnet_tcp_sendrstsk( fnet_socket_t *sk )
{

    struct fnet_tcp_segment segment;
    
    /* Initialize the pointer to the control block.*/
    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control;

    if(sk->state != SS_UNCONNECTED && cb->tcpcb_connection_state != FNET_TCP_CS_SYN_SENT
           && cb->tcpcb_connection_state != FNET_TCP_CS_TIME_WAIT)
    {
        if(cb->tcpcb_connection_state == FNET_TCP_CS_SYN_RCVD)
        {
            /* Send the reset segment with acknowledgment.*/
            segment.seq = 0;
            segment.ack = cb->tcpcb_sndack;
            segment.flags = FNET_TCP_SGT_RST | FNET_TCP_SGT_ACK;
        }  
        else
        {
            /* Send the reset segment without acknowledgment.*/
            segment.seq = cb->tcpcb_rcvack;
            segment.ack = 0;
            segment.flags = FNET_TCP_SGT_RST;
        } 
  
        segment.sockoption = &sk->options; 
        segment.src_addr = sk->local_addr;
        segment.dest_addr = sk->foreign_addr;
        segment.wnd = 0;
        segment.urgpointer = 0;
        segment.options = 0;
        segment.optlen = 0;
        segment.data = 0;
            
        fnet_tcp_sendseg(&segment);                            
    }
}

/************************************************************************
* NAME: fnet_tcp_abortsk
*
* DESCRIPTION: This function sends the reset segment and closes the socket.
*
* RETURNS: None.                               
*************************************************************************/
static void fnet_tcp_abortsk( fnet_socket_t *sk )
{
   
    /* If the incoming or the partial socket is present, abort it.*/
    if(sk->state == SS_LISTENING)
    {
        while(sk->partial_con)
          fnet_tcp_abortsk(sk->partial_con);

        while(sk->incoming_con)
          fnet_tcp_abortsk(sk->incoming_con);
    }
    else
        fnet_tcp_sendrstsk(sk);

    /* Close the socket.*/
    fnet_tcp_closesk(sk);

}

/************************************************************************
* NAME: fnet_tcp_addopt
*
* DESCRIPTION: This function adds the options in the segment.             
*              fnet_tcp_addopt can be used only before the data adding and only after 
*              the creating of the main header.   
*
* RETURNS: If no error occurs, this function returns FNET_OK. Otherwise,
*          it returns FNET_ERR.
*************************************************************************/
static int fnet_tcp_addopt( fnet_netbuf_t *segment, unsigned char len, void *data )
{
    fnet_netbuf_t   *buf; 
    int             i;             
    char            size;   /* Size of options.*/

    /* Create the buffer of the options.*/
    buf = fnet_netbuf_new(FNET_TCP_SIZE_OPTIONS, FNET_FALSE);

    /* Check the memory allocatio.*/
    if(!buf)
        return FNET_ERR_NOMEM;

    /* Reset the buffer and add it to the header.*/
    fnet_memset_zero(buf->data_ptr, FNET_TCP_SIZE_OPTIONS); /* Set to FNET_TCP_OTYPES_END.*/
    segment = fnet_netbuf_concat(segment, buf);
    buf = segment->next;

    /* Copy the options.*/
    for (i = 0; i < len; ++i)
      FNET_TCP_GETUCHAR(buf->data_ptr, i) = FNET_TCP_GETUCHAR(data, i);

    /* Set the length  (this value is saved in 4-byte words format).*/
    if(len & 0x3)
        size = (char)((len & 0xfffc) + 4);
    else
        size = (char)len;

    /* Trim the buffer of the options.*/
    fnet_netbuf_trim(&segment, size - FNET_TCP_SIZE_OPTIONS);

    /* Recalculate the header length.*/
    FNET_TCP_SET_LENGTH(segment) = (unsigned char)((buf->length + FNET_TCP_SIZE_HEADER) << 2);

    return FNET_OK;
}

/************************************************************************
* NAME: fnet_tcp_getopt
*
* DESCRIPTION: This function processes the received options.
*
* RETURNS: None.
*************************************************************************/
static void fnet_tcp_getopt( fnet_socket_t *sk, fnet_netbuf_t *segment )
{
    int i; /* index variable.*/

    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control;

    if(!segment)
        return;

    /* Start position of the options.*/
    i = FNET_TCP_SIZE_HEADER;

    /* Receive the of the options.*/
    while(i < FNET_TCP_LENGTH(segment) && FNET_TCP_GETUCHAR(segment->data_ptr, i) != FNET_TCP_OTYPES_END)
    {
        if(FNET_TCP_GETUCHAR(segment->data_ptr, i) == FNET_TCP_OTYPES_NOP)
        {
            ++i;
        }
        else
        {
            if(i + 1 >= FNET_TCP_LENGTH(segment)
                   || i + FNET_TCP_GETUCHAR(segment->data_ptr, i + 1) - 1 >= FNET_TCP_LENGTH(segment))
                break;

            /* Process the options.*/
            switch(FNET_TCP_GETUCHAR(segment->data_ptr, i))
            {
                case FNET_TCP_OTYPES_MSS:
                  cb->tcpcb_sndmss = fnet_ntohs(FNET_TCP_GETUSHORT(segment->data_ptr, i + 2));
                  break;

                case FNET_TCP_OTYPES_WINDOW:
                  cb->tcpcb_sendscale = FNET_TCP_GETUCHAR(segment->data_ptr, i + 2);

                  if(cb->tcpcb_sendscale > FNET_TCP_MAX_WINSHIFT)
                      cb->tcpcb_sendscale = FNET_TCP_MAX_WINSHIFT;

                  cb->tcpcb_flags |= FNET_TCP_CBF_RCVD_SCALE;
                  break;
            }

            i += FNET_TCP_GETUCHAR(segment->data_ptr, i + 1);
        }
    }

}

/************************************************************************
* NAME: fnet_tcp_setsynopt
*
* DESCRIPTION: This function sets the options of the synchronized (SYN )
*              segment.
* RETURNS: None.
*************************************************************************/
static void fnet_tcp_setsynopt( fnet_socket_t *sk, char *options, char *optionlen )
{
    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control;
    *optionlen = 0;                                                      
    
    /* If 0, detect MSS based on interface MTU minus "TCP,IP header size".*/
    if(cb->tcpcb_rcvmss == 0)
    {
    #if FNET_CFG_IP4 //TBD
        fnet_netif_t *netif;
        
        if((netif = fnet_ip_route(((struct sockaddr_in *)(&sk->foreign_addr))->sin_addr.s_addr)) != 0) 
        {
            cb->tcpcb_rcvmss = (unsigned short) (netif->mtu - 40); /* MTU - [TCP,IP header size].*/
        }
    #else
        cb->tcpcb_rcvmss = 0; // TBD create FNET
    #endif /* FNET_CFG_IP4 */ //TBD        
    }
    

    /* Set the MSS option.*/
    *((unsigned long *)(options + *optionlen)) = fnet_htonl((unsigned long)(cb->tcpcb_rcvmss | FNET_TCP_MSS_HEADER));
    *optionlen += FNET_TCP_MSS_SIZE;

    /* Set the window scale option.*/
    *((unsigned long *)(options + *optionlen))
         = fnet_htonl((unsigned long)((cb->tcpcb_recvscale | FNET_TCP_WINDOW_HEADER) << 8));
    *optionlen += FNET_TCP_WINDOW_SIZE;

}

/************************************************************************
* NAME: fnet_tcp_getsynopt
*
* DESCRIPTION: This function performs the initialization depend on
*              options of the synchronized (SYN) segment.
*
* RETURNS: None.
*************************************************************************/
static void fnet_tcp_getsynopt( fnet_socket_t *sk )
{
    /* Pointer to the control block.*/
    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control;
    
    if(!(cb->tcpcb_flags & FNET_TCP_CBF_RCVD_SCALE))
    {
        /* Reset the scale variable if the scale option is not received.*/
        cb->tcpcb_recvscale = 0;

        /* If the size of the input buffer greater than
         * the maximal size of the window , reinitialze the buffer size.*/
        if(cb->tcpcb_rcvcountmax > FNET_TCP_MAXWIN)
            cb->tcpcb_rcvcountmax = FNET_TCP_MAXWIN;
    }

    /* Initialize the congestion window.*/
    cb->tcpcb_cwnd = cb->tcpcb_sndmss;

}

/************************************************************************
* NAME: fnet_tcp_findsk
*
* DESCRIPTION: This function finds the socket with parameters that allow
*              to receive and process the segment. 
*
* RETURNS: If the socket is found this function returns the pointer to the
*          socket. Otherwise, this function returns 0.
*************************************************************************/
static fnet_socket_t *fnet_tcp_findsk( struct sockaddr *src_addr,  struct sockaddr *dest_addr )                                       
{
    fnet_socket_t   *listensk = 0;
    fnet_socket_t   *sk;

    fnet_isr_lock();
    
    sk = fnet_tcp_prot_if.head;

    while(sk)
    {
        if(sk->local_addr.sa_port == dest_addr->sa_port)
        {
            /* Listening socket.*/
            if(sk->state == SS_LISTENING)
            {
                if(!listensk && (fnet_socket_addr_is_unspecified(&sk->local_addr) || fnet_socket_addr_are_equal(&sk->local_addr, dest_addr) ))
                    listensk = sk;
            }
            else
            {
                /* Not listening socket.*/
                if(fnet_socket_addr_are_equal(&sk->local_addr, dest_addr) && sk->state != SS_UNCONNECTED && 
                   fnet_socket_addr_are_equal(&sk->foreign_addr, src_addr) && sk->foreign_addr.sa_port == src_addr->sa_port)
                     break;
            }
        }
        sk = sk->next;
    }

    /* If the listening socket with the same local parameters (address and port) is present.*/
    if(!sk && listensk)
    {
        /* Search the partial socket with the same foreign parameters (address and port).*/
        sk = listensk->partial_con;

        while(sk)
        {
            if(fnet_socket_addr_are_equal(&sk->foreign_addr, src_addr) && sk->foreign_addr.sa_port == src_addr->sa_port)
                break;

            sk = sk->next;
        }

        if(!sk)
        {
            /* Search the incoming socket with the same foreign parameters (address and port).*/
            sk = listensk->incoming_con;

            while(sk)
            {
                if(fnet_socket_addr_are_equal(&sk->foreign_addr, src_addr) && sk->foreign_addr.sa_port == src_addr->sa_port)
                    break;

                sk = sk->next;
            }
        }

        /* Listening socket is unique (with the same local parameters).*/
        if(!sk)
            sk = listensk;
    }

    fnet_isr_unlock();

    return sk;
}

/***********************************************************************
* NAME: fnet_tcp_addpartialsk
*
* DESCRIPTION: This function adds the socket to the partial sockets list.
*
* RETURNS: None.
*************************************************************************/
static void fnet_tcp_addpartialsk( fnet_socket_t *mainsk, fnet_socket_t *partialsk )
{
    mainsk->partial_con_len++;
    partialsk->head_con = mainsk;
    fnet_socket_list_add(&mainsk->partial_con, partialsk);
}

/***********************************************************************
* NAME: fnet_tcp_movesk2incominglist
*
* DESCRIPTION: This function moves socket from the partial list
*              to the incoming list.
*
* RETURNS: None.                
*************************************************************************/
static void fnet_tcp_movesk2incominglist( fnet_socket_t *sk )
{
    fnet_socket_t *mainsk;
    mainsk = sk->head_con;

    mainsk->partial_con_len--;
    mainsk->incoming_con_len++;

    fnet_socket_list_del(&mainsk->partial_con, sk);
    fnet_socket_list_add(&mainsk->incoming_con, sk);
}

/***********************************************************************
* NAME: fnet_tcp_closesk
*
* DESCRIPTION: This function closes the socket.
*         
* RETURNS: None.     
*************************************************************************/
static void fnet_tcp_closesk( fnet_socket_t *sk )
{

    /* Initialize the pointer to the control block.*/
    fnet_tcp_control_t *cb = (fnet_tcp_control_t *)sk->protocol_control;

    if(sk->head_con)
    {
        /* If the socket is partial or incoming.*/
        if(sk->state == SS_CONNECTING)
            /* Delete from the partial list.*/
            fnet_tcp_delpartialsk(sk);
        else
            /* Delete from the incoming list.*/
            fnet_tcp_delincomingsk(sk);
    }
    else
    {
        /* If the socket must be deleted, free the structures
         * Otherwise, change the state and free the unused data.*/
        if(cb->tcpcb_flags & FNET_TCP_CBF_CLOSE)
        {
            fnet_tcp_delsk(&fnet_tcp_prot_if.head, sk);
        }
        else
        {
#if !FNET_CFG_TCP_DISCARD_OUT_OF_ORDER        
            fnet_tcp_deletetmpbuf(cb);
#endif            
            fnet_socket_buffer_release(&sk->send_buffer);
            sk->state = SS_UNCONNECTED;
            fnet_memset_zero(&sk->foreign_addr, sizeof(sk->foreign_addr));
        }
    }
}
/***********************************************************************
* NAME: fnet_tcp_delpartialsk
*
* DESCRIPTION: This function  deletes the socket from 
*              the partial list.
*
* RETURNS: None.                 
*************************************************************************/
static void fnet_tcp_delpartialsk( fnet_socket_t *sk )
{
    sk->head_con->partial_con_len--;
    fnet_tcp_delsk(&sk->head_con->partial_con, sk);
}

/***********************************************************************
* NAME: fnet_tcp_delincomingsk
*
* DESCRIPTION: This function deletes the socket from the incoming list.
*
* RETURNS: None.                
*************************************************************************/
static void fnet_tcp_delincomingsk( fnet_socket_t *sk )
{
    sk->head_con->incoming_con_len--;
    fnet_tcp_delsk(&sk->head_con->incoming_con, sk);
}

/***********************************************************************
* NAME: fnet_tcp_delcb
*
* DESCRIPTION: This function deletes the control block.
*
* RETURNS: None. 
*************************************************************************/
static void fnet_tcp_delcb( fnet_tcp_control_t *cb )
{
    if(!cb)
        return;
#if !FNET_CFG_TCP_DISCARD_OUT_OF_ORDER
    fnet_tcp_deletetmpbuf(cb);
#endif    
    fnet_free(cb);
}

/***********************************************************************
* NAME: fnet_tcp_delcb
*
* DESCRIPTION: This function deletes the temporary buffer.
*
* RETURNS: None. 
*************************************************************************/
#if !FNET_CFG_TCP_DISCARD_OUT_OF_ORDER
static void fnet_tcp_deletetmpbuf( fnet_tcp_control_t *cb )
{
    fnet_netbuf_t *buf, *delbuf;

    buf = cb->tcpcb_rcvchain;


    while(buf)
    {
        delbuf = buf;
        buf = buf->next_chain;
        fnet_netbuf_free_chain(delbuf);
    }

    cb->tcpcb_count = 0;
    cb->tcpcb_rcvchain = 0;
}
#endif

/***********************************************************************
* NAME: fnet_tcp_delsk
*
* DESCRIPTION: This function deletes the socket.
*
* RETURNS: None.                 
*************************************************************************/
static void fnet_tcp_delsk( fnet_socket_t ** head, fnet_socket_t *sk )
{
    fnet_tcp_delcb((fnet_tcp_control_t *)sk->protocol_control);
    fnet_socket_release(head, sk);
}

/***********************************************************************
* NAME: fnet_tcp_hit
*
* DESCRIPTION: 
*
* RETURNS:  This function returns TRUE if position (pos) is located 
*           in the interval between startpos and endpos. Otherwise
*           this function returns FALSE.             
*************************************************************************/
static int fnet_tcp_hit( unsigned long startpos, unsigned long endpos, unsigned long pos )
{
    if(endpos >= startpos)
    {
        if(pos < startpos || pos > endpos)
            return 0;
    }
    else
    {
        if(pos < startpos && pos > endpos)
            return 0;
    }

    return 1;
}

/***********************************************************************
* NAME: fnet_tcp_getsize
*
* DESCRIPTION:
*
* RETURNS: This function returns the length of the interval
*              (from pos1 to pos2 ).                 
*************************************************************************/
static unsigned long fnet_tcp_getsize( unsigned long pos1, unsigned long pos2 )
{
    unsigned long size;

    if(pos1 <= pos2)
    {
        size = pos2 - pos1;
    }
    else
    {
        size = FNET_TCP_MAX_SEQ - pos1 + pos2 + 1;
    }

    return size;
}


/************************************************************************
* NAME: fnet_tcp_trace
*
* DESCRIPTION: Prints TCP header. For debugging purposes.
*************************************************************************/
#if FNET_CFG_DEBUG_TRACE_TCP
void fnet_tcp_trace(char *str, fnet_tcp_header_t *tcp_hdr)
{

    fnet_printf(FNET_SERIAL_ESC_FG_GREEN"%s", str); /* Print app-specific header.*/
    fnet_println("[TCP header]"FNET_SERIAL_ESC_FG_BLACK);
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(SrcPort)                 "FNET_SERIAL_ESC_FG_BLUE"%3u"FNET_SERIAL_ESC_FG_BLACK" |(DestPort)                 "FNET_SERIAL_ESC_FG_BLUE"%3u"FNET_SERIAL_ESC_FG_BLACK" |",
                    fnet_ntohs(tcp_hdr->source_port),
                    fnet_ntohs(tcp_hdr->destination_port));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(Sequence number)                                 0x%010u |",
                    fnet_ntohl(tcp_hdr->sequence_number));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");    
    fnet_println("|(ACK number)                                      0x%010u |",
                    fnet_ntohl(tcp_hdr->ack_number));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(HL)%2u |           |(F)  %u%u%u%u%u%u|(Window)                 %5u |",
                    FNET_TCP_HEADER_GET_HDRLENGTH(tcp_hdr),
                    FNET_TCP_HEADER_GET_FLAGS(tcp_hdr) >> 5 & 1,
                    FNET_TCP_HEADER_GET_FLAGS(tcp_hdr) >> 4 & 1,
                    FNET_TCP_HEADER_GET_FLAGS(tcp_hdr) >> 3 & 1,
                    FNET_TCP_HEADER_GET_FLAGS(tcp_hdr) >> 2 & 1,
                    FNET_TCP_HEADER_GET_FLAGS(tcp_hdr) >> 1 & 1,
                    FNET_TCP_HEADER_GET_FLAGS(tcp_hdr) & 1,
                    fnet_ntohs(tcp_hdr->window)  );
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
    fnet_println("|(Checksum)             0x%04u |(Urgent ptr)             %5u |",
                    fnet_ntohs(tcp_hdr->checksum),
                    fnet_ntohs(tcp_hdr->urgent_ptr));
    fnet_println("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");     
}
#endif /* FNET_CFG_DEBUG_TRACE_TCP */



#if 0 /* For Debug needs.*/
/*************************************************************************/
int FNET_DEBUG_check_send_buffer( fnet_socket_t *sk)
{
    fnet_netbuf_t *head_nb;
    fnet_netbuf_t *nb = (sk->send_buffer.net_buf_chain);
    int i = 0;


    if(nb == 0)
        return FNET_OK;
      

    head_nb = nb;


    while(nb->next != 0)
    {
         i++;
         nb = nb->next;
         if(i > 100)
         {
            fnet_println("!!!SEND BUF CHAIN CRASH!!!");
            return FNET_ERR;
         }
    }

    return FNET_OK;
    
}
#endif    


#endif

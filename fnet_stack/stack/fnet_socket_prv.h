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
* @file fnet_socket_prv.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.44.0
*
* @brief Private. Socket API.
*
***************************************************************************/

#ifndef _FNET_SOCKET_PRV_H_

#define _FNET_SOCKET_PRV_H_

#include "fnet_error.h"
#include "fnet_socket.h"

#if FNET_CFG_TCP

#include "fnet_tcp.h"

#endif

#include "fnet_netbuf.h"
#include "fnet_ip_prv.h"
#include "fnet_ip6_prv.h"


/************************************************************************
*  Definitions.
*************************************************************************/

/* A allocated port will always be
 * FNET_SOCKET_PORT_RESERVED < local_port <= FNET_SOCKET_PORT_USERRESERVED (ephemeral port)
 */
#define FNET_SOCKET_PORT_RESERVED       (1024)  /* In host byte order.*/
#define FNET_SOCKET_PORT_USERRESERVED   (5000)  /* In host byte order.*/

#define FNET_SOCKET_DESC_RESERVED       (-1)    /* The descriptor is reserved.*/

extern int fnet_enabled;


/**************************************************************************/ /*!
 * @internal
 * @brief    Structure of socket buffer.
 ******************************************************************************/
typedef struct
{
    unsigned long   count;              /**< Aactual chars in buffer.*/
    unsigned long   count_max;          /**< Max actual char count (9*1024).*/
    fnet_netbuf_t   *net_buf_chain;     /**< The net_buf chain.*/
    int             is_shutdown;        /**< The socket has been shut down for read/write.*/    
} fnet_socket_buffer_t;

/**************************************************************************/ /*!
 * @internal
 * @brief    Structure contains parameter of receive datagram 
 * (only for SOCK_DGRAM).
 ******************************************************************************/
typedef struct
{
    struct sockaddr addr_s;
} fnet_socket_buffer_addr_t;

/**************************************************************************/ /*!
 * @internal
 * @brief    Socket options.
 ******************************************************************************/
typedef struct
{
#if FNET_CFG_IP4
    fnet_ip_sockopt_t   ip_opt;         /**< IP options.*/
#endif    
#if FNET_CFG_IP6
    fnet_ip6_sockopt_t  ip6_opt;        /**< IP options.*/
#endif 
#if FNET_CFG_TCP
    fnet_tcp_sockopt_t  tcp_opt;        /**< TCP options.*/
#endif

    int                 error;          /**< Socket last error.*/
    int                 local_error;    /**< Socket local error (ICMP, on timeout).*/
    int                 flags;          /**< Socket flags.*/
    int                 linger;         /**< Lingers on close if unsent data is present (in timer ticks).*/
} fnet_socket_option_t;

/************************************************************************
*    Structure per socket.
*************************************************************************/
typedef struct _socket
{
    struct _socket          *next;                  /**< Pointer to the next socket structure.*/
    struct _socket          *prev;                  /**< Pointer to the previous socket structure.*/
    SOCKET                  descriptor;             /**< Socket descriptor.*/
    fnet_socket_state_t     state;                  /**< Socket state.*/
    int                     protocol_number;        /**< Protocol number.*/
    struct fnet_prot_if     *protocol_interface;    /**< Points to protocol specific functions (interface).*/
    void                    *protocol_control;      /**< Points to protocol control structure (optional).*/

    /* For sockets with SO_ACCEPTCONN.*/
    struct _socket          *partial_con;           /**< Queue of partial connections.*/
    int                     partial_con_len;        /**< Number of connections on partial_con.*/
    struct _socket          *incoming_con;          /**< Queue of incoming connections.*/
    int                     incoming_con_len;       /**< Number of connections on incoming_con.*/
    int                     con_limit;              /**< Max number queued connections (specified  by "listen").*/
    struct _socket          *head_con;              /**< Back pointer to accept socket.*/

    fnet_socket_buffer_t    receive_buffer;         /**< Socket buffer for incoming data.*/
    fnet_socket_buffer_t    send_buffer;            /**< Socket buffer for outgoing data.*/

    /* Common protocol params.*/
    struct sockaddr         foreign_addr;           /**< Foreign socket address.*/
    struct sockaddr         local_addr;             /**< Lockal socket address.*/
    fnet_socket_option_t    options;                /**< Collection of socket options.*/
    
#if FNET_CFG_MULTICAST
    /* Multicast params.*/
    fnet_ip_multicast_list_entry_t *multicast_entry[FNET_CFG_MULTICAST_SOCKET_MAX];
#endif /* FNET_CFG_MULTICAST */
    
} fnet_socket_t;

/**************************************************************************/ /*!
 * @internal
 * @brief    Transport Protocol interface general API structure.
 ******************************************************************************/
typedef struct fnet_socket_prot_if
{
    int  con_req;                                                                                           /* Flag that protocol is connection oriented.*/
    int  (*prot_attach)(fnet_socket_t *sk);                                                                 /* Protocol "attach" function. */
    int  (*prot_detach)(fnet_socket_t *sk);                                                                 /* Protocol "detach" function. */
    int  (*prot_connect)(fnet_socket_t *sk, struct sockaddr *foreign_addr);                                 /* Protocol "connect" function. */
    fnet_socket_t *( *prot_accept)(fnet_socket_t * sk);                                                     /* Protocol "accept" function. */
    int  (*prot_rcv)(fnet_socket_t *sk, char *buf, int len, int flags, struct sockaddr *foreign_addr );     /* Protocol "receive" function. */
    int  (*prot_snd)(fnet_socket_t *sk, char *buf, int len, int flags, const struct sockaddr *foreign_addr );     /* Protocol "send" function. */
    int  (*prot_shutdown)(fnet_socket_t *sk, int how);                                                      /* Protocol "shutdown" function. */
    int  (*prot_setsockopt)(fnet_socket_t *sk, int level, int optname, char *optval, int optlen);           /* Protocol "setsockopt" function. */
    int  (*prot_getsockopt)(fnet_socket_t *sk, int level, int optname, char *optval, int *optlen);          /* Protocol "getsockopt" function. */
    int  (*prot_listen)(fnet_socket_t *sk, int backlog);                                                    /* Protocol "listen" function.*/
                                                                           
} fnet_socket_prot_if_t;

/************************************************************************
*     Function Prototypes
*************************************************************************/
void fnet_socket_init( void );
void fnet_socket_list_add( fnet_socket_t ** head, fnet_socket_t *s );
void fnet_socket_list_del( fnet_socket_t ** head, fnet_socket_t *s );
void fnet_socket_set_error( fnet_socket_t *sock, int error );
fnet_socket_t *fnet_socket_lookup( fnet_socket_t *head,  struct sockaddr *local_addr, struct sockaddr *foreign_addr, int protocol_number);
unsigned short fnet_socket_get_uniqueport(fnet_socket_t *head, struct sockaddr *local_addr);
int fnet_socket_conflict( fnet_socket_t *head,  const struct sockaddr *local_addr, 
                          const struct sockaddr *foreign_addr /*optional*/, int wildcard );
fnet_socket_t *fnet_socket_copy( fnet_socket_t *sock );
void fnet_socket_release( fnet_socket_t ** head, fnet_socket_t *sock );
int fnet_socket_buffer_append_address( fnet_socket_buffer_t *sb, fnet_netbuf_t *nb, struct sockaddr *addr);
int fnet_socket_buffer_append_record( fnet_socket_buffer_t *sb, fnet_netbuf_t *nb );
int fnet_socket_buffer_read_address( fnet_socket_buffer_t *sb, char *buf, int len, struct sockaddr *foreign_addr, int remove );
int fnet_socket_buffer_read_record( fnet_socket_buffer_t *sb, char *buf, int len, int remove );
void fnet_socket_buffer_release( fnet_socket_buffer_t *sb );

int fnet_ip_setsockopt( fnet_socket_t *sock, int level, int optname, char *optval, int optlen );
int fnet_ip_getsockopt( fnet_socket_t *sock, int level, int optname, char *optval, int *optlen );

int fnet_socket_addr_is_multicast(const struct sockaddr *addr);
int fnet_socket_addr_is_broadcast(const struct sockaddr *addr, fnet_netif_t *netif);

int fnet_socket_addr_are_equal(const struct sockaddr *addr1, const struct sockaddr *addr2);
void fnet_socket_ip_addr_copy(const struct sockaddr *from_addr, struct sockaddr *to_addr);
void fnet_socket_addr_copy(const struct sockaddr *from_addr, struct sockaddr *to_addr);
fnet_netif_t *fnet_socket_addr_route(const struct sockaddr *dest_addr);

#endif

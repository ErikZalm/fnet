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
* @file fnet_socket.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.76.0
*
* @brief Socket interface implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_socket.h"
#include "fnet_socket_prv.h"
#include "fnet_timer_prv.h"
#include "fnet_isr.h"
#include "fnet_stdlib.h"
#include "fnet_prot.h"
#include "fnet_debug.h"
#include "fnet.h"
#include "fnet_stdlib.h"

/************************************************************************
*     Global Data Structures
*************************************************************************/

int fnet_enabled = 0;   /* Flag that the stack is initialized. */

static unsigned short fnet_port_last = FNET_SOCKET_PORT_RESERVED + 1;

/* Array of sockets descriptors. */
static fnet_socket_t *fnet_socket_desc[FNET_CFG_SOCKET_MAX];

/************************************************************************
*     Function Prototypes
*************************************************************************/
static SOCKET fnet_socket_desc_alloc(void);
static void fnet_socket_desc_set(SOCKET desc, fnet_socket_t *sock);
static void fnet_socket_desc_free(SOCKET desc);
static fnet_socket_t *fnet_socket_desc_find(SOCKET desc);
static int fnet_socket_addr_check_len(const struct sockaddr *addr, unsigned int addr_len);

/************************************************************************
* NAME: fnet_socket_init
*
* DESCRIPTION: Initialization of the socket layer.
*************************************************************************/
void fnet_socket_init( void )
{
    fnet_memset_zero(fnet_socket_desc, sizeof(fnet_socket_desc));
}

/************************************************************************
* NAME: fnet_socket_set_error
*
* DESCRIPTION: This function sets socket error. 
*************************************************************************/
void fnet_socket_set_error( fnet_socket_t *sock, int error )
{
    if(sock->options.local_error != FNET_OK)
    {
        error = sock->options.local_error;
        sock->options.local_error = FNET_OK;
    }

    sock->options.error = error;
    
    fnet_error_set(error);
}

/************************************************************************
* NAME: fnet_socket_list_add
*
* DESCRIPTION: This function adds socket into the queue.
*************************************************************************/
void fnet_socket_list_add( fnet_socket_t ** head, fnet_socket_t *s )
{
    fnet_isr_lock();
    s->next = *head;

    if(s->next != 0)
        s->next->prev = s;

    s->prev = 0;
    *head = s;
    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_socket_list_del
*
* DESCRIPTION: This function removes socket from the queue 
*************************************************************************/
void fnet_socket_list_del( fnet_socket_t ** head, fnet_socket_t *s )
{
    fnet_isr_lock();

    if(s->prev == 0)
      *head=s->next;
    else
        s->prev->next = s->next;

    if(s->next != 0)
        s->next->prev = s->prev;

    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_socket_desc_alloc
*
* DESCRIPTION: This function reserves socket descriptor.
*************************************************************************/
static SOCKET fnet_socket_desc_alloc( void )
{
    int i;
    int res = FNET_ERR;

    fnet_isr_lock();

    for (i = 0; i < FNET_CFG_SOCKET_MAX; i++) /* Find the empty descriptor.*/
    {
        if(fnet_socket_desc[i] == 0)
        {
            fnet_socket_desc[i] = (fnet_socket_t *)FNET_SOCKET_DESC_RESERVED;
            res = i;
            break;
        }
    }

    fnet_isr_unlock();

    return (res);
}

/************************************************************************
* NAME: fnet_socket_desc_set
*
* DESCRIPTION: This function assigns the socket descriptor to the socket.
*************************************************************************/
static void fnet_socket_desc_set( SOCKET desc, fnet_socket_t *sock )
{
    fnet_socket_desc[desc] = sock;
    sock->descriptor = desc;
}

/************************************************************************
* NAME: fnet_socket_desc_free
*
* DESCRIPTION: This function frees the socket descriptor.
*************************************************************************/
static void fnet_socket_desc_free( SOCKET desc )
{
    fnet_socket_desc[desc] = 0;
}

/************************************************************************
* NAME: fnet_socket_desc_find
*
* DESCRIPTION: This function looking for socket structure 
*              associated with the socket descriptor.
*************************************************************************/
static fnet_socket_t *fnet_socket_desc_find( SOCKET desc )
{
    fnet_socket_t *s = 0;

    if(fnet_enabled && (desc >= 0) && (desc!=SOCKET_INVALID))
    {
        if((desc < FNET_CFG_SOCKET_MAX))
            s = fnet_socket_desc[desc];
    }

    return (s);
}

/************************************************************************
* NAME: fnet_socket_release
*
* DESCRIPTION: This function release all resources allocated for the socket. 
*************************************************************************/
void fnet_socket_release( fnet_socket_t ** head, fnet_socket_t *sock )
{
    fnet_isr_lock();
    fnet_socket_list_del(head, sock);
    fnet_socket_buffer_release(&sock->receive_buffer);
    fnet_socket_buffer_release(&sock->send_buffer);
    fnet_free(sock);
    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_socket_conflict
*
* DESCRIPTION: Return FNET_TRUE if there's a socket whose addresses 'confict' 
*              with the supplied addresses.
*************************************************************************/
int fnet_socket_conflict( fnet_socket_t *head,  const struct sockaddr *local_addr, 
                          const struct sockaddr *foreign_addr /*optional*/, int wildcard )
{
    fnet_socket_t *sock = head;

    while(sock != 0)
    {
          if((sock->local_addr.sa_port == local_addr->sa_port)
               && ((fnet_socket_addr_is_unspecified(local_addr) && (wildcard)) || fnet_socket_addr_are_equal(&sock->local_addr, local_addr) )
               && (((foreign_addr == 0) && (wildcard)) || fnet_socket_addr_are_equal(&sock->foreign_addr, foreign_addr))
               && (((foreign_addr == 0) && (wildcard)) || (foreign_addr && (sock->foreign_addr.sa_port == foreign_addr->sa_port))) )
            return (FNET_TRUE);

        sock = sock->next;
    }

    return (FNET_FALSE);
}

/************************************************************************
* NAME: fnet_socket_lookup
*
* DESCRIPTION: This function looks for a socket with the best match 
*              to the local and foreign address parameters.
*************************************************************************/
fnet_socket_t *fnet_socket_lookup( fnet_socket_t *head,  struct sockaddr *local_addr, struct sockaddr *foreign_addr, int protocol_number)
{
    fnet_socket_t   *sock;
    fnet_socket_t   *match_sock = 0;
    int             match_wildcard = 3;
    int             wildcard;

    for (sock = head; sock != 0; sock = sock->next)
    {
        /* Compare local port number.*/
        if(sock->protocol_number != protocol_number)
            continue; /* Ignore. */
    
        /* Compare local port number.*/
        if(sock->local_addr.sa_port != local_addr->sa_port)
            continue; /* Ignore. */

        wildcard = 0;

        /* Compare local address.*/
        if(!fnet_socket_addr_is_unspecified(&sock->local_addr))
        {
            if(fnet_socket_addr_is_unspecified(local_addr))
                wildcard++;

            else if(!fnet_socket_addr_are_equal(&sock->local_addr, local_addr))
                continue;
        }
        else
        {
            if(!fnet_socket_addr_is_unspecified(local_addr))
                wildcard++;
        }

        /* Compare foreign address and port number.*/
        if(!fnet_socket_addr_is_unspecified(&sock->foreign_addr))
        {
            if(fnet_socket_addr_is_unspecified(foreign_addr))
                wildcard++;

            else if((!fnet_socket_addr_are_equal(&sock->foreign_addr, foreign_addr)) || (sock->foreign_addr.sa_port != foreign_addr->sa_port))
                continue;
        }
        else
        {
            if(!fnet_socket_addr_is_unspecified(foreign_addr))
                wildcard++;
        }

        if(wildcard < match_wildcard)
        {
            match_sock = sock;

            if((match_wildcard = wildcard) == 0)
                break; /* Exact match is found.*/
        }
    }

    return (match_sock);
}

/************************************************************************
* NAME: fnet_socket_uniqueport
*
* DESCRIPTION: Choose a unique (non-conflicting) local port for the socket
*              list starting at 'head'. The port will always be
*	           FNET_SOCKET_PORT_RESERVED < local_port <= FNET_SOCKET_PORT_USERRESERVED (ephemeral port).
*              In network byte order.
*************************************************************************/
//unsigned short fnet_socket_uniqueport( fnet_socket_t *head, fnet_ip4_addr_t local_addr )
unsigned short fnet_socket_get_uniqueport( fnet_socket_t *head, struct sockaddr *local_addr )
{
    unsigned short local_port = fnet_port_last; 
    struct sockaddr local_addr_tmp = *local_addr;

    fnet_isr_lock();

    do
    {
        if((++local_port <= FNET_SOCKET_PORT_RESERVED) || (local_port > FNET_SOCKET_PORT_USERRESERVED))
            local_port = FNET_SOCKET_PORT_RESERVED + 1;
        
        local_addr_tmp.sa_port = fnet_htons(local_port);    
    } 
    while (fnet_socket_conflict(head, &local_addr_tmp, FNET_NULL, 1));
    
    fnet_port_last = local_port;
    
    fnet_isr_unlock();
    
    return local_addr_tmp.sa_port;
}


/************************************************************************
* NAME: fnet_socket_copy
*
* DESCRIPTION: This function creates new socket structure and fills 
*              its proper fields by values from existing socket 
*************************************************************************/
fnet_socket_t *fnet_socket_copy( fnet_socket_t *sock )
{
    fnet_socket_t *sock_cp;

    if((sock_cp = (fnet_socket_t *)fnet_malloc(sizeof(fnet_socket_t))) != 0)
    {
        fnet_memcpy(sock_cp, sock, sizeof(fnet_socket_t));

        sock_cp->next = 0;
        sock_cp->prev = 0;
        sock_cp->descriptor = FNET_SOCKET_DESC_RESERVED;
        sock_cp->state = SS_UNCONNECTED;
        sock_cp->protocol_control = 0;
        sock_cp->head_con = 0;
        sock_cp->partial_con = 0;
        sock_cp->incoming_con = 0;
        sock_cp->receive_buffer.count = 0;
        sock_cp->receive_buffer.net_buf_chain = 0;
        sock_cp->send_buffer.count = 0;
        sock_cp->send_buffer.net_buf_chain = 0;
        sock_cp->options.error = FNET_OK;
        sock_cp->options.local_error = FNET_OK;
        return (sock_cp);
    }
    else
        return (0);
}
/************************************************************************
* NAME: socket
*
* DESCRIPTION: This function creates a socket and returns 
*              the descriptor to the application.
*************************************************************************/
SOCKET socket( fnet_address_family_t family, fnet_socket_type_t type, int protocol )
{
    fnet_prot_if_t  *prot;
    fnet_socket_t   *sock;
    SOCKET          res;
    int             error = FNET_OK;

    fnet_os_mutex_lock();

    if(fnet_enabled == 0) /* Stack is disabled */
    {
        error = FNET_ERR_SYSNOTREADY;
        goto ERROR_1;
    }

    res = fnet_socket_desc_alloc();

    if(res == FNET_ERR)
    {
        error = FNET_ERR_NO_DESC; /* No more socket descriptors are available.*/
        goto ERROR_1;
    }

    if((prot = fnet_prot_find(family, type, protocol)) == 0)
    {
        error = FNET_ERR_PROTONOSUPPORT; /* Protocol not supported.*/
        goto ERROR_2;
    }

    if((sock = (fnet_socket_t *)fnet_malloc(sizeof(fnet_socket_t))) == 0)
    {
        error = FNET_ERR_NOMEM; /* Cannot allocate memory.*/
        goto ERROR_2;
    }

    fnet_memset_zero(sock, sizeof(fnet_socket_t));
    fnet_socket_desc_set(res, sock);
    sock->protocol_interface = prot;
    sock->local_addr.sa_family = family;
    sock->state = SS_UNCONNECTED;
    
    /* Save protocol number.*/
    sock->protocol_number = protocol;
    if(!sock->protocol_number)
        sock->protocol_number = prot->protocol;
    
    sock->foreign_addr.sa_family = family;
    
    fnet_socket_list_add(&prot->head, sock);

    if(prot->socket_api->prot_attach && (prot->socket_api->prot_attach(sock) == SOCKET_ERROR))
    {
        fnet_socket_release(&sock->protocol_interface->head, sock);
        error = fnet_error_get();
        goto ERROR_2;
    }

    fnet_os_mutex_unlock();
    return (res);

    ERROR_2:
    fnet_socket_desc_free(res);

    ERROR_1:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_INVALID);
}

/************************************************************************
* NAME: connect
*
* DESCRIPTION: This function establishes a connection to 
*              a specified socket.
*************************************************************************/
int connect( SOCKET s, struct sockaddr *name, int namelen )
{
    fnet_socket_t       *sock;
    int                 error = FNET_OK;
    struct sockaddr     foreign_addr;
    struct sockaddr     local_addr_tmp;
    int                 result;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(sock->state == SS_LISTENING) /* The socket is marked to accept connections (listen).*/
        {
            error = FNET_ERR_OPNOTSUPP; /*  Operation not supported.*/
            goto ERROR_SOCK;
        }
        
        /* The protocol is connection oriented.*/
        if(sock->protocol_interface->socket_api->con_req) 
        {
            /* A connection has already been initiated.*/
            if(sock->state == SS_CONNECTED)
            {
                error = FNET_ERR_ISCONN; /* Socket is already connected.*/
                goto ERROR_SOCK;
            }

            if(sock->state == SS_CONNECTING)
            {
                error = FNET_ERR_INPROGRESS; /* The action is in progress. */
                goto ERROR_SOCK;
            }
        }


        if((error = fnet_socket_addr_check_len(name, (unsigned int)namelen)) != FNET_OK)
        {
            goto ERROR_SOCK;
        }
        
        foreign_addr = *name;

        if (fnet_socket_addr_is_unspecified(&foreign_addr))
        {
            error = FNET_ERR_DESTADDRREQ; /* Destination address required.*/
            goto ERROR_SOCK;
        }

        if((foreign_addr.sa_port == 0) && (sock->protocol_interface->type != SOCK_RAW))
        {
            error = FNET_ERR_ADDRNOTAVAIL; /* Can't assign requested port.*/
            goto ERROR_SOCK;
        }
        
        local_addr_tmp = sock->local_addr; 
        

        if (fnet_socket_addr_is_unspecified(&local_addr_tmp))
        {
            switch(local_addr_tmp.sa_family)
            {
    #if FNET_CFG_IP4
                case AF_INET:
                    {
                        fnet_netif_t *netif;

                        if((netif = fnet_ip_route(((struct sockaddr_in *)(&foreign_addr))->sin_addr.s_addr)) == 0)
                        {
                            error = FNET_ERR_NETUNREACH; /* No route. */
                            goto ERROR_SOCK;
                        }

                        ((struct sockaddr_in *)(&local_addr_tmp))->sin_addr.s_addr = netif->ip4_addr.address;
                    }
                    break;
    #endif /* FNET_CFG_IP4 */
    #if FNET_CFG_IP6                
                case AF_INET6:
                    {
                        const fnet_ip6_addr_t *local_ip6_addr;
                        if((local_ip6_addr = fnet_ip6_select_src_addr(FNET_NULL, &((struct sockaddr_in6 *)(&foreign_addr))->sin6_addr.s6_addr))== FNET_NULL)
                        {
                            error = FNET_ERR_NETUNREACH; /* No route. */
                            goto ERROR_SOCK;
                        }
                        
                        FNET_IP6_ADDR_COPY(local_ip6_addr, &((struct sockaddr_in6 *)(&local_addr_tmp))->sin6_addr.s6_addr);
                    }
                    break;
    #endif /* FNET_CFG_IP6 */
            }            
        }

        if(local_addr_tmp.sa_port == 0)
        {
            local_addr_tmp.sa_port = fnet_socket_get_uniqueport(sock->protocol_interface->head,
                                                &local_addr_tmp); /* Get ephemeral port.*/
        }
  
            
        if(fnet_socket_conflict(sock->protocol_interface->head, &local_addr_tmp, &foreign_addr, 1))
        {
            error = FNET_ERR_ADDRINUSE; /* Address already in use. */
            goto ERROR_SOCK;
        }

        sock->local_addr = local_addr_tmp;
        
        /* Start the appropriate protocol connection.*/
        if(sock->protocol_interface->socket_api->prot_connect)
            result = sock->protocol_interface->socket_api->prot_connect(sock, &foreign_addr);
        else
            result = FNET_OK;
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

ERROR_SOCK:
    fnet_socket_set_error(sock, error);

ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: bind
*
* DESCRIPTION: This function associates a local address with a socket.
*************************************************************************/
int bind( SOCKET s, const struct sockaddr *name, int namelen )
{
    fnet_socket_t   *sock;
    int             error = FNET_OK;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {

        if((error = fnet_socket_addr_check_len(name, (unsigned int)namelen)) != FNET_OK)
        {
            goto ERROR_SOCK;
        }

        if((sock->local_addr.sa_port == 0) && fnet_socket_addr_is_unspecified(&sock->local_addr))
        {
            if(!fnet_socket_addr_is_multicast(name)) /* Is not multicast.*/
            {
                if(!fnet_socket_addr_is_unspecified(name) && !fnet_socket_addr_is_broadcast(&sock->local_addr, FNET_NULL) && (fnet_netif_get_by_sockaddr(name) == FNET_NULL))
                {
                    /* The specified address is not a valid address for this system.*/
                    error = FNET_ERR_ADDRNOTAVAIL; 
                    goto ERROR_SOCK;
                }
                
                if((name->sa_port != 0)
                     && fnet_socket_conflict(sock->protocol_interface->head, name, FNET_NULL, 0))
                {
                    error = FNET_ERR_ADDRINUSE; /* Address already in use. */
                    goto ERROR_SOCK;
                }
            }

            fnet_socket_ip_addr_copy(name , &sock->local_addr);

            if((name->sa_port == 0) && (sock->protocol_interface->type != SOCK_RAW))
            {
                sock->local_addr.sa_port = fnet_socket_get_uniqueport(sock->protocol_interface->head, &sock->local_addr); /* Get ephemeral port.*/
            }
            else
                sock->local_addr.sa_port = name->sa_port;
                

            fnet_socket_buffer_release(&sock->receive_buffer);
            fnet_socket_buffer_release(&sock->send_buffer);
        }
        else
        {
            error = FNET_ERR_INVAL; /* The socket is already bound to an address.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        /* Bad descriptor.*/
        fnet_error_set(FNET_ERR_BAD_DESC);
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (FNET_OK);

ERROR_SOCK:
    fnet_socket_set_error(sock, error);

ERROR:
    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: closesocket
*
* DESCRIPTION: This function closes an existing socket.
*************************************************************************/
int closesocket( SOCKET s )
{
    fnet_socket_t *sock;
    int result = FNET_OK;
    int error;

    fnet_os_mutex_lock();

    //if(fnet_enabled == 0) /* Stack is disabled */
    //{
    //    error = FNET_ERR_SYSNOTREADY;
    //    goto ERROR;
    //}

    if((sock = fnet_socket_desc_find(s)) != 0)
    {

#if FNET_CFG_MULTICAST && FNET_CFG_IP4
        /* Leave all multicast groups.*/
        {
            int i;
            for(i = 0; i < FNET_CFG_MULTICAST_SOCKET_MAX; i++)
            {
                if (sock->multicast_entry[i]!= FNET_NULL) 
                {
                    fnet_ip_multicast_leave(sock->multicast_entry[i]);
                }
            }
        }                        
#endif /* FNET_CFG_MULTICAST */



        if(sock->protocol_interface->socket_api->prot_detach)
            result = sock->protocol_interface->socket_api->prot_detach(sock);

        if(result == FNET_OK)
            fnet_socket_desc_free(s);
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);
    ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: shutdown
*
* DESCRIPTION: This function to disable reception, transmission, or both.
*************************************************************************/
int shutdown( SOCKET s, int how )
{
    fnet_socket_t *sock;
    int result = FNET_OK;
    int error;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(sock->protocol_interface && sock->protocol_interface->socket_api->prot_shutdown)
            result = sock->protocol_interface->socket_api->prot_shutdown(sock, how);
    }
    else
    {
        error = FNET_ERR_BAD_DESC; /* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);
ERROR:
    fnet_error_set(error);

    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: listen
*
* DESCRIPTION: This function places the socket into the state where 
*              it is listening for an incoming connection.
*************************************************************************/
int listen( SOCKET s, int backlog )
{
    fnet_socket_t *sock;

    int error;
    int result = FNET_OK;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if((sock->state == SS_CONNECTING) || (sock->state == SS_CONNECTED))
        {
            error = FNET_ERR_ISCONN; /* Operation not supported.*/
            goto ERROR_SOCK;
        }

        if(sock->local_addr.sa_port == 0)
        {
            error = FNET_ERR_BOUNDREQ; /* The socket has not been bound.*/
            goto ERROR_SOCK;
        }

        if(backlog < 0)
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }

        if(sock->protocol_interface && sock->protocol_interface->socket_api->prot_listen)
        {
            result = sock->protocol_interface->socket_api->prot_listen(sock, backlog);
        }
        else
        {
            error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        fnet_error_set(FNET_ERR_BAD_DESC);/* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

ERROR_SOCK:
    fnet_socket_set_error(sock, error);

ERROR:
    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: accept
*
* DESCRIPTION: This function accepts a connection on a specified socket.
*************************************************************************/
SOCKET accept( SOCKET s, struct sockaddr *addr, int *addrlen )
{
    fnet_socket_t   *sock;
    fnet_socket_t   *sock_new;
    SOCKET          desc;
    int             error;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(sock->protocol_interface && sock->protocol_interface->socket_api->prot_accept)
        {
            if(sock->state != SS_LISTENING)
            {
                error = FNET_ERR_INVAL; /* Invalid argument.*/
                goto ERROR_SOCK;
            }

            if(addr && addrlen)
            {
                if((error = fnet_socket_addr_check_len(&sock->local_addr, (unsigned int)(*addrlen) )) != FNET_OK )
                {
                    goto ERROR_SOCK;
                }
            }

            if((desc = fnet_socket_desc_alloc()) != FNET_ERR)
            {
                fnet_isr_lock();

                if((sock_new = sock->protocol_interface->socket_api->prot_accept(sock)) == 0)
                {
                    fnet_socket_desc_free(desc);
                    fnet_isr_unlock();
                    error = FNET_ERR_AGAIN;
                    goto ERROR_SOCK;
                };

                fnet_socket_desc_set(desc, sock_new);
                fnet_socket_list_add(&sock->protocol_interface->head, sock_new);
                
                fnet_isr_unlock();
                
                if(addr && addrlen)
                {
                    fnet_socket_addr_copy(&sock_new->foreign_addr, addr);
                }
            }
            else
            {
                error = FNET_ERR_NO_DESC; /* No more socket descriptors are available.*/
                goto ERROR_SOCK;
            }
        }
        else
        {
            error = FNET_ERR_OPNOTSUPP; /*  Operation not supported.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        fnet_error_set(FNET_ERR_BAD_DESC);/* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (desc);

ERROR_SOCK:
    fnet_socket_set_error(sock, error);

ERROR:
    fnet_os_mutex_unlock();
    return (SOCKET_INVALID);
}

/************************************************************************
* NAME: send
*
* DESCRIPTION: This function sends data on a connected socket. 
*************************************************************************/
int send( SOCKET s, char *buf, int len, int flags )
{
    return sendto(s, buf, len, flags, FNET_NULL, 0);
}

/************************************************************************
* NAME: sendto
*
* DESCRIPTION: This function sends data to a specific destination. 
*************************************************************************/
int sendto( SOCKET s, char *buf, int len, int flags, const struct sockaddr *to, int tolen )
{
    fnet_socket_t   *sock;
    int             error;
    int             result = FNET_OK;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        
        if((to == FNET_NULL) || (tolen == 0))
        {
            if(fnet_socket_addr_is_unspecified(&sock->foreign_addr))
            {
                error = FNET_ERR_NOTCONN; /* Socket is not connected.*/
                goto ERROR_SOCK;
            }
            
            to = FNET_NULL;
        }
        else
        {
            if((error = fnet_socket_addr_check_len(to, (unsigned int)tolen)) != FNET_OK)
            {
                goto ERROR_SOCK;
            }     

            if(fnet_socket_addr_is_unspecified(to))
            {
                error = FNET_ERR_DESTADDRREQ; /* Destination address required.*/
                goto ERROR_SOCK;
            }
        }    
        
        if(buf && (len >= 0))
        {

            /* If the socket is shutdowned, return.*/
            if(sock->send_buffer.is_shutdown)
            {
                error = FNET_ERR_SHUTDOWN;
                goto ERROR_SOCK;
            }

            if(sock->protocol_interface->socket_api->prot_snd)
            {
                result = sock->protocol_interface->socket_api->prot_snd(sock, buf, len, flags, to);
            }
            else
            {
                error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
                goto ERROR_SOCK;
            }
        }
        else
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        fnet_error_set(FNET_ERR_BAD_DESC);/* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

ERROR_SOCK:
    fnet_socket_set_error(sock, error);

ERROR:
    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}


/************************************************************************
* NAME: recv
*
* DESCRIPTION: This function receives data from a connected socket. 
*************************************************************************/
int recv( SOCKET s, char *buf, int len, int flags )
{
    return recvfrom(s, buf, len, flags, FNET_NULL, FNET_NULL);
}

/************************************************************************
* NAME: recvfrom
*
* DESCRIPTION: This function reads incoming data of socket and captures 
*              the address from which the data was sent.  
*************************************************************************/
int recvfrom( SOCKET s, char *buf, int len, const int flags, struct sockaddr *from, int *fromlen )
{
    fnet_socket_t   *sock;
    int             error;
    int             result = FNET_OK;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(buf && (len >= 0))
        {

            /* The sockets must be bound before calling recv.*/
            if((sock->local_addr.sa_port == 0) && (sock->protocol_interface->type != SOCK_RAW))
            {
                error = FNET_ERR_BOUNDREQ; /* The socket has not been bound with bind().*/
                goto ERROR_SOCK;
            }

            if(from && fromlen)
            {
                if((error = fnet_socket_addr_check_len(&sock->local_addr, (unsigned int)(*fromlen) )) != FNET_OK )
                {
                    goto ERROR_SOCK;
                }
            }
            
            /* If the socket is shutdowned, return.*/
            if(sock->receive_buffer.is_shutdown)
            {
                error = FNET_ERR_SHUTDOWN;
                goto ERROR_SOCK;
            }

            if(sock->protocol_interface->socket_api->prot_rcv)
            {
                result = sock->protocol_interface->socket_api->prot_rcv(sock, buf, len, flags, (from && fromlen) ? from : FNET_NULL);
            }
            else
            {
                error = FNET_ERR_OPNOTSUPP; /* Operation not supported.*/
                goto ERROR_SOCK;
            }
        }
        else
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        fnet_error_set(FNET_ERR_BAD_DESC);/* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

ERROR_SOCK:
    fnet_socket_set_error(sock, error);

ERROR:
    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: getsockname
*
* DESCRIPTION: This function retrieves the current name 
*              for the specified socket. 
*************************************************************************/
int getsockname( SOCKET s, struct sockaddr *name, int *namelen )
{
    fnet_socket_t   *sock;
    int             error;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if((name == 0) || (namelen == 0))
        {
            error = FNET_ERR_INVAL;
            goto ERROR_SOCK;
        }
        
        if((error = fnet_socket_addr_check_len(&sock->local_addr, (unsigned int)(*namelen) )) != FNET_OK )
        {
            goto ERROR_SOCK;
        }

        if((sock->local_addr.sa_port == 0) && (sock->protocol_interface->type != SOCK_RAW))
        {
            error = FNET_ERR_BOUNDREQ; /* The socket has not been bound with bind().*/
            goto ERROR_SOCK;
        }
        
        fnet_socket_addr_copy(&sock->local_addr, name);
    }
    else
    {
        fnet_error_set(FNET_ERR_BAD_DESC);/* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (FNET_OK);

ERROR_SOCK:
    fnet_socket_set_error(sock, error);

ERROR:
    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: getpeername
*
* DESCRIPTION: This function retrieves the name of the peer 
*              connected to the socket
*************************************************************************/
int getpeername( SOCKET s, struct sockaddr *name, int *namelen )
{
    fnet_socket_t   *sock;
    int             error;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if((name == 0) || (namelen == 0) ) 
        {
            error = FNET_ERR_INVAL;
            goto ERROR_SOCK;
        }

        if((error = fnet_socket_addr_check_len(&sock->local_addr, (unsigned int)(*namelen) )) != FNET_OK )
        {
            goto ERROR_SOCK;
        }        

        if(fnet_socket_addr_is_unspecified(&sock->foreign_addr))
        {
            error = FNET_ERR_NOTCONN; /* Socket is not connected.*/
            goto ERROR_SOCK;
        }
        
        fnet_socket_addr_copy(&sock->foreign_addr, name);
    }
    else
    {
        fnet_error_set(FNET_ERR_BAD_DESC);/* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (FNET_OK);

ERROR_SOCK:
    fnet_socket_set_error(sock, error);

ERROR:
    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: setsockopt
*
* DESCRIPTION: This function sets the current value for a socket option 
*              associated with a socket
*************************************************************************/
int setsockopt( SOCKET s, int level, int optname, char *optval, int optlen )
{
    fnet_socket_t   *sock;
    int             error;
    int             result = FNET_OK;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(optval && optlen)
        {
            if(level != SOL_SOCKET)
            {
                if(sock->protocol_interface && sock->protocol_interface->socket_api->prot_setsockopt)
                    result = sock->protocol_interface->socket_api->prot_setsockopt(sock, level, optname, optval, optlen);
                else
                {
                    error = FNET_ERR_INVAL; /* Invalid argument.*/
                    goto ERROR_SOCK;
                }
            }
            else
            {
                switch(optname)     /* Socket options processing.*/
                {
                    case SO_LINGER: /* Linger on close if data present.*/
                      if(optlen != sizeof(struct linger))
                      {
                          error = FNET_ERR_INVAL;
                          goto ERROR_SOCK;
                      }

                      sock->options.linger = ((struct linger *)optval)->l_linger
                                                 * (1000 / FNET_TIMER_PERIOD_MS);

                      if(((struct linger *)optval)->l_onoff)
                          sock->options.flags |= optname;
                      else
                          sock->options.flags &= ~optname;

                      break;

                    case SO_KEEPALIVE: /* Keep connections alive.*/
                    case SO_DONTROUTE: /* Just use interface addresses.*/
                #if FNET_CFG_TCP_URGENT                    
                    case SO_OOBINLINE: /* Leave received OOB data in line.*/
                #endif                    
                      if(optlen < sizeof(int))
                      {
                          error = FNET_ERR_INVAL;
                          goto ERROR_SOCK;
                      }

                      if(*((int *)optval))
                          sock->options.flags |= optname;
                      else
                          sock->options.flags &= ~optname;

                      break;

                    case SO_SNDBUF: /* Send buffer size.*/
                    case SO_RCVBUF: /* Receive buffer size.*/
                      if((optlen < sizeof(unsigned long)))
                      {
                          error = FNET_ERR_INVAL;
                          goto ERROR_SOCK;
                      }

                      if(optname == SO_SNDBUF)
                          sock->send_buffer.count_max = *((unsigned long *)optval);
                      else
                          sock->receive_buffer.count_max = *((unsigned long *)optval);

                      break;
                    default:
                      error = FNET_ERR_NOPROTOOPT; /* The option is unknown or unsupported. */
                      goto ERROR_SOCK;
                }
            }
        }
        else
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        fnet_error_set(FNET_ERR_BAD_DESC);/* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

ERROR_SOCK:
    fnet_socket_set_error(sock, error);

ERROR:
    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: getsockopt
*
* DESCRIPTION: This function retrieves the current value for 
*              a socket option associated with a socket 
*************************************************************************/
int getsockopt( SOCKET s, int level, int optname, char *optval, int *optlen )
{
    fnet_socket_t   *sock;
    int             error;
    int             result = FNET_OK;

    fnet_os_mutex_lock();

    if((sock = fnet_socket_desc_find(s)) != 0)
    {
        if(optval && optlen)
        {
            if(level != SOL_SOCKET)
            {
                if(sock->protocol_interface && sock->protocol_interface->socket_api->prot_getsockopt)
                    result = sock->protocol_interface->socket_api->prot_getsockopt(sock, level, optname, optval, optlen);
                else
                {
                    error = FNET_ERR_INVAL; /* Invalid argument.*/
                    goto ERROR_SOCK;
                }
            }
            else
            {
                switch(optname)     /* Socket options processing.*/
                {
                    case SO_LINGER: /* Linger on close if data present.*/
                        if(*optlen < sizeof(struct linger))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(struct linger);
                        ((struct linger *)optval)->l_onoff
                                = (unsigned short)((sock->options.flags & SO_LINGER) > 0);
                        ((struct linger *)optval)->l_linger
                            = (unsigned short)((sock->options.linger * FNET_TIMER_PERIOD_MS) / 1000);
                        sock->options.linger = ((struct linger *)optval)->l_linger;
                        break;

                    case SO_KEEPALIVE: /* Keep connections alive.*/
                    case SO_DONTROUTE: /* Just use interface addresses.*/
                #if FNET_CFG_TCP_URGENT                    
                    case SO_OOBINLINE: /* Leave received OOB data in line.*/
                #endif                    
                        if(*optlen < sizeof(int))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(int);
                        *((int*)optval) = (int)((sock->options.flags & optname) > 0);
                        break;

                    case SO_ACCEPTCONN: /* Socket is listening. */
                        if(*optlen < sizeof(int))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(int);
                        *((int*)optval) = (int)(sock->state == SS_LISTENING);
                        break;

                    case SO_SNDBUF: /* Send buffer size.*/
                    case SO_RCVBUF: /* Receive buffer size.*/
                        if(*optlen < sizeof(unsigned long))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(unsigned long);

                        if(optname == SO_SNDBUF)
                            *((unsigned long*)optval)=sock->send_buffer.count_max;
                        else
                            *((unsigned long *)optval) = sock->receive_buffer.count_max;
                        break;
                    case SO_STATE: /* State of the socket.*/
                        if(*optlen < sizeof(fnet_socket_state_t))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(fnet_socket_state_t);
                        *((fnet_socket_state_t*)optval) = sock->state;
                        break;

                    case SO_RCVNUM:  /* Use to determine the amount of data pending in the network's input buffer that can be read from socket.*/
                    case SO_SNDNUM: /* Use to determine the amount of data in the network's output buffer.*/
                        if(*optlen < sizeof(unsigned long))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(unsigned long);

                        if(optname == SO_RCVNUM)
                            *((unsigned long*)optval) = sock->receive_buffer.count;
                        else
                            *((unsigned long *)optval) = sock->send_buffer.count;
                        break;
                    case SO_ERROR: /* Socket error.*/
                        if(*optlen < sizeof(int))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(int);
                        *((int *)optval) = sock->options.error;
                        sock->options.error = FNET_OK; /* Reset error.*/
                        break;

                    case SO_TYPE:
                        if(*optlen < sizeof(int))
                        {
                            error = FNET_ERR_INVAL;
                            goto ERROR_SOCK;
                        }

                        *optlen = sizeof(int);
                        *((int *)optval) = (sock->protocol_interface ? sock->protocol_interface->type : 0);
                        break;

                    default:
                        error = FNET_ERR_NOPROTOOPT; /* The option is unknown or unsupported. */
                        goto ERROR_SOCK;
                }/* case*/
            }/* else */
        }
        else
        {
            error = FNET_ERR_INVAL; /* Invalid argument.*/
            goto ERROR_SOCK;
        }
    }
    else
    {
        fnet_error_set(FNET_ERR_BAD_DESC);/* Bad descriptor.*/
        goto ERROR;
    }

    fnet_os_mutex_unlock();
    return (result);

ERROR_SOCK:
    fnet_socket_set_error(sock, error);

ERROR:
    fnet_os_mutex_unlock();
    return (SOCKET_ERROR);
}

/************************************************************************
* NAME: fnet_socket_buffer_release
*
* DESCRIPTION: Discards any buffers in the socket buffer
*************************************************************************/
void fnet_socket_buffer_release( fnet_socket_buffer_t *sb )
{
    fnet_netbuf_t   *nb_ptr;
    fnet_netbuf_t   *tmp_nb_ptr;

    fnet_isr_lock();

    if(sb && sb->net_buf_chain)
    {
        nb_ptr = sb->net_buf_chain;

        while(nb_ptr != 0)
        {
            tmp_nb_ptr = nb_ptr->next_chain;
            fnet_netbuf_free_chain(nb_ptr);
            nb_ptr = tmp_nb_ptr;
        }

        sb->net_buf_chain = 0;
        sb->count = 0;
    }

    fnet_isr_unlock();
}

/************************************************************************
* NAME: fnet_socket_buffer_append_record
*
* DESCRIPTION: Append the record to the end of the socket buffer.
*************************************************************************/
int fnet_socket_buffer_append_record( fnet_socket_buffer_t *sb, fnet_netbuf_t *nb )
{
    fnet_isr_lock();

    if((nb->total_length + sb->count) > sb->count_max)
    {
        fnet_isr_unlock();
        return FNET_ERR;
    }

    sb->net_buf_chain = fnet_netbuf_concat(sb->net_buf_chain, nb);

    sb->count += nb->total_length;
    fnet_isr_unlock();

    return FNET_OK;
}

/************************************************************************
* NAME: fnet_socket_buffer_append_address
*
* DESCRIPTION: Constract net_buf chain  and add it to the queue. 
*              The chain contains the address of the message 
*              and the message data.
*************************************************************************/
int fnet_socket_buffer_append_address( fnet_socket_buffer_t *sb, fnet_netbuf_t *nb, struct sockaddr *addr)
{
    fnet_socket_buffer_addr_t   *sb_address;
    fnet_netbuf_t               *nb_addr;

    fnet_isr_lock();

    if((nb->total_length + sb->count) > sb->count_max)
    {
        goto ERROR;
    }

    if((nb_addr = fnet_netbuf_new(sizeof(fnet_socket_buffer_addr_t), 0)) == 0)
    {
        goto ERROR;
    }

    sb_address = (fnet_socket_buffer_addr_t *)nb_addr->data_ptr;

    sb_address->addr_s = *addr;

    sb->count += nb->total_length;

    nb = fnet_netbuf_concat(nb_addr, nb);
    fnet_netbuf_add_chain(&sb->net_buf_chain, nb);
    fnet_isr_unlock();
    
	/* Wake-up user application.*/
 	fnet_os_event_raise(); 

    return FNET_OK;
    
ERROR:  
    fnet_isr_unlock();
    return FNET_ERR;  
}

/************************************************************************
* NAME: fnet_socket_buffer_read_record
*
* DESCRIPTION: This function reads data from socket buffer and 
*              put this data into application buffer. 
*************************************************************************/
int fnet_socket_buffer_read_record( fnet_socket_buffer_t *sb, char *buf, int len, int remove )
{
    if(sb->net_buf_chain)
    {
        if(len > sb->net_buf_chain->total_length)
            len = (int)sb->net_buf_chain->total_length;

        fnet_netbuf_to_buf(sb->net_buf_chain, 0, len, buf);

        if(remove)
        {
            fnet_isr_lock();
            fnet_netbuf_trim(&sb->net_buf_chain, len);
            sb->count -= len;
            fnet_isr_unlock();
        }
    }
    else
        len = 0;

    return len;
}

/************************************************************************
* NAME: fnet_socket_buffer_read_address
*
* DESCRIPTION:This function reads data from socket buffer and 
*             put this data into application buffer. 
*             And captures the address information from which the data was sent. 
*************************************************************************/
int fnet_socket_buffer_read_address( fnet_socket_buffer_t *sb, char *buf, int len, struct sockaddr *foreign_addr, int remove )
{
    fnet_netbuf_t   *nb;
    fnet_netbuf_t   *nb_addr;

    if(((nb_addr = sb->net_buf_chain) != 0) ) 
    {
        if((nb = nb_addr->next) != 0)
        {
            if(len > nb->total_length)
                len = (int)nb->total_length;

            fnet_netbuf_to_buf(nb, 0, len, buf);
        }
        else
            len = 0;
        
        *foreign_addr = ((fnet_socket_buffer_addr_t *)(nb_addr->data_ptr))->addr_s;
        
        if(len < (nb_addr->total_length - sizeof(fnet_socket_buffer_addr_t)))
            len = FNET_ERR;

        if(remove)
        {
            fnet_isr_lock();

            if(nb)
                sb->count -= nb->total_length;
            fnet_netbuf_del_chain(&sb->net_buf_chain, nb_addr);
            
            fnet_isr_unlock();
        }
    }
    else
        len = 0;
 
    return len;
}

/************************************************************************
* NAME: fnet_socket_addr_check_len
*
* DESCRIPTION: This function check sockaddr structure and its size.
*************************************************************************/
static int fnet_socket_addr_check_len(const struct sockaddr *addr, unsigned int addr_len )
{
    int result = FNET_OK;
    
    if(addr && addr_len)
    {
#if FNET_CFG_IP6
        if(addr->sa_family & AF_INET6)
        {
            if(addr_len < sizeof(struct sockaddr_in6))
            {
                result = FNET_ERR_INVAL;    /* Invalid argument.*/
            }
        }
        else
#endif /* FNET_CFG_IP4 */

#if FNET_CFG_IP4
        if(addr->sa_family & AF_INET)
        {
            if(addr_len < sizeof(struct sockaddr_in))
            {
                result = FNET_ERR_INVAL;    /* Invalid argument.*/
            }
        }
        else
#endif /* FNET_CFG_IP4 */
        {
            result = FNET_ERR_AFNOSUPPORT; 
        }
    }
    else
    {
        result = FNET_ERR_INVAL;
    }

    return result;
}

/************************************************************************
* NAME: fnet_socket_addr_is_multicast
*
* DESCRIPTION: Returns FNET_FALSE if the address is not multicast.
*************************************************************************/
int fnet_socket_addr_is_multicast(const struct sockaddr *addr)
{
    int     result = FNET_FALSE;
    
    if(addr)
    {
#if FNET_CFG_IP6
        if(addr->sa_family & AF_INET6)
        {
            result = FNET_IP6_ADDR_IS_MULTICAST( &((struct sockaddr_in6 *)addr)->sin6_addr.s6_addr);
        }
        else
#endif /* FNET_CFG_IP4 */

#if FNET_CFG_IP4
        if(addr->sa_family & AF_INET)
        {
            result = FNET_IP4_ADDR_IS_MULTICAST( ((struct sockaddr_in *)addr)->sin_addr.s_addr);
        }
        else
#endif /* FNET_CFG_IP4 */
        {};
    }

    return result;
}

/************************************************************************
* NAME: fnet_socket_addr_is_broadcast
*
* DESCRIPTION: Returns FNET_FALSE if the address is not broadcast.
*************************************************************************/
int fnet_socket_addr_is_broadcast(const struct sockaddr *addr, fnet_netif_t *netif)
{
    int result = FNET_FALSE;
    
#if FNET_CFG_IP4
    if(addr)
    {
        if(addr->sa_family & AF_INET)
        {
            result = fnet_ip_addr_is_broadcast( ((struct sockaddr_in *)addr)->sin_addr.s_addr, netif );
        }
    }        
#else
    FNET_COMP_UNUSED_ARG(netif);
    FNET_COMP_UNUSED_ARG(addr);
#endif /* FNET_CFG_IP4 */

    return result;
}

/************************************************************************
* NAME: fnet_socket_addr_is_unspecified
*
* DESCRIPTION: Returns FNET_FALSE if the address is not specified.
*************************************************************************/
int fnet_socket_addr_is_unspecified(const struct sockaddr *addr)
{
    int result = FNET_TRUE;
    
    if(addr)
    {
#if FNET_CFG_IP6
        if(addr->sa_family & AF_INET6)
        {
            result = FNET_IP6_ADDR_IS_UNSPECIFIED( &((struct sockaddr_in6 *)addr)->sin6_addr.s6_addr);
        }
        else
#endif /* FNET_CFG_IP4 */
#if FNET_CFG_IP4
        if(addr->sa_family & AF_INET)
        {
            result = FNET_IP4_ADDR_IS_UNSPECIFIED( ((struct sockaddr_in *)addr)->sin_addr.s_addr);
        }
        else
#endif /* FNET_CFG_IP4 */
        {};
    }

    return result;
}

/************************************************************************
* NAME: fnet_socket_addr_are_equal
*
* DESCRIPTION: Returns FNET_FALSE if the addresses are not equal.
*************************************************************************/
int fnet_socket_addr_are_equal(const struct sockaddr *addr1, const struct sockaddr *addr2 )
{
    int result = FNET_FALSE;
    
    if(addr1 && addr2 && (addr1->sa_family == addr2->sa_family))
    {
#if FNET_CFG_IP6
        if(addr1->sa_family & AF_INET6)
        {
            result =  FNET_IP6_ADDR_EQUAL( &((struct sockaddr_in6 *)addr1)->sin6_addr.s6_addr, &((struct sockaddr_in6 *)addr2)->sin6_addr.s6_addr);
        }
        else
#endif /* FNET_CFG_IP4 */
#if FNET_CFG_IP4
        if(addr1->sa_family & AF_INET)
        {
            result = (((struct sockaddr_in *)addr1)->sin_addr.s_addr == (((struct sockaddr_in *)addr2)->sin_addr.s_addr));
        }
        else
#endif /* FNET_CFG_IP4 */
        {};
    }

    return result;
}

/************************************************************************
* NAME: fnet_socket_ip_addr_copy
*
* DESCRIPTION: 
*************************************************************************/
void fnet_socket_ip_addr_copy(const struct sockaddr *from_addr, struct sockaddr *to_addr)
{
   
    if(from_addr && to_addr && (to_addr->sa_family == from_addr->sa_family))
    {
#if FNET_CFG_IP6
        if(from_addr->sa_family & AF_INET6)
        {
            FNET_IP6_ADDR_COPY(&((struct sockaddr_in6 *)from_addr)->sin6_addr.s6_addr, &((struct sockaddr_in6 *)to_addr)->sin6_addr.s6_addr);
        }
        else
#endif /* FNET_CFG_IP4 */
#if FNET_CFG_IP4
        if(from_addr->sa_family & AF_INET)
        {
            ((struct sockaddr_in *)to_addr)->sin_addr.s_addr = ((struct sockaddr_in *)from_addr)->sin_addr.s_addr;
        }
        else
#endif /* FNET_CFG_IP4 */
        {};
    }
}

/************************************************************************
* NAME: fnet_socket_addr_copy
*
* DESCRIPTION: 
*************************************************************************/
void fnet_socket_addr_copy(const struct sockaddr *from_addr, struct sockaddr *to_addr)
{
    if(from_addr && to_addr)
    {
#if FNET_CFG_IP6
        if(from_addr->sa_family & AF_INET6)
        {
            fnet_memcpy(to_addr, from_addr, sizeof(struct sockaddr_in6));
        }
        else
#endif /* FNET_CFG_IP4 */
#if FNET_CFG_IP4
        if(from_addr->sa_family & AF_INET)
        {
            fnet_memcpy(to_addr, from_addr, sizeof(struct sockaddr_in));
        }
        else
#endif /* FNET_CFG_IP4 */
        {};
    }
}

/************************************************************************
* NAME: fnet_socket_addr_route
*
* DESCRIPTION: 
*************************************************************************/
fnet_netif_t *fnet_socket_addr_route(const struct sockaddr *dest_addr)
{
    fnet_netif_t *result = FNET_NULL;
   
    if(dest_addr) 
    {
        switch(dest_addr->sa_family)
        {
#if FNET_CFG_IP4
            case AF_INET:
                result = fnet_ip_route(((struct sockaddr_in *)dest_addr)->sin_addr.s_addr);
                break;
#endif /* FNET_CFG_IP4 */
#if FNET_CFG_IP6                
            case AF_INET6:

                /* Check Scope ID.*/
                if((result = fnet_netif_get_by_scope_id( ((struct sockaddr_in6 *)dest_addr)->sin6_scope_id )) == FNET_NULL)
                {
                    fnet_ip6_addr_t *src_ip;
                    src_ip = (fnet_ip6_addr_t *)fnet_ip6_select_src_addr(FNET_NULL, &((struct sockaddr_in6 *)dest_addr)->sin6_addr.s6_addr);
                
                    if(src_ip)
                    {
                        result = fnet_netif_get_by_ip6_addr(src_ip);
                    } 
                }
                break;
#endif /* FNET_CFG_IP6 */
        }
    }
    
    return result;
}



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
* @file fnet_telnet.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.41.0
*
* @brief FNET Telnet Server implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_TELNET

#include "fnet_telnet.h"
#include "fnet_timer.h"
#include "fnet_debug.h"
#include "fnet_stdlib.h"
#include "fnet_shell.h"
#include "fnet_poll.h"

/************************************************************************
*     Definitions
************************************************************************/

#if FNET_CFG_DEBUG_TELNET    
    #define FNET_DEBUG_TELNET   FNET_DEBUG
#else
    #define FNET_DEBUG_TELNET(...)
#endif

#define FNET_TELNET_WAIT_SEND_MS        (2000)  /* ms*/

#define FNET_TELNET_TX_BUFFER_SIZE      FNET_CFG_TELNET_SOCKET_BUF_SIZE
#define FNET_TELNET_RX_BUFFER_SIZE      (10)

#if FNET_TELNET_TX_BUFFER_SIZE > 90     /* Check maximum value for TX application/stream buffer (optional check).*/
    #undef FNET_TELNET_TX_BUFFER_SIZE
    #define FNET_TELNET_TX_BUFFER_SIZE  (90)
#endif

#if (FNET_TELNET_TX_BUFFER_SIZE  < 5)   /* Check minimum value for TX application/stream buffer.*/
    error "FNET_TELNET_TX_BUFFER_SIZE must be > 4"
#endif


/* Keepalive probe retransmit limit.*/
#define FNET_TELNET_TCP_KEEPCNT         (2)

/* Keepalive retransmit interval.*/
#define FNET_TELNET_TCP_KEEPINTVL       (4) /*sec*/

/* Time between keepalive probes.*/
#define FNET_TELNET_TCP_KEEPIDLE        (7) /*sec*/


/* RFC:
* All TELNET commands consist of at least a two byte sequence: the
* "Interpret as Command" (IAC) escape character followed by the code
* for the command. The commands dealing with option negotiation are
* three byte sequences, the third byte being the code for the option
* referenced.
*/
/* RFC:
* Since the NVT is what is left when no options are enabled, the DON’T and
* WON’T responses are guaranteed to leave the connection in a state
* which both ends can handle. Thus, all hosts may implement their
* TELNET processes to be totally unaware of options that are not
* supported, simply returning a rejection to (i.e., refusing) any
* option request that cannot be understood.
*
* The Network Virtual Terminal (NVT) is a bi-directional character
* device.
*/


#define FNET_TELNET_CMD_IAC   ((char)255) /* "Interpret as Command" (IAC) escape character followed by the code for the command. */
#define FNET_TELNET_CMD_WILL  ((char)251) /* Indicates the desire to begin performing, or confirmation that
                                     * you are now performing, the indicated option.*/
#define FNET_TELNET_CMD_WONT  ((char)252) /* Indicates the refusal to perform, or continue performing, the
                                     * indicated option. */
#define FNET_TELNET_CMD_DO    ((char)253) /* Indicates the request that the other party perform, or
                                     * confirmation that you are expecting the other party to perform, the
                                     * indicated option. */
#define FNET_TELNET_CMD_DONT  ((char)254) /* Indicates the demand that the other party stop performing,
                                     * or confirmation that you are no longer expecting the other party
                                     * to perform, the indicated option. */

/*****************************************************************************
 * Telnet server states.
 ******************************************************************************/
typedef enum
{
    FNET_TELNET_STATE_DISABLED = 0,     /* Telnet server service is 
                                         * not initialized.
                                         */
    FNET_TELNET_STATE_LISTENING,        /* Telnet server is listening 
                                         * for client socket.
                                         */                                         
    FNET_TELNET_STATE_RECEIVING,        /* Ready to receive data from a Telnet client. */
    FNET_TELNET_STATE_IAC,              /* Received IAC symbol. */
    FNET_TELNET_STATE_DONT ,            /* Prepare to send DON'T. */
    FNET_TELNET_STATE_WONT ,            /* Prepare to send WON'T. */
    FNET_TELNET_STATE_SKIP ,            /* Ignore next received character.*/
    FNET_TELNET_STATE_CLOSING           /* Closing Telnet session.*/                               
} fnet_telnet_state_t;                                     

/************************************************************************
*    Telnet interface control structure.
*************************************************************************/
struct fnet_telnet_session_if
{
    fnet_telnet_state_t         state;              /* Current state.*/
    SOCKET                      socket_foreign;     /* Foreign socket.*/
    char                        tx_buffer[FNET_TELNET_TX_BUFFER_SIZE];  /* Transmit liner buffer. */
    int                         tx_buffer_head_index;                   /* TX buffer index (write place).*/
    char                        rx_buffer[FNET_TELNET_RX_BUFFER_SIZE];  /* RX circular buffer */    
    char                        *rx_buffer_head;    /* The RX circular buffer write pointer. */
    char                        *rx_buffer_tail;    /* The RX circular buffer read pointer. */
    char                        *rx_buffer_end;     /* Pointer to the end of the Rx circular buffer. */
    fnet_shell_desc_t           shell_descriptor;
    struct fnet_shell_params    shell_params;
    char                        cmd_line_buffer[FNET_CFG_TELNET_CMD_LINE_BUF_SIZE];
    struct fnet_serial_stream   stream;
}; 

/************************************************************************
*    Telnet interface control structure.
*************************************************************************/
struct fnet_telnet_if
{
    SOCKET                          socket_listen;      /* Listening socket.*/
    fnet_poll_desc_t                service_descriptor; /* Descriptor of polling service.*/    
    int                             enabled;
    int                             backlog;
    struct fnet_telnet_session_if   *session_active;
    struct fnet_telnet_session_if   session[FNET_CFG_TELNET_SESSION_MAX];
}; 


/* The Telnet interface */ 
static struct fnet_telnet_if telnet_if_list[FNET_CFG_TELNET_MAX];


/************************************************************************
*     Function Prototypes
*************************************************************************/
static void fnet_telnet_send(struct fnet_telnet_session_if *session);
static void tx_buffer_write(struct fnet_telnet_session_if *session, char data);
static int tx_buffer_free_space(struct fnet_telnet_session_if *session);
static void rx_buffer_write (struct fnet_telnet_session_if *session, char data);
static char rx_buffer_read(struct fnet_telnet_session_if *session);
static int rx_buffer_free_space(struct fnet_telnet_session_if *session);
static void fnet_telnet_putchar(long id, int character);
static int fnet_telnet_getchar(long id);
static void fnet_telnet_flush(long id);
static void fnet_telnet_send_cmd(struct fnet_telnet_session_if *session, char command, char option);
static void fnet_telnet_state_machine(void *telnet_if_p);


/************************************************************************
* Buffer functions. 
************************************************************************/
/* Write to Tx liner buffer. */
/* It's posible to write FNET_TELNET_TX_BUFFER_SIZE-1 characters. */
static void tx_buffer_write (struct fnet_telnet_session_if *session, char data)
{
   session->tx_buffer[session->tx_buffer_head_index] = data;
   session->tx_buffer_head_index++;
}

/* Free space in Tx Liner buffer. */
static int tx_buffer_free_space(struct fnet_telnet_session_if *session)
{
   return(FNET_TELNET_TX_BUFFER_SIZE  - session->tx_buffer_head_index);   
}


/* Write to Rx circular buffer. */
/* It's posible to write FNET_TELNET_RX_BUFFER_SIZE-1 characters. */
static void rx_buffer_write (struct fnet_telnet_session_if *session, char data)
{
   *session->rx_buffer_head = data;
   if(++session->rx_buffer_head==session->rx_buffer_end)
      session->rx_buffer_head=session->rx_buffer;
}

/* Read from Rx circular buffer. */
static char rx_buffer_read (struct fnet_telnet_session_if *session)
{
   char data = *session->rx_buffer_tail;
   if(++session->rx_buffer_tail==session->rx_buffer_end)
      session->rx_buffer_tail=session->rx_buffer;
   return data;       
}

/* Free space in Rx circular buffer. */
static int rx_buffer_free_space(struct fnet_telnet_session_if *session)
{
   int  space = session->rx_buffer_tail - session->rx_buffer_head;
   if (space<=0)
      space += FNET_TELNET_RX_BUFFER_SIZE;    
   
   return (space-1);   
}

/************************************************************************
* NAME: fnet_telnet_putchar
*
* DESCRIPTION: 
************************************************************************/
static void fnet_telnet_putchar(long id, int character)
{
    struct fnet_telnet_session_if *session = (struct fnet_telnet_session_if *)id;
    
    if(session->state != FNET_TELNET_STATE_CLOSING)
    {
        tx_buffer_write(session, (char)character);         

        if(tx_buffer_free_space(session) < 1) /* Buffer is full => flush. */
        {
            fnet_telnet_send(session);
        }
    }
}

/************************************************************************
* NAME: fnet_telnet_getchar
*
* DESCRIPTION: 
************************************************************************/
static int fnet_telnet_getchar(long id)
{
    struct fnet_telnet_session_if *session = (struct fnet_telnet_session_if *)id;
    
    if(session->rx_buffer_tail != session->rx_buffer_head) /* If there is something. */
    {
        return rx_buffer_read (session);
    }
    else
        return FNET_ERR;
}

/************************************************************************
* NAME: fnet_telnet_flush
*
* DESCRIPTION: 
************************************************************************/
static void fnet_telnet_flush(long id)
{
    struct fnet_telnet_session_if *session = (struct fnet_telnet_session_if *)id;
    
    fnet_telnet_send(session);
}

/************************************************************************
* NAME: fnet_telnet_send
*
* DESCRIPTION: 
************************************************************************/
static void fnet_telnet_send(struct fnet_telnet_session_if *session)
{
    int             res;
    int             tx_buffer_tail_index = 0;
    unsigned long   timeout = fnet_timer_ticks();
    
    /* Send all data in the buffer.*/
    while(tx_buffer_tail_index != session->tx_buffer_head_index)
    {
        if((res = send(session->socket_foreign, &session->tx_buffer[tx_buffer_tail_index], session->tx_buffer_head_index - tx_buffer_tail_index, 0)) != SOCKET_ERROR)
        {
            if(res) /* >0 */
            {
                /* Update buffer pointers. */
                tx_buffer_tail_index += res;

                /* Reset timeout. */
                timeout = fnet_timer_ticks();           
            }
            else if( fnet_timer_get_interval(timeout, fnet_timer_ticks())
                         > (FNET_TELNET_WAIT_SEND_MS / FNET_TIMER_PERIOD_MS) ) /* Check timeout */
            {
                FNET_DEBUG_TELNET("TELNET:Send timeout.");
                break; /* Time-out. */
            }
        }
        else /* Error.*/
        {
            FNET_DEBUG_TELNET("TELNET:Send error.");
            session->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
            break; 
        }
    }
    
    /* Reset TX buffer index. */ 
    session->tx_buffer_head_index = 0;
}

/************************************************************************
* NAME: fnet_telnet_send_cmd
*
* DESCRIPTION: Wrie command to the TX buffer.
************************************************************************/
static void fnet_telnet_send_cmd(struct fnet_telnet_session_if *session, char command, char option )
{
    tx_buffer_write(session, (char)FNET_TELNET_CMD_IAC);
    tx_buffer_write(session, command);
    tx_buffer_write(session, option);
    
    /* Send the command.*/
    fnet_telnet_send(session);
    
    FNET_DEBUG_TELNET("TELNET: Send option = %d", option);
}

/************************************************************************
* NAME: fnet_telnet_state_machine
*
* DESCRIPTION: Telnet server state machine.
************************************************************************/
static void fnet_telnet_state_machine( void *telnet_if_p )
{
    struct sockaddr                 foreign_addr;
    int                             res;
    struct fnet_telnet_if           *telnet = (struct fnet_telnet_if *)telnet_if_p;
    char                            rx_data[1];
    int                             len;
    int                             i;
    struct fnet_telnet_session_if   *session;
    
    for(i=0; i<FNET_CFG_TELNET_SESSION_MAX; i++) 
    { 
        session = &telnet->session[i];
        telnet->session_active = session;
        
        do
        {
            switch(session->state)
            {
                /*---- LISTENING ------------------------------------------------*/
                case FNET_TELNET_STATE_LISTENING:
                     len = sizeof(foreign_addr);
                    session->socket_foreign = accept(telnet->socket_listen, (struct sockaddr *) &foreign_addr, &len);
                    
                    if(session->socket_foreign != SOCKET_INVALID)
                    {
                        #if FNET_CFG_DEBUG_TELNET
                        {
                            char ip_str[FNET_IP_ADDR_STR_SIZE];
                            fnet_inet_ntop(foreign_addr.sa_family, foreign_addr.sa_data, ip_str, sizeof(ip_str)); 
                            FNET_DEBUG_TELNET("\nTELNET: New connection: %s; Port: %d.", ip_str, fnet_ntohs(foreign_addr.sa_port));
                        }
                        #endif

                        /* Init Shell. */
                        session->shell_descriptor = fnet_shell_init(&session->shell_params); 
        
                        if(session->shell_descriptor == FNET_ERR)
                        {
                            session->shell_descriptor = 0;
                            
                            FNET_DEBUG_TELNET("TELNET: Shell Service registration error.");
                            session->state = FNET_TELNET_STATE_CLOSING;   /*=> CLOSING */
                        }
                        else
                        {    
                            listen(telnet->socket_listen, --telnet->backlog); /* Ignor other connections.*/
                            
                            /* Reset TX timeout. */
                            session->state = FNET_TELNET_STATE_RECEIVING; /* => WAITING data */
                        }
                    }
                    break;
                /*---- NORMAL -----------------------------------------------*/
                case FNET_TELNET_STATE_RECEIVING:
                    if(rx_buffer_free_space(session)>0) 
                    {                
                        res = recv(session->socket_foreign, rx_data, 1, 0);
                        if(res == 1)
                        {
                            if(rx_data[0] == FNET_TELNET_CMD_IAC )
                            {
                                session->state = FNET_TELNET_STATE_IAC; /*=> Handle IAC */
                            }
                            else
                            {
                                rx_buffer_write (session, rx_data[0]);
                            }
                        }
                        else if (res == SOCKET_ERROR)
                        {              
                            session->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
                        }
                    }                
                    break;
                /*---- IAC -----------------------------------------------*/    
                case FNET_TELNET_STATE_IAC:
                    FNET_DEBUG_TELNET("TELNET: STATE_IAC");
                    
                    if((res = recv(session->socket_foreign, rx_data, 1, 0) )!= SOCKET_ERROR)
                    {
                        if(res)
                        {
                            switch(rx_data[0])
                            {
                                case FNET_TELNET_CMD_WILL:
                                    session->state = FNET_TELNET_STATE_DONT;
                                    break;
                                case FNET_TELNET_CMD_DO:
                                    session->state = FNET_TELNET_STATE_WONT;
                                    break;                        
                                case FNET_TELNET_CMD_WONT:
                                case FNET_TELNET_CMD_DONT:
                                    session->state = FNET_TELNET_STATE_SKIP ;
                                    break;   
                                case FNET_TELNET_CMD_IAC:
                                    /*
                                    the IAC need be doubled to be sent as data, and
                                    the other 255 codes may be passed transparently.
                                    */
                                    rx_buffer_write (session, rx_data[0]);
                                default:
                                    session->state = FNET_TELNET_STATE_RECEIVING; /*=> Ignore commands */ 
                            }
                        }
                    }
                    else
                    {              
                        session->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
                    }
                    break;
                /*---- DONT & WONT -----------------------------------------------*/     
                case FNET_TELNET_STATE_DONT:
                case FNET_TELNET_STATE_WONT:
                    {
                        char command;
                        
                        if(session->state == FNET_TELNET_STATE_DONT)
                        {
                            FNET_DEBUG_TELNET("TELNET: STATE_DONT");
                            command = FNET_TELNET_CMD_DONT;
                        }
                        else
                        {
                            FNET_DEBUG_TELNET("TELNET: STATE_WONT");
                            command =  FNET_TELNET_CMD_WONT;
                        }
                         
                        if(tx_buffer_free_space(session) >= 3)
        	            {
                            res = recv(session->socket_foreign, rx_data, 1, 0);
                            
                            if(res == 1)
                            {
                                /* Send command. */
                                fnet_telnet_send_cmd(session, command, rx_data[0]);
                                session->state = FNET_TELNET_STATE_RECEIVING; 
                            }
                            else if (res == SOCKET_ERROR)
                            {              
                                session->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
                            }
                        }
                    }
                    break;
                /*---- SKIP -----------------------------------------------*/                    
                case FNET_TELNET_STATE_SKIP:
                    FNET_DEBUG_TELNET("TELNET: STATE_SKIP");
                     
                    res = recv(session->socket_foreign, rx_data, 1, 0);
                    if(res == 1)
                    {
                        session->state = FNET_TELNET_STATE_RECEIVING; 
                    }
                    else if (res == SOCKET_ERROR)
                    {              
                        session->state = FNET_TELNET_STATE_CLOSING; /*=> CLOSING */
                    }

                    break;
                /*---- CLOSING --------------------------------------------*/
                case FNET_TELNET_STATE_CLOSING:
                    FNET_DEBUG_TELNET("TELNET: STATE_CLOSING");
                    
                    if(session->shell_descriptor)
                    {
                        fnet_shell_release(session->shell_descriptor);
                        session->shell_descriptor = 0;
                    }
                    
                    session->rx_buffer_head = session->rx_buffer;
                    session->rx_buffer_tail = session->rx_buffer; 
                  
                    closesocket(session->socket_foreign);
                    session->socket_foreign = SOCKET_INVALID;
                    
                    listen(telnet->socket_listen, ++telnet->backlog); /* Allow connection.*/
                    
                    session->state = FNET_TELNET_STATE_LISTENING; /*=> LISTENING */
                    break;
                default:
                    break;

            }

        }
        while(session->state == FNET_TELNET_STATE_CLOSING);
    }
}

/************************************************************************
* NAME: fnet_telnet_init
*
* DESCRIPTION: Initialization of the Telnet server.
*************************************************************************/
fnet_telnet_desc_t fnet_telnet_init( struct fnet_telnet_params *params )
{
    struct sockaddr         local_addr;
    struct fnet_telnet_if   *telnet_if = 0;
    
    /* Socket options. */
    const struct linger     linger_option ={1, /*l_onoff*/
                                        4  /*l_linger*/};
    const unsigned long     bufsize_option = FNET_CFG_TELNET_SOCKET_BUF_SIZE;
    const int               keepalive_option = 1;
    const int               keepcnt_option = FNET_TELNET_TCP_KEEPCNT;
    const int               keepintvl_option = FNET_TELNET_TCP_KEEPINTVL;
    const int               keepidle_option = FNET_TELNET_TCP_KEEPIDLE;
    int                     i;
    
    if(params == 0 )
    {
        FNET_DEBUG_TELNET("TELNET: Wrong init parameters.");
        goto ERROR_1;
    }

    /* Try to find free Telnet server descriptor. */
#if (FNET_CFG_TELNET_MAX > 1)
    {
        int i;
        for(i=0; i<FNET_CFG_TELNET_MAX; i++)
        {
            if(telnet_if_list[i].enabled == FNET_FALSE)
            {
                telnet_if = &telnet_if_list[i];
                break; 
            }
        }
    }
#else
    if(telnet_if_list[0].enabled == FNET_FALSE)
        telnet_if = &telnet_if_list[0];
#endif

    /* No free Telnet server descriptor. */
    if(telnet_if == 0)
    {
        FNET_DEBUG_TELNET("TELNET: No free Telnet Server.");
        goto ERROR_1;
    }
   
    local_addr = params->address;
    
    if(local_addr.sa_port == 0)
        local_addr.sa_port = FNET_CFG_TELNET_PORT;  /* Aply the default port. */
    
    if(local_addr.sa_family == AF_UNSPEC)
        local_addr.sa_family = AF_SUPPORTED;   /* Asign supported families.*/
    
     /* Create listen socket */
    if((telnet_if->socket_listen = socket(local_addr.sa_family, SOCK_STREAM, 0)) == SOCKET_INVALID)
    {
        FNET_DEBUG_TELNET("TELNET: Socket creation error.");
        goto ERROR_1;
    }   

    if(bind(telnet_if->socket_listen, (struct sockaddr *)(&local_addr), sizeof(local_addr)) == SOCKET_ERROR)
    {
        FNET_DEBUG_TELNET("TELNET: Socket bind error.");
        goto ERROR_2;
    }
    
    /* Set socket options. */    
    if( /* Setup linger option. */
        (setsockopt (telnet_if->socket_listen, SOL_SOCKET, SO_LINGER, (char *)&linger_option, sizeof(linger_option)) == SOCKET_ERROR) ||
         /* Set socket buffer size. */
        (setsockopt(telnet_if->socket_listen, SOL_SOCKET, SO_RCVBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) ||
        (setsockopt(telnet_if->socket_listen, SOL_SOCKET, SO_SNDBUF, (char *) &bufsize_option, sizeof(bufsize_option))== SOCKET_ERROR) ||
        /* Enable keepalive_option option. */
        (setsockopt (telnet_if->socket_listen, SOL_SOCKET, SO_KEEPALIVE, (char *)&keepalive_option, sizeof(keepalive_option)) == SOCKET_ERROR) ||
        /* Keepalive probe retransmit limit. */
        (setsockopt (telnet_if->socket_listen, IPPROTO_TCP, TCP_KEEPCNT, (char *)&keepcnt_option, sizeof(keepcnt_option)) == SOCKET_ERROR) ||
        /* Keepalive retransmit interval.*/
        (setsockopt (telnet_if->socket_listen, IPPROTO_TCP, TCP_KEEPINTVL, (char *)&keepintvl_option, sizeof(keepintvl_option)) == SOCKET_ERROR) ||
        /* Time between keepalive probes.*/
        (setsockopt (telnet_if->socket_listen, IPPROTO_TCP, TCP_KEEPIDLE, (char *)&keepidle_option, sizeof(keepidle_option)) == SOCKET_ERROR)
    )
    {
        FNET_DEBUG_TELNET("TELNET: Socket setsockopt() error.");
        goto ERROR_2;
    }

    telnet_if->backlog = FNET_CFG_TELNET_SESSION_MAX;
        
    if(listen(telnet_if->socket_listen, telnet_if->backlog) == SOCKET_ERROR)
    {
        FNET_DEBUG_TELNET("TELNET: Socket listen error.");
        goto ERROR_2;
    }
    
    /* Register service. */
    telnet_if->service_descriptor = fnet_poll_service_register(fnet_telnet_state_machine, (void *) telnet_if);
    
    if(telnet_if->service_descriptor == (fnet_poll_desc_t)FNET_ERR)
    {
        FNET_DEBUG_TELNET("TELNET: Service registration error.");
        goto ERROR_2;
    }
  
    for(i=0; i<FNET_CFG_TELNET_SESSION_MAX; i++) 
    {
        struct fnet_telnet_session_if   *session = &telnet_if->session[i];
         
        /* Reset buffer pointers. Move it to init state. */ 
        session->tx_buffer_head_index = 0;
        session->rx_buffer_head = session->rx_buffer;
        session->rx_buffer_tail = session->rx_buffer; 
        session->rx_buffer_end = &session->rx_buffer[FNET_TELNET_RX_BUFFER_SIZE]; 

        /* Setup stream. */
        session->stream.id = (long)(session);
        session->stream.putchar = fnet_telnet_putchar;
        session->stream.getchar = fnet_telnet_getchar;
        session->stream.flush = fnet_telnet_flush;
        
        /* Init shell. */
        session->shell_params.shell = params->shell;
        session->shell_params.cmd_line_buffer = session->cmd_line_buffer;
        session->shell_params.cmd_line_buffer_size = sizeof(session->cmd_line_buffer);
        session->shell_params.stream = &session->stream;
        session->shell_params.echo = FNET_CFG_TELNET_SHELL_ECHO;

        session->socket_foreign = SOCKET_INVALID;
                
        session->state = FNET_TELNET_STATE_LISTENING;
    }
    
    
    telnet_if->session_active = FNET_NULL;
    telnet_if->enabled = FNET_TRUE;
    
    return (fnet_telnet_desc_t)telnet_if;

ERROR_2:
    closesocket(telnet_if->socket_listen);

ERROR_1:
    return (fnet_telnet_desc_t)FNET_ERR;
}

/************************************************************************
* NAME: fnet_telnet_release
*
* DESCRIPTION: Telnet server release.
************************************************************************/
void fnet_telnet_release(fnet_telnet_desc_t desc)
{
    struct fnet_telnet_if   *telnet_if = (struct fnet_telnet_if *) desc;
    int                     i;
    
    if(telnet_if && (telnet_if->enabled == FNET_TRUE))
    {
        for(i=0; i<FNET_CFG_TELNET_SESSION_MAX; i++) 
        {
            struct fnet_telnet_session_if   *session = &telnet_if->session[i];
            
            closesocket(session->socket_foreign);        
            session->socket_foreign = SOCKET_INVALID;
            
            if(session->shell_descriptor)
            {
                fnet_shell_release(session->shell_descriptor);
                session->shell_descriptor = 0;
            }
            session->state = FNET_TELNET_STATE_DISABLED;
        }
        closesocket(telnet_if->socket_listen);
        fnet_poll_service_unregister(telnet_if->service_descriptor); /* Delete service.*/

        
        telnet_if->enabled = FNET_FALSE;
    }
}

/************************************************************************
* NAME: fnet_telnet_close_session
*
* DESCRIPTION: Close current Telnet server session.
************************************************************************/
void fnet_telnet_close_session(fnet_telnet_desc_t desc)
{
    struct fnet_telnet_if *telnet_if = (struct fnet_telnet_if *) desc;
    
    if(telnet_if && (telnet_if->enabled == FNET_TRUE) && telnet_if->session_active)
    {
        telnet_if->session_active->state = FNET_TELNET_STATE_CLOSING;
    }
}

/************************************************************************
* NAME: fnet_telnet_enabled
*
* DESCRIPTION: This function returns FNET_TRUE if the Telnet server 
*              is enabled/initialised.
************************************************************************/
int fnet_telnet_enabled(fnet_telnet_desc_t desc)
{
    struct fnet_telnet_if   *telnet_if = (struct fnet_telnet_if *) desc;
    int                     result;
    
    if(telnet_if)
        result = telnet_if->enabled;
    else
        result = FNET_FALSE;    
    
    return result;
}

#endif /* FNET_CFG_TELNET */

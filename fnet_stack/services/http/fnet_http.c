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
* @file fnet_http.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.48.0
*
* @brief FNET HTTP/0.9 Server implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_HTTP

#include "fnet_http.h"
#include "fnet_http_prv.h"
#include "fnet_timer.h"
#include "fnet_debug.h"
#include "fnet_stdlib.h"
#include "fnet_fs.h"
#include "fnet_serial.h"

#if FNET_CFG_HTTP_AUTHENTICATION_BASIC
#include "fnet_http_auth_prv.h"
#endif

/************************************************************************
*     Definitions
************************************************************************/
/* Automated time-outs */
#define FNET_HTTP_WAIT_TX_MS    (10000)  /* ms*/
#define FNET_HTTP_WAIT_RX_MS    (15000)  /* ms*/

#define FNET_HTTP_BACKLOG_MAX   (1)

#define FNET_HTTP_VERSION_HEADER    "HTTP/" /* Protocol version HTTP/x.x*/
#define FNET_HTTP_ITERATION_NUMBER  (2)

#define FNET_HTTP_HEADER_FIELD_CONTENT_TYPE     "Content-Type:"
#define FNET_HTTP_HEADER_FIELD_CONTENT_LENGTH   "Content-Length:"
#define FNET_HTTP_HEADER_FIELD_AUTHENTICATE     "WWW-Authenticate:"
#define FNET_HTTP_HEADER_FIELD_AUTHORIZATION    "Authorization:"

/* Supported method list. */
static const struct fnet_http_method *fnet_http_method_list[] = 
{
   &fnet_http_method_get,  /* GET method. */
#if FNET_CFG_HTTP_POST   
   &fnet_http_method_post, /* POST method. */
#endif   
   0                /* End of the list */
};

/* File handler list.`*/
static const struct fnet_http_file_handler *fnet_http_file_handler_list[] =
{
#if FNET_CFG_HTTP_SSI
    &fnet_http_ssi_handler, /* SSI handler */
#endif
#if FNET_CFG_HTTP_CGI    
    &fnet_http_cgi_handler, /* CGI handler */
#endif   
    /* Add your file-handler here.*/ 
    0
};

/* Default file handler.*/
static const struct fnet_http_file_handler fnet_http_default_handler =
{
    "", fnet_http_default_handle, 
        fnet_http_default_send,  
        fnet_http_default_close
};

const struct fnet_http_content_type fnet_http_content_css = {"css", "text/css"};
const struct fnet_http_content_type fnet_http_content_jpg = {"jpg", "image/jpeg"};
const struct fnet_http_content_type fnet_http_content_gif = {"gif", "image/gif"};
const struct fnet_http_content_type fnet_http_content_js = {"js", "application/javascript"};

/************************************************************************
*    File content-type list.
*************************************************************************/
const struct fnet_http_content_type *fnet_http_content_type_list[] = 
{
    &fnet_http_content_css,
    &fnet_http_content_jpg,
    &fnet_http_content_gif,
    &fnet_http_content_js,
    /* Add your content-type here. */
    0
};

/* The HTTP interface */ 
static struct fnet_http_if http_if_list[FNET_CFG_HTTP_MAX];

static void fnet_http_state_machine( void *http_if_p );

#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/

    /************************************************************************
    *    HTTP response status list.
    *************************************************************************/
    struct fnet_http_status fnet_http_status_list[] =
    {
        {FNET_HTTP_STATUS_CODE_OK,                  FNET_HTTP_REASON_PHRASE_OK},
        {FNET_HTTP_STATUS_CODE_CREATED,             FNET_HTTP_REASON_PHRASE_CREATED},
        {FNET_HTTP_STATUS_CODE_ACCEPTED,            FNET_HTTP_REASON_PHRASE_ACCEPTED},
        {FNET_HTTP_STATUS_CODE_NO_CONTENT,          FNET_HTTP_REASON_PHRASE_NO_CONTENT},
        {FNET_HTTP_STATUS_CODE_MOVED_PERMANENTLY,   FNET_HTTP_REASON_PHRASE_MOVED_PERMANENTLY},
        {FNET_HTTP_STATUS_CODE_MOVED_TEMPORARILY,   FNET_HTTP_REASON_PHRASE_MOVED_TEMPORARILY},
        {FNET_HTTP_STATUS_CODE_NOT_MODIFIED,        FNET_HTTP_REASON_PHRASE_NOT_MODIFIED},
        {FNET_HTTP_STATUS_CODE_BAD_REQUEST,         FNET_HTTP_REASON_PHRASE_BAD_REQUEST},
        {FNET_HTTP_STATUS_CODE_UNAUTHORIZED,        FNET_HTTP_REASON_PHRASE_UNAUTHORIZED},
        {FNET_HTTP_STATUS_CODE_FORBIDDEN,           FNET_HTTP_REASON_PHRASE_FORBIDDEN},
        {FNET_HTTP_STATUS_CODE_NOT_FOUND,           FNET_HTTP_REASON_PHRASE_NOT_FOUND},
        {FNET_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR, FNET_HTTP_REASON_PHRASE_INTERNAL_SERVER_ERROR},
        {FNET_HTTP_STATUS_CODE_NOT_IMPLEMENTED,     FNET_HTTP_REASON_PHRASE_NOT_IMPLEMENTED},
        {FNET_HTTP_STATUS_CODE_BAD_GATEWAY,         FNET_HTTP_REASON_PHRASE_BAD_GATEWAY},
        {FNET_HTTP_STATUS_CODE_SERVICE_UNAVAILABLE, FNET_HTTP_REASON_PHRASE_SERVICE_UNAVAILABLE},
        {FNET_HTTP_STATUS_CODE_NONE,                ""} /* End of the list.*/
    };


    static void fnet_http_version_parse(char * in_str, struct fnet_http_version * version);
    static int fnet_http_tx_status_line (struct fnet_http_if * http);
    static int fnet_http_status_ok(int status);
#endif /* FNET_CFG_HTTP_VERSION_MAJOR */

/************************************************************************
* NAME: fnet_http_state_machine
*
* DESCRIPTION: Http server state machine.
************************************************************************/
static void fnet_http_state_machine( void *http_if_p )
{
    struct sockaddr         foreign_addr;
    int                     len;
    int                     res;
    struct fnet_http_if     *http = (struct fnet_http_if *)http_if_p;
    int                     iteration;
    char                    *ch;
    int                     i;
    struct fnet_http_session_if   *session;
    
    for(i=0; i<FNET_CFG_HTTP_SESSION_MAX; i++) 
    { 
        session = &http->session[i];
        http->session_active = session;



    for(iteration = 0; iteration < FNET_HTTP_ITERATION_NUMBER; iteration++)
    {
        switch(session->state)
        {
            
            /*---- LISTENING ------------------------------------------------*/
            case FNET_HTTP_STATE_LISTENING:
                len = sizeof(foreign_addr);

                if((session->socket_foreign = accept(http->socket_listen, &foreign_addr, &len)) != SOCKET_INVALID)
                {
#if FNET_CFG_DEBUG_HTTP
                    {
                        char ip_str[FNET_IP_ADDR_STR_SIZE];
                        fnet_inet_ntop(foreign_addr.sa_family, (char*)(foreign_addr.sa_data), ip_str, sizeof(ip_str)); 
                        FNET_DEBUG_HTTP("");
                        FNET_DEBUG_HTTP("HTTP: RX Request From: %s; Port: %d.", ip_str, fnet_ntohs(foreign_addr.sa_port));
                    }
#endif

                    /* Reset response & request parameters.*/
                    fnet_memset_zero(&session->response, sizeof(struct fnet_http_response));
                    fnet_memset_zero(&session->request, sizeof(struct fnet_http_request));

#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/             
                    session->response.content_length = -1; /* No content length by default.*/ 
                    /* Default HTTP version response.*/
                    session->response.version.major = FNET_HTTP_VERSION_MAJOR;
                    session->response.version.minor = FNET_HTTP_VERSION_MINOR;
                    session->response.tx_data = fnet_http_tx_status_line;
#endif                    
                    session->state_time = fnet_timer_ticks();          /* Reset timeout. */
                    session->buffer_actual_size = 0;
                    session->state = FNET_HTTP_STATE_RX_REQUEST; /* => WAITING HTTP REQUEST */
                }
                break;
            /*---- RX_LINE -----------------------------------------------*/
            case FNET_HTTP_STATE_RX_REQUEST:    
                
                do
                {
                    /* Read character by character.*/
                    ch = &session->buffer[session->buffer_actual_size];
                    
                    if((res = recv(session->socket_foreign, ch, 1, 0) )!= SOCKET_ERROR)
                    {
                        if(res > 0) /* Received a data.*/
                        {
                            session->state_time = fnet_timer_ticks();  /* Reset timeout.*/
                            
                            session->buffer_actual_size++;
                        
                            if(*ch == '\r')
                                *ch = '\0';
                            else if(*ch == '\n')
                            /* Line received.*/
                            {
                                char * req_buf = &session->buffer[0];

                                *ch = '\0'; 
                                
                                if(session->request.method == 0)
                                /* Parse Request line.*/
                                {
                                    const struct fnet_http_method **method = &fnet_http_method_list[0];
                                    
                                    FNET_DEBUG_HTTP("HTTP: RX Request: %s", req_buf);
                                    
                                    /* Determine the method type. */
               	                    while(*method)
            	                    {
            			                if ( !fnet_strcmp_splitter(req_buf, (*method)->token, ' ') ) 
            			                {				 
            				                req_buf+=fnet_strlen((*method)->token);
            				                session->request.method = *method;
            				                break;
            			                }
            			                method++;
            			            }
            			            
            			            /* Check if the method is supported? */
            			            if(session->request.method && session->request.method->handle) 
        			                {
                			            /* Parse URI.*/
                			            req_buf = fnet_http_uri_parse(req_buf, &session->request.uri);
                			            
                			            FNET_DEBUG_HTTP("HTTP: URI Path = %s; Query = %s", http->request.uri.path, http->request.uri.query);
                			            
    #if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/                            
                                        /* Parse HTTP/x.x version.*/
                                        fnet_http_version_parse(++req_buf, &session->response.version);
                                        
                                        /* Check the highest supported HTTP version.*/
                                        if(((session->response.version.major<<8)|session->response.version.minor) > ((FNET_HTTP_VERSION_MAJOR<<8)|FNET_HTTP_VERSION_MINOR))
                                        {
                                            session->response.version.major = FNET_HTTP_VERSION_MAJOR;
                                            session->response.version.minor = FNET_HTTP_VERSION_MINOR;
                                        }
                                        
                                        if(session->response.version.major == 0) 
                                        /* HTTP/0.x */
                                        {
                                            session->state = FNET_HTTP_STATE_CLOSING; /* Client does not support HTTP/1.x*/
                                            break;
                                        }
                                        
    #if FNET_CFG_HTTP_AUTHENTICATION_BASIC                                    
                                        /* Check Authentification.*/
                                        fnet_http_auth_validate_uri(http);
    #endif
    #endif/*FNET_CFG_HTTP_VERSION_MAJOR*/
                                         
                                        /* Call method initial handler.*/
                                        res = session->request.method->handle(http, &session->request.uri);
                                        
    #if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/                                       
                                        if(fnet_http_status_ok(res) == FNET_OK)
    #else                                            
                                        if((res == FNET_OK))                     
    #endif                                    
                                           
                                        {
    #if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
                                            session->buffer_actual_size = 0;
                                                /* => Parse Header line.*/ 
    #else /* HTTP/0.9 */
                    			            session->response.tx_data = session->request.method->send;
                    			            
                                            /* Reset buffer pointers.*/
            		                        session->buffer_actual_size = 0;
                                            session->state = FNET_HTTP_STATE_TX; /* Send data.*/
    #endif                    			                
                    			        }
                                        /* Method error.*/
            			                else /* Error.*/
            			                {
    #if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
                                            /* Default code = FNET_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR. */
                                            if(res != FNET_ERR)
                                                session->response.status.code = (fnet_http_status_code_t)res;
                                            
                                            /* Send status line.*/
                                            session->buffer_actual_size = 0;
                                            session->state = FNET_HTTP_STATE_TX; /* Send error.*/
    #else /* HTTP/0.9 */
            			                    session->state = FNET_HTTP_STATE_CLOSING;
    #endif     			                
            			                }
            			            }
            			            /* Method is not supported.*/
            			            else /* Error.*/
            			            {
    #if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
                                        session->response.status.code = FNET_HTTP_STATUS_CODE_NOT_IMPLEMENTED;    
                                        /* Send status line.*/
                                        session->buffer_actual_size = 0;
                                        session->state = FNET_HTTP_STATE_TX; /* Send error.*/
    #else /* HTTP/0.9 */
                			            session->state = FNET_HTTP_STATE_CLOSING;
    #endif                			                
            			            } 
                                
                                }
    #if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/                            
                                /* Parse Header line.*/
                                else
                                {
                                    if(session->request.skip_line == 0)
                                    {
                                        if(*req_buf == 0)
                                        /* === Empty line => End of the request header. ===*/
                                        {
    #if FNET_CFG_HTTP_AUTHENTICATION_BASIC
                                            if(session->response.auth_entry)
                                                /* Send UNAUTHORIZED error.*/
                                                session->response.status.code = FNET_HTTP_STATUS_CODE_UNAUTHORIZED;
                                            else /* Send Data.*/
    #endif                                            
                                                session->response.status.code = FNET_HTTP_STATUS_CODE_OK;

    #if FNET_CFG_HTTP_POST
                                            if(session->request.content_length > 0)
                                            /* RX Entity-Body.*/
                                            {
                                                session->buffer_actual_size = 0;
                                                session->state = FNET_HTTP_STATE_RX; 
                                            }
                                            else
    #endif                                            
                                            /* TX Full-Responce.*/
                                            {
                                                /* Send status line.*/
                                                session->buffer_actual_size = 0;
                                                session->state = FNET_HTTP_STATE_TX; 
                                            }
                                            break;
                                        }
                                        else
                                        /* === Parse header fields. ===*/
                                        {
                                            
                                            FNET_DEBUG_HTTP("HTTP: RX Header: %s", req_buf);                                   
                                            
    #if FNET_CFG_HTTP_AUTHENTICATION_BASIC
                                            /* --- Authorization: ---*/
                                            if (session->response.auth_entry && fnet_strncmp(req_buf, FNET_HTTP_HEADER_FIELD_AUTHORIZATION, sizeof(FNET_HTTP_HEADER_FIELD_AUTHORIZATION)-1) == 0)
                                            /* Authetication is required.*/
                                            {
                                                char *auth_str = &req_buf[sizeof(FNET_HTTP_HEADER_FIELD_AUTHORIZATION)-1];
                                                
                                                /* Validate credentials.*/    
                                                if(fnet_http_auth_validate_credentials(http, auth_str) == FNET_OK)
                                                    session->response.auth_entry = 0; /* Authorization is succesful.*/
                                            }
    #endif                                             
    
    #if FNET_CFG_HTTP_POST                                            
                                            /* --- Content-Length: ---*/ 
                                            if (session->request.method->receive && fnet_strncmp(req_buf, FNET_HTTP_HEADER_FIELD_CONTENT_LENGTH, sizeof(FNET_HTTP_HEADER_FIELD_CONTENT_LENGTH)-1) == 0)
                                            {
                                                char *p;
                                                char *length_str = &req_buf[sizeof(FNET_HTTP_HEADER_FIELD_CONTENT_LENGTH)-1];
                                                
                                                session->request.content_length = (long)fnet_strtoul(length_str,&p,10);
                                            }
    #endif                                            
                                        }
                                    }
                                    /* Line is skiped.*/
                                    else
                                        session->request.skip_line = 0; /* Reset the Skip flag.*/
                                    
                                    session->buffer_actual_size = 0; /* => Parse Next Header line.*/ 
                                }
    #endif/* FNET_CFG_HTTP_VERSION_MAJOR */                            
                            }
                            /* Not whole line received yet.*/
                            else if (session->buffer_actual_size == FNET_HTTP_BUF_SIZE) 
                            /* Buffer is full.*/
                            { 
    #if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
                                if(session->request.method != 0)
                                /* For header, skip the line.*/
                                {
                                    /* Skip line.*/
                                    session->request.skip_line = 1;
                                    session->buffer_actual_size = 0;
                                }
                                else /* Error.*/
                                {
                                    /* Default code = FNET_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR. */ 
                                    session->buffer_actual_size = 0;
                                    session->state = FNET_HTTP_STATE_TX; /* Send error.*/
                                }
                                    
    #else /* HTTP/0.9 */ 
            			        session->state = FNET_HTTP_STATE_CLOSING;
    #endif            			                
                            }
                        }
                        /* No data.*/
                        else if(fnet_timer_get_interval(session->state_time, fnet_timer_ticks()) /* Time out? */
                                      > (FNET_HTTP_WAIT_RX_MS / FNET_TIMER_PERIOD_MS))
                        {
                                session->state = FNET_HTTP_STATE_CLOSING; /*=> CLOSING */
                        }
                        /* else => WAITING REQUEST. */
                    }
                    /* recv() error.*/
                    else  
                    {
                        session->state = FNET_HTTP_STATE_CLOSING; /*=> CLOSING */
                    }
                }
                while ((res > 0) && (session->state == FNET_HTTP_STATE_RX_REQUEST)); /* Till receiving the request header.*/
                break;
    #if FNET_CFG_HTTP_POST
            /*---- RX --------------------------------------------------*/
            case FNET_HTTP_STATE_RX: /* Receive data (Entity-Body). */
                if((res = recv(session->socket_foreign, &session->buffer[session->buffer_actual_size], (int)(FNET_HTTP_BUF_SIZE-session->buffer_actual_size), 0) )!= SOCKET_ERROR)
                {
                    session->buffer_actual_size += res;
                    session->request.content_length -= res;
                    
                    if(res > 0)
                    /* Some Data.*/
                    {
                        session->state_time = fnet_timer_ticks();  /* Reset timeout.*/
                        res = session->request.method->receive(http); 
                        if(fnet_http_status_ok(res) != FNET_OK)
                        {
                            if(res != FNET_ERR)
                                session->response.status.code = (fnet_http_status_code_t)res;
                            else
                                session->response.status.code = FNET_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;    
                            
                            session->request.content_length = 0;
                        }
                        
                        if(session->request.content_length <= 0) /* The last data.*/
                        {
                            session->state = FNET_HTTP_STATE_TX; /* Send data.*/
                        }
                        
                        session->buffer_actual_size = 0;
                    }
                    else
                    /* No Data.*/
                    {
                        if(fnet_timer_get_interval(session->state_time, fnet_timer_ticks())
                              > (FNET_HTTP_WAIT_RX_MS / FNET_TIMER_PERIOD_MS))
                        /* Time out.*/
                        {
                            session->state = FNET_HTTP_STATE_CLOSING; /*=> CLOSING */
                        }
                    }
                }
                else
                /* Socket error.*/
                {
                    session->state = FNET_HTTP_STATE_CLOSING; /*=> CLOSING */
                }
                
                break;            
    #endif /* FNET_CFG_HTTP_POST.*/
            /*---- TX --------------------------------------------------*/
            case FNET_HTTP_STATE_TX: /* Send data. */
                if(fnet_timer_get_interval(session->state_time, fnet_timer_ticks())
                     < (FNET_HTTP_WAIT_TX_MS / FNET_TIMER_PERIOD_MS)) /* Check timeout */
                {
                    int send_size;
                  
                    if(session->buffer_actual_size == session->response.buffer_sent)
                    {
                        /* Reset counters.*/
                        session->buffer_actual_size =0;
                        session->response.buffer_sent = 0;
                        
                        if(session->response.send_eof || session->response.tx_data(http) == FNET_ERR) /* get data for sending */
                        {
                            session->state = FNET_HTTP_STATE_CLOSING; /*=> CLOSING */
                            break;
                        }
                    }
                   
                    send_size = (int)(session->buffer_actual_size - (int)session->response.buffer_sent);

                    if(send_size > http->send_max)
                        send_size = (int)http->send_max;
                    
                    if((res = send(session->socket_foreign, session->buffer
                                  + session->response.buffer_sent, send_size, 0)) != SOCKET_ERROR)
                    {
                        if(res)
                        {
                            FNET_DEBUG_HTTP("HTTP: TX %d bytes.", res);

                            session->state_time = fnet_timer_ticks();              /* reset timeout */
                            session->response.buffer_sent += res;
                        }
                        break; /* => SENDING */ 
                    }
                }

                session->state = FNET_HTTP_STATE_CLOSING; /*=> CLOSING */
                break;
                /*---- CLOSING --------------------------------------------------*/
            case FNET_HTTP_STATE_CLOSING:
                if(session->request.method && session->request.method->close)
                    session->request.method->close(http);

                closesocket(session->socket_foreign);
                session->socket_foreign = SOCKET_INVALID;
                    
                session->state = FNET_HTTP_STATE_LISTENING; /*=> LISTENING */
                break;
            default:
                break;                
        }
    }

    } /*for(sessions)*/
}

/************************************************************************
* NAME: fnet_http_init
*
* DESCRIPTION: Initialization of the HTTP server.
*************************************************************************/
fnet_http_desc_t fnet_http_init( struct fnet_http_params *params )
{
    struct sockaddr         local_addr;
    struct fnet_http_uri    uri;
    int                     i;
    struct fnet_http_if     *http_if = 0;
    const struct linger     linger_option ={1, /*l_onoff*/
                                             4  /*l_linger*/};
    int                     opt_len;                                        
    
    
    if(params == 0 || params->root_path == 0 || params->index_path == 0)
    {
        FNET_DEBUG_HTTP("HTTP: Wrong init parameters.");
        goto ERROR_1;
    }

    /* Try to find free HTTP server. */
#if (FNET_CFG_HTTP_MAX > 1)
    for(i=0; i<FNET_CFG_HTTP_MAX; i++)
    {
        if(http_if_list[i].enabled == FNET_FALSE)
        {
            http_if = &http_if_list[i]; 
            break
        }
    }
#else
    if(http_if_list[0].enabled == FNET_FALSE)
        http_if = &http_if_list[0];
#endif

    /* Is HTTP server already initialized. */
    if(http_if == 0)
    {
        FNET_DEBUG_HTTP("HTTP: No free HTTP Server.");
        goto ERROR_1;
    }

#if FNET_CFG_HTTP_SSI
    http_if->ssi.ssi_table = params->ssi_table;
#endif    

#if FNET_CFG_HTTP_CGI    
    http_if->cgi_table = params->cgi_table;
#endif

#if FNET_CFG_HTTP_AUTHENTICATION_BASIC
    http_if->auth_table = params->auth_table;
#endif

#if FNET_CFG_HTTP_POST
    http_if->post_table = params->post_table;
#endif
    
    local_addr = params->address;
    
    if(local_addr.sa_port == 0)
        local_addr.sa_port = FNET_CFG_HTTP_PORT; /* Aply the default port.*/
    
    if(local_addr.sa_family == AF_UNSPEC)
        local_addr.sa_family = AF_SUPPORTED; /* Asign supported families.*/

    /* Create listen socket */
    if((http_if->socket_listen = socket(local_addr.sa_family, SOCK_STREAM, 0)) == SOCKET_INVALID)
    {
        FNET_DEBUG_HTTP("HTTP: Socket creation error.");
        goto ERROR_1;
    }
    
    /* Bind.*/
    if(bind(http_if->socket_listen, &local_addr, sizeof(local_addr)) == SOCKET_ERROR)
    {
        FNET_DEBUG_HTTP("HTTP: Socket bind error.");
        goto ERROR_2;
    }
    
    /* Set socket options.*/
    setsockopt (http_if->socket_listen, SOL_SOCKET, SO_LINGER, 
                                    (char *)  &linger_option, sizeof(linger_option));   
    /* Get size of the socket send buffer */
    opt_len = sizeof(http_if->send_max);
    if(getsockopt(http_if->socket_listen, SOL_SOCKET, SO_SNDBUF, 
                                (char *) &http_if->send_max, &opt_len) == SOCKET_ERROR)
    {
        FNET_DEBUG_HTTP("HTTP: Socket getsockopt() error.");
        goto ERROR_2;
    }

    /* Listen.*/
    if(listen(http_if->socket_listen, FNET_HTTP_BACKLOG_MAX) == SOCKET_ERROR)
    {
        FNET_DEBUG_HTTP("HTTP: Socket listen error.");
        goto ERROR_2;
    }
    
    /* Open root dir. */
    http_if->root_dir = fnet_fs_opendir(params->root_path);
    if(http_if->root_dir == 0)
    {
        FNET_DEBUG_HTTP("HTTP: Root directory is failed.");
        goto ERROR_2;
    }

    /* Open index file. */
    http_if->index_file = fnet_fs_fopen_re(params->index_path,"r", http_if->root_dir);
    if(http_if->index_file == 0)
    {
        FNET_DEBUG_HTTP("HTTP: Root directory is failed.");
        goto ERROR_3;
    }
    
    fnet_http_uri_parse(params->index_path, &uri);
    http_if->index_file_handler = fnet_http_find_handler(http_if, &uri); /* Find Handler for the index file. */
    
#if FNET_CFG_HTTP_VERSION_MAJOR    
    http_if->index_file_content_type = fnet_http_find_content_type(http_if, &uri); /* Find Content-Type for the index file. */
#endif    

    /* Init session parameters.*/
    for(i=0; i<FNET_CFG_HTTP_SESSION_MAX; i++) 
    {
        struct fnet_http_session_if   *session = &http_if->session[i];
        
        session->socket_foreign = SOCKET_INVALID;
      
        session->state = FNET_HTTP_STATE_LISTENING;
    }

    http_if->service_descriptor = fnet_poll_service_register(fnet_http_state_machine, (void *) http_if);
    if(http_if->service_descriptor == (fnet_poll_desc_t)FNET_ERR)
    {
        FNET_DEBUG_HTTP("HTTP: Service registration error.");
        goto ERROR_4;
    }

    http_if->session_active = FNET_NULL;
    http_if->enabled = FNET_TRUE;

    return (fnet_http_desc_t)http_if;
    
ERROR_4:
    fnet_fs_fclose(http_if->index_file);

ERROR_3:
    fnet_fs_closedir(http_if->root_dir);

ERROR_2:
    closesocket(http_if->socket_listen);

ERROR_1:
    return FNET_ERR;
}

/************************************************************************
* NAME: fnet_http_release
*
* DESCRIPTION: HTTP server release.
************************************************************************/
void fnet_http_release(fnet_http_desc_t desc)
{
    struct fnet_http_if     *http_if = (struct fnet_http_if *) desc;
    int                     i;
    
    if(http_if && (http_if->enabled == FNET_TRUE))
    {
        for(i=0; i<FNET_CFG_HTTP_SESSION_MAX; i++) 
        {
            struct fnet_http_session_if   *session = &http_if->session[i];
            
            closesocket(session->socket_foreign);        
            session->socket_foreign = SOCKET_INVALID;
            session->state = FNET_HTTP_STATE_DISABLED;
            fnet_fs_fclose(session->send_param.file_desc);
        }


        fnet_fs_fclose(http_if->index_file);

        fnet_fs_closedir(http_if->root_dir);
          
        closesocket(http_if->socket_listen);
        fnet_poll_service_unregister(http_if->service_descriptor); /* Delete service.*/

        http_if->enabled = FNET_FALSE;
    }
}

/************************************************************************
* NAME: fnet_http_enabled
*
* DESCRIPTION: This function returns FNET_TRUE if the HTTP server 
*              is enabled/initialised.
************************************************************************/
int fnet_http_enabled(fnet_http_desc_t desc)
{
    struct fnet_http_if     *http_if = (struct fnet_http_if *) desc;
    int                     result;
    
    if(http_if)
        result = http_if->enabled;
    else
        result = FNET_FALSE;    
    
    return result;
}


#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/

/************************************************************************
* NAME: fnet_http_tx_status_line
*
* DESCRIPTION: 
************************************************************************/
static int fnet_http_tx_status_line (struct fnet_http_if * http)
{
    struct fnet_http_status     *status ;
    unsigned long               result = 0;
    unsigned long               result_state;
    struct fnet_http_session_if *session =  http->session_active;
    
    session->response.send_eof = 0; /* Data to be sent.*/
   
    do
    {
        result_state = 0;
        
        switch(session->response.status_line_state)
        {
            case 0:
                if(session->response.status.code == 0)
                {   /* If the code was not found, produce a 501 internal server error */
                    session->response.status.code = FNET_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
                    session->response.status.phrase = 0;
                }
                
                if(session->response.status.phrase == 0) /* If no phrase is defined.*/
                {
                    for(status = fnet_http_status_list; status->code > 0; status++) /* Find phrase.*/
                    {
                        if (status->code == session->response.status.code)
                        {
                            break;
                        }
                    }
                    session->response.status.phrase = status->phrase; /* If no phrase is fond it will pont to empty string.*/ 
                }
                 
                /* Print status line.*/
                result_state = (unsigned long)fnet_snprintf(session->buffer, FNET_HTTP_BUF_SIZE, "%s%d.%d %d %s%s", FNET_HTTP_VERSION_HEADER, session->response.version.major, session->response.version.minor,
                                                                        session->response.status.code, session->response.status.phrase, 
                                                                                   "\r\n");
                break;                                                                         
            /* Add HTTP header fields where applicable. */
            case 1:
#if FNET_CFG_HTTP_AUTHENTICATION_BASIC            
                /* Authentificate.*/
                if(session->response.status.code == FNET_HTTP_STATUS_CODE_UNAUTHORIZED)
                {
                    result_state = (unsigned long)fnet_snprintf(&session->buffer[result], (FNET_HTTP_BUF_SIZE - result), "%s ", FNET_HTTP_HEADER_FIELD_AUTHENTICATE);
                    result_state += fnet_http_auth_generate_challenge(http, &session->buffer[result+result_state], FNET_HTTP_BUF_SIZE - (result + result_state));
                    session->response.content_length = -1; /* No content length .*/
                }
#endif                
                break;
            case 2:
                /* Content-Length */
                if(session->response.content_length >= 0)
                {
                    result_state = (unsigned long)fnet_snprintf(&session->buffer[result], (FNET_HTTP_BUF_SIZE - result), "%s %d%s", FNET_HTTP_HEADER_FIELD_CONTENT_LENGTH, session->response.content_length, "\r\n");
                }
                break; 
            case 3:
        	    /* Add MIME Content Type field, based on file extension.*/
			    if(session->response.send_file_content_type)
			    {
			        result_state = (unsigned long)fnet_snprintf(&session->buffer[result], 
			                        (FNET_HTTP_BUF_SIZE - result), "%s %s%s",
				                    FNET_HTTP_HEADER_FIELD_CONTENT_TYPE, session->response.send_file_content_type->content_type,"\r\n");
                }
	            break;
            case 4:
                /*Final CRLF.*/
                result_state = (unsigned long)fnet_snprintf(&session->buffer[result], (FNET_HTTP_BUF_SIZE - result),"%s","\r\n");
            
                if(session->response.status.code != FNET_HTTP_STATUS_CODE_OK)
                    session->response.send_eof = 1; /* Only sataus (without data).*/
                
                session->response.tx_data = session->request.method->send;
                break;
        }
        
        if((result+result_state) == (FNET_HTTP_BUF_SIZE-1)) /* Buffer overload.*/
        {
            if(result == 0)
            {
                fnet_sprintf(&session->buffer[FNET_HTTP_BUF_SIZE-2], "%s","\r\n");   /* Finish line.*/
                session->response.status_line_state++;
            }
            /* else. Do not send last state data.*/
            break; /* Send data.*/  
        }
        else
        {
            result += result_state;
            session->response.status_line_state++;
        }
    }
    while (session->response.status_line_state <= 4);
    
    session->buffer_actual_size =  result;
    FNET_DEBUG_HTTP("HTTP: TX Status: %s", http->buffer); 
    
    return FNET_OK;
}

#endif /* FNET_CFG_HTTP_VERSION_MAJOR */

/************************************************************************
* NAME: fnet_http_find_handler
*
* DESCRIPTION: 
************************************************************************/
const struct fnet_http_file_handler * fnet_http_find_handler (struct fnet_http_if * http, struct fnet_http_uri * uri)
{
    const struct fnet_http_file_handler * result = &fnet_http_default_handler;
    const struct fnet_http_file_handler **handler = &fnet_http_file_handler_list[0];

    if(uri)
    {
        if (!fnet_strcmp(uri->path, "/")) /* Default index file. */
        {
            result = http->index_file_handler;
        }    
        else
        {
            while(*handler)
            {
                if ( !fnet_strcmp( uri->extension, (*handler)->file_extension) ) 
                {				 
                    result = *handler;
                    break;
                }
                handler++;
            }
        }
            
    } 
   
    return result;
}

/************************************************************************
* NAME: fnet_http_find_content_type
*
* DESCRIPTION: 
************************************************************************/
#if FNET_CFG_HTTP_VERSION_MAJOR
const struct fnet_http_content_type *fnet_http_find_content_type (struct fnet_http_if * http, struct fnet_http_uri * uri)
{
    const struct fnet_http_content_type **content_type = &fnet_http_content_type_list[0];
    const struct fnet_http_content_type *result = FNET_NULL;    

    if(uri)
    {
        if (!fnet_strcmp(uri->path, "/")) /* Default index file. */
        {
            result = http->index_file_content_type;
        }    
        else
        {
            while(*content_type)
            {
                if ( !fnet_strcmp(uri->extension, (*content_type)->file_extension)) 
                {				 
                    result = *content_type;
                    break;
                }
                content_type++;
            }
        }
    } 
   
    return result;
}
#endif /* FNET_CFG_HTTP_VERSION_MAJOR */

/************************************************************************
* NAME: fnet_http_default_send
*
* DESCRIPTION: 
************************************************************************/
int fnet_http_default_handle (struct fnet_http_if * http, struct fnet_http_uri * uri)
{
    int                         result;
    struct fnet_http_session_if *session =  http->session_active;
    
    if (!fnet_strcmp(uri->path, "/")) /* Default index file */
    {
        fnet_fs_rewind(http->index_file);
        session->send_param.file_desc = http->index_file;
    }    
    else
    {
        session->send_param.file_desc = fnet_fs_fopen_re(uri->path,"r", http->root_dir);
    }
		                            
    if (session->send_param.file_desc)
    {
#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
        {
            struct fnet_fs_dirent dirent; 
            fnet_fs_finfo(session->send_param.file_desc, &dirent);
            session->response.content_length = (long)dirent.d_size;
        }
#endif        
        result = FNET_OK;
    }
    else
    {
#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
        session->response.status.code = FNET_HTTP_STATUS_CODE_NOT_FOUND;
#endif        
        result = FNET_ERR;
    }
    
    return result;
}

/************************************************************************
* NAME: fnet_http_default_send
*
* DESCRIPTION: 
************************************************************************/
unsigned long fnet_http_default_send (struct fnet_http_if * http)
{
    struct fnet_http_session_if *session =  http->session_active;

    return fnet_fs_fread(session->buffer, sizeof(session->buffer), session->send_param.file_desc);
}

/************************************************************************
* NAME: fnet_http_default_close
*
* DESCRIPTION: 
************************************************************************/
void fnet_http_default_close (struct fnet_http_if * http)
{
    struct fnet_http_session_if *session =  http->session_active;

    if(session->send_param.file_desc != http->index_file)
        fnet_fs_fclose(session->send_param.file_desc); /* Close file */ 
}

/************************************************************************
* NAME: fnet_http_query_unencode
*
* DESCRIPTION: 
************************************************************************/
void fnet_http_query_unencode(char * dest, char * src)
{
    if(dest && src)
    {
        for(; *src != 0; src++, dest++)
            if(*src == '+')
                *dest = ' ';
            else if(*src == '%')
            {
                int i;
                char val = 0;
                
                for(i=0; (i<2) && (*src != 0) ; i++)
                {
                    src++;
                    if((*src >= '0') && (*src <= '9'))
                    {
                        val = (char)((val << 4) + (*src - '0'));
                        continue;
                    }
                    else if(((*src >= 'a') && (*src <= 'f')) || ((*src >= 'A') && (*src <= 'F')))
                    {
                        val = (char)((val << 4) + (*src + 10 - (((*src >= 'a') && (*src <= 'f')) ? 'a' : 'A')));
                        continue;
                    }
                    break;
                }
                if(i==2)
                    *dest = val;
                else
                    *dest = '?';
            }     
            else
                *dest = *src;

         *dest = '\0';
     }
}


/************************************************************************
* NAME: fnet_http_uri_parse
*
* DESCRIPTION: 
*   Returns pointer to the end of URI str.
************************************************************************/
char *fnet_http_uri_parse(char * in_str, struct fnet_http_uri * uri)
{
    char * cur = in_str;
    
    /* rel_uri       = [ path ] [ "?" query ]*/
    if(cur && uri) 
    {
        /*Parse the Request-URI. */
        /*Extract file name. */
        
        /* Ignore any initial spaces */
        while (*cur == ' ') 
            cur++;
        
        uri->path = cur;
        uri->query = 0;
        uri->extension = 0;
                            
        /* Find end of the file name. */
        while( *cur != '\0') 
        {
            if (*cur == ' ')	/* Path end is found */
            { 
                *cur = '\0';                    /* Mark path end */
                break;   
                
            }
            else if(*cur == '?')                 /* Query is found */
            {
                    char *end_query;
                     *cur = '\0';               /* Mark path end */
                    
                    uri->query = cur + 1;       /* Point to the next symbol after '?' */ 
                    
                    /* Find end of query.*/
                    end_query = fnet_strchr( uri->query, ' ');
                    if(end_query)
                    {
                        *end_query = '\0';      /* Mark query end */
                        cur = end_query;
                        break;
                    }
            }
            cur ++;
        }
        
        uri->extension = fnet_strrchr(uri->path, '.'); /* Find file extension. */
        
        if( uri->query == 0)        /* No query.*/
            uri->query = cur;       /* Point to the blank string. */        
        
        if( uri->extension == 0)    /* No extension.*/
            uri->extension = cur;   /* Point to the blank string. */
        else
            uri->extension ++;      /* Skip the point character. */
    }
    
    return cur;
}


#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
/************************************************************************
* NAME: fnet_http_version_parse
*
* DESCRIPTION: 
************************************************************************/
static void fnet_http_version_parse(char * in_str, struct fnet_http_version * version)
{
    char *cur = in_str;
    char *ptr;
    char *point_ptr;
    
    /* rel_uri       = [ path ] [ "?" query ]*/
    if(cur && version) 
    {
        /* Ignore any initial spaces */
        while (*cur == ' ') 
            cur++;
        
        /* Find "HTTP/"*/
        if((cur = fnet_strstr( cur, FNET_HTTP_VERSION_HEADER )) !=0)
        {
            cur += sizeof(FNET_HTTP_VERSION_HEADER)-1;
            
            /* Find '.'*/
            if((point_ptr = fnet_strchr(cur, '.')) != 0)
            {
                *point_ptr = 0; /* Split numbers.*/
                
                /*Major number*/
                version->major = (char)fnet_strtoul (cur, &ptr, 10);
                if (!((version->major == 0) && (ptr == cur)))
                {
                    cur = point_ptr + 1;
                    
                    /* Minor number.*/
                    version->minor = (char)fnet_strtoul (cur, &ptr, 10);
                    if (!((version->minor == 0) && (ptr == cur)))
                        goto EXIT;    /* Exit.*/
                }
            }
        }
        
        /* No version info found.*/
        version->major = 0;
        version->minor = 9;
    }

EXIT:
    return;  
}

/************************************************************************
* NAME: fnet_http_status_ok
*
* DESCRIPTION: 
************************************************************************/
static int fnet_http_status_ok(int status)
{
    int result;
    
    if((status == FNET_OK) || ((status/100) == 2) /* Successful 2xx. */)
        result = FNET_OK;
    else
        result = FNET_ERR;
    
    return result;
}
#endif /* FNET_CFG_HTTP_VERSION_MAJOR */



#endif

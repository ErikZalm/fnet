/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2009 by Andrey Butok. Freescale Semiconductor, Inc.
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
**********************************************************************/
/*!
*
* @file fnet_http_prv.h
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.15.0
*
* @brief Private. FNET HTTP Server API.
*
***************************************************************************/

#ifndef _FNET_HTTP_PRV_H_

#define _FNET_HTTP_PRV_H_

#include "fnet_config.h"


#if FNET_CFG_HTTP && FNET_CFG_FS


#include "fnet.h"
#include "fnet_fs.h"
#include "fnet_http.h"
#if FNET_CFG_HTTP_SSI
#include "fnet_http_ssi_prv.h"
#endif

#if FNET_CFG_HTTP_POST
#include "fnet_http_post.h"
#endif

/* Minimum size of the internal buffer */
#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
    #define FNET_HTTP_BUF_SIZE_MIN  (20)
#else /* HTTP/0.9*/
    #define FNET_HTTP_BUF_SIZE_MIN  (10)
#endif

/* Minimum buffer size protection.*/
#if FNET_CFG_HTTP_REQUEST_SIZE_MAX > FNET_HTTP_BUF_SIZE_MIN
    #define FNET_HTTP_BUF_SIZE  FNET_CFG_HTTP_REQUEST_SIZE_MAX
#else
    #define FNET_HTTP_BUF_SIZE  FNET_HTTP_BUF_SIZE_MIN
#endif

#if FNET_CFG_DEBUG_HTTP    
    #define FNET_DEBUG_HTTP   FNET_DEBUG
#else
    #define FNET_DEBUG_HTTP(...)
#endif

#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/

/************************************************************************
*    HTTP 1.0 Reason-Phrase definitions.
*    RFC1945: The Reason-Phrase is intended to give a short textual 
*             description of the Status-Code.
*************************************************************************/
#define FNET_HTTP_REASON_PHRASE_OK                      "OK"
#define FNET_HTTP_REASON_PHRASE_CREATED                 "Created"
#define FNET_HTTP_REASON_PHRASE_ACCEPTED                "Accepted"
#define FNET_HTTP_REASON_PHRASE_NO_CONTENT              "No Content"
#define FNET_HTTP_REASON_PHRASE_MOVED_PERMANENTLY       "Moved Permanently"
#define FNET_HTTP_REASON_PHRASE_MOVED_TEMPORARILY       "Moved Temporarily"
#define FNET_HTTP_REASON_PHRASE_NOT_MODIFIED            "Not Modified"
#define FNET_HTTP_REASON_PHRASE_BAD_REQUEST             "Bad Request"
#define FNET_HTTP_REASON_PHRASE_UNAUTHORIZED            "Unauthorized"
#define FNET_HTTP_REASON_PHRASE_FORBIDDEN               "Forbidden"
#define FNET_HTTP_REASON_PHRASE_NOT_FOUND               "Not Found"
#define FNET_HTTP_REASON_PHRASE_INTERNAL_SERVER_ERROR   "Internal Server Error"
#define FNET_HTTP_REASON_PHRASE_NOT_IMPLEMENTED         "Not Implemented"
#define FNET_HTTP_REASON_PHRASE_BAD_GATEWAY             "Bad Gateway"
#define FNET_HTTP_REASON_PHRASE_SERVICE_UNAVAILABLE     "Service Unavailable"

/************************************************************************
*    HTTP response status structure.
*************************************************************************/
struct fnet_http_status
{
    fnet_http_status_code_t   code;     /* Status-Code.*/
    char                      *phrase;  /* An optional Reason-Phrase. May be set to NULL */
};

/************************************************************************
*    HTTP version structure.
*************************************************************************/
struct fnet_http_version
{
    unsigned char major; 
    unsigned char minor;
};

#endif /* FNET_CFG_HTTP_VERSION_MAJOR */


/******************************************************************************
 * HTTP server states.
 ******************************************************************************/
typedef enum
{
    FNET_HTTP_STATE_DISABLED = 0,       /**< @brief HTTP server service is 
                                         * not initialized.
                                         */
    FNET_HTTP_STATE_LISTENING = 1,      /**< @brief HTTP server is listening 
                                         * for client socket.
                                         */
    FNET_HTTP_STATE_RX_REQUEST = 2,     /**< @brief HTTP server is waiting or receiving  
                                         * a HTTP request.
                                         */
#if FNET_CFG_HTTP_POST 
    FNET_HTTP_STATE_RX = 3,             /**< @brief HTTP server is receiving the 
                                         * Entity-Body of a HTTP request.
                                         */
#endif
    
    FNET_HTTP_STATE_TX = 4,             /**< @brief HTTP server is sending a 
                                         * response to a client.
                                         */
    FNET_HTTP_STATE_CLOSING = 5         /**< @brief HTTP server is closing 
                                         * the socket connection.
                                         */
} fnet_http_state_t;



struct fnet_http_if;

/************************************************************************
*    HTTP response parameters structure.
*************************************************************************/
struct fnet_http_response
{
    const struct fnet_http_file_handler     *send_file_handler;
    int                                     (*tx_data)(struct fnet_http_if * http); /* TX state handler.*/
    char                                    send_eof;                   /* Optional EOF flag. It means nomore data for send*/
    unsigned long                           buffer_sent;                /* A number of bytes were sent.*/
    int                                     status_line_state;
    long                                    cookie;
    
#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/    
    const struct fnet_http_content_type     *send_file_content_type; /* MIME Content-Type.*/
    struct fnet_http_status                 status;             /* Status of the response.*/
    struct fnet_http_version                version;            /* Protocol version used for current request.*/
    long                                    content_length;     /* The total size of the data to send (is -1 if unknown).*/
#endif
    
#if FNET_CFG_HTTP_AUTHENTICATION_BASIC    
    const struct fnet_http_auth             *auth_entry;
    const struct fnet_http_auth_scheme      *auth_scheme;
#endif 
};

/************************************************************************
*    Parsed URI (Uniform Resource Identifier) structure.
*************************************************************************/
struct fnet_http_uri
{
	char * path;            /* File path (with file extension). */
	char * extension;       /* File extension. */
	char * query;           /* Optional query string. */
};

/************************************************************************
*    HTTP request parameters structure.
*************************************************************************/
struct fnet_http_request
{
    const struct fnet_http_method   *method;
    struct fnet_http_uri            uri;
    long                            content_length;
#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/ 
    int                             skip_line; 
#endif           
};

/************************************************************************
*    HTTP session control structure.
*************************************************************************/
struct fnet_http_session_if
{
    fnet_http_state_t           state;                          /* Current state.*/
    unsigned long               state_time;                     /* Start time used by the state machine for timeout calculation.*/
    SOCKET                      socket_foreign;                 /* Foreign socket.*/
    char                        buffer[FNET_HTTP_BUF_SIZE+1];   /* Receive/Transmit buffer */
    unsigned long               buffer_actual_size;             /* Size of the actual data in the buffer.*/
    union 
    {
        FNET_FS_FILE file_desc;
        void * data_ptr;
    } send_param;
    struct fnet_http_response   response;                       /* Holds the accumulated data for the HTTP 1.0 response header */
    struct fnet_http_request    request; 
}; 

/************************************************************************
*    HTTP interface control structure.
*************************************************************************/
struct fnet_http_if
{
    SOCKET              socket_listen;              /* Listening socket.*/
    fnet_poll_desc_t    service_descriptor;         /* Descriptor of polling service.*/
    int                 enabled;
    FNET_FS_DIR         root_dir;
    FNET_FS_FILE        index_file;
    const struct fnet_http_file_handler     *index_file_handler;
    unsigned long       send_max;                 /* Socket maximum tx buffer.*/

    struct fnet_http_session_if *session_active;
    struct fnet_http_session_if session[FNET_CFG_HTTP_SESSION_MAX];

#if FNET_CFG_HTTP_VERSION_MAJOR
    const struct fnet_http_content_type *index_file_content_type; /* MIME Content-Type of Index File.*/
#endif        

#if FNET_CFG_HTTP_SSI    
    struct fnet_http_ssi_if     ssi;
#endif

#if FNET_CFG_HTTP_CGI    
    const struct fnet_http_cgi *cgi_table;
#endif 

#if FNET_CFG_HTTP_AUTHENTICATION_BASIC
    const struct fnet_http_auth *auth_table;	        
#endif

#if FNET_CFG_HTTP_POST
    const struct fnet_http_post *post_table;
#endif
}; 

/************************************************************************
*    HTTP request handler structure.
*************************************************************************/
struct fnet_http_method
{
	const char  * token;	            /* Method token, which will identify protocol.
	                                    * It indicates the method to be performed on the resource identified 
	                                    * by the Request-URI.*/
	int         (* handle)(struct fnet_http_if * http, struct fnet_http_uri * uri);
    int         (* receive)(struct fnet_http_if * http);
	int         (* send)(struct fnet_http_if * http);
	void        (* close)(struct fnet_http_if * http);
};

/************************************************************************
*    File handler structure.
*************************************************************************/
struct fnet_http_file_handler
{
	const char *    file_extension;	      /* File extension */
	int             (*file_handle)(struct fnet_http_if * http, struct fnet_http_uri * uri);
	unsigned long   (*file_send)(struct fnet_http_if * http);
	void            (*file_close)(struct fnet_http_if * http);
};

/************************************************************************
*    File content type structure.
*************************************************************************/
struct fnet_http_content_type
{
	const char *    file_extension;	      /* File extension */
    const char *    content_type;	      /* Content type string */
};

extern const struct fnet_http_file_handler fnet_http_cgi_handler;
extern const struct fnet_http_method fnet_http_method_get;
#if FNET_CFG_HTTP_POST
extern const struct fnet_http_method fnet_http_method_post;
#endif
int fnet_http_default_handle (struct fnet_http_if * http, struct fnet_http_uri * uri);
unsigned long fnet_http_default_send (struct fnet_http_if * http);
void fnet_http_default_close (struct fnet_http_if * http);
char *fnet_http_uri_parse(char * in_str, struct fnet_http_uri * uri);
const struct fnet_http_file_handler * fnet_http_find_handler(struct fnet_http_if * http, struct fnet_http_uri * uri);
const struct fnet_http_content_type * fnet_http_find_content_type(struct fnet_http_if * http, struct fnet_http_uri * uri);

#endif /* FNET_CFG_HTTP && FNET_CFG_FS */

#endif /* _FNET_HTTP_PRV_H_ */

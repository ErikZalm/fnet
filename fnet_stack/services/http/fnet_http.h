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
* @file fnet_http.h
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.32.0
*
* @brief FNET HTTP Server API.
*
***************************************************************************/

#ifndef _FNET_HTTP_H_

#define _FNET_HTTP_H_

#include "fnet_config.h"


#if FNET_CFG_HTTP || defined(__DOXYGEN__)

#include "fnet.h"
#include "fnet_poll.h"
#include "fnet_http_ssi.h"
#include "fnet_http_cgi.h"
#include "fnet_http_auth.h"
#include "fnet_http_post.h"


/*! @addtogroup fnet_http
* The current version of the FNET HTTP Server supports:
* - HTTP/0.9 or HTTP/1.0 protocol responces. 
* - GET and POST (enabled by @ref FNET_CFG_HTTP_POST) HTTP requests. 
* - Server Side Includes (SSI). @n
*   SSI directives have the following format: 
*   @code 
*   <!--#command [parameter(s)]--> 
*   @endcode
* - Common Gateway Interface (CGI).
* - Basic Access Authentication (enabled by @ref FNET_CFG_HTTP_AUTHENTICATION_BASIC).
* - FNET File System Interface.
* 
* @n
* After the FNET HTTP server is initialized by calling the @ref fnet_http_init() 
* function, the user application should call the main service polling function  
* @ref fnet_poll_services() periodically in background. @n
* @n
* For HTTP server usage example, refer to FNET demo application source code.@n
* @n
* Configuration parameters:
* - @ref FNET_CFG_HTTP
* - @ref FNET_CFG_HTTP_VERSION_MAJOR  
* - @ref FNET_CFG_HTTP_MAX
* - @ref FNET_CFG_HTTP_SESSION_MAX  
* - @ref FNET_CFG_HTTP_SSI 
* - @ref FNET_CFG_HTTP_CGI 
* - @ref FNET_CFG_HTTP_POST
* - @ref FNET_CFG_HTTP_AUTHENTICATION_BASIC 
* - @ref FNET_CFG_HTTP_PORT 
* - @ref FNET_CFG_HTTP_REQUEST_SIZE_MAX 
*/
/*! @{ */

/**************************************************************************/ /*!
 * @def FNET_HTTP_VERSION_MAJOR
 * @brief The major version number of HTTP protocol supported by the HTTP server.
 ******************************************************************************/
#if FNET_CFG_HTTP_VERSION_MAJOR /*HTTP/1.x*/
    #define FNET_HTTP_VERSION_MAJOR     (1)
#else   /*HTTP/0.9*/
    #define FNET_HTTP_VERSION_MAJOR     (0)
#endif

/**************************************************************************/ /*!
 * @def FNET_HTTP_VERSION_MINOR
 * @brief The minor version number of HTTP protocol supported by the HTTP server.
 ******************************************************************************/
#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
    #define FNET_HTTP_VERSION_MINOR     (0)
#else   /*HTTP/0.9*/
    #define FNET_HTTP_VERSION_MINOR     (9)
#endif


#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/ || defined(__DOXYGEN__)

/********************************************************************/ /*!
* @brief HTTP/1.0 Status-Code definitions according to RFC1945.
*
* Can be used as return value for @ref fnet_http_cgi_handle_t(), 
* @ref fnet_http_post_handle_t() and @ref fnet_http_post_receive_t() 
* call-back functions.
* @see fnet_http_cgi_handle_t(), fnet_http_post_handle_t(), fnet_http_post_receive_t() 
*************************************************************************/
typedef enum
{
    FNET_HTTP_STATUS_CODE_NONE                  = 0,    /**< @brief NONE.@n 
                                                        * Status code is not defined.
                                                        */
    FNET_HTTP_STATUS_CODE_OK                    = 200,  /**< @brief OK.@n 
                                                        * The request has succeeded.
                                                        */
    FNET_HTTP_STATUS_CODE_CREATED               = 201,  /**< @brief Created.@n 
                                                        * The request has been fulfilled 
                                                        * and resulted in a new resource 
                                                        * being created.
                                                        */
    FNET_HTTP_STATUS_CODE_ACCEPTED              = 202,  /**< @brief Accepted.@n 
                                                        * The request has been accepted 
                                                        * for processing, but the processing
                                                        * has not been completed.
                                                        */
    FNET_HTTP_STATUS_CODE_NO_CONTENT            = 204,  /**< @brief No Content.@n 
                                                        * The server has fulfilled the request 
                                                        * but there is no new information to 
                                                        * send back.
                                                        */
    FNET_HTTP_STATUS_CODE_MOVED_PERMANENTLY     = 301,  /**< @brief Moved Permanently.@n 
                                                        * The requested resource has been 
                                                        * assigned a new permanent URL and
                                                        * any future references to 
                                                        * this resource should be 
                                                        * done using that URL.@n
                                                        * It's not supported by the FNET HTTP Server.
                                                        */
    FNET_HTTP_STATUS_CODE_MOVED_TEMPORARILY     = 302,  /**< @brief Moved Temporarily.@n 
                                                        * The requested resource resides 
                                                        * temporarily under a different URL.@n
                                                        * It's not supported by the FNET HTTP Server.
                                                        */
    FNET_HTTP_STATUS_CODE_NOT_MODIFIED          = 304,  /**< @brief Not Modified.@n 
                                                        * If the client has performed a 
                                                        * conditional GET request and access is
                                                        * allowed, but the document has not been 
                                                        * modified since the date and time specified 
                                                        * in the If-Modified-Since field, the server 
                                                        * must respond with this status code and 
                                                        * not send an Entity-Body to the client.@n
                                                        * It's not supported by the FNET HTTP Server.
                                                        */
    FNET_HTTP_STATUS_CODE_BAD_REQUEST           = 400,  /**< @brief Bad Request.@n 
                                                        * The request could not be understood by 
                                                        * the server due to malformed syntax.
                                                        */
    FNET_HTTP_STATUS_CODE_UNAUTHORIZED          = 401,  /**< @brief Unauthorized.@n 
                                                        * The request requires user authentication.
                                                        */
    FNET_HTTP_STATUS_CODE_FORBIDDEN             = 403,  /**< @brief Forbidden.@n 
                                                        * The server understood the request, but is 
                                                        * refusing to fulfill it.
                                                        */
    FNET_HTTP_STATUS_CODE_NOT_FOUND             = 404,  /**< @brief Not Found.@n 
                                                        * The server has not found anything matching 
                                                        * the Request-URI.
                                                        */
    FNET_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR = 500,  /**< @brief Internal Server Error.@n 
                                                        * The server encountered an unexpected condition 
                                                        * which prevented it from fulfilling the request.
                                                        */
    FNET_HTTP_STATUS_CODE_NOT_IMPLEMENTED       = 501,  /**< @brief Not Implemented.@n 
                                                        * The server does not support the functionality 
                                                        * required to fulfill the request.
                                                        */
    FNET_HTTP_STATUS_CODE_BAD_GATEWAY           = 502,  /**< @brief Bad Gateway.@n 
                                                        * The server, while acting as a gateway or proxy, 
                                                        * received an invalid response from the upstream 
                                                        * server it accessed in attempting to fulfill the request.
                                                        */
    FNET_HTTP_STATUS_CODE_SERVICE_UNAVAILABLE   = 503   /**< @brief Service Unavailable.@n 
                                                        * The server is currently unable to handle the 
                                                        * request due to a temporary overloading or 
                                                        * maintenance of the server.
                                                        */

}fnet_http_status_code_t;
#endif


/**************************************************************************/ /*!
 * @brief Input parameters for @ref fnet_http_init().
 ******************************************************************************/
struct fnet_http_params
{
    char * root_path;           /**< @brief Server root-directory path (null-terminated string). */
    char * index_path;          /**< @brief Index file path (null-terminated string). @n
                                 *   It's relative to the @c root_path.*/
    struct sockaddr address;    /**< @brief Server socket address. @n
                                 * If server IP address is set to @c 0s, the server will listen to all current network interfaces. @n
                                 * If server address family is set to @c 0, it will be assigned to @ref AF_SUPPORTED. @n
                                 * If server port number is set to @c 0, it will be assigned to the default port number defined by @ref FNET_CFG_HTTP_PORT.*/
#if FNET_CFG_HTTP_SSI || defined(__DOXYGEN__)
    const struct fnet_http_ssi *ssi_table;      /**< @brief Pointer to the optional
                                                 * SSI callback function table. */
#endif
#if FNET_CFG_HTTP_CGI || defined(__DOXYGEN__)    
    const struct fnet_http_cgi *cgi_table;      /**< @brief Pointer to the optional
                                                 * CGI callback function table. */
#endif
#if FNET_CFG_HTTP_AUTHENTICATION_BASIC || defined(__DOXYGEN__)
    const struct fnet_http_auth  *auth_table;   /**< @brief Pointer to the optional
                                                 * HTTP Access Authentification table. */	        
#endif
#if FNET_CFG_HTTP_POST || defined(__DOXYGEN__)
    const struct fnet_http_post *post_table;    /**< @brief Pointer to the optional
                                                 * POST callback function table. */
#endif
};

/**************************************************************************/ /*!
 * @brief HTTP server descriptor.
 * @see fnet_http_init()
 ******************************************************************************/
typedef long fnet_http_desc_t;

/***************************************************************************/ /*!
 *
 * @brief    Initializes the HTTP Server service.
 *
 * @param params     Initialization parameters defined by @ref fnet_http_params.
 *
 * @return This function returns:
 *   - HTTP server descriptor if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_http_release()
 *
 ******************************************************************************
 *
 * This function initializes the HTTP server service. It allocates all
 * resources needed, and registers the HTTP server service in the polling list.@n
 * After the initialization, the user application should call the main polling 
 * function  @ref fnet_poll_services() periodically to run the HTTP server in background.
 *
 ******************************************************************************/
fnet_http_desc_t fnet_http_init( struct fnet_http_params * params);

/***************************************************************************/ /*!
 *
 * @brief    Releases the HTTP Server service.
 *
 * @param desc     HTTP server descriptor to be unregistered.
 *
 * @see fnet_http_init()
 *
 ******************************************************************************
 *
 * This function releases the HTTP Server assigned to the @c desc 
 * descriptor.@n 
 * It releases all occupied resources, and unregisters the HTTP service from 
 * the polling list.
 *
 ******************************************************************************/
void fnet_http_release(fnet_http_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Detects if the HTTP Server service is enabled or disabled.
 *
 * @param desc     HTTP server descriptor
 *
 * @return This function returns:
 *          - @ref FNET_TRUE if the HTTP Server is successfully initialized.
 *          - @ref FNET_FALSE if the HTTP Server is not initialized or is released.
 *
 ******************************************************************************
 *
 * This function detects if the HTTP Server service is initialized or is released.
 *
 ******************************************************************************/
int fnet_http_enabled(fnet_http_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Converts escaped string to an original format. 
 *
 * @param dest    Destination string.
 *
 * @param src     Source string.
 *
 ******************************************************************************
 *
 * This function converts encoded string to the original format.
 * The '+' symbol is replaced by the space symbol, and the  % symbol followed 
 * by two hexadecimal digits is replaced by proper ASCII value 
 * (for example the exclamation mark encoded as \%21). @n
 * This function should be used by CGI functions to eliminate escape symbols 
 * from a query string.@n
 * @n
 * INFO:@n
 * RFC1945:The Request-URI is transmitted as an encoded string, where some
 * characters may be escaped using the "% HEX HEX" encoding defined by
 * RFC 1738. The origin server must decode the Request-URI in order
 * to properly interpret the request.
 *
 ******************************************************************************/
void fnet_http_query_unencode(char * dest, char * src);

/*! @} */


#endif /* FNET_CFG_HTTP */


#endif

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
**********************************************************************/ /*!
*
* @file fnet_http_cgi.h
*
* @author Andrey Butok
*
* @date Jan-28-2013
*
* @version 0.1.17.0
*
* @brief FNET HTTP Server CGI API.
*
***************************************************************************/

#ifndef _FNET_HTTP_CGI_H_

#define _FNET_HTTP_CGI_H_

#include "fnet_config.h"


#if (FNET_CFG_HTTP && FNET_CFG_HTTP_CGI) || defined(__DOXYGEN__)


#include "fnet.h"

/*! @addtogroup fnet_http
 @{ */

/**************************************************************************/ /*!
 * @brief CGI file extension. @n
 * All HTTP requests to the files that have this extension will be 
 * handled by the CGI handler.
 * @showinitializer
 ******************************************************************************/ 
#define FNET_HTTP_CGI_EXTENSION    "cgi" 



/**************************************************************************/ /*!
 * @brief Callback function prototype of the CGI query handler.
 *
 * @param query     CGI query string (null-terminated). @n
 *                  The query string is set to whatever appears 
 *                  after the question mark in the URL itself.
 * @param cookie    This parameter points to the value, initially set to zero,
 *                  which can be used to associate a custom information
 *                  with a connection instance. If application store context 
 *                  information in the @c cookie, it will be preserved 
 *                  for future calls for this request. This allows the 
 *                  application to associate some request-specific state.
 *
 * @return This function must return:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs. 
 *   - If the HTTP server works according to HTTP/1.x 
 *     (@ref FNET_CFG_HTTP_VERSION_MAJOR is @c 1), 
 *     this function may return a HTTP response status-code defined by 
 *     @ref fnet_http_status_code_t.
 *
 * @see fnet_http_cgi, fnet_http_cgi_send_t
 *
 * The CGI handler invokes this callback function when the server gets
 * CGI request and the requested CGI file name corresponds to the name 
 * registered in the CGI table.@n
 * The @c query points to the CGI query string.
 * If the CGI request does not have any data, the @c query will point to the 
 * blank string.
 * 
 ******************************************************************************/ 
typedef int(*fnet_http_cgi_handle_t)(char * query, long *cookie);
 

/**************************************************************************/ /*!
 * @brief Callback function prototype of the CGI response function.
 *
 * @param buffer           Output buffer where CGI response content will be copied to.
 *
 * @param buffer_size      Size of the output @c buffer. @n
 *                         It's defined by the @ref FNET_CFG_HTTP_REQUEST_SIZE_MAX parameter.
 *
 * @param eof              Condition flag:
 *                         - @c 0 =  there is still more data to send.
 *                           The @ref fnet_http_cgi_send_t function will be called 
 *                           during the next iteration again.
 *                         - @c 1 =  no more data is available for the CGI response.
 *                           It was the last call of the @ref fnet_http_cgi_send_t 
 *                           function for the current CGI response.
 * @param cookie    This parameter points to the value, initially set to zero,
 *                  which can be used to associate a custom information
 *                  with a connection instance. If application store context 
 *                  information in the @c cookie, it will be preserved 
 *                  for future calls for this request. This allows the 
 *                  application to associate some request-specific state. 
 *
 * @return This function returns the number of bytes actually written to the buffer
 *         pointed to by @c buffer.@n
 *         The condition flag @c eof indicates if the CGI data-end condition has occurred.
 *
 * @see fnet_http_cgi, fnet_http_cgi_handle_t
 *
 * This function creates the CGI response content.@n
 * An application should use the @c buffer as output buffer for the dynamic content
 * and set @c eof flag to @c 1 if no data for output are available.@n
 * The CGI handler invokes this callback function after successful call of the 
 * @ref fnet_http_cgi_handle_t function and continues to call this function repeatedly 
 * till the @c eof will be set to @c 1 or the return result is set to @c 0.
 * 
 ******************************************************************************/ 
typedef unsigned long(*fnet_http_cgi_send_t)(char * buffer, unsigned long buffer_size, char * eof, long *cookie);

/**************************************************************************/ /*!
 * @brief CGI callback function table.
 *
 * The last table element must have all fields set to zero as the end-of-table mark.@n
 * @n
 * The Common Gateway Interface (CGI) is a standard for external
 * gateway programs to interface with HTTP servers. A CGI function is
 * executed in real-time, so that it can output dynamic information.
 *
 * @see fnet_http_params
 ******************************************************************************/
struct fnet_http_cgi
{
	char *name;				            /**< @brief CGI file name. */
	fnet_http_cgi_handle_t handle;      /**< @brief Pointer to the CGI query handler. It's optional. */
    fnet_http_cgi_send_t send;    /**< @brief Pointer to the CGI response function. 
                                         * This function actually creates dynamic content of
                                         * the CGI response. It's optional. */
};

/*! @} */


#endif


#endif

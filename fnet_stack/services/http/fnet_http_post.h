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
* @file fnet_http_post.h
*
* @author Andrey Butok
*
* @date Jan-28-2013
*
* @version 0.1.9.0
*
* @brief FNET HTTP Server POST method API.
*
***************************************************************************/

#ifndef _FNET_HTTP_POST_H_

#define _FNET_HTTP_POST_H_

#include "fnet_config.h"


#if (FNET_CFG_HTTP && FNET_CFG_HTTP_POST)|| defined(__DOXYGEN__)


#include "fnet.h"


/*! @addtogroup fnet_http
 @{ */

/**************************************************************************/ /*!
 * @brief Callback function prototype of the POST-method query handler.
 *
 * @param query     POST-method query string (null-terminated). @n
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
 *   - Also this function may return a HTTP response status-code defined by 
 *     @ref fnet_http_status_code_t.
 * @see fnet_http_post, fnet_http_post_receive_t, fnet_http_post_send_t
 *
 * The HTTP server invokes this callback function when gets
 * POST-method request and the requested file name corresponds to the name 
 * registered in the POST table defined @ref fnet_http_post.@n
 * If the query string does not have any data, the @c query will point to the 
 * blank string.
 * 
 ******************************************************************************/ 
typedef int(*fnet_http_post_handle_t)(char * query, long *cookie);

/**************************************************************************/ /*!
 * @brief Callback function prototype of the POST-method receive function.
 *
 * @param buffer           Data buffer that contains data received from 
 *                         the remote HTTP client.
 *
 * @param buffer_size      Size of the input @c buffer in bytes.
 * 
 * @param cookie    This parameter points to the value,
 *                  which can be used to associate a custom information
 *                  with a connection instance. If application store context 
 *                  information in the @c cookie, it will be preserved 
 *                  for future calls for this request. This allows the 
 *                  application to associate some request-specific state.
 *
 * @return This function must return:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs. 
 *   - Also this function may return a HTTP response status-code defined by 
 *     @ref fnet_http_status_code_t.
 * @see fnet_http_post, fnet_http_post_handle_t, fnet_http_post_send_t
 *
 * This function is invoked by the HTTP server when there is any data 
 * in the entity-body of the POST request.
 * This function can be invoked multiple times to process all received data.
 * At each invocation a new chunk of data must be processed.
 * The HTTP server invokes this callback function after call of the 
 * @ref fnet_http_post_handle_t function.
 * 
 ******************************************************************************/ 
typedef int(*fnet_http_post_receive_t)(char * buffer, unsigned long buffer_size, long *cookie);

/**************************************************************************/ /*!
 * @brief Callback function prototype of the POST-method response function.
 *
 * @param buffer           Output buffer where POST response content will be copied to.
 *
 * @param buffer_size      Size of the output @c buffer. @n
 *                         It's defined by the @ref FNET_CFG_HTTP_REQUEST_SIZE_MAX parameter.
 *
 * @param eof              Condition flag:
 *                         - @c 0 =  there is still more data to send.
 *                           The @ref fnet_http_post_send_t function will be called 
 *                           during the next iteration again.
 *                         - @c 1 =  no more data is available for the POST response.
 *                           It was the last call of the @ref fnet_http_post_send_t 
 *                           function for the current POST response.
 *
 * @param cookie    This parameter points to the value,
 *                  which can be used to associate a custom information
 *                  with a connection instance. If application store context 
 *                  information in the @c cookie, it will be preserved 
 *                  for future calls for this request. This allows the 
 *                  application to associate some request-specific state.
 *
 * @return This function must return the number of bytes actually written to the buffer
 *         pointed to by @c buffer.@n
 *         The condition flag @c eof indicates if the POST-method end condition has occurred.
 *
 * @see fnet_http_post, fnet_http_post_handle_t, fnet_http_post_receive_t
 *
 * This function creates the POST-method response content.@n
 * An application should use the @c buffer as output buffer for the dynamic content
 * and set @c eof flag to @c 1 if no data for output are available.@n
 * The HTTP server invokes this callback function after call of the 
 * @ref fnet_http_post_handle_t and @ref fnet_http_post_receive_t functions and continues to call this function repeatedly 
 * till the @c eof will be set to @c 1 or the return result is set to @c 0.
 * 
 ******************************************************************************/ 
typedef unsigned long(*fnet_http_post_send_t)(char * buffer, unsigned long buffer_size, char * eof, long *cookie);

/**************************************************************************/ /*!
 * @brief POST-method callback function table.
 *
 * The last table element must have all fields set to zero as the end-of-table mark.@n
 * @n
 * The POST request method is used when the client needs to send data to the server 
 * as part of the request, such as when uploading a file or submitting a completed form.
 *
 * @see fnet_http_params
 ******************************************************************************/
struct fnet_http_post
{
	char *name;				            /**< @brief File name associated with the POST-request. */
	fnet_http_post_handle_t handle;     /**< @brief Pointer to the POST query handler. It's optional. */
	fnet_http_post_receive_t receive;   /**< @brief Pointer to the POST receive function. It's optional.@n 
	                                     * This function can be invoked multiple times to process 
	                                     * all received data.*/
    fnet_http_post_send_t send;         /**< @brief Pointer to the POST response function. It's optional.@n 
                                         * This function actually creates dynamic content of
                                         * the POST response.  */
                                        					
};

/*! @} */


#endif


#endif

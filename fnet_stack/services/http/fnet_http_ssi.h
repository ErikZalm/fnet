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
**********************************************************************/ /*!
*
* @file fnet_http_ssi.h
*
* @author Andrey Butok
*
* @date Jan-28-2013
*
* @version 0.1.16.0
*
* @brief FNET HTTP Server SSI API.
*
***************************************************************************/

#ifndef _FNET_HTTP_SSI_H_

#define _FNET_HTTP_SSI_H_

#include "fnet_config.h"


#if (FNET_CFG_HTTP && FNET_CFG_HTTP_SSI)|| defined(__DOXYGEN__)


#include "fnet.h"

/*! @addtogroup fnet_http
 @{ */

/**************************************************************************/ /*!
 * @brief SSI file extension. @n
 * All files that have this extension will be parsed by the SSI parser.
 * @showinitializer
 ******************************************************************************/
#define FNET_HTTP_SSI_EXTENSION     "shtml"

/**************************************************************************/ /*!
 * @brief Callback function prototype of the SSI parameters handler.
 *
 * @param query     SSI directive parameter string (null-terminated).@n
 *                  The parameter string is set to whatever appears 
 *                  between SSI command tag and @c --\> in the SSI directive itself. 
 * @param cookie    This parameter points to the value, initially set to zero,
 *                  which can be used to associate a custom information
 *                  with a connection instance. If application store context 
 *                  information in the @c cookie, it will be preserved 
 *                  for future calls for this request. This allows the 
 *                  application to associate some request-specific state. 
 *
 * @return This function must return:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs. @n
 *     SSI directive will be eliminated from the result HTTP page.
 *
 * @see fnet_http_ssi, fnet_http_ssi_send_t
 *
 * The SSI parser invokes this callback function, when it meets SSI directive
 * in the HTML file and the SSI command name corresponds to the name 
 * registered in the SSI table.@n
 * The @c query points to the SSI parameters string.
 * If SSI directive does not have any parameters, the @c query points to the 
 * blank string.
 * 
 ******************************************************************************/
typedef int(*fnet_http_ssi_handle_t)(char * query, long *cookie);	

/**************************************************************************/ /*!
 * @brief Callback function prototype of the SSI include function.
 *
 * @param buffer           Output buffer where SSI content will be copied to.
 *
 * @param buffer_size      Size of the output @c buffer.
 *
 * @param eof              Condition flag:
 *                         - @c 0 =  there is still more data to include.
 *                           The @ref fnet_http_ssi_send_t function will be called 
 *                           during the next iteration again.
 *                         - @c 1 =  no more data is available for the SSI to include.
 *                           It was the last call of the @ref fnet_http_ssi_send_t 
 *                           function for the current SSI.
 * @param cookie    This parameter points to the value,
 *                  which can be used to associate a custom information
 *                  with a connection instance. If application store context 
 *                  information in the @c cookie, it will be preserved 
 *                  for future calls for this request. This allows the 
 *                  application to associate some request-specific state. 
 *
 * @return This function returns the number of bytes actually written to the buffer,
 *         pointed to by @c buffer.@n
 *         The condition flag @c eof indicates, if the SSI end condition has occurred.
 *
 * @see fnet_http_ssi, fnet_http_ssi_handle_t
 *
 * This function creates SSI dynamic content.@n
 * An application should use the @c buffer as output buffer for the dynamic content
 * and set @c eof flag to @c 1 if no data are available to include.@n
 * The SSI parser invokes this callback function after successful call of the 
 * @ref fnet_http_ssi_handle_t function and continues to call this function repeatedly, 
 * till the @c eof will be set to @c 1 or the return result is set to @c 0.
 * 
 ******************************************************************************/
typedef unsigned long(*fnet_http_ssi_send_t)(char * buffer, unsigned long buffer_size, char * eof, long *cookie);

/**************************************************************************/ /*!
 * @brief SSI callback function table.
 *
 * The last table element must have all fields set to zero as the end-of-table mark.@n
 * @n
 * SSI (Server Side Includes) are directives that are placed in HTML pages, 
 * and evaluated on the server, while the pages are being served. 
 * They let a web server application add dynamically-generated content to 
 * an existing HTML page.@n
 * SSI directives have the following format:@n 
 *      @code <!--#command [parameter(s)]--> @endcode @n 
 * There should be no spaces between the @c \<!--  and the @c #.@n
 * If, for any reason, a document containing SSI directives is served to 
 * the client unparsed, the HTML comment format means the directive's 
 * coding will not be visible.
 *
 * @see fnet_http_params
 ******************************************************************************/
struct fnet_http_ssi
{
	char *name;				            /**< @brief SSI command name. */
	fnet_http_ssi_handle_t handle;      /**< @brief Pointer to the SSI parameters 
	                                     *   handler. It's optional. */
    fnet_http_ssi_send_t send;          /**< @brief Pointer to the SSI include function. 
                                         * This function actually inserts dynamic content to
                                         * an existing HTML page. It's optional. */							
};
/*! @} */


#endif /* FNET_CFG_HTTP && FNET_CFG_HTTP_SSI */


#endif

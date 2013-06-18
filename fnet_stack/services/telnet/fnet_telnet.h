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
**********************************************************************/
/*!
*
* @file fnet_telnet.h
*
* @author Andrey Butok
*
* @date Jan-28-2013
*
* @version 0.1.25.0
*
* @brief FNET Telnet Server API.
*
***************************************************************************/

#ifndef _FNET_TELNET_H_

#define _FNET_TELNET_H_

#include "fnet_config.h"

#if FNET_CFG_TELNET || defined(__DOXYGEN__)

#include "fnet.h"
#include "fnet_poll.h"


/*! @addtogroup fnet_telnet
* The Telnet server provides a simple command-line interface for a 
* remote host via a virtual terminal connection. @n
* Current version of the Telnet server supports maximum of one simultaneously 
* connected Telnet client.@n
* @n
* After the FNET Telnet server is initialized by calling the @ref fnet_telnet_init() 
* function, the user application should call the main service polling function  
* @ref fnet_poll_services() periodically in background. @n
*
* For example:
* @code
*
*...
*
* const struct fnet_shell fapp_telnet_shell =
* {
*    fapp_telnet_cmd_table,                                 
*    (sizeof(fapp_telnet_cmd_table) / sizeof(struct fnet_shell_command)), 
*    "TELNET>",                                
*    fapp_shell_init,                                      
* };
*
*...
*
* void main()
* {
*    struct fnet_telnet_params params;
* 
*    ...
*    
*    params.ip_address = FNET_HTONL(INADDR_ANY);             
*    params.port = FNET_HTONS(0);       //Default port number.
*    params.shell= &fapp_telnet_shell;
*    params.cmd_line_buffer = fapp_telnet_cmd_line_buffer;
*    params.cmd_line_buffer_size = sizeof(fapp_telnet_cmd_line_buffer)
*
*    // Init Telnet server.
*    fapp_telnet_desc = fnet_http_init(&params);
*    if(fapp_telnet_desc != FNET_ERR)
*    {
*        fnet_printf("\n FNET Telnet server started.\n");
*        while(1)
*        {
*           // Do something.
*           ...
*           fnet_poll_services();
*        }
*    }
*    else
*    {
*        fnet_printf("\n FNET Telnet server initialization is failed.\n");
*    }
*
* }
* @endcode
* For Telnet usage example, refer to FNET demo application source code.@n
* @n
* Configuration parameters:
* - @ref FNET_CFG_TELNET
* - @ref FNET_CFG_TELNET_MAX
* - @ref FNET_CFG_TELNET_SESSION_MAX
* - @ref FNET_CFG_TELNET_PORT 
* - @ref FNET_CFG_TELNET_SHELL_ECHO
* - @ref FNET_CFG_TELNET_SOCKET_BUF_SIZE 
*/
/*! @{ */

/**************************************************************************/ /*!
 * @brief Input parameters for @ref fnet_telnet_init().
 ******************************************************************************/
struct fnet_telnet_params
{
    struct sockaddr address;    /**<  @brief Server socket address. @n
                                 * If server IP address is set to @c 0s, the server will listen to all current network interfaces. @n
                                 * If server address family is set to @c 0, it will be assigned to @ref AF_SUPPORTED. @n
                                 * If server port number is set to @c 0, it will be assigned to the default port number defined by @ref FNET_CFG_TELNET_PORT.*/
    const struct fnet_shell *shell;     /**< @brief Root-shell structure. */
};

/**************************************************************************/ /*!
 * @brief Telnet server descriptor.
 * @see fnet_telnet_init()
 ******************************************************************************/
typedef long fnet_telnet_desc_t;

/***************************************************************************/ /*!
 *
 * @brief    Initializes the Telnet Server service.
 *
 * @param params     Initialization parameters defined by @ref fnet_telnet_params.
 *
 * @return This function returns:
 *   - Telnet server descriptor if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_telnet_release()
 *
 ******************************************************************************
 *
 * This function initializes the Telnet server service. It allocates all
 * resources needed, and registers the Telnet server service in the polling list.@n
 * After the initialization, the user application should call the main polling 
 * function  @ref fnet_poll_services() periodically to run the Telnet server in background.@n
 * The Telnet service executes parsing of user-entered commands received via 
 * the Telnet protocol, and calls user-defined command functions, 
 * if the parsing was successful.
 *
 ******************************************************************************/
fnet_telnet_desc_t fnet_telnet_init( struct fnet_telnet_params * params);

/***************************************************************************/ /*!
 *
 * @brief    Releases the Telnet Server service.
 *
 * @param desc     Telnet server descriptor to be unregistered.
 *
 * @see fnet_telnet_init()
 *
 ******************************************************************************
 *
 * This function releases the Telnet Server assigned to the @c desc 
 * descriptor.@n 
 * It releases all occupied resources, and unregisters the Telnet service from 
 * the polling list.
 *
 ******************************************************************************/
void fnet_telnet_release(fnet_telnet_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Closes the currently active session of the Telnet Server.
 *
 * @param desc     Telnet server descriptor
 *
 ******************************************************************************
 *
 * This function closes the current Telnet session.@n
 * It can be used in a Telnet user-command to close the current 
 * session. This is the alternative to closure of the Telnet-client terminal applicatioin.
 *
 ******************************************************************************/
void fnet_telnet_close_session(fnet_telnet_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Detects if the Telnet Server service is enabled or disabled.
 *
 * @param desc     Telnet server descriptor
 *
 * @return This function returns:
 *          - @ref FNET_TRUE if the Telnet Server is successfully initialized.
 *          - @ref FNET_FALSE if the Telnet Server is not initialized or is released.
 *
 ******************************************************************************
 *
 * This function detects if the Telnet Server service is initialized or is released.
 *
 ******************************************************************************/
int fnet_telnet_enabled(fnet_telnet_desc_t desc);

/*! @} */

#endif  /* FNET_CFG_TELNET */


#endif  /* _FNET_TELNET_H_ */

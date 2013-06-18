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
* @file fnet_dns.h
*
* @author Andrey Butok
*
* @date Jan-28-2013
*
* @version 0.1.10.0
*
* @brief DNS Resolver API.
*
***************************************************************************/

#ifndef _FNET_DNS_H_

#define _FNET_DNS_H_

#include "fnet_config.h"


#if FNET_CFG_DNS_RESOLVER || defined(__DOXYGEN__)


#include "fnet.h"
#include "fnet_poll.h"

/*! @addtogroup fnet_dns 
*
* The DNS client/resolver service allows user application to resolve IP addresses 
* of internet hosts that are identified by a host name. @n
* It does this by sending DNS requests to a DNS Server. 
* The IP address of a DNS Server is specified manually or can be obtained from 
* the DHCP Server for the Local Area Network. @n
* @n
* After the DNS client is initialized by calling the @ref fnet_dns_init() function,
* the user application should call the main service-polling function  
* @ref fnet_poll_services() periodically in background. @n
* The resolved IP-address will be passed to the @ref fnet_dns_handler_resolved_t callback function,
* which is set during the DNS-client service initialization.
* @n
* The DNS client service is released automatically as soon as the requested host name is 
* fully resolved or an error occurs. Your application code may still continue
* to call @ref fnet_poll_services() to handle other services, but this will not have any 
* impact on the DNS client communication until you initialize the next IP address resolving by calling 
* @ref fnet_dns_init() again. @n
* @n
* For the DNS-client service example, refer to the FNET Shell demo source code.@n
* @note
* Current version of the DNS client:
*  - does not cache the resolved IP addresses.
*  - can process only one request at a time.
*  - uses UDP protocol, without message truncation.
*  - does not support DNS servers without recursion (all real-life DNS servers support it).
*  - takes the first resolved IP address, even if the DNS server provides several ones.
* 
* Configuration parameters:
* - @ref FNET_CFG_DNS 
* - @ref FNET_CFG_DNS_RESOLVER 
* - @ref FNET_CFG_DNS_PORT  
* - @ref FNET_CFG_DNS_RETRANSMISSION_MAX  
* - @ref FNET_CFG_DNS_RETRANSMISSION_TIMEOUT  
*  
*/

/*! @{ */

/**************************************************************************/ /*!
 * @brief DNS-client states.@n
 * Used mainly for debugging purposes.
 ******************************************************************************/
typedef enum
{
    FNET_DNS_STATE_DISABLED = 0,    /**< @brief The DNS-client service is not 
                                    * initialized or is released.
                                    */
    FNET_DNS_STATE_TX,              /**< @brief The DNS-client service sends the 
                                    * request to the DNS server. 
                                    */
    FNET_DNS_STATE_RX,             /**< @brief The DNS-client service waits a response from the DNS server.
                                    */                                     
    FNET_DNS_STATE_RELEASE          /**< @brief The DNS resolving is completed 
                                    * or received error.
                                    */
} fnet_dns_state_t;


/**************************************************************************/ /*!
 * @brief Prototype of the DNS-client callback function that is 
 * called when the DNS client has completed the resolving.
 *
 * @param address    Result of the DNS client resolving, that equals to:
 *                        - The resolved IP address (in network byte order), 
 *                          if no error occurs.
 *                        - @ref FNET_ERR if the resolving was failed.   
 * @param cookie     User-application specific parameter. It's set during 
 *                   the DNS-client service initialization as part of 
 *                   @ref fnet_dns_params.
 *
 * @see fnet_dns_resolve(), fnet_dns_params
 ******************************************************************************/  
 typedef void(*fnet_dns_handler_resolved_t)(fnet_ip4_addr_t address, long cookie);


/**************************************************************************/ /*!
 * @brief Initialization parameters for the @ref fnet_dns_init() function.
 ******************************************************************************/
struct fnet_dns_params
{
    fnet_ip4_addr_t dns_server;              /**< @brief DNS server IP address (in network byte order). 
                                             */
    char * host_name;                       /**< @brief Host name to resolve (null-terminated string).
                                             */
    fnet_dns_handler_resolved_t handler;    /**< @brief Pointer to the callback function defined by 
                                             * @ref fnet_dns_handler_resolved_t. It is called when the 
                                             * DNS-client resolving is finished or an error is occurred.
                                             */
    long cookie;                            /**< @brief Optional application-specific parameter. @n 
                                             * It's passed to the @c handler callback 
                                             * function as input parameter.
                                             */
};

/***************************************************************************/ /*!
 *
 * @brief    Initializes DNS client service and starts the host name resolving.
 *
 * @param params     Initialization parameters.
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_dns_params, fnet_dns_handler_resolved_t, fnet_dns_release()
 *
 ******************************************************************************
 *
 * This function initializes the DNS client service and starts the 
 * host name resolving. It allocates all
 * resources needed and registers the DNS service in the polling list.@n
 * After the initialization, the user application should call the main polling 
 * function @ref fnet_poll_services() periodically to run the DNS service routine 
 * in the background.@n
 * The resolved IP-address will be passed to the @ref fnet_dns_handler_resolved_t callback function,
 * which is set in @c params. @n
 * The DNS service is released automatically as soon as the 
 * resolving is finished or an error is occurred.
 *
 ******************************************************************************/
int fnet_dns_init( struct fnet_dns_params *params );

/***************************************************************************/ /*!
 *
 * @brief    Aborts the resolving and releases the DNS-client service.
 *
 * @see fnet_dns_init()
 *
 ******************************************************************************
 *
 * This function stops the DNS-client service. It releases all resources 
 * used by the service, and unregisters it from the polling list.@n
 * Use this function only in the case of the early termination of the service,
 * because the DNS service is released automatically as soon as the 
 * resolving is finished. 
 *
 ******************************************************************************/
void fnet_dns_release(void);

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the current state of the DNS-client service (for debugging purposes).
 *
 * @return This function returns the current state of the DNS-client service.
 *   The state is defined by the @ref fnet_dns_state_t.
 *
 ******************************************************************************
 *
 * This function returns the current state of the DNS-client service.
 * If the state is @ref FNET_DNS_STATE_DISABLED, the DNS client is not initialized
 * or released.@n
 * It is used mainly for debugging purposes.
 *
 ******************************************************************************/
fnet_dns_state_t fnet_dns_state(void);


/*! @} */


#endif /* FNET_CFG_DNS_RESOLVER */

#endif /* _FNET_DNS_H_ */

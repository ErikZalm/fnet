/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
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
* @file fnet_ping.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.5.0
*
* @brief PING API.
*
***************************************************************************/

#ifndef _FNET_PING_H_

#define _FNET_PING_H_

#include "fnet_config.h"


#if FNET_CFG_PING || defined(__DOXYGEN__)

#include "fnet.h"
#include "fnet_poll.h"

/*! @addtogroup fnet_ping 
*
* The PING service is used to test if a system can communicate over the 
* network with another device or computer.@n
* It operates by sending Internet Control Message Protocol (ICMPv4/v6) Echo Request
* messages to the destination device and waiting for a response.
* @n
* After the PING service is initialized the @ref fnet_ping_request() function,
* the user application should call the main service-polling function  
* @ref fnet_poll_services() periodically in background. @n
* When correct echo response is received, the PING service passes an 
* echo-reply socket address to the @ref fnet_ping_handler_t callback function. @n
* For the PING service usage example, refer to the FNET Shell demo source code.@n
* 
* @note The PING service uses the RAW sockets.
*
* Configuration parameters:
* - @ref FNET_CFG_PING 
* - @ref FNET_CFG_PING_PACKET_MAX
* - @ref FNET_CFG_PING_IDENTIFIER 
*  
*/

/*! @{ */

/**************************************************************************/ /*!
 * @brief PING service states.@n
 * Used mainly for debugging purposes.
 ******************************************************************************/
typedef enum
{
    FNET_PING_STATE_DISABLED = 0,   /**< @brief The PING service is not initialized or is released.
                                     */
    FNET_PING_STATE_SENDING_REQUEST,/**< @brief The PING service is going to send echo request.
                                     */                                     
    FNET_PING_STATE_WAITING_REPLY,  /**< @brief The PING service is waiting for echo reply.
                                     */
    FNET_PING_STATE_WAITING_TIMEOUT,/**< @brief The PING service is waying a timeout till next request.
                                     */
                                   
} fnet_ping_state_t;

/**************************************************************************/ /*!
 * @brief Prototype of the PING-service callback function that is 
 * called when the PING-service has completed the requesting.
 *
 * @param result        Result of the PING-service request, which equals to:
 *                          - FNE_OK - if correct reply is received.
 *                          - FNET_ERR_TIMEDOUT - if there is no correct response during specified timeout.
 *                          - Error code, if any happened during receiving.
 * @param packet_count  Number of request packets to be sent, till the PING-service release.@n
 *                      It equals to 0, when the last packet was sent.
 * @param address       Pointer to a socket address, which equals to:
 *                          - Target socket address, if correct reply is received during specified timeout.
 *                          - @ref FNET_NULL, if there is no correct response during specified timeout.   
 * @param cookie        User-application specific parameter. It's set during 
 *                      the PING service initialization as part of @ref fnet_ping_params.
 *
 * @see fnet_ping_request(), fnet_ping_params
 ******************************************************************************/  
 typedef void(*fnet_ping_handler_t)(int result, unsigned long packet_count, struct sockaddr *target_addr, long cookie);


/**************************************************************************/ /*!
 * @brief Initialization parameters for the @ref fnet_ping_request() function.
 ******************************************************************************/
struct fnet_ping_params
{
    struct sockaddr     target_addr;    /**< @brief Socket address of the destination to ping.
                                         */
    unsigned long       packet_size;    /**< @brief The size of the echo request, in bytes (without ICMP header). @n
                                         *  The maximum value is limited by @ref FNET_CFG_PING_PACKET_MAX value.
                                         */
    unsigned long       packet_count;   /**< @brief Number of packets to be sent.
                                         */                                         
    unsigned long       timeout;        /**< @brief Timeout value in milliseconds, that service 
                                         * waits for reply on ping request.
                                         */
    int                 ttl;            /**< @brief IPv4 Time To Live (TTL) or IPv6 Hop Limit value. @n
                                         */                                         
    unsigned char       pattern;        /**< @brief  Pattern byte to fill out the packet.@n
                                         *   This is useful for diagnosing data-dependent problems in a network.
                                         */                                        
    fnet_ping_handler_t handler;        /**< @brief Pointer to the callback function defined by 
                                         * @ref fnet_ping_handler_t. It is called when the 
                                         * correct echo response is receved or timeout is occured.
                                         */
    long                cookie;         /**< @brief Optional application-specific parameter. @n 
                                         * It's passed to the @c handler callback 
                                         * function as input parameter.
                                         */
};

/***************************************************************************/ /*!
 *
 * @brief    Initializes PING service.
 *
 * @param params     Initialization parameters.
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_ping_params, fnet_ping_handler_t, fnet_ping_release()
 *
 ******************************************************************************
 *
 * This function initializes the PING service .
 * It allocates all resources needed and registers the PING service in 
 * the polling list.@n
 * After the initialization, the user application should call the main polling 
 * function @ref fnet_poll_services() periodically to run the PING service routine 
 * in the background.@n
 * The @ref fnet_ping_handler_t callback function,
 * which is set in @c params, will be called when correct echo-reply is 
 * received or timeout is occurred. @n
 * The PING service is released automatically.@n
 * Call fnet_ping_release() if you need to terminate it earlier.
 *
 ******************************************************************************/
int fnet_ping_request( struct fnet_ping_params *params );

/***************************************************************************/ /*!
 *
 * @brief    Releases the PING service.
 *
 * @see fnet_ping_request()
 *
 ******************************************************************************
 *
 * This function terminates the PING service. It releases all resources 
 * used by the service, and unregisters it from the polling list.@n
 * Use this function only in the case of the early termination of the service,
 * because the PING service is released automatically as soon as the 
 * pinging is finished. 
 *
 ******************************************************************************/
void fnet_ping_release(void);

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the current state of the PING service (for debugging purposes).
 *
 * @return This function returns the current state of the PING service.
 *          The state is defined by the @ref fnet_ping_state_t.
 *
 ******************************************************************************
 *
 * This function returns the current state of the PING service.
 * If the state is @ref FNET_PING_STATE_DISABLED, the PING service is not 
 * initialized or released.@n
 * It is used mainly for debugging purposes.
 *
 ******************************************************************************/
fnet_ping_state_t fnet_ping_state(void);


/*! @} */


#endif /* FNET_CFG_PING */

#endif /* _FNET_PING_H_ */

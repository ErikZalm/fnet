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
* @file fnet_poll.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.17.0
*
* @brief FNET Services polling mechanism API.
*
***************************************************************************/

#ifndef _FNET_POLL_H_

#define _FNET_POLL_H_

/*! @addtogroup fnet_polling 
* The polling mechanism enables the execution of registered services
* (DHCP client, TFTP client, Shell, Telnet server or HTTP server) in 
* "background" - during the application 
* idle time. Typically, the polling callback is registered during the service
* initialization (for example the @ref fnet_http_init() or other service initialization
* function).
* In order to make the polling mechanism work, the user application should 
* call the @ref fnet_poll_services() API function periodically, during the idle time.@n
* @n
* Configuration parameters:
* - @ref FNET_CFG_POLL_MAX  
*/
/*! @{ */

/**************************************************************************/ /*!
 * @brief Descriptor of a registered service.
 ******************************************************************************/
typedef unsigned int fnet_poll_desc_t;

/**************************************************************************/ /*!
 * @brief Service callback function prototype.
 *
 * @param service_param   This parameter is assigned during 
 *                        a service registration by the @ref 
 *                        fnet_poll_service_register().
 *                        
 ******************************************************************************/
typedef void (* fnet_poll_service_t)(void* service_param);

/***************************************************************************/ /*!
 *
 * @brief    Main polling function.
 *
 * @see fnet_poll_service_register()
 * 
 ******************************************************************************
 *
 * This function calls all registered service routines.@n
 * The user application should call this function periodically, after any service 
 * initialization.
 *
 ******************************************************************************/
void fnet_poll_services(void);


/***************************************************************************/ /*!
 *
 * @brief    Unregisters all registered service routines.
 *
 * @see fnet_poll_service_register(), fnet_poll_service_unregister()
 *
 ******************************************************************************
 *
 * This function unregisters all registered service routines from 
 * the polling list.
 *
 ******************************************************************************/ 
void fnet_poll_services_release(void);

/***************************************************************************/ /*!
 *
 * @brief    Registers the service routine in the polling list.
 *
 * @param service        Pointer to the service-polling routine.
 *
 * @param service_param  Service-polling-routine-specific parameter.
 *
 * @return This function returns:
 *   - Service descriptor, if no error occurs.
 *   - @ref FNET_ERR, if an error occurs.
 *
 * @see fnet_poll_service_unregister()
 *
 ******************************************************************************
 *
 * This function adds the service routine into the polling list.@n
 * This function is usually called during a service initialization. 
 * User application should not call this function directly.
 *
 ******************************************************************************/
fnet_poll_desc_t fnet_poll_service_register( fnet_poll_service_t service, void *service_param );

/***************************************************************************/ /*!
 *
 * @brief    Unregisters the service routine.
 *
 * @param desc       Service descriptor to be unregistered.
 *
 * @return This function returns:
 *   - @ref FNET_OK, if no error occurs.
 *   - @ref FNET_ERR, if an error occurs.
 *
 * @see fnet_poll_service_register()
 *
 ******************************************************************************
 *
 * This function unregisters the service routine assigned to the @c desc 
 * descriptor.@n
 * This function is usually called during a service release. 
 * User application should not call this function directly.
 *
 ******************************************************************************/
int fnet_poll_service_unregister( fnet_poll_desc_t desc );

/*! @} */

#endif

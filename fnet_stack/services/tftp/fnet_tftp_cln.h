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
* @file fnet_tftp_cln.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.23.0
*
* @brief TFTP Client API.
*
***************************************************************************/

#ifndef _FNET_TFTP_CLN_H_

#define _FNET_TFTP_CLN_H_

#include "fnet_config.h"


#if FNET_CFG_TFTP_CLN || defined(__DOXYGEN__)


#include "fnet.h"
#include "fnet_poll.h"
#include "fnet_tftp.h"


/*! @addtogroup fnet_tftp_cln 
* The user application can use the TFTP-client service to download and upload files from/to 
* a remote TFTP server. @n
* After the TFTP client is initialized by calling the @ref fnet_tftp_cln_init() function,
* the user application should call the main service-polling function  
* @ref fnet_poll_services() periodically in the background. @n
* The TFP client service is released automatically as soon as the requested file is 
* fully received/sent or an error occurs. Your application code may still continue
* to call @ref fnet_poll_services() to handle other services, but this will not have any 
* impact on the TFTP client communication until you initialize the next file transfer by calling 
* @ref fnet_tftp_cln_init() again. @n
* @n
* For the TFTP-client service example, refer to the FNET Bootloader source code.@n
* @n
* Configuration parameters:
* - @ref FNET_CFG_TFTP_CLN  
* - @ref FNET_CFG_TFTP_CLN_PORT  
* - @ref FNET_CFG_TFTP_CLN_TIMEOUT  
*/
/*! @{ */



/**************************************************************************/ /*!
 * @brief TFTP-client event handler callback function prototype, that is 
 * called when the TFTP client has received a new data packet
 * (@c request_type equals to @ref FNET_TFTP_REQUEST_READ),
 * when the TFTP client is ready to send a new data packet to 
 * the TFTP server (@c request_type equals to @ref FNET_TFTP_REQUEST_WRITE),
 * or when an error occurs (@c tftp_result equals to @c FNET_ERR).
 *
 * @param request_type      Request type (read or write) defined by @ref fnet_tftp_request_t. 
 *                          It's set during the TFTP-client service initialization as part of the 
 *                          @ref fnet_tftp_cln_params.
 *
 * @param data              Pointer to the data buffer which content defined by @c request_type:
 *                          - If @c request_type equals to @ref FNET_TFTP_REQUEST_READ, @n 
 *                            this parameter points to the data buffer that contains data received 
 *                            from the TFTP server.
 *                          - If @c request_type equals to @ref FNET_TFTP_REQUEST_WRITE, @n
 *                            this parameter points to the data buffer which should be filled by 
 *                            the application with a data that will be sent to 
 *                            the remote TFTP server. If the written data size is  
 *                            less than @c data_size (@ref FNET_TFTP_DATA_SIZE_MAX),
 *                            it will mean that this data packet is the last one. @n
 *                          - If the @c tftp_result parameter is equal to @ref FNET_ERR, @n
 *                            this parameter points to an error message string (null-terminated).
 *
 * @param data_size         Size of the buffer pointed by the @c data parameter,
 *                          in bytes. 
 *                          - If @c request_type equals to @ref FNET_TFTP_REQUEST_READ, @n
 *                          this parameter can have value from 0 till @ref FNET_TFTP_DATA_SIZE_MAX.
 *                          If this number is less than @ref FNET_TFTP_DATA_SIZE_MAX, it will 
 *                          mean that this data packet is the last one (the TFTP-client 
 *                          service is released automatically). 
 *                          - If @c request_type equals to @ref FNET_TFTP_REQUEST_WRITE, @n
 *                          this parameter always equals to @ref FNET_TFTP_DATA_SIZE_MAX. @n
 *                          - If the @c tftp_result parameter is equal to @ref FNET_ERR, @n
 *                          this parameter contains the TFTP error code defined by 
 *                          @ref fnet_tftp_error_t.
 *
 * @param tftp_result       Result code returned by the TFTP-client service:
 *                           - @c FNET_OK = No error.
 *                           - @c FNET_ERR = There is an error. The TFTP-client service is released automatically.
 *
 * @param handler_param     User-application specific parameter. It's set during 
 *                          the TFTP-client service initialization as part of 
 *                          @ref fnet_tftp_cln_params.
 *
 * @return 
 *   - If @c request_type equals to @ref FNET_TFTP_REQUEST_READ,@n
 *     this function should return @ref FNET_OK if no errors.
 *   - If @c request_type equals to @ref FNET_TFTP_REQUEST_WRITE, @n
 *     this function should return number of bytes written to the buffer pointed by @c data. If this 
 *     number is less than @ref FNET_TFTP_DATA_SIZE_MAX, it will mean that this
 *     data packet is the last one (the TFTP-client service is released automatically 
 *     after the last packet is acknowledged by the remote server).
 *   - This function should return @ref FNET_ERR if an error occurs. The TFTP-client service  will be
 *     released automatically.
 *
 * @see fnet_tftp_cln_params
 ******************************************************************************/ 
typedef int(*fnet_tftp_cln_handler_t)(fnet_tftp_request_t request_type, unsigned char* data, unsigned short data_size, int tftp_result, void *handler_param);


/**************************************************************************/ /*!
 * @brief Input parameters for the @ref fnet_tftp_cln_init() function.
 ******************************************************************************/
struct fnet_tftp_cln_params
{
    fnet_tftp_request_t request_type; /**< @brief Request type (read or write) defined by @ref fnet_tftp_request_t.
                                       */ 
    struct sockaddr server_addr;     /**< @brief Socket address of the remote TFTP server to 
                                     * connect to.
                                     */
    char *file_name;                /**< @brief Name of the file to retrieve or create on 
                                     * the remote TFTP server.
                                     */
    fnet_tftp_cln_handler_t handler;/**< @brief Pointer to the callback function 
                                     * defined by @ref fnet_tftp_cln_handler_t().
                                     */
    void *handler_param;            /**< @brief Optional application-specific 
                                     * parameter. @n 
                                     * It is passed to the @c handler callback 
                                     * function as an input parameter.
                                     */
   unsigned char timeout;           /**< @brief Timeout for the TFTP server response 
                                     * in seconds. @n
                                     * If no response from a TFTP server is 
                                     * received during this timeout, the TFTP 
                                     * client is released automatically.@n
                                     * If it is set to @c 0 the default timeout will be 
                                     * used that is defined by the 
                                     * @ref FNET_CFG_TFTP_CLN_TIMEOUT parameter.
                                     */
};

/**************************************************************************/ /*!
 * @brief TFTP-client states.@n
 * Used mainly for debugging purposes.
 ******************************************************************************/
typedef enum
{
    FNET_TFTP_CLN_STATE_DISABLED = 0,   /**< @brief The TFTP-client service is not 
                                         * initialized or released.
                                         */
    FNET_TFTP_CLN_STATE_SEND_REQUEST,   /**< @brief The TFTP-client service is initialized.
                                         * Sends Read/Write request (PRQ).
                                         */
    FNET_TFTP_CLN_STATE_HANDLE_REQUEST, /**< @brief Receives or sends DATA packets from/to the remote server.
                                         */                                     
    FNET_TFTP_CLN_STATE_RELEASE         /**< @brief The DATA transfer is completed, 
                                         * or received error, or terminated by the application.
                                         * Frees the allocated resources.
                                         */
} fnet_tftp_cln_state_t;



/***************************************************************************/ /*!
 *
 * @brief    Initializes the file transfer with the TFTP-client service.
 *
 * @param params     Initialization parameters.
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_tftp_release()
 *
 ******************************************************************************
 *
 * This function initializes the TFTP-client service. It allocates all
 * resources needed and registers the TFTP service in the polling list.@n
 * After the initialization, the user application should call the main polling 
 * function @ref fnet_poll_services() periodically to run the TFTP service routine 
 * in the background.
 *
 ******************************************************************************/
int fnet_tftp_cln_init( struct fnet_tftp_cln_params *params );

/***************************************************************************/ /*!
 *
 * @brief    Aborts the transfer and releases the TFTP-client service.
 *
 * @see fnet_tftp_cln_init()
 *
 ******************************************************************************
 *
 * This function stops the TFTP-client service. It releases all resources 
 * used by the service, and unregisters it from the polling list.@n
 * Use this function only in the case of the early termination of the service,
 * because the TFP service is released automatically as soon as the 
 * requested file is fully received/transferred or an error is occurred. 
 *
 ******************************************************************************/
void fnet_tftp_cln_release(void);

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the current state of the TFTP-client service.
 *
 * @return This function returns the current state of the TFTP-client service.
 *   The state is defined by the @ref fnet_tftp_cln_state_t.
 *
 ******************************************************************************
 *
 * This function returns the current state of the TFTP-client service.
 * If the state is @ref FNET_TFTP_CLN_STATE_DISABLED, the TFTP client is not initialized
 * or released.
 *
 ******************************************************************************/
fnet_tftp_cln_state_t fnet_tftp_cln_state( void );

/*! @} */


#endif /* FNET_CFG_TFTP_CLN */


#endif /* _FNET_TFTP_CLN_H_ */

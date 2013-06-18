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
* @file fnet_tftp_srv.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.20.0
*
* @brief TFTP Server API.
*
***************************************************************************/

#ifndef _FNET_TFTP_SRV_H_

#define _FNET_TFTP_SRV_H_

#include "fnet_config.h"

#if FNET_CFG_TFTP_SRV || defined(__DOXYGEN__)

#include "fnet.h"
#include "fnet_poll.h"
#include "fnet_tftp.h"


/*! @addtogroup fnet_tftp_srv
* The user application can use the TFTP-server service to download and upload files from/to 
* a remote TFTP client. @n
* Current version of the TFTP server supports maximum of one simultaneously 
* connected TFTP clients. @n
* After the TFTP server is initialized by calling the @ref fnet_tftp_srv_init() function,
* the user application should call the main service-polling function  
* @ref fnet_poll_services() periodically in the background. 
* @n
* For the TFTP-server service example, refer to the FNET Bootloader source code.@n
* @n
* Configuration parameters:
* - @ref FNET_CFG_TFTP_SRV   
* - @ref FNET_CFG_TFTP_SRV_PORT      
* - @ref FNET_CFG_TFTP_SRV_MAX  
* - @ref FNET_CFG_TFTP_SRV_TIMEOUT  
* - @ref FNET_CFG_TFTP_SRV_TIMEOUT 
* - @ref FNET_CFG_TFTP_SRV_RETRANSMIT_MAX  
*/
/*! @{ */

/**************************************************************************/ /*!
 * @brief Prototype of the TFTP-server callback function that is 
 * called when the TFTP server has received a new read or write request from a
 * remote TFTP client.
 *
 * @param request_type      Request type (read or write) defined by @ref fnet_tftp_request_t. 
 * @param address           TFTP-client socket address that sent this request.
 * @param filename          Requested file name (null-terminated string).
 * @param mode              Data transfer mode (null-terminated string). It contains the
 *                          string "netascii", "octet", or "mail" . According to RFC1350:
 *                          - "Octet" mode is used to transfer a file that is in the 8-bit
 *                            format of the machine from which the file is being transferred. 
 *                          - In "netascii" mode data a host must translate the data to its own
 *                            format. 
 *                          - "Mail" mode uses the name of a mail recipient in place of the @c filename.
 * @param error_code        Pointer to the error code that will be sent to the remote TFTP client.
 *                          Changing of this parameter is optional, by default the error code is set to 
 *                          @ref FNET_TFTP_ERROR_NOT_DEFINED. @n
 *                          This parameter is used only when this callback function returns @ref FNET_ERR.
 * @param error_message     Pointer to a null-terminated error message string.
 *                          Changing of this parameter is optional, by default the error string is set to 
 *                          "Not defined". @n
 *                          This parameter is used only when this callback function returns @ref FNET_ERR.       
 * @param handler_param     User-application specific parameter. It's set during 
 *                          the TFTP-server service initialization as part of 
 *                          @ref fnet_tftp_srv_params.
 *
 * @return 
 *   - @ref FNET_OK if the application accepts the receive request and there is no any other error.
 *   - @ref FNET_ERR if the application cancels the receive request or an error occurs.
 *
 * @see fnet_tftp_srv_init(), fnet_tftp_srv_params
 ******************************************************************************/ 
typedef int(*fnet_tftp_srv_request_handler_t)(fnet_tftp_request_t request_type,
                                                const struct sockaddr *address,
                                                char *filename,        /* null-terminated.*/
                                                char *mode,            /* null-terminated.*/
                                                fnet_tftp_error_t *error_code,     /* error code [0-7] RFC783, if result==FNET_ERR*/
                                                char* *error_message, /* optinal, if result==FNET_ERR*/
                                                void *handler_param); 

/**************************************************************************/ /*!
 * @brief Prototype of the TFTP-server callback function prototype that is 
 * called when the TFTP server has received a new data packet
 * (@c request_type equals to @ref FNET_TFTP_REQUEST_WRITE). This callback function is also called
 * when the TFTP server is ready to send a new data packet to 
 * the TFTP client (@c request_type equals to @ref FNET_TFTP_REQUEST_READ).
 *
 * @param request_type      Request type (read or write) defined by @ref fnet_tftp_request_t. 
 * @param data            Pointer to the data buffer which content defined by @c request_type:
 *                          - If @c request_type equals to @ref FNET_TFTP_REQUEST_WRITE, @n 
 *                            this parameter points to the data buffer that contains data received 
 *                            from the remote TFTP client.
 *                          - If @c request_type equals to @ref FNET_TFTP_REQUEST_READ, @n
 *                            this parameter points to the data buffer which should be filled by 
 *                            the application with a data that will be sent to 
 *                            the remote TFTP client. If the written data size is  
 *                            less than @c data_size (@ref FNET_TFTP_DATA_SIZE_MAX),
 *                            it will mean that this data packet is the last one. 
  * @param data_size        Size of the buffer pointed by the @c data parameter,
 *                          in bytes. 
 *                          - If @c request_type equals to @ref FNET_TFTP_REQUEST_WRITE, @n
 *                          this parameter can have value from 0 till @ref FNET_TFTP_DATA_SIZE_MAX.
 *                          If this number is less than @ref FNET_TFTP_DATA_SIZE_MAX, it will 
 *                          mean that this data packet is the last one. 
 *                          - If @c request_type equals to @ref FNET_TFTP_REQUEST_READ, @n
 *                          this parameter always equals to @ref FNET_TFTP_DATA_SIZE_MAX. @n
 * @param error_code        Pointer to the error code that will be sent to the remote TFTP client.
 *                          Changing of this parameter is optional, by default the error code is set to 
 *                          @ref FNET_TFTP_ERROR_NOT_DEFINED. @n
 *                          This parameter is used only when this callback function returns @ref FNET_ERR.
 * @param error_message     Pointer to a null-terminated error message string.
 *                          Changing of this parameter is optional, by default the error string is set to 
 *                          "Not defined". @n
 *                          This parameter is used only when this callback function returns @ref FNET_ERR.       
 * @param handler_param     User-application specific parameter. It's set during 
 *                          the TFTP-server service initialization as part of 
 *                          @ref fnet_tftp_srv_params.
 *
 * @return 
 *   - If @c request_type equals to @ref FNET_TFTP_REQUEST_WRITE,@n
 *     this function should return @ref FNET_OK if no errors.
 *   - If @c request_type equals to @ref FNET_TFTP_REQUEST_READ, @n
 *     this function should return number of bytes written to the buffer pointed by @c data. If this 
 *     number is less than @ref FNET_TFTP_DATA_SIZE_MAX, it will mean that this
 *     data packet is the last one.
 *   - This function should return @ref FNET_ERR if an error occurs. The TFTP-client service  will be
 *     released automatically.
 *
 *
 * @see fnet_tftp_srv_init(), fnet_tftp_srv_params
 ******************************************************************************/ 
typedef int(*fnet_tftp_srv_data_handler_t)(fnet_tftp_request_t request,
                                        unsigned char *data, 
                                        unsigned short data_size, 
                                        fnet_tftp_error_t *error_code,
                                        char* *error_message,
                                        void *handler_param);

/**************************************************************************/ /*!
 * @brief Prototype of the TFTP-server callback function that is 
 * called when the TFTP server has completed file data transfer.
 *
 * @param request_type    Request type (read or write) defined by @ref fnet_tftp_request_t. 
 * @param status          File transfer completion  status that equals to:
 *                        - @ref FNET_OK if file transfer was successfull.
 *                        - @ref FNET_ERR if file transfer was failed.   
 * @param handler_param     User-application specific parameter. It's set during 
 *                          the TFTP-server service initialization as part of 
 *                          @ref fnet_tftp_srv_params.
 *
 * @see fnet_tftp_srv_init(), fnet_tftp_srv_params
 ******************************************************************************/                                        
typedef void(*fnet_tftp_srv_complete_handler_t)(fnet_tftp_request_t request,
                                                int status,
                                                void *handler_param);                                       

/**************************************************************************/ /*!
 * @brief Input parameters for the @ref fnet_tftp_srv_init() function.
 ******************************************************************************/
struct fnet_tftp_srv_params
{
    struct sockaddr address;        /**<  @brief Server socket address. @n
                                     * If server IP address is set to @c 0s, the server will listen to all current network interfaces. @n
                                     * If server address family is set to @c 0, it will be assigned to @ref AF_SUPPORTED. @n
                                     * If server port number is set to @c 0, it will be assigned to the default port number defined by @ref FNET_CFG_TFTP_SRV_PORT.
                                     */
    fnet_tftp_srv_request_handler_t request_handler;    /**< @brief Pointer to the callback function
                                                         * defined by @ref fnet_tftp_srv_request_handler_t().
                                                         */
    fnet_tftp_srv_data_handler_t data_handler;          /**< @brief Pointer to the callback function 
                                                         * defined by @ref fnet_tftp_srv_data_handler_t().
                                                         */
    fnet_tftp_srv_complete_handler_t complete_handler;  /**< @brief Pointer to the optional callback function 
                                                         * defined by @ref fnet_tftp_srv_complete_handler_t().@n
                                                         * This parameter is optional and can be set to zero.
                                                         */                                                                          
    void *handler_param;            /**< @brief Optional application-specific 
                                     * parameter. @n 
                                     * It is passed to the @c request_handler, @c data_handler 
                                     * and @c complete_handler  callback 
                                     * functions as an input parameter.
                                     */
    unsigned long timeout;          /**< @brief Timeout for the TFTP client response in seconds. @n
                                     * If no response from a TFTP client is received during this timeout,
                                     * the last packet is retransmitted to the TFTP client automatically. @n
                                     * If it is set to @c 0 the default timeout will be 
                                     * used, that is defined by the @ref FNET_CFG_TFTP_SRV_TIMEOUT parameter.
                                     */
    unsigned int retransmit_max;    /**< @brief Maximum number of retransmissions. @n
                                     * If no response from a TFTP client is received
                                     * till maximum retransmission number is reached, 
                                     * the TFTP server cancels the data transfer.@n
                                     * If it is set to @c 0 the default number of retransmissions will be 
                                     * used, that is defined by the @ref FNET_CFG_TFTP_SRV_RETRANSMIT_MAX parameter.
                                     */                                     
};

/**************************************************************************/ /*!
 * @brief TFTP server descriptor.
 * @see fnet_tftp_srv_init()
 ******************************************************************************/
typedef long fnet_tftp_srv_desc_t;

/**************************************************************************/ /*!
 * @brief TFTP server states.@n
 * Used mainly for debugging purposes.
 ******************************************************************************/
typedef enum
{
    FNET_TFTP_SRV_STATE_DISABLED = 0,       /**< @brief The TFTP server is not 
                                             * initialized or released.
                                             */
    FNET_TFTP_SRV_STATE_WAITING_REQUEST,    /**< @brief TFTP server is waiting 
                                             * for a request from a TFTP client.
                                             */
    FNET_TFTP_SRV_STATE_HANDLE_REQUEST,     /**< @brief Sends or receives DATA packets to the remote TFTP client.
                                             */
    FNET_TFTP_SRV_STATE_CLOSE               /**< @brief The DATA transfer is completed, 
                                             * or received error, or terminated by the application.
                                             */                                     
} fnet_tftp_srv_state_t;

/***************************************************************************/ /*!
 *
 * @brief    Initializes the TFTP-server service.
 *
 * @param params     Initialization parameters.
 *
 * @return This function returns:
 *   - TFTP-server descriptor if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_tftp_srv_release()
 *
 ******************************************************************************
 *
 * This function initializes the TFTP-server service. It allocates all
 * resources needed, and registers the TFTP-server service in the polling list.@n
 * After the initialization, the user application should call the main polling 
 * function  @ref fnet_poll_services() periodically to run the TFTP server in background.
 *
 ******************************************************************************/
fnet_tftp_srv_desc_t fnet_tftp_srv_init( struct fnet_tftp_srv_params *params );

/***************************************************************************/ /*!
 *
 * @brief    Releases the TFTP-server service.
 *
 * @param desc     TFTP-server descriptor to be unregistered.
 *
 * @see fnet_tftp_srv_init()
 *
 ******************************************************************************
 *
 * This function releases the TFTP Server assigned to the @c desc 
 * descriptor.@n 
 * It releases all occupied resources, and unregisters the TFTP service from 
 * the polling list.
 *
 ******************************************************************************/
void fnet_tftp_srv_release(fnet_tftp_srv_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Retrieves the current state of the TFTP-server service.
 *
 * @param desc     TFTP-server descriptor.
 *
 * @return This function returns the current state of the TFTP-server service.
 * The state is defined by the @ref fnet_tftp_srv_state_t.
 *
 ******************************************************************************
 *
 * This function returns the current state of the TFTP-server service.
 * If the state is @ref FNET_TFTP_SRV_STATE_DISABLED, the TFTP server is not initialized
 * or released.
 *
 ******************************************************************************/
fnet_tftp_srv_state_t fnet_tftp_srv_state(fnet_tftp_srv_desc_t desc);

/*! @} */


#endif /* FNET_CFG_TFTP_SRV */


#endif /* _FNET_TFTP_SRV_H_ */

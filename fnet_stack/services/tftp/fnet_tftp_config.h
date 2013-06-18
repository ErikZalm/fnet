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
* If not, see <http://www.gnu.org/licenses/>..
*
**********************************************************************/ /*!
*
* @file fnet_tftp_config.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.11.0
*
* @brief TFTP services configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_TFTP_CONFIG_H_

#define _FNET_TFTP_CONFIG_H_

/*! @addtogroup fnet_tftp_config */
/*! @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_TFTP_CLN
 * @brief    TFTP Client support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 ******************************************************************************/
#ifndef FNET_CFG_TFTP_CLN
    #define FNET_CFG_TFTP_CLN   (0)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_TFTP_SRV
 * @brief    TFTP Server support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 ******************************************************************************/
#ifndef FNET_CFG_TFTP_SRV
    #define FNET_CFG_TFTP_SRV   (0)
#endif

/****************************************************************************** 
 *              TFTP-client service config parameters
 ******************************************************************************/

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_CLN_PORT
 * @brief   TFTP server port number (in network byte order) used by TFTP-client
 *          service. @n
 *          Default value is FNET_HTONS(69).
 * @showinitializer 
 ******************************************************************************/ 
#ifndef FNET_CFG_TFTP_CLN_PORT
    #define FNET_CFG_TFTP_CLN_PORT              (FNET_HTONS(69))
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_CLN_TIMEOUT
 * @brief   Timeout for TFTP server response in seconds. @n
 *          If no response from a TFTP server is received during this timeout,
 *          the TFTP-client service is released automatically.@n
 *          Default value is @b @c 10.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_TFTP_CLN_TIMEOUT
    #define FNET_CFG_TFTP_CLN_TIMEOUT           (10) /*sec*/
#endif

/****************************************************************************** 
 *              TFTP-server service config parameters
 ******************************************************************************/

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_SRV_PORT
 * @brief   Default TFTP server port number (in network byte order) used by TFTP-server service.@n
 *          It can be changed during the TFTP server initialization by the 
 *          @ref fnet_tftp_srv_init() function. @n
 *          Default value is FNET_HTONS(69).
 * @showinitializer 
 ******************************************************************************/ 

#ifndef FNET_CFG_TFTP_SRV_PORT
    #define FNET_CFG_TFTP_SRV_PORT              (FNET_HTONS(69))
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_SRV_MAX
 * @brief   Maximum number of the TFTP Servers that can be run simultaneously. @n
 *          Default value is @b @c 1. 
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_TFTP_SRV_MAX
    #define FNET_CFG_TFTP_SRV_MAX               (1)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_SRV_TIMEOUT
 * @brief   Default timeout for TFTP client response in seconds. @n
 *          If no response from a TFTP client is received during this timeout,
 *          the last packet is retransmitted to the TFTP client automatically.@n
 *          It can be changed during the TFTP server initialization by the 
 *          @ref fnet_tftp_srv_init() function. @n
 *          Default value is @b @c 3.  
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_TFTP_SRV_TIMEOUT
    #define FNET_CFG_TFTP_SRV_TIMEOUT           (3) /*sec*/
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_TFTP_SRV_RETRANSMIT_MAX
 * @brief   Default maximum number of retransmissions. @n
 *          If no response from a TFTP client is received
 *          till maximum retransmission number is reached, 
 *          the TFTP server cancels the data transfer.@n
 *          It can be changed during the TFTP server initialization by the 
 *          @ref fnet_tftp_srv_init() function. @n
 *          Default value is @b @c 4. 
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_TFTP_SRV_RETRANSMIT_MAX
    #define FNET_CFG_TFTP_SRV_RETRANSMIT_MAX    (4) 
#endif


/*! @} */


#endif /* _FNET_TFTP_CONFIG_H_ */

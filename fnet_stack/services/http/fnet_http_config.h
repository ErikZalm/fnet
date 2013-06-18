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
**********************************************************************/ /*!
*
* @file fnet_http_config.h
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.0.22.0
*
* @brief FNET HTTP Server configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_HTTP_CONFIG_H_

#define _FNET_HTTP_CONFIG_H_

/*! @addtogroup fnet_http_config */
/*! @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_HTTP
 * @brief    HTTP Server service support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 ******************************************************************************/
#ifndef FNET_CFG_HTTP
    #define FNET_CFG_HTTP       (0)
#endif

#if FNET_CFG_HTTP
    /* Force FS if HTTP is defined. */
    #undef FNET_CFG_FS
    #define FNET_CFG_FS         (1)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_MAX
 * @brief   Maximum number of the HTTP Servers that can be run simultaneously.@n
 *          Default value @b @c 1.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_HTTP_MAX
    #define FNET_CFG_HTTP_MAX               (1)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_SESSION_MAX
 * @brief   Maximum number of simultaneous user-session that can be handled 
 *          by the HTTP server.@n
 *          Default value is @b @c 3.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_HTTP_SESSION_MAX
    #define FNET_CFG_HTTP_SESSION_MAX       (3)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_SSI
 * @brief   HTTP Server SSI (Server Side Includes) support:
 *               - @b @c 1 = is enabled (Default value).
 *               - @c 0 = is disabled.
 * @showinitializer 
 ******************************************************************************/ 
#ifndef FNET_CFG_HTTP_SSI
    #define FNET_CFG_HTTP_SSI               (1) 
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_CGI
 * @brief   HTTP Server CGI (Common Gateway Interface) support:
 *               - @b @c 1 = is enabled (Default value).
 *               - @c 0 = is disabled.
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_HTTP_CGI
    #define FNET_CFG_HTTP_CGI               (1) 
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_PORT
 * @brief   Default HTTP port number (in network byte order).@n
 *          It can be changed during the HTTP server initialization by the 
 *          @ref fnet_http_init() function.@n
 *          Default value FNET_HTONS(80).
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_HTTP_PORT
    #define FNET_CFG_HTTP_PORT              (FNET_HTONS(80))
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_REQUEST_SIZE_MAX
 * @brief   Maximum size of an incoming request.@n 
 *          Also it defines the maximum number of bytes to use for internal 
 *          buffering (parsing, receive and transmit buffering).@n
 *          Default value @b @c 300.
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_HTTP_REQUEST_SIZE_MAX
    #define FNET_CFG_HTTP_REQUEST_SIZE_MAX  (300) 
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_VERSION_MAJOR
 * @brief   Hypertext Transfer Protocol HTTP version 1.x support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled. It means HTTP/0.9 (Default value).
 ******************************************************************************/  
#ifndef FNET_CFG_HTTP_VERSION_MAJOR
    #define FNET_CFG_HTTP_VERSION_MAJOR     (0) 
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_AUTHENTICATION_BASIC
 * @brief   The HTTP/1.x Basic Authentification Scheme (RFC2617) support:
 *               - @b @c 1 = is enabled (default value).
 *               - @c 0 = is disabled.
 ******************************************************************************/  
#ifndef FNET_CFG_HTTP_AUTHENTICATION_BASIC
    #define FNET_CFG_HTTP_AUTHENTICATION_BASIC  (0) 
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_HTTP_POST
 * @brief   The HTTP/1.x POST method support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 * @showinitializer 
 ******************************************************************************/  
#ifndef FNET_CFG_HTTP_POST
    #define FNET_CFG_HTTP_POST                  (0) 
#endif

#if FNET_CFG_HTTP_AUTHENTICATION_BASIC || FNET_CFG_HTTP_POST 
    /* Push HTTP/1.0*/
    #undef FNET_CFG_HTTP_VERSION_MAJOR
    #define FNET_CFG_HTTP_VERSION_MAJOR     (1)
#endif

/*! @} */

#endif /* _FNET_HTTP_CONFIG_H_ */

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
* @file fnet_http_get.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.16.0
*
* @brief FNET HTTP Server GET method implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_HTTP && FNET_CFG_FS

#include "fnet_http.h"
#include "fnet_http_prv.h"
#include "fnet_timer.h"
#include "fnet_eth.h"
#include "fnet_debug.h"
#include "fnet_stdlib.h"
#include "fnet_fs.h"

/* Prototypes */
static int fnet_http_get_handle(struct fnet_http_if * http, struct fnet_http_uri * uri);
static int fnet_http_get_send(struct fnet_http_if * http);
static void fnet_http_get_close(struct fnet_http_if * http);

/* GET method. */
const struct fnet_http_method fnet_http_method_get =
{
    "GET", 
    fnet_http_get_handle, 
    0,
    fnet_http_get_send,
    fnet_http_get_close
};

/************************************************************************
* NAME: fnet_http_get_handle
*
* DESCRIPTION: 
************************************************************************/
static int fnet_http_get_handle(struct fnet_http_if * http, struct fnet_http_uri * uri)
{
    struct fnet_http_session_if *session =  http->session_active;
    int                         result = FNET_ERR;
    
    /* Request is found */
    if(uri)
    {
        session->response.send_file_handler = fnet_http_find_handler(http, uri);           /* Find file handler.*/

    #if FNET_CFG_HTTP_VERSION_MAJOR
        session->response.send_file_content_type = fnet_http_find_content_type(http, uri); /* Find file content-type.*/
    #endif        
       
        result = session->response.send_file_handler->file_handle(http, uri);              /* Initial handling. */
    }

    return result;
}

/************************************************************************
* NAME: fnet_http_get_send
*
* DESCRIPTION: Simple-Response. Simple-responce consists only of the 
* entity body and is terminated by the server closing connection.
************************************************************************/
static int fnet_http_get_send(struct fnet_http_if * http)
{
    struct fnet_http_session_if *session =  http->session_active;
    int                         result;
    
    if((session->buffer_actual_size = session->response.send_file_handler->file_send(http)) > 0)
        result = FNET_OK;                            
    else
        result = FNET_ERR;
    
    return result;
}

/************************************************************************
* NAME: fnet_http_get_close
*
* DESCRIPTION: 
************************************************************************/
static void fnet_http_get_close(struct fnet_http_if * http)
{
    struct fnet_http_session_if *session =  http->session_active;

    if(session->response.send_file_handler && session->response.send_file_handler->file_close)
        session->response.send_file_handler->file_close(http);
}

#endif

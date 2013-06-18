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
* @file fnet_http_post.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.10.0
*
* @brief The FNET HTTP server POST method implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_HTTP && FNET_CFG_HTTP_POST

#include "fnet_http.h"
#include "fnet_http_prv.h"
#include "fnet_timer.h"
#include "fnet_debug.h"

#include "fnet_http_post.h"



/* Prototypes */
static int fnet_http_post_handle(struct fnet_http_if * http, struct fnet_http_uri * uri);
static int fnet_http_post_receive(struct fnet_http_if * http);
static int fnet_http_post_send(struct fnet_http_if * http);

/* POST method. */
const struct fnet_http_method fnet_http_method_post =
{
    "POST", 
    fnet_http_post_handle,
    fnet_http_post_receive, 
    fnet_http_post_send,
    0 
};

/************************************************************************
* NAME: fnet_http_post_handle
*
* DESCRIPTION: 
************************************************************************/
static int fnet_http_post_handle(struct fnet_http_if * http, struct fnet_http_uri * uri)
{
    struct fnet_http_session_if *session =  http->session_active;
    int                         result = FNET_ERR;
    const struct fnet_http_post *post_ptr;
    
    if(http->post_table)
    /* POST table is initialized.*/
    {
        /* Skip first '/' and ' ' */
        while(*uri->path == '/' || *uri->path == ' ')
            uri->path++;
        
        session->send_param.data_ptr = 0; /* Clear. */    
        
        /* Find POST function */
        for(post_ptr = http->post_table; post_ptr->name; post_ptr++)
    	{
    	    if (!fnet_strcmp(uri->path, post_ptr->name)) 
    		{				 
    		    session->send_param.data_ptr = (void*)post_ptr;
    		    if(post_ptr->handle)
    		        result = post_ptr->handle(uri->query, &session->response.cookie);
    		    else
    		        result = FNET_OK;
    		        
    	        break;
    	    }
    	}
	}
    return result;
}

/************************************************************************
* NAME: fnet_http_post_receive
*
* DESCRIPTION: 
************************************************************************/
static int fnet_http_post_receive(struct fnet_http_if * http)
{
    struct fnet_http_session_if *session =  http->session_active;    
    int                         result = FNET_ERR;
    const struct fnet_http_post * post_ptr;
    
    if(session->send_param.data_ptr)
    {
        post_ptr = (const struct fnet_http_post *)session->send_param.data_ptr;
        
        if(post_ptr->receive)
            result = post_ptr->receive(session->buffer, session->buffer_actual_size, &session->response.cookie);
        else
            result = FNET_OK;
    }
    
    return result;
}

/************************************************************************
* NAME: fnet_http_post_send
*
* DESCRIPTION: 
************************************************************************/
static int fnet_http_post_send(struct fnet_http_if * http)
{
    struct fnet_http_session_if *session =  http->session_active; 
    int                         result = FNET_ERR;
    const struct fnet_http_post *post_ptr = (const struct fnet_http_post *) session->send_param.data_ptr;
    
    if(post_ptr && post_ptr->send)
        if(post_ptr->send(session->buffer, sizeof(session->buffer), &session->response.send_eof, &session->response.cookie) > 0)
                result = FNET_OK;
        
    return result;
}

#endif

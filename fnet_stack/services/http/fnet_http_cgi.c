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
* @file fnet_http_cgi.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.19.0
*
* @brief FNET HTTP server  CGI implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_HTTP && FNET_CFG_HTTP_CGI

#include "fnet_http.h"
#include "fnet_http_prv.h"
#include "fnet_timer.h"
#include "fnet_eth.h"
#include "fnet_debug.h"
#include "fnet_stdlib.h"
#include "fnet_fs.h"


static int fnet_http_cgi_handle (struct fnet_http_if * http, struct fnet_http_uri * uri);
static unsigned long fnet_http_cgi_send (struct fnet_http_if * http);

/************************************************************************
*     Definitions
************************************************************************/

const struct fnet_http_file_handler fnet_http_cgi_handler =
{
    FNET_HTTP_CGI_EXTENSION,  
    fnet_http_cgi_handle, 
    fnet_http_cgi_send, 
    0
};

/************************************************************************
* NAME: fnet_http_cgi_handle
*
* DESCRIPTION: 
************************************************************************/
static int fnet_http_cgi_handle (struct fnet_http_if * http, struct fnet_http_uri * uri)
{
    int                         result = FNET_ERR;
    const struct                fnet_http_cgi *cgi_ptr;
    struct fnet_http_session_if *session =  http->session_active;
    
    if(http->cgi_table)
    /* CGI table is initialized.*/
    {
        /* Skip first '/' and ' ' */
        while(*uri->path == '/' || *uri->path == ' ')
            uri->path++;
        
        session->send_param.data_ptr = 0; /* Clear. */    
        
        /* Find CGI function */
        for(cgi_ptr = http->cgi_table; cgi_ptr->name; cgi_ptr++)
    	{
    	    if (!fnet_strncmp( uri->path, 
    		                   cgi_ptr->name,
    		                   fnet_strlen(cgi_ptr->name))) 
    		{				 
    		    session->send_param.data_ptr = (void*)cgi_ptr;
    		    if(cgi_ptr->handle)
    		        result = cgi_ptr->handle(uri->query, &session->response.cookie);
    		    else
    		        result = FNET_OK;
    		        
    	        break;
    	    }
    	}
	}
    return result;
}

/************************************************************************
* NAME: fnet_http_cgi_send
*
* DESCRIPTION: 
************************************************************************/
static unsigned long fnet_http_cgi_send (struct fnet_http_if * http)
{
    const struct fnet_http_cgi  *cgi_ptr;
    unsigned long               result = 0;
    struct fnet_http_session_if *session =  http->session_active;
    
    if(session->send_param.data_ptr)
    {
        cgi_ptr = (const struct fnet_http_cgi *) session->send_param.data_ptr;
        
        if(cgi_ptr->send)
            result = cgi_ptr->send(session->buffer, sizeof(session->buffer), &session->response.send_eof, &session->response.cookie);
    }
        
    return result;
}

#endif /* FNET_CFG_HTTP && FNET_CFG_HTTP_CGI */

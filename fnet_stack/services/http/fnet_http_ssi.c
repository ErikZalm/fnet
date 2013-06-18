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
* @file fnet_http_ssi.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.22.0
*
* @brief FNET HTTP Server SSI implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_HTTP && FNET_CFG_HTTP_SSI

#include "fnet_http.h"
#include "fnet_http_prv.h"
#include "fnet_timer.h"
#include "fnet_eth.h"
#include "fnet_debug.h"
#include "fnet_stdlib.h"
#include "fnet_fs.h"
#include "fnet_http_ssi.h"
#include "fnet_http_ssi_prv.h"

/************************************************************************
*     Definitions
************************************************************************/
static unsigned long fnet_http_ssi_send (struct fnet_http_if * http);

#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
static int fnet_http_ssi_handle (struct fnet_http_if * http, struct fnet_http_uri * uri);
#endif

static const char fnet_http_ssi_head[] = {'<','!','-','-','#'};
static const char fnet_http_ssi_tail[] = {'-','-','>'};

const struct fnet_http_file_handler fnet_http_ssi_handler =
{
    FNET_HTTP_SSI_EXTENSION,
#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
    fnet_http_ssi_handle, 
#else
    fnet_http_default_handle, 
#endif    
    
    fnet_http_ssi_send, 
    fnet_http_default_close            
};

#if FNET_CFG_HTTP_VERSION_MAJOR /* HTTP/1.x*/
/************************************************************************
* NAME: fnet_http_ssi_handle
*
* DESCRIPTION: 
************************************************************************/
static int fnet_http_ssi_handle (struct fnet_http_if * http, struct fnet_http_uri * uri)
{
    int     result = fnet_http_default_handle (http, uri);
    http->session_active->response.content_length = -1; /* No content length.*/ 
    return result;
}
#endif

/************************************************************************
* NAME: fnet_http_ssi_send
*
* DESCRIPTION: 
************************************************************************/
static unsigned long fnet_http_ssi_send (struct fnet_http_if * http)
{
    unsigned long               result = 0;
    unsigned long               read_result = 0;
    int                         ssi_head_index = 0;
    int                         ssi_tail_index = 0;
    struct fnet_http_session_if *session =  http->session_active; 
    char                        *buffer = session->buffer;
    int                         next = 0;
    
    while (result<sizeof(session->buffer) && (next == 0))
    {
        if(http->ssi.state != FNET_HTTP_SSI_INCLUDING) /* Read from file if not in including. */
            if((read_result = fnet_fs_fread(buffer, 1, session->send_param.file_desc)) == 0)
                break; /*EOF*/
        
        switch (http->ssi.state)
        {
            case FNET_HTTP_SSI_WAIT_HEAD:
                if(*buffer == fnet_http_ssi_head[ssi_head_index])
                {
                    ssi_head_index++;
                    if(ssi_head_index == sizeof(fnet_http_ssi_head))
                    { /* Head is found */
                        
                        if(result >= sizeof(fnet_http_ssi_head)) 
                        { /* Found in the middle */
                            fnet_fs_fseek (session->send_param.file_desc, -sizeof(fnet_http_ssi_head), FNET_FS_SEEK_CUR);
                            next = 1; /* break */
                            result -= sizeof(fnet_http_ssi_head); /* Correct result */
                        }
                        else
                            http->ssi.state = FNET_HTTP_SSI_WAIT_TAIL;
                        
                    }
                }
                else
                    ssi_head_index = 0;
                break;
                
            case FNET_HTTP_SSI_WAIT_TAIL:
                if(*buffer == fnet_http_ssi_tail[ssi_tail_index])
                {
                    ssi_tail_index++;
                    if(ssi_tail_index == sizeof(fnet_http_ssi_tail))
                    { /* Tail is found */
                        const struct fnet_http_ssi  *ssi_ptr = 0;
                        char                        *ssi_name = &session->buffer[sizeof(fnet_http_ssi_head)];
                        char                        *ssi_param;

                        http->ssi.send = 0;
                        
                        session->buffer[buffer + 1 - session->buffer - sizeof(fnet_http_ssi_tail)] = '\0'; /* Mark end of the SSI. */

                        /* Find SSI parameters. */
                        if((ssi_param = fnet_strchr( session->buffer, ' ' )) !=0)
                        {
                            *ssi_param = '\0';  /* Mark end of the SSI name. */
                            ssi_param ++;       /* Point to the begining of params. */
                        }
                        
                        if(http->ssi.ssi_table)
                        /* SSI table is initialized.*/
                        {
                            /* Find SSI handler */
    	                    for(ssi_ptr = http->ssi.ssi_table; ssi_ptr->name && ssi_ptr->send; ssi_ptr++)
    	                    {
    		                    if (!fnet_strcmp( ssi_name, 
    		                                        ssi_ptr->name))                    
    		                    {				 
    		                        http->ssi.send = ssi_ptr->send;
    		                        break;
    	                        }
    	                    }
	                    }
                       
                        read_result = 0; /* Eliminate the include. */
                        if(http->ssi.send)
                        { /* SSI Handler is found. */
                            if((ssi_ptr->handle == 0) || (ssi_ptr->handle(ssi_param, &session->response.cookie) == FNET_OK))
                            {
                                buffer = session->buffer; /* Reset */
                                result = 0;
                                
                                http->ssi.state = FNET_HTTP_SSI_INCLUDING;
                            }
                            else
                                http->ssi.state = FNET_HTTP_SSI_WAIT_HEAD;
                        }
                        else
                            http->ssi.state = FNET_HTTP_SSI_WAIT_HEAD;
                    }

                }
                else
                    ssi_tail_index = 0;
                break;
            case FNET_HTTP_SSI_INCLUDING:
                {
                    char eof;
                    read_result = (unsigned long) http->ssi.send(session->buffer, sizeof(session->buffer), &eof, &session->response.cookie);
                    if((read_result == 0) || (eof == 1))
                        http->ssi.state = FNET_HTTP_SSI_WAIT_HEAD;
                    
                    next = 1; /* break */
                }
                break;
        
        }
        buffer+=read_result;
        result+=read_result;
    }
    
    if(read_result && (next == 0) && ssi_head_index && (http->ssi.state == FNET_HTTP_SSI_WAIT_HEAD) )
    { /* Potential SSI is splitted => parse in the next itteration */
        result-=ssi_head_index; /* adjust result */
        fnet_fs_fseek(session->send_param.file_desc, -ssi_head_index, FNET_FS_SEEK_CUR);
    }
    
    return result;
}

#endif /* FNET_CFG_HTTP && FNET_CFG_HTTP_SSI */

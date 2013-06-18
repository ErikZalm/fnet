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
* @file fnet_http_auth.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.13.0
*
* @brief FNET HTTP Server Authentication implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_HTTP && FNET_CFG_HTTP_AUTHENTICATION_BASIC


#include "fnet_http_prv.h"
#include "fnet_debug.h"
#include "fnet_stdlib.h"
#include "fnet_http_auth_prv.h"
#include "fnet_serial.h"



/************************************************************************
*     Definitions
************************************************************************/
static void fnet_http_auth_decode_base64(char *src);
static int fnet_http_auth_scheme_basic_generate(struct fnet_http_if * http, char *buffer, unsigned int buffer_size);
static int fnet_http_auth_scheme_basic_validate (const struct fnet_http_auth *auth_entry, char * auth_param);
static unsigned char decode_base64_char(int c);

/************************************************************************
*     Authentication scheme table
************************************************************************/

#define FNET_HTTP_AUTH_SCHEME_TABLE_SIZE (FNET_CFG_HTTP_AUTHENTICATION_BASIC)

static const struct fnet_http_auth_scheme  fnet_http_auth_scheme_table[FNET_HTTP_AUTH_SCHEME_TABLE_SIZE]=
{
    {FNET_HTTP_AUTH_SCHEME_BASIC, "Basic", fnet_http_auth_scheme_basic_validate, fnet_http_auth_scheme_basic_generate},
    /* TBD {FNET_HTTP_AUTH_SCHEME_DIGEST, "Digest",0} */
};

/************************************************************************
* NAME: 
*
* DESCRIPTION: 
************************************************************************/
void fnet_http_auth_validate_uri(struct fnet_http_if * http)
{
    const struct fnet_http_auth     *auth_entry = http->auth_table;
    int                             i;
    struct fnet_http_session_if     *session =  http->session_active;

    if(auth_entry) /* Check if the table is defined.*/
    {
        /* Check if authorization is required for the dir. */
        while(auth_entry->dir_name)
        {
            if ( !fnet_strcmp_splitter(session->request.uri.path, auth_entry->dir_name, '/' ) )
            /* Authorization is required.*/
            {				 
                /* Find Authentication scheme.*/
                for(i=0; i<FNET_HTTP_AUTH_SCHEME_TABLE_SIZE; i++)
                {
                    if(fnet_http_auth_scheme_table[i].id == auth_entry->scheme)
                    {
                        session->response.auth_scheme = &fnet_http_auth_scheme_table[i];
                        break; /* Scheme is found.*/
                    }
                }
                
                if(session->response.auth_scheme)
                   session->response.auth_entry = auth_entry;
                   
                break; /* Exit.*/
            }
            auth_entry ++;
        }
    }    
}

/************************************************************************
* NAME: fnet_http_auth_validate_credentials
*
* DESCRIPTION: 
************************************************************************/
int fnet_http_auth_validate_credentials(struct fnet_http_if * http, char *credentials)
{
    const struct fnet_http_auth         *auth_entry = http->auth_table;
    struct fnet_http_session_if         *session =  http->session_active;
    const struct fnet_http_auth_scheme  *scheme = session->response.auth_scheme;
    int result = FNET_ERR;

    while (*credentials == ' ') 
        credentials++; /* Strip leading spaces */
        
    if ( !fnet_strcmp_splitter(credentials, scheme->name, ' ' ) )
    {
        char * auth_param = &credentials[fnet_strlen(scheme->name)];
            
        while (*auth_param == ' ') auth_param++; /* Strip leading spaces */
                
        /* Call scheme handler.*/
        result = scheme->validate(auth_entry, auth_param);            
    }

    return result;
}

/************************************************************************
* NAME: fnet_http_auth_generate_challenge
*
* DESCRIPTION: 
************************************************************************/
int fnet_http_auth_generate_challenge(struct fnet_http_if * http, char *buffer, unsigned int buffer_size)
{
    int                         result = 0;
    struct fnet_http_session_if *session =  http->session_active;
    
    /* Print auth-scheme.*/
    result += fnet_snprintf(buffer, buffer_size, "%s ", session->response.auth_scheme->name);
    /* Print auth-params.*/
    result += session->response.auth_scheme->generate(http, &buffer[result], buffer_size - result); 
    
    return result;
}

/************************************************************************
* NAME: fnet_http_auth_scheme_basic_validate
*
* DESCRIPTION: 
************************************************************************/
static int fnet_http_auth_scheme_basic_validate (const struct fnet_http_auth *auth_entry, char * auth_param)
{
    int result =  FNET_ERR;
    char *password;

    /* Base64 => "userid:password".*/
    fnet_http_auth_decode_base64(auth_param); 

    if((password = fnet_strchr( auth_param, ':' ))!=0) /* Find end of the "userid".*/
    {
        *password++= '\0'; /* Mark end of userid.*/
        if(fnet_strcmp(auth_param, auth_entry->userid) == 0) /* Check "userid".*/
        {
            if(fnet_strcmp(password, auth_entry->password) == 0) /* Check "password".*/
            {
                result = FNET_OK; 
            }
        }
    }
    
    return result;    
}

/************************************************************************
* NAME: decode_base64_char
*
* DESCRIPTION: Decode a base64 character.
************************************************************************/
static unsigned char decode_base64_char(int c) 
{
    unsigned char result;
    if(c >= 'A' && c <= 'Z') 
        result = (unsigned char)(c - 'A');
    else if(c >= 'a' && c <= 'z') 
        result = (unsigned char)(c - 'a' + 26);
    else if(c >= '0' && c <= '9') 
        result = (unsigned char)(c - '0' + 52);
    else if(c == '+')             
        result = (unsigned char)62;
    else
        result = 63;
        
    return result;
}

/************************************************************************
* NAME: fnet_http_auth_decode_base64
*
* DESCRIPTION: Decode the base64 encoded string.
************************************************************************/
static void fnet_http_auth_decode_base64(char *src) 
{
    unsigned char *dest = (unsigned char *)src;
    int k;
    int length = (int)fnet_strlen(src);
  
    
    for(k=0; k<length; k+=4) 
    {
        char c1='A', c2='A', c3='A', c4='A';
        unsigned char b1=0, b2=0, b3=0, b4=0;
      
        c1= src[k];
      
        if(k+1<length) 
            c2= src[k+1];
        if(k+2<length) 
            c3= src[k+2];
        if(k+3<length) 
            c4= src[k+3];
      
        b1= decode_base64_char(c1);
        b2= decode_base64_char(c2);
        b3= decode_base64_char(c3);
        b4= decode_base64_char(c4);
      
        *dest++=(unsigned char)((b1<<2)|(b2>>4) );
      
        if(c3 != '=') 
            *dest++=(unsigned char)(((b2&0xf)<<4)|(b3>>2) );
      
        if(c4 != '=') 
            *dest++=(unsigned char)(((b3&0x3)<<6)|b4 );
    }
    
    /* Mark end of line.*/
    *dest=(unsigned char)'\0';
}

/************************************************************************
* NAME: fnet_http_auth_decode_base64
*
* DESCRIPTION: Decode the base64 encoded string.
************************************************************************/
static int fnet_http_auth_scheme_basic_generate(struct fnet_http_if * http, char *buffer, unsigned int buffer_size)
{
    int result = 0;
    
    result += fnet_snprintf(buffer, buffer_size, "realm=\"%s\"%s", http->session_active->response.auth_entry->dir_name, "\r\n" );
    
    return result;
}

#endif

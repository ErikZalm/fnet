/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2009 by Andrey Butok. Freescale Semiconductor, Inc.
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
* @file fnet_http_auth_prv.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.6.0
*
* @brief Private. FNET HTTP Server Authentication API.
*
***************************************************************************/

#ifndef _FNET_HTTP_AUTH_PRV_H_

#define _FNET_HTTP_AUTH_PRV_H_

#include "fnet_config.h"


#if FNET_CFG_HTTP && FNET_CFG_HTTP_AUTHENTICATION_BASIC


#include "fnet.h"
#include "fnet_http_auth.h"

typedef int(*fnet_http_auth_scheme_validate_t)(const struct fnet_http_auth *auth_entry, char * auth_param);
typedef int(*fnet_http_auth_scheme_generate_t)(struct fnet_http_if * http, char *buffer, unsigned int buffer_size);

struct fnet_http_auth_scheme
{
    fnet_http_auth_scheme_t id;
    const char *name;
    fnet_http_auth_scheme_validate_t validate; /* Validate credentials params.*/
    fnet_http_auth_scheme_generate_t generate; /* Generate challenge params.*/
};

void fnet_http_auth_validate_uri(struct fnet_http_if * http);
int fnet_http_auth_validate_credentials(struct fnet_http_if * http, char *credentials);
int fnet_http_auth_generate_challenge(struct fnet_http_if * http, char *buffer, unsigned int buffer_size);

#endif


#endif

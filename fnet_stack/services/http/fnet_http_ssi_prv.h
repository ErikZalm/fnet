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
**********************************************************************/
/*!
*
* @file fnet_http_ssi_prv.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.8.0
*
* @brief Private. FNET HTTP Server SSI API.
*
***************************************************************************/

#ifndef _FNET_HTTP_SSI_PRV_H_

#define _FNET_HTTP_SSI_PRV_H_

#include "fnet_config.h"


#if FNET_CFG_HTTP && FNET_CFG_HTTP_SSI


#include "fnet.h"

/* SSI statemachine state. */
typedef enum
{
    FNET_HTTP_SSI_WAIT_HEAD = 0,
    FNET_HTTP_SSI_WAIT_TAIL,
    FNET_HTTP_SSI_INCLUDING
}
fnet_http_ssi_state_t;

/* SSI private control structure. */
struct fnet_http_ssi_if
{
    const struct fnet_http_ssi *ssi_table; /* Pointer to the SSI table.*/
    fnet_http_ssi_send_t send;    /* Pointer to the respond callback.*/
    fnet_http_ssi_state_t state;        /* State. */
};

extern const struct fnet_http_file_handler fnet_http_ssi_handler; /* SSI file handler. */


#endif


#endif

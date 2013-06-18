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
* @file fnet_poll.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.13.0
*
* @brief FNET Services polling mechanism implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet.h"

#include "fnet_poll.h"
#include "fnet_error.h"


/************************************************************************
*     Definitions
*************************************************************************/

/* Polling list element type definition */

typedef struct
{
    fnet_poll_service_t service;
    void *service_param;
} fnet_poll_list_entry_t;

/* Polling interface structure */
static struct
{
    fnet_poll_list_entry_t list[FNET_CFG_POLL_MAX]; /* Polling list.*/
    fnet_poll_desc_t last;                      /* Index of the last valid entry plus 1, in the polling list.*/
} fnet_poll_if;

/************************************************************************
* NAME: fnet_poll_services
*
* DESCRIPTION: This function calls all registered service routines in 
*              the polling list.
*************************************************************************/
void fnet_poll_services( void )
{
    fnet_poll_desc_t i;

    for (i = 0; i < fnet_poll_if.last; i++)
    {
        if(fnet_poll_if.list[i].service)
            fnet_poll_if.list[i].service(fnet_poll_if.list[i].service_param);
    }
}

/************************************************************************
* NAME: fnet_poll_services_release
*
* DESCRIPTION: This function calls all registered service routines in 
*              the polling list.
*************************************************************************/
void fnet_poll_services_release( void )
{
    fnet_memset_zero(&fnet_poll_if, sizeof(fnet_poll_if));
}

/************************************************************************
* NAME: fnet_poll_service_register
*
* DESCRIPTION: This function adds service routine into the polling list.
*************************************************************************/
fnet_poll_desc_t fnet_poll_service_register( fnet_poll_service_t service, void *service_param )
{
    fnet_poll_desc_t i = 0;
    fnet_poll_desc_t result = (fnet_poll_desc_t)FNET_ERR;

    if(service)
    {
        while(fnet_poll_if.list[i].service && i < FNET_CFG_POLL_MAX)
        {
            i++;
        };

        if(i != FNET_CFG_POLL_MAX)
        {
            fnet_poll_if.list[i].service = service;
            fnet_poll_if.list[i].service_param = service_param;
            result = i;

            if(result >= fnet_poll_if.last)
                fnet_poll_if.last = result + 1;
        }
    }

    return result;
}

/************************************************************************
* NAME: fnet_poll_service_unregister
*
* DESCRIPTION: This function removes service routine from the polling list.
*************************************************************************/
int fnet_poll_service_unregister( fnet_poll_desc_t descriptor )
{
    int result;

    if(descriptor < FNET_CFG_POLL_MAX)
    {
        fnet_poll_if.list[descriptor].service = 0;
        result = FNET_OK;
    }
    else
        result = FNET_ERR;

    return result;
}


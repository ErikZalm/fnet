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
**********************************************************************//*!
*
* @file fnet_stack.c
*
* @date Dec-19-2012
*
* @version 0.1.12.0
*
* @brief FNET init/release functions.
*
***************************************************************************/

#include "fnet.h"
#include "fnet_socket_prv.h"
#include "fnet_prot.h"

/************************************************************************
*     Function Prototypes
*************************************************************************/
static int fnet_stack_init( void );
static void fnet_stack_release( void );


/************************************************************************
* NAME: fnet_init
*
* DESCRIPTION: 
*************************************************************************/
int fnet_init( struct fnet_init_params *init_params )
{
    int result = FNET_ERR;

    if(init_params 
        && (fnet_os_mutex_init() == FNET_OK)
        && (fnet_os_event_init() == FNET_OK))
    {
        fnet_os_mutex_lock();

        if(fnet_enabled == 0) /* Is enabled already?. */
        {
            if((result = fnet_heap_init(init_params->netheap_ptr, init_params->netheap_size)) == FNET_OK )
              if((result = fnet_stack_init()) == FNET_OK)
                fnet_enabled = 1; /* Mark the stack is enabled. */
        }

        fnet_os_mutex_unlock();
    }

    return result;
}

/************************************************************************
* NAME: fnet_init_static
*
* DESCRIPTION: 
*************************************************************************/
int fnet_init_static()
{
    static unsigned char heap[FNET_CFG_HEAP_SIZE];
    struct fnet_init_params init_params;

    init_params.netheap_ptr = heap;
    init_params.netheap_size = FNET_CFG_HEAP_SIZE;

    return fnet_init(&init_params);
}

/************************************************************************
* NAME: fnet_release
*
* DESCRIPTION: 
*************************************************************************/
void fnet_release()
{
    fnet_os_mutex_lock();

    if(fnet_enabled)
    {
        fnet_stack_release();
        fnet_enabled = 0;
    }

    fnet_os_mutex_unlock();

    fnet_os_mutex_release();
}

/************************************************************************
* NAME: fnet_stack_init
*
* DESCRIPTION: TCP/IP Stack initialization.
************************************************************************/
static int fnet_stack_init( void )
{
    fnet_isr_init();
   
    if (fnet_timer_init(FNET_TIMER_PERIOD_MS) == FNET_ERR)
        goto ERROR;

#if FNET_CFG_DEBUG_STARTUP_MS
	fnet_println("\n Waiting %d Seconds...", FNET_CFG_DEBUG_STARTUP_MS/1000);
	fnet_timer_delay(fnet_timer_ms2ticks(FNET_CFG_DEBUG_STARTUP_MS));
#endif
 
    if(fnet_prot_init() == FNET_ERR)
        goto ERROR;
    fnet_socket_init();

    if(fnet_netif_init_all() == FNET_ERR)
        goto ERROR;

    return (FNET_OK);
ERROR:
    fnet_stack_release();

    return (FNET_ERR);
}

/************************************************************************
* NAME: fnet_stack_release
*
* DESCRIPTION: TCP/IP Stack release.
************************************************************************/
static void fnet_stack_release( void )
{
    fnet_netif_release_all();
    fnet_prot_release();
    fnet_timer_release();
    fnet_mem_release();
}

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
* @file fnet_stack.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.48.0
*
* @brief Main including header for the FNET TCP/IP stack.
*
***************************************************************************/

#ifndef _FNET_STACK_H_

#define _FNET_STACK_H_

#include "fnet.h"
#include "fnet_socket.h"
#include "fnet_inet.h"
#include "fnet_ip.h"
#include "fnet_ip6.h"
#include "fnet_netif.h"
#include "fnet_timer.h"
#include "fnet_error.h"
#include "fnet_stdlib.h"
#include "fnet_debug.h"
#include "fnet_eth.h"
#include "fnet_isr.h"


/*! @addtogroup fnet_stack_init
* - The @ref fnet.h file includes all the other header files needed to use the FNET TCP/IP stack
*   user interface. This means that it is the only file the application developer needs to include 
*   in the source code using the FNET stack API.
* - The function @ref fnet_init() or @ref fnet_init_static() must be called
*   in order to initialize the FNET TCP/IP stack.
*   The return value from @ref fnet_init()/@ref fnet_init_static() must be verified to indicate the success before
*   calling any other TCP/IP functions.
* - After @ref fnet_init()/@ref fnet_init_static() returns the @ref FNET_OK value, the FNET TCP/IP stack is ready
*   for data transmission.
*
* For example:
* @code
* ...
*    static unsigned char stack_heap[FNET_CFG_HEAP_SIZE];
*    struct fnet_init_params init_params;
*
*    // Input parameters for FNET stack initialization.
*    init_params.netheap_ptr = stack_heap;
*    init_params.netheap_size = FNET_CFG_HEAP_SIZE;
*
*    // Init FNET stack.
*    if(fnet_init(&init_params) != FNET_ERR)
*    {
*        // Place your code here.
*    }
*    else
*        fnet_printf("ERROR: FNET stack initialization is failed!\n");
* ...
* @endcode
*/
/*! @{ */

/**************************************************************************/ /*!
 * @brief Input parameters structure for @ref fnet_init()
 ******************************************************************************/
struct fnet_init_params
{
    void *netheap_ptr; /**< @brief Pointer to the FNET heap buffer. @n 
                        * @n 
                        * The FNET uses this heap buffer for the internal
                        * dynamic data allocation as:
                        *  - Ethernet Tx/Rx frame buffers.
                        *  - Sockets Input/Output buffers.
                        *  - Protocol headers and service information.
                        *  - Various control structures.
                        *  - Temporary data.@n
                        * @n
                        * An application can allocate this buffer statically, 
                        * dynamically, or use a special memory region (for example SRAM).
                        */
    unsigned long netheap_size;/**< @brief Size of the FNET heap buffer. */
};
 
/***************************************************************************/ /*!
 *
 * @brief    Initializes the FNET TCP/IP stack.
 *
 * @param init_params    Pointer to the initialization parameter structure.
 *
 * @return   This function returns:
 *   - @c FNET_OK  = Stack initialization is successful.
 *   - @c FNET_ERR = Stack initialization has failed.
 *
 *
 * @see fnet_init_static(), fnet_release()
 *
 ******************************************************************************
 *
 * This function executes the initialization of the FNET TCP/IP stack.@n
 * Only after a succesful initialization, the application may use other FNET API 
 * functions and services.
 *
 ******************************************************************************/ 
int fnet_init( struct fnet_init_params *init_params );

/***************************************************************************/ /*!
 *
 * @brief    Initializes the FNET TCP/IP stack with an internally pre-allocated 
 * static heap buffer.
 *
 * @return   This function returns:
 *   - @c FNET_OK  = Stack initialization is successful.
 *   - @c FNET_ERR = Stack initialization has failed.
 *
 * @see fnet_init(), fnet_release()
 *
 ******************************************************************************
 *
 * This function executes the initialization of the FNET TCP/IP stack.
 * It's has the same functionality as @ref fnet_init().
 * The only difference is that the FNET heap buffer is allocated internally 
 * as a static buffer and its size is defined by the @ref FNET_CFG_HEAP_SIZE.@n
 * Only after a successful initialization, the application may use other FNET API 
 * functions and services.
 *
 ******************************************************************************/  
int fnet_init_static(void);

/***************************************************************************/ /*!
 *
 * @brief    Releases the FNET TCP/IP stack.
 *
 * @see fnet_init(), fnet_init_static()
 *
 ******************************************************************************
 *
 * This function releases all resources occupied by the FNET TCP/IP stack.
 * But it does not release resources occupied by FNET services.
 *
 ******************************************************************************/  
void fnet_release(void);


/*! @} */

#endif /* _FNET_STACK_H_ */


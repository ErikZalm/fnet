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
* @file fnet_flash.h
*
* @author Andrey Butok
*
* @date Jan-28-2013
*
* @version 0.1.10.0
*
* @brief On-chip Flash Module driver API.
*
***************************************************************************/
#ifndef _FNET_FLASH_H_

#define _FNET_FLASH_H_

#include "fnet_config.h" 

#if FNET_CFG_FLASH || defined(__DOXYGEN__)

/*! @addtogroup fnet_cfm 
*
* The Flash driver provides the ability to 
* reprogram the internal Flash memory while an application is running 
* in normal operational mode.@n
* @n
* The on-chip Flash module is a non-volatile memory (NVM) module 
* integrated  with a CPU. The Flash memory is ideal for program and 
* data storage for single-chip applications, allowing field 
* reprogramming without requiring external programming voltage sources.@n
* @n
*
* For Falsh driver example, refer to FNET demo application source code.@n
* @n
* Configuration parameters:
* - @ref FNET_CFG_FLASH 
* - @ref FNET_CFG_CPU_FLASH_ADDRESS
* - @ref FNET_CFG_CPU_FLASH_SIZE
* - @ref FNET_CFG_CPU_FLASH_PAGE_SIZE
*/
/*! @{ */

#if !FNET_CFG_CPU_FLASH
#error "Flash Module Driver is not implemented/tested for your platform!"
#endif


/***************************************************************************/ /*!
 *
 * @brief    Erases the specified range of the Flash memory.
 *
 * @param flash_addr      Address in the Flash to erase from.
 *
 * @param bytes           Number of bytes to erase in the Flash memory.
 *
 * @see fnet_flash_memcpy()
 *
 ******************************************************************************
 *
 * This function attempt to erase the number of @c bytes bytes beginning 
 * at @c flash_addr.@n
 * It should be noted that the Flash is block oriented when erasing. 
 * It is not possible to erase a few bytes within a page, the whole page will 
 * be erased.
 * The @c flash_addr parameter may be anywhere within the first page to be 
 * erased and @c flash_addr+ @c bytes may be anywhere in the last block to 
 * be erased. @n
 * Erase page size is defined by @ref FNET_CFG_CPU_FLASH_PAGE_SIZE.
 *
 ******************************************************************************/
void fnet_flash_erase( void *flash_addr, unsigned bytes);

/***************************************************************************/ /*!
 *
 * @brief    Writes the specified data to the Flash memory.
 *
 * @param flash_addr      Address in the Flash to write to.
 *
 * @param src             Pointer to the buffer containing the 
 *                        data to write to the Flash memory.
 *
 * @param bytes           Number of bytes contained in the data buffer
 *                        pointed by @c src.
 *
 * @see fnet_flash_erase()
 *
 ******************************************************************************
 *
 * This function copies the number of @c bytes bytes from the location
 * pointed by @c src directly to the Flash memory pointed by @c flash_addr.
 *
 ******************************************************************************/
void fnet_flash_memcpy( FNET_COMP_PACKED_VAR void *flash_addr, FNET_COMP_PACKED_VAR const void *src, unsigned bytes );

/*! @} */


#endif /* FNET_CFG_FLASH */

#endif /* _FNET_FLASH_H_ */

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
* @file fapp_mem.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.19.0
*
* @brief FNET Shell Demo API.
*
***************************************************************************/

#ifndef _FAPP_MEM_H_

#define _FAPP_MEM_H_

#include "fapp_config.h"
#include "fapp.h"

/************************************************************************
*     Definitions.
*************************************************************************/
#define FAPP_APPLICATION_ADDRESS    FAPP_CFG_APPLICATION_ADDRESS /* Actually is the start address of the flash.*/
#define FAPP_APPLICATION_SIZE       FAPP_CFG_APPLICATION_SIZE

#define FAPP_SRAM_ADDRESS           FNET_CFG_CPU_SRAM_ADDRESS
#define FAPP_SRAM_SIZE              FNET_CFG_CPU_SRAM_SIZE

#define FAPP_FLASH_ADDRESS          FNET_CFG_CPU_FLASH_ADDRESS
#define FAPP_FLASH_SIZE             FNET_CFG_CPU_FLASH_SIZE

#define FAPP_FLASH_PARAMS_SIZE      FNET_CFG_CPU_FLASH_PAGE_SIZE
#define FAPP_FLASH_PARAMS_ADDRESS   (FAPP_FLASH_ADDRESS + FAPP_FLASH_SIZE - FAPP_FLASH_PARAMS_SIZE) /* Last sectopr of the flash.*/

struct fapp_mem_region
{
    char *description;
    unsigned long address;
    unsigned long size;
    void (*memcpy)( FNET_COMP_PACKED_VAR void *dest, const FNET_COMP_PACKED_VAR void *src, unsigned n );
    void (*erase)( void *address, unsigned n );
    unsigned long erase_size; /* Logical page size, that can be erased separately. */
};

extern const struct fapp_mem_region fapp_mem_regions[];


#if FAPP_CFG_MEM_CMD
void fapp_mem_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
#endif

#if FAPP_CFG_ERASE_CMD
void fapp_mem_erase_cmd ( fnet_shell_desc_t desc, int argc, char ** argv );
#endif




int fapp_mem_memcpy (fnet_shell_desc_t desc, void *dest, const void *src, unsigned n );
int fapp_mem_region_is_protected( unsigned long start, unsigned long n);
void fapp_mem_erase_all(fnet_shell_desc_t desc);

#endif

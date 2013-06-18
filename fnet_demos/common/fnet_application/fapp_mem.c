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
* @file fapp_mem.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.23.0
*
* @brief FNET Shell Demo implementation.
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"
#include "fapp_mem.h"

#if FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD || FAPP_CFG_TFTPS_CMD || FAPP_CFG_ERASE_CMD || FAPP_CFG_MEM_CMD

/************************************************************************
*     Definitions.
*************************************************************************/


/* Flash regions should be aligned to logical border. */
struct fapp_mem_region_reserved
{
    char *description;
    unsigned long address;
    unsigned long size;
};



const struct fapp_mem_region fapp_mem_regions[] = 
{
#if FNET_CFG_FLASH 
    {"FLASH", FAPP_FLASH_ADDRESS, FAPP_FLASH_SIZE, fnet_flash_memcpy, fnet_flash_erase, FNET_CFG_CPU_FLASH_PAGE_SIZE},
#endif
    {"SRAM", FAPP_SRAM_ADDRESS, FAPP_SRAM_SIZE, fnet_memcpy, 0, 0},
    {0,0,0,0, 0, 0} /* End */
};


static const struct fapp_mem_region_reserved fapp_mem_regions_reserved[] = 
{
    {"FNET ROM", FAPP_APPLICATION_ADDRESS, FAPP_APPLICATION_SIZE},
    {"FNET Params", FAPP_FLASH_PARAMS_ADDRESS, FAPP_FLASH_PARAMS_SIZE},
    {0,0,0} /* End */
};


#define MEM_STR             "  %-14s   %#08X   %#08X"
#define FAPP_MEM_HEADER     "\n   %11s       Start         End"
#define FAPP_MEM_DELIMITER  "  ----------------------------------------"

#define FAPP_MEM_ERASE_ALL      "all"
#define FAPP_MEM_ERASE_ERASED   " 0x%08X to 0x%08X erased"
#define FAPP_MEM_ERASE_SKIPPED  " 0x%08X to 0x%08X skipped"
#define FAPP_MEM_ERASE_FAILED   " 0x%08X t7yo 0x%08X failed"


#define FAPP_MEM_ERROR_WRITEFAILED "\n Writing %d bytes to 0x%08X failed!"

/************************************************************************
* NAME: fapp_mem_region_is_protected
*
* DESCRIPTION: 
************************************************************************/
int fapp_mem_region_is_protected( unsigned long start, unsigned long n)
{
    int result = 0;
    const struct fapp_mem_region_reserved *region_reserved = fapp_mem_regions_reserved;

   	while(region_reserved->description)
	{
	    if((region_reserved->address < (start+n))&&( start <= (region_reserved->address + region_reserved->size - 1)))
	 	{
	        result = 1;
	        break;
	    }
	    region_reserved++;
	}
	
	return result;      
}

/************************************************************************
* NAME: fapp_mem_region_find
*
* DESCRIPTION: 
************************************************************************/
static const struct fapp_mem_region *fapp_mem_region_find( unsigned long start, unsigned long n)
{
    const struct fapp_mem_region *result = 0;
    const struct fapp_mem_region *region = fapp_mem_regions;

   	while(region->description)
	{
	    if((region->address <= start)&&( (start+n) <= region->address + region->size))
	 	{
	        result = region;
	        break;
	    }
	    region++;
	}
	
	return result;      
}

/************************************************************************
* NAME: fapp_mem_memcpy
*
* DESCRIPTION: 
************************************************************************/
int fapp_mem_memcpy (fnet_shell_desc_t desc, void *dest, const void *src, unsigned n )
{
    int i;
    int result = FNET_ERR;
    const struct fapp_mem_region *region;
	
	if(fapp_mem_region_is_protected( (unsigned long)dest, n) == 0)
	{
    	/* Find memory region.*/
        region = fapp_mem_region_find( (unsigned long) dest, n);
        
        if(region && region->memcpy)
        {
            region->memcpy(dest, src, n);
                
            /* Verify result. */
            for(i=0; i<n; i++)
            {
                if(((char *)dest)[i]!=((char *)src)[i])
                    break;    
            }
            if(i==n)
                result = FNET_OK;
        }
	}
	
	if(result == FNET_ERR)
	{
	    fnet_shell_println(desc, FAPP_MEM_ERROR_WRITEFAILED, n, dest);
	}
	    
    return result;
}

/************************************************************************
* NAME: fapp_mem_cmd
*
* DESCRIPTION: Shows the memory-map information for MCU. 
* Also shows the memory regions are reserved for FNET (protected).
************************************************************************/
#if FAPP_CFG_MEM_CMD
void fapp_mem_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
    const struct fapp_mem_region *mem = fapp_mem_regions;
	const struct fapp_mem_region_reserved *region_reserved = fapp_mem_regions_reserved;

    FNET_COMP_UNUSED_ARG(desc);
    FNET_COMP_UNUSED_ARG(argc);
    FNET_COMP_UNUSED_ARG(argv);

    /* Print memory types. */
	fnet_shell_println(desc, FAPP_MEM_HEADER, "Memory type");
	fnet_shell_println(desc, FAPP_MEM_DELIMITER);
	while(mem->description)
	{
	    fnet_shell_println(desc, MEM_STR, mem->description, mem->address,	
					        mem->address + mem->size - 1);
	    mem++;
	}

    /* Print protected/reserved memory regions.*/
	fnet_shell_println(desc, FAPP_MEM_HEADER, "Reserved");
	fnet_shell_println(desc, FAPP_MEM_DELIMITER);
	while(region_reserved->description)
	{
	    fnet_shell_println(desc, MEM_STR, region_reserved->description, region_reserved->address,	
									region_reserved->address + region_reserved->size - 1);
	    region_reserved++;
	}
    fnet_shell_println(desc, "");	

}
#endif


#if FAPP_CFG_ERASE_CMD
/************************************************************************
* NAME: fapp_mem_erase
*
* DESCRIPTION: 
************************************************************************/
static int fapp_mem_erase( void *addr, unsigned n)
{
    int result = FNET_ERR;
    const struct fapp_mem_region *region;
    
    if(fapp_mem_region_is_protected( (unsigned long)addr, n) == 0)
	{
    	/* Find memory region.*/
        region = fapp_mem_region_find( (unsigned long)addr, n);
        
        if(region)
        {
            if(region->erase)
            {
                region->erase(addr, n);
                result = FNET_OK;
            }
        }
	}
    return result;
}

/************************************************************************
* NAME: fapp_mem_erase_all
*
* DESCRIPTION: 
************************************************************************/
static void fapp_mem_erase_all(fnet_shell_desc_t desc)
{
    unsigned long addr;
    const struct fapp_mem_region *region = fapp_mem_regions;
    
    unsigned long log_start_addr;
    unsigned long log_erase_size;
    unsigned long log_skip_size;

   	/* Check all regions if it has erase function. */
   	while(region->description)
	{
        if(region->erase && region->erase_size)
        {
            addr = region->address;
            
            log_start_addr = addr;
            log_erase_size = 0;
            log_skip_size = 0;
            
            while(addr < (region->address+region->size)) 
            {
                if(fapp_mem_erase((void *)addr, region->erase_size) == FNET_OK)
                {
                    if(log_skip_size)
                    {
                        fnet_shell_println(desc, FAPP_MEM_ERASE_SKIPPED, log_start_addr, log_start_addr+log_skip_size-1); 
                        log_skip_size = 0;
                        log_start_addr = addr;
                    }
                    
                    log_erase_size += region->erase_size;
                }
                else
                {
                    if(log_erase_size)
                    {
                        fnet_shell_println(desc, FAPP_MEM_ERASE_ERASED, log_start_addr, log_start_addr+log_erase_size-1); 
                        log_erase_size = 0;
                        log_start_addr = addr;
                    }
                    
                    log_skip_size += region->erase_size;
                }
                addr+=region->erase_size;
            }
            
            if(log_erase_size)
                fnet_shell_println(desc, FAPP_MEM_ERASE_ERASED, log_start_addr, log_start_addr+log_erase_size-1);
            else
                fnet_shell_println(desc, FAPP_MEM_ERASE_SKIPPED, log_start_addr, log_start_addr+log_skip_size-1); 
    	}
    	region++;
	}
}

/************************************************************************
* NAME: fapp_mem_erase_cmd
*
* DESCRIPTION:  
************************************************************************/
void fapp_mem_erase_cmd ( fnet_shell_desc_t desc, int argc, char ** argv )
{
	unsigned long address;
	unsigned long size;
	char *p;
	int result;

    FNET_COMP_UNUSED_ARG(desc);
	
	fnet_shell_println(desc, "Erasing...");
	
	if (argc == 3)
	{
		address = fnet_strtoul(argv[1],&p,16);
		if ((address == 0) && (p == argv[1]))
        {
            fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1] );     
            return;
        }
        
        size = fnet_strtoul(argv[2],&p,10);
		if ((size == 0) && (p == argv[2]))
        {
            fnet_shell_println(desc, FAPP_PARAM_ERR, argv[2] );     
            return;
        }
        
        result = fapp_mem_erase( (void *)address, size);
        if( result == FNET_OK)
            fnet_shell_println(desc, FAPP_MEM_ERASE_ERASED, address, address+size-1 );
        else
            fnet_shell_println(desc, FAPP_MEM_ERASE_FAILED, address, address+size-1 );
        
    }
    else if ((argc == 2) && fnet_strcasecmp(FAPP_MEM_ERASE_ALL, argv[1]) == 0) /* Erase all */
    {
        fapp_mem_erase_all(desc);
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);
    }
}
#endif

#endif /* FAPP_CFG_TFTP_CMD || FAPP_CFG_TFTPUP_CMD || FAPP_CFG_TFTPS_CMD || FAPP_CFG_ERASE_CMD || FAPP_CFG_MEM_CMD */



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
* @file fapp_params.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.27.0
*
* @brief FNET Shell Demo implementation.
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"
#include "fapp_mem.h"
#include "fapp_params.h"


#if FNET_CFG_FLASH
#define fapp_params_erase   fnet_flash_erase
#define fapp_params_memcpy  fnet_flash_memcpy
#endif


/* Default parameter values.
* One-time setup done as a part of the manufacturing process/flashing
* Default application parameters (in ROM) set during flashing. 
*/
#if FAPP_CFG_PARAMS_REWRITE_FLASH

#if FNET_CFG_COMP_CW
	#if FNET_MK
		#pragma define_section fapp_params ".fapp_params" ".fapp_params" ".fapp_params" far_abs RW
		__declspec(fapp_params)
	#else
		#pragma define_section fapp_params ".fapp_params" far_abs RW	
		__declspec(fapp_params)
	#endif    
#endif
#if FNET_CFG_COMP_IAR
    #pragma segment="fapp_params"
    #pragma location="fapp_params" 
    #if FNET_MK      
        __root
    #endif      
#endif
#if FNET_CFG_COMP_UV
__attribute__((section(".fapp_params"))) 
#endif
   const struct fapp_params_flash fapp_params_config 
#if FNET_CFG_COMP_UV
__attribute__((used))
#endif	 
 =  {
        FAPP_PARAMS_SIGNATURE,
        {
            FAPP_CFG_PARAMS_IP_ADDR,    /* address */
            FAPP_CFG_PARAMS_IP_MASK,    /* netmask */
            FAPP_CFG_PARAMS_IP_GW,      /* gateway */
            FAPP_CFG_PARAMS_IP_DNS,     /* DNS */
            FAPP_CFG_PARAMS_MAC_ADDR,   /* MAC address */
        },
        {
            FAPP_CFG_PARAMS_BOOT_MODE,          /* boot */
            FAPP_CFG_PARAMS_BOOT_DELAY,         /* boot_delay */
            FAPP_CFG_PARAMS_BOOT_GO_ADDRESS,    /* go_address */                                      
            FAPP_CFG_PARAMS_BOOT_SCRIPT         /* boot_script */
        },
        {
            FAPP_CFG_PARAMS_TFTP_SERVER,            /* tftp_ip */
            FAPP_CFG_PARAMS_TFTP_FILE_TYPE,         /* image_type */  
            FAPP_CFG_PARAMS_TFTP_FILE_RAW_ADDRESS,  /* raw_address */ 
            FAPP_CFG_PARAMS_TFTP_FILE_NAME,         /* image */
        }
    };
#endif /* FAPP_CFG_PARAMS_REWRITE_FLASH */


/* Local confiruration parameters.
* Will be overwritten by parameters from flash if FAPP_CFG_PARAMS_READ_FLASH set to 1.
*/
#if FAPP_CFG_PARAMS_BOOT 
struct fapp_params_boot fapp_params_boot_config =
{
    FAPP_CFG_PARAMS_BOOT_MODE,          /* mode */
    FAPP_CFG_PARAMS_BOOT_DELAY,         /* delay */
    FAPP_CFG_PARAMS_BOOT_GO_ADDRESS,    /* go_address */                                      
    FAPP_CFG_PARAMS_BOOT_SCRIPT         /* boot_script */
};
#endif

#if FAPP_CFG_PARAMS_TFTP 
struct fapp_params_tftp fapp_params_tftp_config =
{
    FAPP_CFG_PARAMS_TFTP_SERVER,            /* tftp_ip */
    FAPP_CFG_PARAMS_TFTP_FILE_TYPE,         /* image_type */  
    FAPP_CFG_PARAMS_TFTP_FILE_RAW_ADDRESS,  /* raw_address */ 
    FAPP_CFG_PARAMS_TFTP_FILE_NAME,         /* image */
};
#endif


/************************************************************************
* NAME: fapp_params_to_flash
*
* DESCRIPTION: Save current configuration parameters to the flash.
************************************************************************/
#if FAPP_CFG_SAVE_CMD 
int fapp_params_to_flash()
{
    struct fapp_params_fnet     fnet_params;
    struct fapp_params_flash    *fapp_params = (struct fapp_params_flash *)FAPP_FLASH_PARAMS_ADDRESS;
    fnet_netif_desc_t           netif = fapp_default_netif;

#if FNET_CFG_IP4    
    /* Save IP address only if it was allocated manually/statically. */
    if(fnet_netif_get_ip4_addr_automatic(netif) != 0)
    {
        fnet_params.address = fapp_params->fnet_params.address; /* Preserve the old value.*/
    }
    else
    {
        fnet_params.address = fnet_netif_get_ip4_addr(netif);    /* Take the current value. */
    }

    fnet_params.netmask = fnet_netif_get_ip4_subnet_mask(netif);
    fnet_params.gateway = fnet_netif_get_ip4_gateway(netif);
#else
    /* Preserve the old value.*/
    fnet_params = fapp_params->fnet_params;
#endif
    
    fnet_netif_get_hw_addr(netif, fnet_params.mac, sizeof(fnet_mac_addr_t));
    
    /* Erase one paage allocated for configuration parameters.*/
    fapp_params_erase( (void *)(fapp_params), sizeof(struct fapp_params_flash));
    
    /* Simple check if erased. */
    if( fnet_memcmp((void *)(fapp_params), FAPP_PARAMS_SIGNATURE, sizeof(FAPP_PARAMS_SIGNATURE)) !=0 )
    {
        /* Write FNET parameters to the flash.*/
       fapp_params_memcpy( (void *)&fapp_params->fnet_params, &fnet_params, sizeof(struct fapp_params_fnet)  );
        
        /* Write BOOT parameters to the flash.*/
        #if FAPP_CFG_PARAMS_BOOT 
        fapp_params_memcpy( (void *)&fapp_params->boot_params, &fapp_params_boot_config, sizeof(struct fapp_params_boot)  );
        #endif
        
        /* Write TFTP parameters to the flash.*/
        #if FAPP_CFG_PARAMS_TFTP
        fapp_params_memcpy( (void *)&fapp_params->tftp_params, &fapp_params_tftp_config, sizeof(struct fapp_params_tftp)  );
        #endif
       
        /* Write Signature.*/
        fapp_params_memcpy( (void *)&fapp_params->signature, FAPP_PARAMS_SIGNATURE, sizeof(FAPP_PARAMS_SIGNATURE)  );
        
        /* Simple check if it was written. */
        if( fnet_memcmp((void *)(fapp_params), FAPP_PARAMS_SIGNATURE, sizeof(FAPP_PARAMS_SIGNATURE)) == 0 )
            return FNET_OK;
    }
    
    return FNET_ERR;
}
#endif

/************************************************************************
* NAME: fapp_params_from_flash
*
* DESCRIPTION: Load configuration parameters from flash.
************************************************************************/
#if FAPP_CFG_PARAMS_READ_FLASH
int fapp_params_from_flash()
{
    struct fapp_params_flash *fnet_params = (struct fapp_params_flash *)FAPP_FLASH_PARAMS_ADDRESS;
    int result;
    fnet_netif_desc_t netif = fapp_default_netif;
    
    /* Check signature. */
    if(fnet_strncmp( fnet_params->signature, FAPP_PARAMS_SIGNATURE, FAPP_PARAMS_SIGNATURE_SIZE )==0)
    {
        /* FNET stack parameters. */
        fnet_netif_set_hw_addr(netif, fnet_params->fnet_params.mac, sizeof(fnet_mac_addr_t));
        
    #if FNET_CFG_IP4        
        fnet_netif_set_ip4_addr(netif, fnet_params->fnet_params.address); 
        fnet_netif_set_ip4_gateway(netif, fnet_params->fnet_params.gateway);       
        fnet_netif_set_ip4_subnet_mask(netif, fnet_params->fnet_params.netmask);
    #endif /* FNET_CFG_IP4 */
        
        #if FAPP_CFG_PARAMS_BOOT 
        fapp_params_boot_config = fnet_params->boot_params; /* Boot parameters. */
        #endif
        #if FAPP_CFG_PARAMS_TFTP
        fapp_params_tftp_config = fnet_params->tftp_params; /* TFTP loader parameters. */
        #endif
               
        result = FNET_OK;
    }
    else
        result = FNET_ERR;
    
    return result;
  
}
#endif


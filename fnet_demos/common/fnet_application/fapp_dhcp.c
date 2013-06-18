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
* @file fapp_dhcp.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.35.0
*
* @brief FNET Shell Demo implementation.
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"
#include "fapp_dhcp.h"

#if FAPP_CFG_DHCP_CMD && FNET_CFG_DHCP && FNET_CFG_IP4
/************************************************************************
*     Definitions.
*************************************************************************/
#define FAPP_DHCP_DISCOVER_STR      "Sending DHCP discover..."
#define FAPP_DHCP_NEWIP_STR         " DHCP has updated/renewed parameters:"

/************************************************************************
*     Function Prototypes
*************************************************************************/
#define FAPP_DHCP_COMMAND_REBOOT    "reboot"

static long fapp_dhcp_discover_counter;
static fnet_ip4_addr_t fapp_dhcp_ip_old;

/************************************************************************
* NAME: fapp_dhcp_on_ctrlc
*
* DESCRIPTION:
************************************************************************/
static void fapp_dhcp_on_ctrlc(fnet_shell_desc_t desc)
{
    /* Release DHCP. */
    fapp_dhcp_release();
    /* Restore old ip address, as DHCP set it to zero. */
    fnet_netif_set_ip4_addr( fapp_default_netif, fapp_dhcp_ip_old );
    fnet_shell_println( desc, FAPP_CANCELLED_STR);  
}

/************************************************************************
* NAME: fapp_dhcp_handler_updated
*
* DESCRIPTION: Event handler on new IP from DHCP client. 
************************************************************************/
static void fapp_dhcp_handler_updated( fnet_netif_desc_t netif, void *shl_desc )
{
    fnet_shell_desc_t desc = (fnet_shell_desc_t) shl_desc;

    fapp_dhcp_discover_counter = -1; /* Infinite for future. */
    
    /* Optionally, unregister DHCP event handlers, just to do not 
     * disturb a user. */
    fnet_dhcp_handler_updated_set(0, 0);
    fnet_dhcp_handler_discover_set(0, 0);
    
    fnet_shell_unblock((fnet_shell_desc_t)shl_desc); /* Unblock the shell. */
   
    /* Print updated parameters info. */
    fnet_shell_println( desc, "\n%s", FAPP_DELIMITER_STR);
    fnet_shell_println( desc, FAPP_DHCP_NEWIP_STR);
    fnet_shell_println( desc, FAPP_DELIMITER_STR);

    fapp_netif_info_print( desc, netif );   
  
}

/************************************************************************
* NAME: fapp_dhcp_handler_discover
*
* DESCRIPTION: Event handler on new IP from DHCP client. 
************************************************************************/
static void fapp_dhcp_handler_discover( fnet_netif_desc_t netif,void *shl_desc )
{
    fnet_shell_desc_t desc = (fnet_shell_desc_t) shl_desc;
    FNET_COMP_UNUSED_ARG(netif);
    
    if(fapp_dhcp_discover_counter-- == 0)
    {
        fnet_shell_unblock((fnet_shell_desc_t)shl_desc);
        fapp_dhcp_on_ctrlc((fnet_shell_desc_t)shl_desc); /* Cancel DHCP.*/
    }
    else
        fnet_shell_println(desc, FAPP_DHCP_DISCOVER_STR);
}

/************************************************************************
* NAME: fapp_dhcp_release
*
* DESCRIPTION: Releases DHCP client.
*************************************************************************/
void fapp_dhcp_release()
{
    fnet_dhcp_release();
}

/************************************************************************
* NAME: fapp_dhcp_cmd
*
* DESCRIPTION: Enable DHCP client. 
************************************************************************/
void fapp_dhcp_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    struct fnet_dhcp_params dhcp_params;
    fnet_netif_desc_t netif = fapp_default_netif;

    if(argc == 1    /* By default is "init".*/
#if 0 /* DHCP reboot feature not used too much. */
    || fnet_strcasecmp(&FAPP_DHCP_COMMAND_REBOOT[0], argv[1]) == 0
#endif    
    ) /* [reboot] */
    {
               
        fnet_memset_zero(&dhcp_params, sizeof(struct fnet_dhcp_params));

        fapp_dhcp_discover_counter = FAPP_CFG_DHCP_CMD_DISCOVER_MAX; /* reset counter.*/
        
#if 0 /* DHCP reboot feature not used too much. */
        if(fnet_strcasecmp(&FAPP_DHCP_COMMAND_REBOOT[0], argv[1]) == 0) /* [reboot] */
            dhcp_params.requested_ip_address.s_addr = fnet_netif_get_ip4_addr(netif);
#endif            

        fapp_dhcp_ip_old = fnet_netif_get_ip4_addr(netif); /* Save ip to restore if cancelled. */
        
        /* Enable DHCP client */
        if(fnet_dhcp_init(netif, &dhcp_params) != FNET_ERR)
        {
            /* Register DHCP event handlers. */
            fnet_dhcp_handler_updated_set(fapp_dhcp_handler_updated, (void *)desc);
            fnet_dhcp_handler_discover_set(fapp_dhcp_handler_discover, (void *)desc);
            
            fnet_shell_println(desc, FAPP_TOCANCEL_STR);
            fnet_shell_block(desc, fapp_dhcp_on_ctrlc); /* Block shell. */
        }
        else
        {
            fnet_shell_println(desc, FAPP_INIT_ERR, "DHCP");
        }
    }
    else if(argc == 2 && fnet_strcasecmp(&FAPP_COMMAND_RELEASE[0], argv[1]) == 0) /* [release] */
    {
        fapp_dhcp_release();
    }
    else
    {
        fnet_shell_println(desc, FAPP_PARAM_ERR, argv[1]);
    }
}

/************************************************************************
* NAME: fapp_dhcp_info
*
* DESCRIPTION:
*************************************************************************/
void fapp_dhcp_info(fnet_shell_desc_t desc)
{
    char ip_str[FNET_IP4_ADDR_STR_SIZE];
    int dhcp_enabled = (fnet_dhcp_state() != FNET_DHCP_STATE_DISABLED);
    int address_automatic = fnet_netif_get_ip4_addr_automatic(fapp_default_netif);
    
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "DHCP Client", dhcp_enabled ? FAPP_SHELL_INFO_ENABLED : FAPP_SHELL_INFO_DISABLED);

    if(dhcp_enabled && address_automatic)
    {
        struct fnet_dhcp_options options;
        fnet_dhcp_get_options(&options);

        fnet_inet_ntoa(*(struct in_addr *)( &options.dhcp_server), ip_str);
        
        fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "DHCP Server", ip_str);
        
        fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_D, "Lease Time", fnet_ntohl(options.lease_time));
    }
}

#endif /* FAPP_CFG_DHCP_CMD && FNET_CFG_DHCP && FNET_CFG_IP4 */















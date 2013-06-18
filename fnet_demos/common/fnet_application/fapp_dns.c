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
* @file fapp_dns.c
*
* @author Andrey Butok
*
* @date Mar-25-2013
*
* @version 0.1.11.0
*
* @brief FNET Shell Demo implementation (DNS Resolver).
*
***************************************************************************/

#include "fapp.h"
#include "fapp_prv.h"
#include "fapp_dns.h"
#include "fnet.h"

#if FAPP_CFG_DNS_CMD && FNET_CFG_DNS && FNET_CFG_DNS_RESOLVER && FNET_CFG_IP4 
/************************************************************************
*     Definitions.
*************************************************************************/
//#define FNET_DNS_RESOLUTION_FAILED  "Resolution is failed!"

const char FNET_DNS_RESOLUTION_FAILED[]="FAILED";

/************************************************************************
*     Function Prototypes
*************************************************************************/



/************************************************************************
* NAME: fapp_dhcp_handler_updated
*
* DESCRIPTION: Event handler on new IP from DHCP client. 
************************************************************************/
static void fapp_dns_handler_resolved (fnet_ip4_addr_t address, long shl_desc)
{
    char ip_str[FNET_IP4_ADDR_STR_SIZE];
    fnet_shell_desc_t desc = (fnet_shell_desc_t) shl_desc;
    
    fnet_shell_unblock((fnet_shell_desc_t)shl_desc); /* Unblock the shell. */
    
    fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "Resolved address", 
    (address != (fnet_ip4_addr_t)FNET_ERR)?(fnet_inet_ntoa(*(struct in_addr *)( &address), ip_str)):FNET_DNS_RESOLUTION_FAILED );
}

/************************************************************************
* NAME: fapp_dhcp_on_ctrlc
*
* DESCRIPTION:
************************************************************************/
static void fapp_dns_on_ctrlc(fnet_shell_desc_t desc)
{
    /* Terminate DNS service. */
    fnet_dns_release();
    fnet_shell_println( desc, FAPP_CANCELLED_STR);  
}

/************************************************************************
* NAME: fapp_dns_cmd
*
* DESCRIPTION: Start DNS client/resolver. 
************************************************************************/
void fapp_dns_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    
    struct fnet_dns_params dns_params;
    fnet_netif_desc_t netif = fapp_default_netif;
    char ip_str[FNET_IP4_ADDR_STR_SIZE];
    
    FNET_COMP_UNUSED_ARG(argc);
    
    /* Set DNS client/resolver parameters.*/
    fnet_memset_zero(&dns_params, sizeof(struct fnet_dns_params));
    dns_params.dns_server = fnet_netif_get_ip4_dns(netif); /* Get DNS server address of default netif.*/
    dns_params.host_name = argv[1];                 /* Get host name via command argument.*/
    dns_params.handler = fapp_dns_handler_resolved; /* Callback function.*/
    dns_params.cookie = (long)desc;                 /* Application-specific parameter 
                                                       which will be passed to fapp_dns_handler_resolved().*/

    /* Run DNS cliebt/resolver. */
    if(fnet_dns_init(&dns_params) != FNET_ERR)
    {
        fnet_shell_println(desc, FAPP_DELIMITER_STR);
        fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "Resolving", dns_params.host_name);
        fnet_inet_ntoa(*(struct in_addr *)( &dns_params.dns_server), ip_str);
        fnet_shell_println(desc, FAPP_SHELL_INFO_FORMAT_S, "DNS Server", ip_str);
        
        fnet_shell_println(desc, FAPP_TOCANCEL_STR);
        fnet_shell_println(desc, FAPP_DELIMITER_STR);
        
        fnet_shell_block(desc, fapp_dns_on_ctrlc); /* Block the shell input.*/
    }
    else
    {
        fnet_shell_println(desc, FAPP_INIT_ERR, "DNS");
    }
}

#endif /* FAPP_CFG_DNS_CMD && FNET_CFG_DNS && FNET_CFG_DNS_RESOLVER && FNET_CFG_IP4  */















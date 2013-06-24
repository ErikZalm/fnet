/**************************************************************************
*
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
* @file fnet_stm32_eth.c
*
* @author EvdZ and inca
*
* @date Jun-24-2013
*
* @version 0.0
*
* @brief STM32 ethernet driver interafce.
*
***************************************************************************/

#include "fnet_config.h"
#if FNET_STM32 && (FNET_CFG_CPU_ETH0 ||FNET_CFG_CPU_ETH1)

#include "fnet_stm32_eth.h"
#include "fnet_eth_prv.h"


fnet_stm32_eth_if_t fnet_stm32_eth0_if; // Blank

/************************************************************************
* Network interface API structure.
*************************************************************************/
const fnet_netif_api_t fnet_stm32_mac_api =
{
    FNET_NETIF_TYPE_ETHERNET,           /* Data-link type. */
    sizeof(fnet_mac_addr_t),
    fnet_stm32_init,                    /* Initialization function.*/
    fnet_stm32_release,                 /* Shutdown function.*/
#if FNET_CFG_IP4
	fnet_eth_output_ip4,            /* IPv4 Transmit function.*/
#endif
	fnet_eth_change_addr_notify,    /* Address change notification function.*/
	fnet_eth_drain,                 /* Drain function.*/
	fnet_stm32_get_hw_addr,
	fnet_stm32_set_hw_addr,
	fnet_stm32_is_connected,
	fnet_stm32_get_statistics
#if FNET_CFG_MULTICAST
    #if FNET_CFG_IP4
        ,
	    fnet_eth_multicast_join_ip4,
	    fnet_eth_multicast_leave_ip4
    #endif
#endif
};

/************************************************************************
* Ethernet interface structure.
*************************************************************************/
fnet_eth_if_t fnet_stm32_eth0_if =
{
    //&fnet_stm32_eth0_if /* CPU-specific control data structure of the if. */
    &ETHD1                /* MACDriver */
    ,0                    /* MAC module number [0-1]. */
    ,fnet_stm32_eth_output
#if FNET_CFG_MULTICAST
    ,      
    fnet_stm32_multicast_join,
    fnet_stm32_multicast_leave,
#endif /* FNET_CFG_MULTICAST */     
};

/************************************************************************
* Network interface structure.
* DESCRIPTION: Indexes ethernet interface and network API structures.
*************************************************************************/
fnet_netif_t fnet_eth0_if =
{
	0,                          /* Pointer to the next net_if structure.*/
	0,                          /* Pointer to the previous net_if structure.*/
	"eth0",                     /* Network interface name.*/
	FNET_CFG_CPU_ETH0_MTU,      /* Maximum transmission unit.*/
	&fnet_stm32_eth0_if,        /* Ethernet interface specific data structure.*/
	&fnet_stm32_mac_api         /* Interface API */
};


/************************************************************************
* NAME: fnet_eth_io_init
*
* DESCRIPTION: Ethernet IO initialization for STM32F4.
*************************************************************************/
void fnet_eth_io_init() 
{
#if FNET_CFG_CPU_STM32F4
  // Potentially add:
  //   PAL pin configuration
  //   Clock enables
#endif
}

/************************************************************************
* NAME: fnet_eth_phy_init
*
* DESCRIPTION: Ethernet Physical Transceiver initialization and/or reset.
*************************************************************************/
void fnet_eth_phy_init(fnet_stm32_eth_if_t *ethif) 
{
  FNET_COMP_UNUSED_ARG(ethif);
  /*MACDriver *macp;
  const MACConfig *config;

  macStart(macp, config);*/
}



#endif /* FNET_MK && FNET_CFG_ETH */




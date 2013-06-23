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
* @file fnet_mk_eth.c
*
* @author Andrey Butok
*
* @date Aug-2-2012
*
* @version 0.1.9.0
*
* @brief Ethernet driver interafce.
*
***************************************************************************/

#include "fnet_config.h"
#if FNET_STM32 && (FNET_CFG_CPU_ETH0 ||FNET_CFG_CPU_ETH1)

#include "fnet_fec.h"
#include "fnet_eth_prv.h"

/************************************************************************
* Ethernet interface structure.
*************************************************************************/
fnet_eth_if_t fnet_mk_eth0_if =
{
    &fnet_fec0_if               /* Points to CPU-specific control data structure of the interface. */
    ,0
    ,fnet_fec_output
#if FNET_CFG_MULTICAST
    ,      
    fnet_fec_multicast_join,
    fnet_fec_multicast_leave,
#endif /* FNET_CFG_MULTICAST */     
};

fnet_netif_t fnet_eth0_if =
{
	0,                          /* Pointer to the next net_if structure.*/
	0,                          /* Pointer to the previous net_if structure.*/
	"eth0",                     /* Network interface name.*/
	FNET_CFG_CPU_ETH0_MTU,      /* Maximum transmission unit.*/
	&fnet_mk_eth0_if,           /* Points to interface specific data structure.*/
	&fnet_fec_api               /* Interface API */
};

/************************************************************************
* NAME: fnet_eth_io_init
*
* DESCRIPTION: Ethernet IO initialization.
*************************************************************************/
void fnet_eth_io_init() 
{
#if FNET_CFG_CPU_STM32F4
  
    FNET_MK_PORT_MemMapPtr pctl;
    FNET_MK_SIM_MemMapPtr  sim  = (FNET_MK_SIM_MemMapPtr)FNET_MK_SIM_BASE_PTR;
    
    pctl = (FNET_MK_PORT_MemMapPtr)FNET_MK_PORTA_BASE_PTR;    
    pctl->PCR[12] = 0x00000400;     /* PTA12, RMII0_RXD1/MII0_RXD1      */
    pctl->PCR[13] = 0x00000400;     /* PTA13, RMII0_RXD0/MII0_RXD0      */
    pctl->PCR[14] = 0x00000400;     /* PTA14, RMII0_CRS_DV/MII0_RXDV    */
    pctl->PCR[15] = 0x00000400;     /* PTA15, RMII0_TXEN/MII0_TXEN      */
    pctl->PCR[16] = 0x00000400;     /* PTA16, RMII0_TXD0/MII0_TXD0      */
    pctl->PCR[17] = 0x00000400;     /* PTA17, RMII0_TXD1/MII0_TXD1      */
    
    
    pctl = (FNET_MK_PORT_MemMapPtr)FNET_MK_PORTB_BASE_PTR;
    pctl->PCR[0] = FNET_MK_PORT_PCR_MUX(4) | FNET_MK_PORT_PCR_ODE_MASK; /* PTB0, RMII0_MDIO/MII0_MDIO   */
    pctl->PCR[1] = FNET_MK_PORT_PCR_MUX(4);                     /* PTB1, RMII0_MDC/MII0_MDC     */
    
    /* Enable clock for ENET module */
    sim->SCGC2 |= FNET_MK_SIM_SCGC2_ENET_MASK;
    
    /*FSL: allow concurrent access to MPU controller. Example: ENET uDMA to SRAM, otherwise bus error*/
    FNET_MK_MPU_CESR = 0;  /* MPU is disabled. All accesses from all bus masters are allowed.*/
    
#endif  

}

/************************************************************************
* NAME: fnet_eth_phy_init
*
* DESCRIPTION: Ethernet Physical Transceiver initialization and/or reset.
*************************************************************************/
void fnet_eth_phy_init(fnet_fec_if_t *ethif) 
{
    FNET_COMP_UNUSED_ARG(ethif);
}



#endif /* FNET_MK && FNET_CFG_ETH */




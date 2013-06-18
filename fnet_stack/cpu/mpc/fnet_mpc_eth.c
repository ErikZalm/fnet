
/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2011 by Andrey Butok and Gordon Jahn. Freescale Semiconductor, Inc.
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
* @file fnet_mpc_eth.c
*
* @author Andrey Butok
*
* @date Dec-17-2012
*
* @version 0.1.1.0
*
* @brief Ethernet driver interafce.
*
***************************************************************************/


#include "fnet_config.h"
#if FNET_MPC && (FNET_CFG_CPU_ETH0 ||FNET_CFG_CPU_ETH1)
#include "fnet_fec.h"
#include "fnet_eth_prv.h"

/************************************************************************
* Ethernet interface structure.
*************************************************************************/
struct fnet_eth_if fnet_mpc_eth0_if =
{
    &fnet_fec0_if
    ,0                             /* MAC module number.*/
    ,fnet_fec_output
#if FNET_CFG_MULTICAST
    ,fnet_fec_multicast_join
    ,fnet_fec_multicast_leave
#endif /* FNET_CFG_MULTICAST */    
};

fnet_netif_t fnet_eth0_if =
{
	0,                      /* Pointer to the next net_if structure.*/
	0,                      /* Pointer to the previous net_if structure.*/
	"eth0",                 /* Network interface name.*/
	FNET_CFG_CPU_ETH0_MTU,  /* Maximum transmission unit.*/
	&fnet_mpc_eth0_if,      /* Points to interface specific data structure.*/
	&fnet_fec_api           /* Interface API */  
};


/************************************************************************
* NAME: fnet_eth_io_init
*
* DESCRIPTION: Ethernet IO initialization.
*************************************************************************/
void fnet_eth_io_init() 
{

#if FNET_CFG_CPU_MPC5668G /* FADO */
	/*
	 PG[11]	PCR[107]	ALT2	RX_CLK		FEC	I
	 PH[6]	PCR[118]	ALT2	RXD[2]		FEC	I
	 PH[5]	PCR[117]	ALT2	RXD[1]		FEC	I
	 PH[4]	PCR[116]	ALT2	RXD[0]		FEC	I
	 PH[0]	PCR[112]	ALT2	COL			FEC	I
	 PH[3]	PCR[115]	ALT2	RX_ER		FEC	I
	 PG[9]	PCR[105]	ALT2	CRS			FEC	I
	 PH[7]	PCR[119]	ALT2	RXD[3]		FEC	I
	 PG[7]	PCR[103]	ALT2	MDIO		FEC	I/O
	 PH[1]	PCR[113]	ALT2	RX_DV		FEC	I
	 PG[6]	PCR[102]	ALT2	MDC			FEC	O
	 PG[8]	PCR[104]	ALT2	TX_CLK		FEC	I
	 PG[14]	PCR[110]	ALT2	TXD[2]		FEC	O
	 PG[15]	PCR[111]	ALT2	TXD[3]		FEC	O
	 PG[13]	PCR[109]	ALT2	TXD[1]		FEC	O
	 PG[12]	PCR[108]	ALT2	TXD[0]		FEC	O
	 PH[2]	PCR[114]	ALT2	TX_EN		FEC	O
	 PG[10]	PCR[106]	ALT2	TX_ER		FEC	O
	 */
 	FNET_MPC_GPIO_PCR(103) = 0xB06;     /* Set to FEC_MDIO	  */
	FNET_MPC_GPIO_PCR(102) = 0xA04;     /* Set to FEC_MDC 	  */
	FNET_MPC_GPIO_PCR(114) = 0xA04;     /* Set to FEC_TX_EN  */
	FNET_MPC_GPIO_PCR(108) = 0xA04;     /* Set to FEC_TXD(0)	*/  
	FNET_MPC_GPIO_PCR(109) = 0xA04;     /* Set to FEC_TXD(1) */ 
	FNET_MPC_GPIO_PCR(110) = 0xA04;     /* Set to FEC_TXD(2) */
	FNET_MPC_GPIO_PCR(111) = 0xA04;     /* Set to FEC_TXD(3) */ 
	FNET_MPC_GPIO_PCR(106) = 0xA04 ;    /* Set to FEC_TX_ER  */
	FNET_MPC_GPIO_PCR(112) = 0x906;     /* Set to FEC_COL	  */
	FNET_MPC_GPIO_PCR(104) = 0x906;     /* Set to FEC_TX_CLK */ 
	FNET_MPC_GPIO_PCR(105) = 0x906;     /* Set to FEC_CRS	  */
	FNET_MPC_GPIO_PCR(107) = 0x906;     /* Set to FEC_RX_CLK */
	FNET_MPC_GPIO_PCR(113) = 0x906;     /* Set to FEC_RX_DV  */
	FNET_MPC_GPIO_PCR(116) = 0x906;     /* Set to FEC_RXD(0) */  
	FNET_MPC_GPIO_PCR(117) = 0x906;     /* Set to FEC_RXD(1) */
	FNET_MPC_GPIO_PCR(118) = 0x906;     /* Set to FEC_RXD(2) */
	FNET_MPC_GPIO_PCR(119) = 0x906;     /* Set to FEC_RXD(3) */
	FNET_MPC_GPIO_PCR(115) = 0x906;     /* Set to FEC_RX_ER  */
#endif

#if FNET_CFG_CPU_MPC564xBC /* Bolero3M */

    /*
    PA[3]	PCR[3]				RX_CLK	FEC	I
    PA[7]	PCR[7]				RXD[2]		FEC	I
    PA[8]	PCR[8]				RXD[1]		FEC	I
    PA[9]	PCR[9]				RXD[0]		FEC	I
    PA[10]	PCR[10]				COL			FEC	I
    PA[11]	PCR[11]				RX_ER		FEC	I
    PE[12]	PCR[76]				CRS			FEC	I
    PE[13]	PCR[77]				RXD[3]		FEC	I
    PF[14]	PCR[94]		ALT4	MDIO		FEC	I/O
    PF[15]	PCR[95]				RX_DV		FEC	I
    PG[0]	PCR[96]		ALT4	MDC			FEC	O
    PG[1]	PCR[97]				TX_CLK		FEC	I
    PG[12]	PCR[108]	ALT4	TXD[2]		FEC	O
    PG[13]	PCR[109]	ALT4	TXD[3]		FEC	O
    PH[0]	PCR[112]	ALT4	TXD[1]		FEC	O
    PH[1]	PCR[113]	ALT4	TXD[0]		FEC	O
    PH[2)	PCR[114)	ALT4	TX_EN		FEC	O
    PH(3)	PCR(115)	ALT4	TX_ER		FEC	O
    */
  	FNET_MPC_GPIO_PCR(94) = 0x1306;     /* Set to FEC_MDIO	  */
	FNET_MPC_GPIO_PCR(96) = 0x1202;     /* Set to FEC_MDC 	  */
	FNET_MPC_GPIO_PCR(114) = 0x1204;    /* Set to FEC_TX_EN  */
	FNET_MPC_GPIO_PCR(113) = 0x1204;    /* Set to FEC_TXD(0)	*/  
	FNET_MPC_GPIO_PCR(112) = 0x1204;    /* Set to FEC_TXD(1) */ 
	FNET_MPC_GPIO_PCR(108) = 0x1204;    /* Set to FEC_TXD(2) */
	FNET_MPC_GPIO_PCR(109) = 0x1204;    /* Set to FEC_TXD(3) */ 
	FNET_MPC_GPIO_PCR(115) = 0x1204;    /* Set to FEC_TX_ER  */
	FNET_MPC_GPIO_PCR(10) = 0x106;      /* Set to FEC_COL	  */
	FNET_MPC_GPIO_PCR(97) = 0x102;      /* Set to FEC_TX_CLK */ 
	FNET_MPC_GPIO_PCR(76) = 0x106;      /* Set to FEC_CRS	  */
	FNET_MPC_GPIO_PCR(3) = 0x106;       /* Set to FEC_RX_CLK */
	FNET_MPC_GPIO_PCR(95) = 0x102;      /* Set to FEC_RX_DV  */
	FNET_MPC_GPIO_PCR(9) = 0x106;       /* Set to FEC_RXD(0) */  
	FNET_MPC_GPIO_PCR(8) = 0x106;       /* Set to FEC_RXD(1) */
	FNET_MPC_GPIO_PCR(7) = 0x106;       /* Set to FEC_RXD(2) */
	FNET_MPC_GPIO_PCR(77) = 0x106;      /* Set to FEC_RXD(3) */
	FNET_MPC_GPIO_PCR(11) = 0x106;      /* Set to FEC_RX_ER  */	
#endif
}

/************************************************************************
* NAME: fnet_eth_phy_init
*
* DESCRIPTION: Ethernet Physical Transceiver initialization and/or reset.
*************************************************************************/
void fnet_eth_phy_init(fnet_fec_if_t *ethif) 
{
    fnet_uint16 reg_value;
    fnet_uint16 status_value = 0;

    fnet_fec_mii_read(ethif, FNET_FEC_MII_REG_CR, &reg_value);
   
    /* ANE ENABLED:*/
    fnet_fec_mii_write(ethif, FNET_FEC_MII_REG_CR, (fnet_uint16)(reg_value | FNET_FEC_MII_REG_CR_ANE | FNET_FEC_MII_REG_CR_ANE_RESTART));

	while (status_value != 0x0040) 
	{
        fnet_fec_mii_read(ethif, FNET_FEC_MII_REG_SR, &status_value);
        status_value &= 0x0040;
	}
}

#endif /* FNET_MPC && FNET_CFG_ETH */



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
* @file fnet_stm32_eth.h
*
* @author EvdZ and inca
*
* @date Jun-23-2013
*
* @version 0.0
*
* @brief STM32 MAC (FEC equivalent) module driver definitions.
*
***************************************************************************/

#include "fnet_config.h"
#if (FNET_STM32) && (FNET_CFG_CPU_ETH0 ||FNET_CFG_CPU_ETH1)

#include "fnet.h"
#include "fnet_eth_prv.h"
#include "fnet_error.h"
#include "fnet_debug.h"
#include "fnet_isr.h"
#include "fnet_prot.h"
#include "fnet_arp.h"
#include "fnet_timer_prv.h"
#include "fnet_loop.h"

#include "fnet_stdlib.h"


typedef struct
{

} fnet_stm32_eth_if_t; // fnet_fec_if_t

/* CPU-specific configuration.*/

#if FNET_MK     /* Kinetis.*/
    #define FNET_FEC_CLOCK_KHZ  FNET_MK_PERIPH_CLOCK_KHZ
    /* Transmit buffer descriptor queue. This pointer 
     * must be 32-bit aligned; however, it is recommended it be 
     * made 128-bit aligned (evenly divisible by 16).*/
    #define FNET_FEC_BUF_DESC_DIV       (16)
    /* The transmit buffer pointer, containing the address 
     * of the associated data buffer, 
     * must always be evenly divisible by 8.*/
    #define FNET_FEC_TX_BUF_DIV         (8)
    /* The receive buffer pointer, containing the address 
     * of the associated data buffer, 
     * must always be evenly divisible by 16.*/
    #define FNET_FEC_RX_BUF_DIV         (16)
#endif   


/* frequency of less than or equal to 2.5 MHz to be compliant with 
* the IEEE 802.3 MII specification. */
#define FNET_FEC_MII_CLOCK_KHZ      (2500)


#define FNET_FEC_BUF_SIZE           (((FNET_CFG_CPU_ETH0_MTU>FNET_CFG_CPU_ETH1_MTU)?FNET_CFG_CPU_ETH0_MTU:FNET_CFG_CPU_ETH1_MTU)+FNET_ETH_HDR_SIZE+FNET_ETH_CRC_SIZE+16) /* Ring Buffer sizes in bytes.*/
#define FNET_FEC_TX_BUF_NUM         (FNET_CFG_CPU_ETH_TX_BUFS_MAX)
#define FNET_FEC_RX_BUF_NUM         (FNET_CFG_CPU_ETH_RX_BUFS_MAX)


/************************************************************************
*     MII Register Indexes.
*************************************************************************/
#define FNET_FEC_MII_REG_CR          (0x0000)   /* Control Register */
#define FNET_FEC_MII_REG_SR          (0x0001)   /* Status Register */
#define FNET_FEC_MII_REG_IDR1        (0x0002)   /* Identification Register #1 */
#define FNET_FEC_MII_REG_IDR2        (0x0003)   /* Identification Register #2 */
#define FNET_FEC_MII_REG_ANAR        (0x0004)   /* Auto-Negotiation Advertisement Register */
#define FNET_FEC_MII_REG_ANLPAR      (0x0005)   /* Auto-Negotiation Link Partner Ability Register */
#define FNET_FEC_MII_REG_ANER        (0x0006)   /* Auto-Negotiation Expansion Register */
#define FNET_FEC_MII_REG_ANNPTR      (0x0007)   /* Auto-Negotiation Next Page TX Register */
#define FNET_FEC_MII_REG_ICR         (0x0010)   /* Interrupt Control Register */
#define FNET_FEC_MII_REG_PSR         (0x0011)   /* Proprietary Status Register */
#define FNET_FEC_MII_REG_PCR         (0x0012)   /* Proprietary Control Register */

#define FNET_FEC_MII_REG_SR_LINK_STATUS (0x0004)
#define FNET_FEC_MII_REG_SR_AN_ABILITY  (0x0008)
#define FNET_FEC_MII_REG_SR_AN_COMPLETE (0x0020)

#define FNET_FEC_MII_REG_ANAR_NEXT_PAGE (0x8000)

#define FNET_FEC_MII_REG_CR_RESET       (0x8000)    /* Resetting a port is accomplished by setting this bit to 1.*/
#define FNET_FEC_MII_REG_CR_LOOPBACK    (0x4000)    /* Determines Digital Loopback Mode. */
#define FNET_FEC_MII_REG_CR_DATARATE    (0x2000)    /* Speed Selection bit.*/
#define FNET_FEC_MII_REG_CR_ANE         (0x1000)    /* Auto-Negotiation Enable bit. */
#define FNET_FEC_MII_REG_CR_PDWN        (0x0800)    /* Power Down bit. */
#define FNET_FEC_MII_REG_CR_ISOL        (0x0400)    /* Isolate bit.*/
#define FNET_FEC_MII_REG_CR_ANE_RESTART (0x0200)    /* Restart Auto-Negotiation bit.*/
#define FNET_FEC_MII_REG_CR_DPLX        (0x0100)    /* Duplex Mode bit.*/

#define FNET_FEC_MII_TIMEOUT            (0x10000)   /* Timeout counter for MII communications.*/


/************************************************************************
*     Function Prototypes
*************************************************************************/
int fnet_stm32_init(fnet_netif_t *netif);
void fnet_stm32_release(fnet_netif_t *netif);
void fnet_stm32_input(void);
int fnet_stm32_get_hw_addr(fnet_netif_t *netif, unsigned char * hw_addr);
int fnet_stm32_set_hw_addr(fnet_netif_t *netif, unsigned char * hw_addr);
int fnet_stm32_is_connected(fnet_netif_t *netif);
int fnet_stm32_get_statistics(struct fnet_netif *netif, struct fnet_netif_statistics * statistics);
void fnet_stm32_eth_output(fnet_netif_t *netif, unsigned short type, const fnet_mac_addr_t dest_addr, fnet_netbuf_t* nb);

/* Ethernet IO initialization.*/
void fnet_eth_io_init(void) ;
/* Ethernet On-chip Physical Transceiver initialization and/or reset. */
void fnet_eth_phy_init(fnet_stm32_eth_if_t *ethif);

int fnet_stm32_mii_write(fnet_stm32_eth_if_t *ethif, int reg_addr, fnet_uint16 data);
int fnet_stm32_mii_read(fnet_stm32_eth_if_t *ethif, int reg_addr, fnet_uint16 *data); 

#if FNET_CFG_MULTICAST      
void fnet_stm32_multicast_join(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
void fnet_stm32_multicast_leave(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
#endif /* FNET_CFG_MULTICAST */

/* For debug needs.*/
void fnet_stm32_output_frame(fnet_netif_t *netif, char* frame, int frame_size);
int fnet_stm32_input_frame(fnet_netif_t *netif, char* buf, int buf_size);    
void fnet_stm32_debug_mii_print_regs(fnet_netif_t *netif);
void fnet_stm32_stop(fnet_netif_t *netif);
void fnet_stm32_resume(fnet_netif_t *netif);


#endif /* (FNET_STM32) && FNET_CFG_ETH */




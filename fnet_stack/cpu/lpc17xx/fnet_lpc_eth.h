/**************************************************************************
*
* Copyright 2012-2013 Mathew McBride
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
**********************************************************************/

/*
 * fnet_lpc_eth.h
 *
 *  Created on: Dec 9, 2012
 *      Author: matt
 */

#include "fnet.h"
#include <stdint.h>

#ifndef FNET_LPC_ETH_H_
#define FNET_LPC_ETH_H_

#if FNET_LPC && FNET_CFG_ETH

#define LPC_PCON_ENABLE_ETH_POWER 0x40000000;
enum {
	P10_FUNC_ENET_TXD0 = 0x1,
	P11_FUNC_ENET_TXD1 = (1<<2),
	P14_FUNC_ENET_TX_EN = (1<<8),
	P18_FUNC_ENET_CRS = (1<<16),
	P19_FUNC_ENET_RXD0 = (1<<18),
	P110_FUNC_ENET_RXD1 = (1<<20),
	P114_FUNC_ENET_RX_ER = (1<<25),
	P130_FUNC_ENET_REF_CLK = (1<<30)
} PINSEL2_DEFS;

enum {
	P116_FUNC_ENET_MPC = 0x1,
	P117_FUNC_ENET_MDIO = (1<<2)
} PINSEL3_DEFS;

enum {
	MAC_RECEIVE_ENABLE = 0x1,
	MAC_PASS_ALL_RECEIVE_FRAMES = (1<<1),
	MAC_RESET_TX = (1<<8),
	MAC_RESET_MCS_TX = (1<<9),
	MAC_RESET_RX = (1<<10),
	MAC_RESET_MCS_RX = (1<<11),
	MAC_RESET_SIMULATION = (1<<14),
	MAC_SOFT_RESET = (1<<15)
} MAC1_DEFS;

enum {
	MAC_FULL_DUPLEX = 1,
	MAC_CRC_ENABLE = (1<<4),
	MAC_PAD_SHORT_FRAMES = (1<<5),
	MAC_HUGE_FRAME_ENABLE = (1<<2)
} MAC2_DEFS;
enum {
	COMMAND_ENABLE_RECEIVE = (1<<0),
	COMMAND_ENABLE_TRANSMIT = (1<<1),
	COMMAND_REGISTER_RESET = (1<<3),
	COMMAND_TX_RESET = (1<<4),
	COMMAND_RX_RESET = (1<<5),
	COMMAND_PASS_RUNT_FRAMES = (1<<6),
	COMMAND_PASS_RX_FILTER = (1<<7),
	COMMAND_PASS_RMII = (1<<9),
	COMMAND_FULL_DUPLEX = (1<<10)
} COMMAND_DEFS;

#define set_enable_rxtx_functions(functions) LPC_EMAC->Command &= ~(COMMAND_ENABLE_RECEIVE | COMMAND_ENABLE_TRANSMIT | COMMAND_TX_RESET | COMMAND_RX_RESET); LPC_EMAC->Command |= functions

#define set_pinsel2_ethernet_funcs() LPC_PINCON->PINSEL2 |= P10_FUNC_ENET_TXD0 | P11_FUNC_ENET_TXD1 | P14_FUNC_ENET_TX_EN | P18_FUNC_ENET_CRS | P19_FUNC_ENET_RXD0 | P110_FUNC_ENET_RXD1 | P114_FUNC_ENET_RX_ER | P130_FUNC_ENET_REF_CLK
#define set_pinsel3_ethernet_funcs() LPC_PINCON->PINSEL3 |= P116_FUNC_ENET_MPC | P117_FUNC_ENET_MDIO

#define reset_mac1_internal_modules() LPC_EMAC->MAC1 = MAC_RESET_TX | MAC_RESET_MCS_TX | MAC_RESET_RX | MAC_RESET_MCS_RX | MAC_RESET_SIMULATION | MAC_SOFT_RESET
#define reset_mac_datapaths() LPC_EMAC->Command = COMMAND_REGISTER_RESET | COMMAND_TX_RESET | COMMAND_RX_RESET | COMMAND_PASS_RUNT_FRAMES;


#define mac_enable_receive_frames() LPC_EMAC->MAC1 |= MAC_RECEIVE_ENABLE
#define mac_pass_all_receive_frames() LPC_EMAC->MAC1 = MAC_PASS_ALL_RECEIVE_FRAMES

#define mac_enable_crc_short_frames() LPC_EMAC->MAC2 = MAC_CRC_ENABLE | MAC_PAD_SHORT_FRAMES

#define mac_disable_huge_frames() LPC_EMAC->MAC2 &= ~(MAC_HUGE_FRAME_ENABLE)

#define mac_enable_full_duplex() LPC_EMAC->MAC2 |= MAC_FULL_DUPLEX
#define command_enable_full_duplex() LPC_EMAC->Command |= COMMAND_FULL_DUPLEX
#define phy_set_100() LPC_EMAC->SUPP = (1<<8)
#define LPC_ETH_MAX_FRAME_SIZE 1536
#define set_mac_max_frame_size(size) LPC_EMAC->MAXF = size

#define set_retransmission_max_collision_window(retrans,collision) LPC_EMAC->CLRT = retrans | (collision << 8);
#define LPC_ETH_DEFAULT_RETRANSMISSION_MAX 0xF
#define LPC_ETH_DEFAULT_COLLISION_WIN 0x37

// The non-back-to-back inter packet gap is in the IPGR register, but bit 7 is notched out, so the upper bits need to be moved up one
#define set_non_b2b_pkt_gap(packetgap) LPC_EMAC->IPGR = (0x7F & packetgap) | (0x3F80 << 1)
#define LPC_ETH_DEFAULT_NON_B2B_PKT_GAP 0x12

enum {
	MII_CLK_DIV_4 = 0,
	MII_CLK_DIV_6 = 0x2,
	MII_CLK_DIV_8,
	MII_CLK_DIV_10,
	MII_CLK_DIV_14,
	MII_CLK_DIV_20,
	MII_CLK_DIV_28,
	MII_CLK_DIV_36,
	MII_CLK_DIV_40,
	MII_CLK_DIV_44,
	MII_CLK_DIV_48,
	MII_CLK_DIV_52,
	MII_CLK_DIV_56,
	MII_CLK_DIV_60,
	MII_CLK_DIV_64
} MII_CLK_DIVS;

#define LPC_EMAC_MCFG_CLK_DIV_BASE 2
#define LPC_EMAC_MCFG_RESET 15
#define set_mII_clk_div(div) LPC_EMAC->MCFG |= (div << LPC_EMAC_MCFG_CLK_DIV_BASE)
#define reset_mII_interface() LPC_EMAC->MCFG |= (1 << LPC_EMAC_MCFG_RESET)
#define enable_mII_interface() LPC_EMAC->MCFG &= ~(1<<15)

#define enable_command_register() LPC_EMAC->Command = COMMAND_PASS_RUNT_FRAMES | COMMAND_PASS_RX_FILTER | COMMAND_PASS_RMII

#define LPC_EMAC_MCMD_NONE 0
#define LPC_EMAC_MCMD_READ 1

enum {
	BASIC_CONTROL_REGISTER,
	BASIC_STATUS_REGISTER,
	PHY_IDENTIFIER_1,
	PHY_IDENTIFIER_2,
	AUTONEG_ADVERTISEMENT_REGISTER,
	AUTONEG_ADVERTISEMENT_LINK_PART_ABILITY_REGISTER,
	AUTONEG_EXPANSION_REGISTER
} BASIC_EXTENDED_REGISTERS;

enum {
	L8720_MODE_CONTROL_STATUS_REGISTER = 17,
	L8720_SPECIAL_MODES_REGISTER,
	L8720_SYMBOL_ERROR_COUNTER_REGISTER = 26,
	L8720_CONTROL_STATUS_INDICATION,
	L8720_INTERRUPT_SOURCE_REGISTER = 29,
	L8720_INTERRUPT_MASK_REGISTER,
	L8720_PHY_SPECIAL_CONTROL_STATUS_REGISTER
} LAN8720_REGISTERS;

enum {
	BASIC_CONTROL_SOFT_RESET = (1<<15),
	BASIC_CONTROL_LOOPBACK = (1<<14),
	BASIC_CONTORL_100MBPS = (1<<13),
	BASIC_CONTROL_ENABLE_AUTONEG = (1<<12),
	BASIC_CONTROL_POWER_DOWN = (1<<11),
	BASIC_CONTROL_ISOLATION = (1<<10),
	BASIC_CONTROL_AUTONEG_RESTART = (1<<9),
	BASIC_CONTROL_DUPLEX_FULL = (1<<8)
} BASIC_CONTROL_REGISTER_SETTINGS;
#define LPC_EMAC_DEFAULT_PHY_ADDRESS 1

enum {
	BASIC_STATUS_EXTENDED_CAPABILITIES = 0,
	BASIC_STATUS_JABBER_DETECT = (1<<1),
	BASIC_STATUS_LINK_STATUS = (1<<2),
	BASIC_STATUS_AUTONEG_ABILITY = (1<<3),
	BASIC_STATUS_REMOTE_FAULT = (1<<4),
	BASIC_STATUS_AUTONEG_COMPLETE = (1<<5),
	BASIC_STATUS_EXTENDED_STATUS = (1<<8),
	BASIC_STATUS_100BT2_HALF_DUPLEX = (1<<9),
	BASIC_STATUS_100BT2_FULL_DUPLEX = (1<<10),
	BASIC_STATUS_10BT_HALF_DUPLEX = (1<<11),
	BASIC_STATUS_10BT_FULL_DUPLEX = (1<<12),
	BASIC_STATUS_100BTX_HALF_DUPLEX = (1<<13),
	BASIC_STATUS_100BTX_FULL_DUPLEX = (1<<14),
	BASIC_STATUS_100BT4_ABILITY = (1<<15)
} BASIC_STATUS_REGISTER_BITS;

enum {
	AUTONEG_PARTNER_100BTX_FULL = (1<<8),
	AUTONEG_PARTNER_100BTX = (1<<7),
	AUTONEG_PARTNER_10BTX_FULL = (1<<6),
	AUTONEG_PARTNER_10BTX = (1<<5)
} AUTONEG_PARTNER_BITS;
enum {
	PHY_ID_NSC_DP8384C = 0x20005C90,
	PHY_ID_SMSC_LAN8720A = 0x0007C0F0
} PHYIDS;

#define LPC_EMAC_MII_WRITE_TIMEOUT 0x50000
#define LPC_EMAC_MII_READ_TIMEOUT 0x50000
#define LPC_EMAC_PHY_TIMEOUT 0x10000

#define is_mII_busy() ((LPC_EMAC->MIND & 1) == 1)
typedef struct
{

} fnet_lpceth_if_t;

/** Set up TX and RX DMA memory space */

#define NUM_OF_RX_FRAGMENTS 4
#define NUM_OF_TX_FRAGMENTS 3

#define FNET_LPCETH_DMA_BLOCK_START 0x2007C000 // AHB Block 1 on the LPC
#define FNET_LPCETH_DMA_BLOCK_LEN 0x3FFF // 16k

FNET_COMP_PACKED_BEGIN;
typedef struct {
	uint8_t *packetPtr;
	uint32_t controlWord;
} fnet_lpceth_rx_descriptor;
FNET_COMP_PACKED_END;
#define FNET_LPCETH_RX_DESCRIPTOR_CNTRL_INT_ENABLE (1<<31)
#define FNET_LPCETH_TX_DESCRIPTOR_CNTRL_INT_ENABLE (1<<31)
typedef struct {
	uint32_t statusInfo;
	uint32_t statusHashCRC;
} fnet_lpceth_rx_status;

typedef fnet_lpceth_rx_descriptor fnet_lpceth_tx_descriptor;

typedef struct {
	uint32_t statusInfo;
} fnet_lpceth_tx_status;

// FUNCTION PROTOTYPES
int fnet_lpceth_init(fnet_netif_t *netif);

void fnet_lpceth_release(fnet_netif_t *netif);

void fnet_lpceth_input(fnet_netif_t *netif);
void fnet_lpceth_output(fnet_netif_t *netif, unsigned short type, const fnet_mac_addr_t dest_addr, fnet_netbuf_t* nb);

int fnet_lpceth_get_hw_addr(fnet_netif_t *netif, unsigned char * hw_addr);
int fnet_lpceth_set_hw_addr(fnet_netif_t *netif, unsigned char * hw_addr);

int fnet_lpceth_get_statistics(struct fnet_netif *netif, struct fnet_netif_statistics * statistics);

int fnet_lpceth_is_connected(fnet_netif_t *netif);

void fnet_lpceth_init_dma(void);

uint8_t fnet_lpceth_transmit_packet(uint8_t *packet, uint16_t size);
uint32_t fnet_lpceth_read_packet(void *buffer);



#define TX_DESCRIPTOR_LAST_FRAME (1<<30)
#define TX_DESCRIPTOR_SIZE_BITS 0x7FF
#define RX_STATUS_SIZE_BITS 0x7FF

#define ETH_HASH_FILTER_CRC_MASK 0x1F800000
enum {
	ETH_INTSTATUS_RX_OVERRUN = (1<<0),
	ETH_INTSTATUS_RX_ERROR = (1<<1),
	ETH_INTSTATUS_RX_FINISHED = (1<<2),
	ETH_INTSTATUS_RX_DONE = (1<<3),
	ETH_INTSTATUS_TX_UNDERRUN = (1<<4),
	ETH_INTSTATUS_TX_ERROR = (1<<5),
	ETH_INTSTATUS_TX_FINISHED = (1<<6),
	ETH_INTSTATUS_TX_DONE = (1<<7),
	ETH_INTSTATUS_SOFTINT = (1<<12),
	ETH_INTSTATUS_WAKEUP = (1<<13)
} intStatus_bits;

#define disable_ethernet_interrupts() LPC_EMAC->IntEnable = 0
#define enable_ethernet_interrupts(interrupts_enable) LPC_EMAC->IntEnable = interrupts_enable
#define mask_ethernet_interrupt(interrupts) LPC_EMAC->IntEnable &~ interrupts
#define unmask_ethernet_interrupt(interrupts) LPC_EMAC->IntEnable |= interrupts

#define clear_ethernet_interrupts() LPC_EMAC->IntClear = 0xFFFF;

void fnet_lpceth_interrupt_handler_top(void);
void fnet_lpceth_interrupt_handler_bottom(void);

enum {
	ETH_RECEIVE_UNICAST = (1<<0),
	ETH_RECEIVE_BROADCAST = (1<<1),
	ETH_RECEIVE_MULTICAST = (1<<2),
	ETH_RECEIVE_UNICAST_HASH = (1<<3),
	ETH_RECEIVE_MULTICAST_HASH = (1<<4),
	ETH_RECEIVE_PERFECT = (1<<5),
	ETH_RECEIVE_WOL_MAGIC = (1<<12),
	ETH_RECIEVE_WOL_FILTER = (1<<13)
};

#define set_recieve_filter(accept) LPC_EMAC->RxFilterCtrl = accept

void fnet_lpceth_multicast_join(fnet_netif_t *netif,
		fnet_mac_addr_t multicast_addr);
void fnet_lpceth_multicast_leave(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr );


#endif
#endif /* FNET_LPC_ETH_H_ */

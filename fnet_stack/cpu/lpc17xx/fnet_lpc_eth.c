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
 * fnet_lpc_eth.c
 *
 *  Created on: Dec 8, 2012
 *      Author: matt
 */

#include "fnet.h"
#include "lpc_debug.h"

#if FNET_LPC && FNET_CFG_ETH


#include "fnet_eth_prv.h"
#include "fnet_lpc_eth.h"

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

char loop;

uint8_t *rxFragmentPtr;
uint8_t *txFragmentPtr;

uint32_t recvPackets=0,sentPackets=0;

uint8_t usedTxDescr = 0;
uint8_t transmitLock = 0;
uint8_t recvPacketsWaiting = 0;

char txDescriptorStatus[NUM_OF_TX_FRAGMENTS];

fnet_lpceth_if_t fnet_lpceth0_if; // Blank

/************************************************************************
* Network interface API structure.
*************************************************************************/
const fnet_netif_api_t fnet_lpceth_api =
{
    FNET_NETIF_TYPE_ETHERNET,           /* Data-link type. */
    sizeof(fnet_mac_addr_t),
    fnet_lpceth_init,                   /* Initialization function.*/
    fnet_lpceth_release,                /* Shutdown function.*/
#if FNET_CFG_IP4
	fnet_eth_output_ip4,            /* IPv4 Transmit function.*/
#endif
	fnet_eth_change_addr_notify,    /* Address change notification function.*/
	fnet_eth_drain,                 /* Drain function.*/
	fnet_lpceth_get_hw_addr,
	fnet_lpceth_set_hw_addr,
	fnet_lpceth_is_connected,
	fnet_lpceth_get_statistics
#if FNET_CFG_MULTICAST
    #if FNET_CFG_IP4
        ,
	    fnet_eth_multicast_join_ip4,
	    fnet_eth_multicast_leave_ip4
    #endif
    #if FNET_CFG_IP6
        ,
	    fnet_eth_multicast_join_ip6,
	    fnet_eth_multicast_leave_ip6
    #endif
#endif
#if FNET_CFG_IP6
    ,
    fnet_eth_ip6_output            /* IPv6 Transmit function.*/
#endif
};

/************************************************************************
* Ethernet interface structure.
*************************************************************************/
fnet_eth_if_t fnet_lpceth_eth0_if =
{
    &fnet_lpceth0_if,              /* Points to CPU-specific control data structure of the interface. */
    fnet_lpceth_output
#if FNET_CFG_MULTICAST
    ,
    fnet_lpceth_multicast_join,
    fnet_lpceth_multicast_leave,
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
	FNET_CFG_ETH_MTU,           /* Maximum transmission unit.*/
	&fnet_lpceth_eth0_if,       /* Points to interface specific data structure.*/
	&fnet_lpceth_api            /* Interface API */
};

uint8_t processingRxPacket = 0;

/** Write a byte to the ethernet PHY
 * @arg reg PHY register to write to
 * @arg value value to write to PHY
 */
void fnet_lpceth_write_to_phy(uint8_t reg, uint16_t value) {
	unsigned int loop;
	// Set the address and register we wish to access
	LPC_EMAC->MADR = ((LPC_EMAC_DEFAULT_PHY_ADDRESS & 0x1F) << 8) | (reg & 0xFFFF);
	LPC_EMAC->MWTD = value;
	for (loop = 0; loop < LPC_EMAC_MII_WRITE_TIMEOUT; loop++) {
		if (is_mII_busy() == 0) {break;}
	}
}

uint16_t fnet_lpceth_read_from_phy(uint8_t reg) {
	LPC_EMAC->MADR = ((LPC_EMAC_DEFAULT_PHY_ADDRESS & 0x1F) << 8) | (reg & 0xFFFF);
	LPC_EMAC->MCMD = LPC_EMAC_MCMD_READ;

	unsigned int loop;
	for(loop=0; loop < LPC_EMAC_MII_READ_TIMEOUT; loop++) {
		if (is_mII_busy() == 0) {
			LPC_EMAC->MCMD = LPC_EMAC_MCMD_NONE;
			return LPC_EMAC->MRDD;
		}
	}
	return -1;
}

void fnet_eth_io_init()
{
	LPC_SC->PCONP |= LPC_PCON_ENABLE_ETH_POWER;

	set_pinsel2_ethernet_funcs();

	set_pinsel3_ethernet_funcs();

	reset_mac1_internal_modules();

	reset_mac_datapaths();
	// Short delay
	for (loop = 100; loop; loop--);

	mac_pass_all_receive_frames();

	mac_enable_crc_short_frames();

	set_mac_max_frame_size(LPC_ETH_MAX_FRAME_SIZE);
	mac_disable_huge_frames();

	set_retransmission_max_collision_window(LPC_ETH_DEFAULT_RETRANSMISSION_MAX,LPC_ETH_DEFAULT_COLLISION_WIN);

	set_non_b2b_pkt_gap(LPC_ETH_DEFAULT_NON_B2B_PKT_GAP);

	set_mII_clk_div(MII_CLK_DIV_64);
	reset_mII_interface();

}

int fnet_lpceth_init(fnet_netif_t *netif)
{
	fnet_eth_io_init();
	uint32_t phyId = 0,temp=0;

	// Wait a bit
	for (loop = 100; loop; loop--);
	// Bring the MII out of reset
	enable_mII_interface();

	// Enable the MII interface
	enable_command_register();

	fnet_lpceth_write_to_phy(BASIC_CONTROL_REGISTER,BASIC_CONTROL_SOFT_RESET);
	// Loop until hardware reset completes
	for (loop = 0; loop < 0x100000; loop++) {
		//value = ReadFromPHY (PHY_REG_BMCR);
		temp = fnet_lpceth_read_from_phy(BASIC_CONTROL_REGISTER);
		if (!(temp & BASIC_CONTROL_SOFT_RESET)) {
			// Reset has completed
			break;
		}
	}

	if (phyId != 0) {
		fnet_lpceth_write_to_phy(BASIC_CONTROL_REGISTER,BASIC_CONTROL_ENABLE_AUTONEG);
		for(loop=0; loop<(LPC_EMAC_MII_READ_TIMEOUT+LPC_EMAC_MII_WRITE_TIMEOUT);loop++) {
			temp = fnet_lpceth_read_from_phy(BASIC_STATUS_REGISTER);
			if (temp & BASIC_STATUS_AUTONEG_COMPLETE) {
				break;
			}
		}
	}


	for(loop=0; loop<LPC_EMAC_PHY_TIMEOUT; loop++) {
		temp = fnet_lpceth_read_from_phy(BASIC_STATUS_REGISTER);
		if (temp & BASIC_STATUS_LINK_STATUS) {
			break;
		} else {
			fnet_timer_delay(10); // wait a bit
		}
		// TODO: Figure out what to do in event of timeout..
	}

	// Read the autonegotiation partner register to find out if the other end is 100BASE-TX FULL
	temp = fnet_lpceth_read_from_phy(AUTONEG_ADVERTISEMENT_LINK_PART_ABILITY_REGISTER);
	if (temp & AUTONEG_PARTNER_100BTX_FULL) {
		mac_enable_full_duplex();
		command_enable_full_duplex();
		// Should we set IPGT to 15 as NXP does?
		phy_set_100();
		fnet_printf("100BASE-TX Full Duplex");
	}

	fnet_lpceth_init_dma();

#if FNET_CFG_MULTICAST
	set_recieve_filter(ETH_RECEIVE_BROADCAST | ETH_RECEIVE_PERFECT | ETH_RECEIVE_MULTICAST_HASH);
#else
	set_recieve_filter(ETH_RECEIVE_BROADCAST | ETH_RECEIVE_PERFECT);
#endif

	disable_ethernet_interrupts();
	enable_ethernet_interrupts(ETH_INTSTATUS_RX_DONE | ETH_INTSTATUS_TX_DONE | ETH_INTSTATUS_RX_OVERRUN | ETH_INTSTATUS_TX_UNDERRUN);

	clear_ethernet_interrupts();

	set_enable_rxtx_functions(COMMAND_ENABLE_RECEIVE | COMMAND_ENABLE_TRANSMIT);

	mac_enable_receive_frames();

	NVIC_EnableIRQ(ENET_IRQn);
	fnet_isr_vector_init(ENET_IRQn,fnet_lpceth_interrupt_handler_top,fnet_lpceth_interrupt_handler_bottom,0);

	return FNET_OK;
}

void fnet_lpceth_init_dma() {
	fnet_memset((void *)FNET_LPCETH_DMA_BLOCK_START,0,FNET_LPCETH_DMA_BLOCK_LEN);
	uint8_t loop;

	LPC_EMAC->RxDescriptor = FNET_LPCETH_DMA_BLOCK_START;
	LPC_EMAC->RxStatus = LPC_EMAC->RxDescriptor + sizeof(fnet_lpceth_rx_descriptor) * NUM_OF_RX_FRAGMENTS;
	rxFragmentPtr = (uint8_t *)(LPC_EMAC->RxStatus + sizeof(fnet_lpceth_rx_status) * NUM_OF_RX_FRAGMENTS);
	LPC_EMAC->TxDescriptor = (uint8_t *) (rxFragmentPtr + LPC_ETH_MAX_FRAME_SIZE*NUM_OF_RX_FRAGMENTS);
	LPC_EMAC->TxStatus = LPC_EMAC->TxDescriptor + sizeof(fnet_lpceth_tx_descriptor) * NUM_OF_TX_FRAGMENTS;
    txFragmentPtr = (uint8_t *)(LPC_EMAC->TxStatus + sizeof(fnet_lpceth_tx_status) * NUM_OF_TX_FRAGMENTS);
    LPC_EMAC->RxDescriptorNumber = NUM_OF_RX_FRAGMENTS - 1;
    LPC_EMAC->TxDescriptorNumber = NUM_OF_TX_FRAGMENTS - 1;

    LPC_EMAC->RxConsumeIndex = 0;
    LPC_EMAC->TxProduceIndex = 0;


    for(loop=0; loop<NUM_OF_RX_FRAGMENTS; loop++) {
    	fnet_lpceth_rx_descriptor *rxDesc = (fnet_lpceth_rx_descriptor *) (LPC_EMAC->RxDescriptor + loop*sizeof(fnet_lpceth_rx_descriptor));
    	rxDesc->controlWord = FNET_LPCETH_RX_DESCRIPTOR_CNTRL_INT_ENABLE | (LPC_ETH_MAX_FRAME_SIZE-1);
    	rxDesc->packetPtr = rxFragmentPtr + loop*LPC_ETH_MAX_FRAME_SIZE;
    	fnet_lpceth_rx_status *rxStatus = (fnet_lpceth_rx_status *) (LPC_EMAC->RxStatus + loop*sizeof(fnet_lpceth_rx_status));
    	rxStatus->statusInfo = 0;
    	rxStatus->statusHashCRC = 0;
    	//rxStatus->controlWord = 0;
    }

    for(loop=0; loop<NUM_OF_TX_FRAGMENTS; loop++) {
    	fnet_lpceth_tx_descriptor *txDesc = (fnet_lpceth_tx_descriptor * )(LPC_EMAC->TxDescriptor + loop*sizeof(fnet_lpceth_tx_descriptor));
    	txDesc->packetPtr = txFragmentPtr + loop*LPC_ETH_MAX_FRAME_SIZE;
    	// No need to touch this at the moment
    	//fnet_lpceth_tx_status *txStatus = (fnet_lpceth_tx_status *) (LPC_EMAC->TxStatus + loop*sizeof(fnet_lpceth_tx_status));
    	//txStatus->controlWord = 0;
    	txDescriptorStatus[loop] = 0;
    }

}

void fnet_lpceth_release(fnet_netif_t *netif)
{
}

void fnet_lpceth_input(fnet_netif_t *netif)
{
}

void fnet_lpceth_output(fnet_netif_t *netif, unsigned short type, const fnet_mac_addr_t dest_addr, fnet_netbuf_t* nb) {
#if LPC_DEBUG_LEDS
	led2_on();
#endif
	uint8_t currentIndex = LPC_EMAC->TxProduceIndex;
	uint8_t nextIndex = currentIndex+1;

	if (nb->total_length == 0) {
		return;
	}
	if (usedTxDescr == (NUM_OF_TX_FRAGMENTS)) {
			fnet_println("Out of TX descriptors!");
			return; // memory full
	}

	//led2_on();
	if (nextIndex > LPC_EMAC->TxDescriptorNumber) {
		nextIndex = 0; // loop around once we reach the maximum number of descriptors
	}

	if (txDescriptorStatus[currentIndex]) {
		fnet_println("TX Descriptor %d busy",currentIndex);
	}
	txDescriptorStatus[currentIndex] = 1;
	//fnet_printf("td:%d\n",LPC_EMAC->TxProduceIndex);
	fnet_lpceth_tx_descriptor *descriptor = (fnet_lpceth_tx_descriptor *)(LPC_EMAC->TxDescriptor + currentIndex*sizeof(fnet_lpceth_tx_descriptor));
        
	fnet_lpceth_tx_status *status = (fnet_lpceth_tx_status *)(LPC_EMAC->TxStatus + currentIndex*sizeof(fnet_lpceth_tx_status));
        
	fnet_memset(descriptor->packetPtr,0,1536);
        
	/* Construct the ethernet header directly onto the TX packet buffer */
	fnet_eth_header_t *ethHeader = (fnet_eth_header_t *)descriptor->packetPtr;
        
	fnet_memcpy (ethHeader->destination_addr, dest_addr, sizeof(fnet_mac_addr_t));
        
	fnet_lpceth_get_hw_addr(netif, ethHeader->source_addr);

	ethHeader->type=fnet_htons(type);

	void *packetData = descriptor->packetPtr + FNET_ETH_HDR_SIZE;
	fnet_netbuf_to_buf(nb, 0, FNET_NETBUF_COPYALL, packetData);

	uint16_t frameSize = nb->total_length + FNET_ETH_HDR_SIZE;

	//descriptor->controlWord &= ~TX_DESCRIPTOR_SIZE_BITS;
	descriptor->controlWord = (frameSize-1) | TX_DESCRIPTOR_LAST_FRAME | FNET_LPCETH_TX_DESCRIPTOR_CNTRL_INT_ENABLE ; // don't override any defaults

	LPC_EMAC->TxProduceIndex = nextIndex;
	usedTxDescr++;
	sentPackets++;
	//transmitLock = 0;
	fnet_netbuf_free_chain(nb);
	txDescriptorStatus[currentIndex] = 0;
#if LPC_DEBUG_LEDS
	led2_off();
#endif
}
int fnet_lpceth_get_hw_addr(fnet_netif_t *netif, unsigned char * hw_addr)
{
	hw_addr[0] = (LPC_EMAC->SA0 & 0xFF00) >> 8;
	hw_addr[1] = (LPC_EMAC->SA0 & 0xFF);
	hw_addr[2] = (LPC_EMAC->SA1 & 0xFF00) >> 8;
	hw_addr[3] = (LPC_EMAC->SA1 & 0xFF);
	hw_addr[4] = (LPC_EMAC->SA2 & 0xFF00) >> 8;
	hw_addr[5] = (LPC_EMAC->SA2 & 0xFF);

	return FNET_OK;
}
int fnet_lpceth_set_hw_addr(fnet_netif_t *netif, unsigned char * hw_addr)
{
	LPC_EMAC->SA0 = (hw_addr[0] << 8) | hw_addr[1];
	LPC_EMAC->SA1 = (hw_addr[2] << 8) | hw_addr[3];
	LPC_EMAC->SA2 = (hw_addr[4] << 8) | hw_addr[5];
	return FNET_OK;

}
int fnet_lpceth_get_statistics(struct fnet_netif *netif, struct fnet_netif_statistics * statistics)
{
	statistics->tx_packet = sentPackets;
	statistics->rx_packet = recvPackets;
	return FNET_OK;
}
int fnet_lpceth_is_connected(fnet_netif_t *netif) {
	return FNET_ERR;
}

void fnet_lpceth_interrupt_handler_top() {
	//LPC_EMAC->IntClear = 0xC0; //ignore the top two bits (transmit done status)
	/* if ((LPC_EMAC->IntStatus & ETH_INTSTATUS_RX_FINISHED) == ETH_INTSTATUS_RX_FINISHED) {
		LPC_EMAC->IntClear = ETH_INTSTATUS_RX_FINISHED;
	}
	if ((LPC_EMAC->IntStatus & ETH_INTSTATUS_RX_ERROR) == ETH_INTSTATUS_RX_ERROR) {
		// Often, there is no error, but an ARP packet, and the MAC can't tell the difference
		LPC_EMAC->IntClear = 0x2;
	} */
	if ((LPC_EMAC->IntStatus & ETH_INTSTATUS_RX_DONE) == ETH_INTSTATUS_RX_DONE) {
		//mask_ethernet_interrupt(ETH_INTSTATUS_RX_DONE);
		//LPC_EMAC->IntClear = ETH_INTSTATUS_RX_DONE;
		recvPacketsWaiting++;
		//unmask_ethernet_interrupt(ETH_INTSTATUS_RX_DONE);
	}
	 if ((LPC_EMAC->IntStatus & ETH_INTSTATUS_RX_OVERRUN) == ETH_INTSTATUS_RX_OVERRUN) {
		//LPC_EMAC->IntClear = ETH_INTSTATUS_RX_OVERRUN;
		fnet_printf("RxOverrun!\n");
	}
	if ((LPC_EMAC->IntStatus & ETH_INTSTATUS_TX_UNDERRUN) == ETH_INTSTATUS_TX_UNDERRUN) {
		//LPC_EMAC->IntClear = ETH_INTSTATUS_TX_UNDERRUN;
		fnet_printf("TxUnderrun!\n");
	}
	if ((LPC_EMAC->IntStatus & ETH_INTSTATUS_TX_DONE) == ETH_INTSTATUS_TX_DONE) {
		usedTxDescr--;
		if (usedTxDescr == 255) {
			usedTxDescr = 0; // assume a screw up
		}
		//led2_off();
		//LPC_EMAC->IntClear = ETH_INTSTATUS_TX_DONE;
	}
	clear_ethernet_interrupts();

}
void fnet_lpceth_interrupt_handler_bottom() {
	while (recvPacketsWaiting > 0) {
		//uint8_t x;
		//while(processingRxPacket) {
		//	x++;
		//}
		//processingRxPacket = 1;
		if (LPC_EMAC->RxConsumeIndex == LPC_EMAC->RxProduceIndex) {
			return; // buffer is full
		}
#if LPC_DEBUG_LEDS
		led1_on();
#endif
		fnet_netbuf_t *nb;
		fnet_eth_header_t *ethheader;
		fnet_lpceth_rx_descriptor *rxDescriptor = (fnet_lpceth_rx_descriptor *)(LPC_EMAC->RxDescriptor + LPC_EMAC->RxConsumeIndex*sizeof(fnet_lpceth_rx_descriptor));
		fnet_lpceth_rx_status *rxStatus = (fnet_lpceth_rx_status *)(LPC_EMAC->RxStatus + LPC_EMAC->RxConsumeIndex*sizeof(fnet_lpceth_rx_status));
		uint16_t pktSize = (rxStatus->statusInfo & RX_STATUS_SIZE_BITS) -1;

		/* Note - the ethernet header type will be in big endian, but
		 * this has already been accounted for elsewhere in FNET.
		 * (do not do a endian conversion here)
		 */
		ethheader = (fnet_eth_header_t *)(rxDescriptor->packetPtr);
		fnet_eth_trace("\nRX", ethheader);

		void *layer3Ptr = rxDescriptor->packetPtr + sizeof(fnet_eth_header_t);
		uint16_t sizeOfLayer3 = pktSize - sizeof(fnet_eth_header_t);

		nb = fnet_netbuf_from_buf(layer3Ptr,sizeOfLayer3,FNET_TRUE);
		if (nb != 0) {
			fnet_eth_prot_input(&fnet_eth0_if,nb,ethheader->type);
			recvPackets++;
		}
		// Reset the status and descriptor
		//rxStatus = 0;
		//fnet_memset_zero(rxStatus,8);
		rxStatus->statusInfo = 0;
		rxStatus->statusHashCRC = 0;
		//rxDescriptor->controlWord = (1<<31);

		uint8_t nextDescriptor = LPC_EMAC->RxConsumeIndex+1;
		if (nextDescriptor > LPC_EMAC->RxDescriptorNumber) {
			LPC_EMAC->RxConsumeIndex = 0;
		} else {
			LPC_EMAC->RxConsumeIndex = nextDescriptor;
		}
		//processingRxPacket = 0;
		recvPacketsWaiting--;
#if LPC_DEBUG_LEDS
		led1_off();
#endif
	}
}
/*
 * This acts as the receive handler, trigged by an interrupt on reception of an Ethernet frame
 */
void ENET_IRQHandler(void) {
	fnet_isr_handler(ENET_IRQn);
}

uint8_t fnet_lpceth_transmit_packet(uint8_t *packet, uint16_t size) {
	uint8_t currentIndex = LPC_EMAC->TxProduceIndex;
	uint8_t nextIndex = currentIndex+1;
	if (size == 0)
		return 0;
	//if (LPC_EMAC->TxProduceIndex == LPC_EMAC->TxConsumeIndex) {
	//	return 0; // memory full
	//}
	if (nextIndex > LPC_EMAC->TxDescriptorNumber) {
		nextIndex = 0; // loop around once we reach the maximum number of descriptors
	}

	// Copy the packet to the reserved memory location
	fnet_lpceth_tx_descriptor *descriptor = (fnet_lpceth_tx_descriptor *)(LPC_EMAC->TxDescriptor + currentIndex*sizeof(fnet_lpceth_tx_descriptor));
	fnet_memcpy(descriptor->packetPtr,packet,size);
	descriptor->controlWord &= ~TX_DESCRIPTOR_SIZE_BITS;
	descriptor->controlWord = (size-1) | TX_DESCRIPTOR_LAST_FRAME; // don't override any defaults

	LPC_EMAC->TxProduceIndex = nextIndex;

	return 1;
}

uint32_t fnet_lpceth_read_packet(void *buffer) {
	if (LPC_EMAC->RxConsumeIndex == LPC_EMAC->RxProduceIndex) {
		// array empty
		return 0;
	}

	fnet_lpceth_rx_descriptor *rxDescriptor = (fnet_lpceth_rx_descriptor *)(LPC_EMAC->RxDescriptor + LPC_EMAC->RxConsumeIndex*sizeof(fnet_lpceth_rx_descriptor));
	fnet_lpceth_rx_status *rxStatus = (fnet_lpceth_rx_status *)(LPC_EMAC->RxStatus + LPC_EMAC->RxConsumeIndex*sizeof(fnet_lpceth_rx_status));
	uint32_t pktSize = (rxStatus->statusInfo & RX_STATUS_SIZE_BITS) -1;

	fnet_memcpy(buffer,rxDescriptor->packetPtr,pktSize);

	uint8_t nextDescriptorIndex = LPC_EMAC->RxConsumeIndex+1;
	if (nextDescriptorIndex > LPC_EMAC->RxDescriptorNumber)
		nextDescriptorIndex = 0;
	LPC_EMAC->RxConsumeIndex = nextDescriptorIndex;
	return pktSize;
}
#if FNET_CFG_MULTICAST

/** Calculate a CRC32 hash of a MAC addressed - used for multicast and the imperfect hash filter
 * Copied from fnet_fec_crc_hash
 * */
static uint32_t fnet_lpceth_crc_hash(fnet_mac_addr_t multicast_addr )
{
   uint32_t crc = 0xFFFFFFFFL;
   int i;
   int j;

   for (i=0; i<6; i++)
   {
      uint8_t c = multicast_addr[i];
      for (j=0; j<8; j++)
      {
         if ((c ^ crc) & 1)
         {
            crc >>= 1;
            c >>= 1;
            crc ^= 0xEDB88320L;
         }
         else
         {
            crc >>= 1;
            c >>= 1;
         }
      }
   }
   return  crc;
}

/************************************************************************
* NAME: fnet_lpceth_multicast_join
*
* DESCRIPTION: Joins a multicast group on FEC interface.
*************************************************************************/
void fnet_lpceth_multicast_join(fnet_netif_t *netif,
		fnet_mac_addr_t multicast_addr) {
	//fnet_fec_if_t *ethif = ((fnet_eth_if_t *) (netif->if_ptr))->if_cpu_ptr;
	uint32_t reg_value;
	uint32_t crc;

	/* Set the appropriate bit in the hash table */
	crc = fnet_lpceth_crc_hash(multicast_addr);
	crc = (crc >> 23);
	crc &= 0x3F;

	reg_value = 0x1 << (crc);

	if (crc < 32) {
		LPC_EMAC->HashFilterL |= reg_value;
	} else {
		LPC_EMAC-> HashFilterH |= reg_value;
	}
}


/************************************************************************
* NAME: fnet_fec_multicast_leave
*
* DESCRIPTION: Leavess a multicast group on FEC interface.
*************************************************************************/
void fnet_lpceth_multicast_leave(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr )
{
    //fnet_fec_if_t *ethif = ((fnet_eth_if_t *)(netif->if_ptr))->if_cpu_ptr;
    uint32_t reg_value;
    uint32_t crc;

    /* Set the appropriate bit in the hash table */
    crc = fnet_lpceth_crc_hash(multicast_addr );
    crc = crc >> 23;
    crc &= 0x3F;

    reg_value = 0x1 << (crc);

    if (crc < 32)
    {
        LPC_EMAC->HashFilterL &= ~reg_value;
    }
    else
    {
        LPC_EMAC->HashFilterH &= ~reg_value;
    }
}
#endif

#endif /* FNET_LPC && FNET_CFG_ETH */

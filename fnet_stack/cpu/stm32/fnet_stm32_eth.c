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

#include "ch.h"
#include "hal.h"
#include "evtimer.h"

#define PERIODIC_TIMER_ID       1
#define FRAME_RECEIVED_ID       2

#include "fnet_stm32_eth.h"
#include "fnet_eth_prv.h"

#ifndef FNET_THREAD_PRIORITY
#define FNET_THREAD_PRIORITY    LOWPRIO
#endif

/************************************************************************
* Network interface API structure.
*************************************************************************/
const fnet_netif_api_t fnet_stm32_mac_api =
{
    FNET_NETIF_TYPE_ETHERNET,       /* type: Data-link type. */
    sizeof(fnet_mac_addr_t),        /* hw_addr_size */
    fnet_stm32_init,                /* init: Initialization function.*/
    fnet_stm32_release,             /* release: Shutdown function.*/
#if FNET_CFG_IP4
	fnet_eth_output_ip4,            /* output_ip4: IPv4 Transmit function.*/
#endif
	fnet_eth_change_addr_notify,    /* set_addr_notify: Address change notification function.*/
	fnet_eth_drain,                 /* drain: Drain function.*/
	fnet_stm32_get_hw_addr,         /* get_hw_addr */
	fnet_stm32_set_hw_addr,         /* set_hw_addr */
	fnet_stm32_is_connected,        /* is_connected */
	fnet_stm32_get_statistics       /* get_statistics */
#if FNET_CFG_MULTICAST
    #if FNET_CFG_IP4
        ,
	    fnet_eth_multicast_join_ip4,
	    fnet_eth_multicast_leave_ip4
    #endif
#endif
};

/*****************************************************************************
*     Ethernet Control data structure 
******************************************************************************/
fnet_eth_if_t fnet_stm32_eth0_if =
{
    //&fnet_stm32_eth0_if    /* if_cpu_ptr: CPU-specific control data structure of the if. */
    &ETHD1                   /* if_cpu_ptr: ChibiOS MACDriver */
    ,0                       /* mac_number: MAC module number [0-1]. */
    ,fnet_stm32_eth_output   /* output:
                              fnet_netif_t *netif, unsigned short type, 
                              const fnet_mac_addr_t dest_addr, fnet_netbuf_t* nb */
#if FNET_CFG_MULTICAST    
    ,fnet_stm32_multicast_join
    ,fnet_stm32_multicast_leave
#endif /* FNET_CFG_MULTICAST */
    /* Internal parameters.*/
//    ,(void)NULL //connection_flag
//    ,(void)NULL //eth_timer    /* Optional ETH timer.*/
#if FNET_CFG_IP4    
//    ,arp_if
#endif   
#if FNET_CFG_IP6   
//    ,nd6_if
#endif 
#if !FNET_CFG_CPU_ETH_MIB     
 //   ,struct fnet_netif_statistics statistics
#endif   
};

/************************************************************************
* Network interface structure.
* DESCRIPTION: Indexes ethernet interface and network API structures. See
*  fnet_netif_prv.h for full specification of this struct.
*************************************************************************/
fnet_netif_t fnet_eth0_if =
{
	0,                     /* next: Pointer to the next net_if structure.*/
	0,                     /* prev: Pointer to the previous net_if structure.*/
	"eth0",                /* name: Network interface name.*/
	FNET_CFG_CPU_ETH0_MTU, /* mtu: Maximum transmission unit.*/
	&fnet_stm32_eth0_if,   /* if_ptr: Ethernet interface specific data structure.*/
	&fnet_stm32_mac_api    /* api: Interface API */
        // scope_id, features, ip4_addr, ip6_addr[], nd6_if_ptr, pmtu, pmtu_timestamp, pmtu_timer
};

/************************************************************************
* NAME: fnet_read_thread
*
* DESCRIPTION: FNET read thread. Wait for incoming packages.
*************************************************************************/
static WORKING_AREA(wa_fnet_read_thread, 128);
static msg_t fnet_read_thread(void *arg) {
   (void)arg;
   MACReceiveDescriptor rd;
   fnet_eth_header_t * ethheader;
   fnet_netbuf_t * nb=0;
   size_t size;
   chRegSetThreadName("fnet read thread");

   while (TRUE) {
      if (macWaitReceiveDescriptor(&ETHD1, &rd, MS2ST(50)) == RDY_OK) {
//         macReadReceiveDescriptor(&rd, (uint8_t *)q->payload, (size_t)q->len);
         ethheader = (fnet_eth_header_t *)rd.physdesc->rdes2; /* Point to the ethernet header.*/

         fnet_eth_trace("\nRX", ethheader); /* Print ETH header.*/

         nb = fnet_netbuf_from_buf( (void *)((unsigned long)ethheader + sizeof(fnet_eth_header_t)),
               (size - sizeof(fnet_eth_header_t)), FNET_TRUE );
         if(nb)
         {
            fnet_eth_prot_input(&fnet_eth0_if, nb, ethheader->type);
//TODO            recvPackets++;
         }
         macReleaseReceiveDescriptor(&rd);
      }
   }
}

/************************************************************************
* NAME: inits
*
* DESCRIPTION: Ethernet Physical Transceiver initialization and/or reset.
*************************************************************************/

int fnet_stm32_init(fnet_netif_t *netif)
{
  (void) netif;
  uint8_t mac_addr[6]= { 0x12,0x34,0x56,0x78,0x9A,0xBC };
  static MACConfig mac_config;
  mac_config.mac_address = mac_addr;

  // Init mac.
  macStart(&ETHD1, &mac_config);
  
  // Start read thread. This thread prosecces the incoming packages.
  chThdCreateStatic(wa_fnet_read_thread, sizeof(wa_fnet_read_thread), FNET_THREAD_PRIORITY, fnet_read_thread, NULL);
}

void fnet_stm32_release(fnet_netif_t *netif) { macStop(&ETHD1); }
void fnet_stm32_input(fnet_netif_t *netif) {}

/************************************************************************
* NAME: fnet_stm32_get_mac_addr
*
* DESCRIPTION: This function reads MAC address. 
*************************************************************************/
static void fnet_stm32_get_mac_addr(MACDriver *ethif, fnet_mac_addr_t *mac_addr)
{
  unsigned i;
  //MACDriver *ethif = ((ethif->if_ptr)->if_cpu_ptr);
  for (i = 0; i < 6; i++)
    (*mac_addr)[i] = (unsigned char)(ethif->config->mac_address)[i];
}

int fnet_stm32_get_hw_addr(fnet_netif_t *netif, unsigned char * hw_addr)
{
  //fnet_fec_if_t *ethif ;
  MACDriver *ethif;
  int result;
  //fnet_mac_addr_t macaddr
  
  // Get your MAC address here from some 
  // (&ETHD1->config->mac_address) netif->if_ptr->if_cpu_ptr->config->mac_address.

  if(netif && (netif->api->type==FNET_NETIF_TYPE_ETHERNET) 
    && ((ethif = ((fnet_eth_if_t *)(netif->if_ptr))->if_cpu_ptr) != FNET_NULL)
    && (hw_addr) )
  { 
    fnet_stm32_get_mac_addr(ethif, (fnet_mac_addr_t *) hw_addr);
    result = FNET_OK;			
  }
  else
  {
    result = FNET_ERR;
  }

  return result;
}

int fnet_stm32_set_hw_addr(fnet_netif_t *netif, unsigned char * hw_addr)
{
  unsigned i;
  (void)netif;
  ////MACDriver *ethif = (MACDriver *)((fnet_eth_if_t *)(netif->if_ptr))->if_cpu_ptr);
  //MACConfig *config = ETHD1.config;
  //fnet_mac_addr_t *mac_ptr = config->mac_address;
  //uint8_t *foo = ETHD1.config->mac_address;
  
  uint8_t *mac_ptr = ETHD1.config->mac_address;
  
//  foo[0] = hw_addr[0];
  
  for (i = 0; i < 6; i++)
    mac_ptr[i] = hw_addr[i];
  //  (ethif->config->mac_address)[i] = (*hw_addr)[i];
	  
	return FNET_OK;
}

int fnet_stm32_is_connected(fnet_netif_t *netif) { (void) netif; return (int)macPollLinkStatus(&ETHD1);}
int fnet_stm32_get_statistics(struct fnet_netif *netif, struct fnet_netif_statistics * statistics) 
{
  (void) netif; (void) statistics;
}

/************************************************************************
* NAME: fnet_stm32_output
*
* DESCRIPTION: Ethernet low-level output function.
*************************************************************************/
void fnet_stm32_eth_output(	fnet_netif_t *netif, unsigned short type, 
							const fnet_mac_addr_t dest_addr, fnet_netbuf_t* nb)
{
  struct pbuf *q;
  MACTransmitDescriptor td;
  
  if((nb!=0) && (nb->total_length<=netif->mtu))
  {
       macWaitTransmitDescriptor(&ETHD1, &td, TIME_INFINITE);

       fnet_eth_header_t *ethHeader = (fnet_eth_header_t *)td.physdesc->tdes2;

       fnet_memcpy (ethHeader->destination_addr, dest_addr, sizeof(fnet_mac_addr_t));

       fnet_stm32_get_hw_addr(netif, ethHeader->source_addr);

       ethHeader->type=fnet_htons(type);

       fnet_netbuf_to_buf(nb, 0, FNET_NETBUF_COPYALL, (void *)((unsigned long)ethHeader + FNET_ETH_HDR_SIZE));

 //      size = nb->total_length + FNET_ETH_HDR_SIZE;
       macReleaseTransmitDescriptor(&td);
  }
  
  fnet_netbuf_free_chain(nb);  
}

// Perhaps try their eth driver if the stack works out well
//int fnet_stm32_mii_write(fnet_stm32_eth_if_t *ethif, int reg_addr, fnet_uint16 data);
//int fnet_stm32_mii_read(fnet_stm32_eth_if_t *ethif, int reg_addr, fnet_uint16 *data); 

#if FNET_CFG_MULTICAST      
//void fnet_stm32_multicast_join(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
//void fnet_stm32_multicast_leave(fnet_netif_t *netif, fnet_mac_addr_t multicast_addr);
#endif /* FNET_CFG_MULTICAST */

/* For debug needs.*/
void fnet_stm32_output_frame(fnet_netif_t *netif, char* frame, int frame_size) {}
int fnet_stm32_input_frame(fnet_netif_t *netif, char* buf, int buf_size) {}
//void fnet_stm32_debug_mii_print_regs(fnet_netif_t *netif) {}
void fnet_stm32_stop(fnet_netif_t *netif) {}
void fnet_stm32_resume(fnet_netif_t *netif) {}

#endif /* FNET_MK && FNET_CFG_ETH */




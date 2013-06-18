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
* and the GNU Lesser General Public Licensalong with this program.
* If not, see <http://www.gnu.org/licenses/>.
*
**********************************************************************/ /*!
*
* @file fnet_udp.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.24.0
*
* @brief Private. UDP protocol definitions.
*
***************************************************************************/

#ifndef _FNET_UDP_H_

#define _FNET_UDP_H_

#if FNET_CFG_UDP

#include "fnet.h"
#include "fnet_socket.h"
#include "fnet_socket_prv.h"
#include "fnet_prot.h"

/************************************************************************
*     UDP definitions
*************************************************************************/
#define FNET_UDP_TTL            (64)                        /* Default TTL.*/
#define FNET_UDP_TTL_MULTICAST  (1)                         /* Default TTL for Multicast datagrams.
                                                             * RFC112 6.1: If the upper-layer protocol
                                                             * chooses not to specify a time-to-live, it should
                                                             * default to 1 for all multicast IP datagrams, so that an explicit
                                                             * choice is required to multicast beyond a single network.
                                                             */
#define FNET_UDP_DF             (0)                         /* DF flag.*/
#define FNET_UDP_TX_BUF_MAX     (FNET_CFG_SOCKET_UDP_TX_BUF_SIZE) /* Default maximum size for send socket buffer.*/
#define FNET_UDP_RX_BUF_MAX     (FNET_CFG_SOCKET_UDP_RX_BUF_SIZE) /* Default maximum size for receive socket buffer.*/

/************************************************************************
*     Global Data Structures
*************************************************************************/
extern fnet_prot_if_t fnet_udp_prot_if;

/* Structure of UDP header.*/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned short source_port FNET_COMP_PACKED;      /* Source port number.*/
    unsigned short destination_port FNET_COMP_PACKED; /* Destination port number.*/
    unsigned short length FNET_COMP_PACKED;           /* Length.*/
    unsigned short checksum FNET_COMP_PACKED;         /* Checksum.*/
} fnet_udp_header_t;
FNET_COMP_PACKED_END

/************************************************************************
*     Function Prototypes
*************************************************************************/
static void fnet_udp_release(void);
static int fnet_udp_output(struct sockaddr * src_addr, const struct sockaddr * dest_addr, fnet_socket_option_t *sockoption, fnet_netbuf_t *nb );
static void fnet_udp_input_ip4(fnet_netif_t *netif, fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip4_nb);
static void fnet_udp_input_ip6(fnet_netif_t *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, fnet_netbuf_t *nb, fnet_netbuf_t *ip6_nb);

#endif  /* FNET_CFG_UDP */

#endif /* _FNET_UDP_H_ */

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
* @file fnet_igmp.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.8.0
*
* @brief Private. IGMPv2 protocol function definitions, data structures, etc.
*
***************************************************************************/

#ifndef _FNET_IGMP_H_

#define _FNET_IGMP_H_

#include "fnet_config.h"

#if FNET_CFG_IGMP

#include "fnet.h"
#include "fnet_netif_prv.h"
#include "fnet_prot.h"

/************************************************************************
 * RFC2236: The Internet Group Management Protocol (IGMP) is used by IP hosts to
 * report their host group memberships to any immediately-neighboring
 * multicast routers.
 ************************************************************************/

/************************************************************************
*     Definition of type and code field values.
*************************************************************************/

/************************************************************************
 * @internal
 * @brief    ICMP message header.
 ************************************************************************
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      Type     |    Unused     |           Checksum            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Group Address                         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 ************************************************************************/

FNET_COMP_PACKED_BEGIN
typedef struct
{
    fnet_uint8      type FNET_COMP_PACKED;           /* Type.*/
    fnet_uint8      max_resp_time FNET_COMP_PACKED;  /* IGMPv1 Unused field, zeroed when sent, ignored when received.*/
                                    /* IGMPv2 Max Response Time (is meaningful only in Membership Query).*/
                                    /* NOTE: Current version of FNET ignores this parameter.*/
    fnet_uint16     checksum FNET_COMP_PACKED;       /* The checksum is the 16-bit one’s complement of the one’s
                                     * complement sum of the 8-octet IGMP message.*/
    fnet_ip4_addr_t  group_addr FNET_COMP_PACKED;     /* Group address field.*/                             
} fnet_igmp_header_t;
FNET_COMP_PACKED_END

/* IGMP Types */
#define IGMP_HEADER_TYPE_QUERY          0x11 /* Membership Query.*/
#define IGMP_HEADER_TYPE_REPORT_V1      0x12 /* Version 1 Membership Report.*/
#define IGMP_HEADER_TYPE_REPORT_V2      0x16 /* Version 2 Membership Report.*/
#define IGMP_HEADER_TYPE_LEAVE_GROUP    0x17 /* Leave Group.*/

extern fnet_prot_if_t fnet_igmp_prot_if;

/************************************************************************
*     Function Prototypes
*************************************************************************/
void fnet_igmp_join( fnet_netif_t *netif, fnet_ip4_addr_t  group_addr );
void fnet_igmp_leave( fnet_netif_t *netif, fnet_ip4_addr_t  group_addr );

#endif /* FNET_CFG_IGMP */

#endif /* _FNET_IGMP_H_ */

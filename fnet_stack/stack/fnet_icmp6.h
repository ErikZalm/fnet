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
* @file fnet_icmp6.h
*
* @author Andrey Butok
*
* @date Feb-12-2013
*
* @version 0.1.7.0
*
* @brief Private. ICMP protocol function definitions, data structures, etc.
*
***************************************************************************/

#include "fnet.h"

#ifndef _FNET_ICMP6_H_

#define _FNET_ICMP6_H_
#include "fnet_netbuf.h"


/************************************************************************
*     Definition of type and code field values.
*************************************************************************/

/******************************************************************
* ICMPv6 message types (RFC 4443)
******************************************************************/

/* ICMPv6 error messages:*/
#define FNET_ICMP6_TYPE_DEST_UNREACH            (1)     /* Destination Unreachable. */
#define FNET_ICMP6_TYPE_PACKET_TOOBIG           (2)     /* Packet Too Big. */
#define FNET_ICMP6_TYPE_TIME_EXCEED             (3)     /* Time Exceeded. */
#define FNET_ICMP6_TYPE_PARAM_PROB              (4)     /* Parameter Problem. */

/* ICMPv6 informational messages:*/
#define FNET_ICMP6_TYPE_ECHO_REQ                (128)   /* Echo Request. */
#define FNET_ICMP6_TYPE_ECHO_REPLY              (129)	/* Echo Reply. */

/*  Neighbor Discovery defines five different ICMP packet types (RFC4861):*/
#define FNET_ICMP6_TYPE_ROUTER_SOLICITATION     (133)   /* Router Solicitation. */
#define FNET_ICMP6_TYPE_ROUTER_ADVERTISEMENT    (134)   /* Router Advertisement. */
#define FNET_ICMP6_TYPE_NEIGHBOR_SOLICITATION   (135)   /* Neighbor Solicitation. */
#define FNET_ICMP6_TYPE_NEIGHBOR_ADVERTISEMENT  (136)   /* Neighbor Advertisement. */
#define FNET_ICMP6_TYPE_REDIRECT                (137)   /* Redirect.*/

/* Destination Unreachable codes */
#define FNET_ICMP6_CODE_DU_NO_ROUTE             (0)     /* No route to destination. */
#define FNET_ICMP6_CODE_DU_ADMIN_PROHIBITED     (1)     /* Communication with destination administratively prohibited. */
#define FNET_ICMP6_CODE_DU_BEYOND_SCOPE         (2)     /* Beyond scope of source address.*/
#define FNET_ICMP6_CODE_DU_ADDR_UNREACH         (3)     /* Address unreachable.*/
#define FNET_ICMP6_CODE_DU_PORT_UNREACH         (4)     /* Port unreachable.*/
#define FNET_ICMP6_CODE_DU_ADDR_FAILED          (5)     /* Source address failed ingress/egress policy.*/
#define FNET_ICMP6_CODE_DU_REJECT_ROUTE         (6)     /* Reject route to destination.*/

/* Packet Too Big codes */
#define FNET_ICMP6_CODE_PTB                     (0)  

/* Time Exceeded codes */
#define FNET_ICMP6_CODE_TE_HOP_LIMIT            (0)     /* Hop limit exceeded in transit.*/
#define FNET_ICMP6_CODE_TE_FRG_REASSEMBLY       (1)     /* Fragment reassembly time exceeded.*/

/* Parameter Problem codes */
#define FNET_ICMP6_CODE_PP_HEADER               (0)     /* Erroneous header field encountered.*/
#define FNET_ICMP6_CODE_PP_NEXT_HEADER          (1)     /* Unrecognized Next Header type encountered.*/
#define FNET_ICMP6_CODE_PP_OPTION               (2)     /* Unrecognized IPv6 option encountered.*/


#define FNET_ICMP6_HOP_LIMIT                    (FNET_IP6_HOP_LIMIT_DEFAULT)   /* The Hop Limit of ICMPv6 messages.*/

/*
* Error messages are identified as such by a
* zero in the high-order bit of their message Type field values. Thus,
* error messages have message types from 0 to 127; informational
* messages have message types from 128 to 255.
*/
#define FNET_ICMP6_TYPE_IS_ERROR(t) (((t) & 0x80) == 0x00)


/***********************************************************************
 * Generic ICMP packet header
 ***********************************************************************
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Type      |     Code      |          Checksum             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                                               |
 * +                         Message Body                          +
 * |                                                               |
 *
 ***********************************************************************/ 
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned char  type     FNET_COMP_PACKED;   /* The type of the message.*/
    unsigned char  code     FNET_COMP_PACKED;   /* The code of the message.*/
    unsigned short checksum FNET_COMP_PACKED;   /* The checksum of the message.*/
} fnet_icmp6_header_t;
FNET_COMP_PACKED_END

/***********************************************************************
 * ICMPv6 Echo packet
 ***********************************************************************
 * RFC4443 4:
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Type       |       Code    |             Checksum          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |             Identifier        |       Sequence Number         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Data ...
 * +-+-+-+-+- 
 ***********************************************************************/
FNET_COMP_PACKED_BEGIN 
typedef struct fnet_icmp6_echo_header
{
   fnet_icmp6_header_t  icmp6_header    FNET_COMP_PACKED; 
   unsigned short       id              FNET_COMP_PACKED;
   unsigned short       seq_number      FNET_COMP_PACKED;
} fnet_icmp6_echo_header_t;
FNET_COMP_PACKED_END

/***********************************************************************
 * ICMPv6 Error packet
 ***********************************************************************/
typedef struct fnet_icmp6_err_header
{
   fnet_icmp6_header_t  icmp6_header    FNET_COMP_PACKED; 
   unsigned long        data            FNET_COMP_PACKED;
} fnet_icmp6_err_header_t;

extern struct fnet_prot_if fnet_icmp6_prot_if;

/************************************************************************
*     Function Prototypes
*************************************************************************/
struct fnet_netif; /* Forward declaration.*/
void fnet_icmp6_error(struct fnet_netif *netif, unsigned char type, unsigned char code, unsigned long param, fnet_netbuf_t *origin_nb);
void fnet_icmp6_output(struct fnet_netif *netif, fnet_ip6_addr_t *src_ip, fnet_ip6_addr_t *dest_ip, unsigned char hop_limit, fnet_netbuf_t *nb );

#endif

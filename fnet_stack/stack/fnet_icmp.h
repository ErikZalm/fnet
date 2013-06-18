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
* @file fnet_icmp.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.18.0
*
* @brief Private. ICMP protocol function definitions, data structures, etc.
*
***************************************************************************/

#ifndef _FNET_ICMP_H_

#define _FNET_ICMP_H_
#include "fnet_ip_prv.h"


/************************************************************************
*     Definition of type and code field values.
*************************************************************************/
#define FNET_ICMP_ECHOREPLY                (0)  /* Echo reply message.*/
#define FNET_ICMP_UNREACHABLE              (3)  /* Destination Unreachable Message:*/
#define FNET_ICMP_UNREACHABLE_NET          (0)  /*    -net unreachable;*/
#define FNET_ICMP_UNREACHABLE_HOST         (1)  /*    -host unreachable;*/
#define FNET_ICMP_UNREACHABLE_PROTOCOL     (2)  /*    -protocol unreachable;*/
#define FNET_ICMP_UNREACHABLE_PORT         (3)  /*    -port unreachable;*/
#define FNET_ICMP_UNREACHABLE_NEEDFRAG     (4)  /*    -fragmentation needed and DF set;*/
#define FNET_ICMP_UNREACHABLE_SRCFAIL      (5)  /*    -source route failed;*/
#define FNET_ICMP_UNREACHABLE_NET_UNKNOWN  (6)  /*    -unknown net;*/
#define FNET_ICMP_UNREACHABLE_HOST_UNKNOWN (7)  /*    -unknown host;*/
#define FNET_ICMP_UNREACHABLE_ISOLATED     (8)  /*    -src host isolated;*/
#define FNET_ICMP_UNREACHABLE_NET_PROHIB   (9)  /*    -prohibited access;*/
#define FNET_ICMP_UNREACHABLE_HOST_PROHIB  (10) /*    -ditto;*/
#define FNET_ICMP_UNREACHABLE_TOSNET       (11) /*    -bad tos for net;*/
#define FNET_ICMP_UNREACHABLE_TOSHOST      (12) /*    -bad tos for host;*/
#define FNET_ICMP_SOURCEQUENCH             (4)  /* Source Quench Message; packet lost, slow down.*/
#define FNET_ICMP_REDIRECT                 (5)  /* Redirect Message:*/
#define FNET_ICMP_REDIRECT_NET             (0)  /*    -redirect datagrams for the Network;*/
#define FNET_ICMP_REDIRECT_HOST            (1)  /*    -redirect datagrams for the Host;*/
#define FNET_ICMP_REDIRECT_TOSNET          (2)  /*    -redirect datagrams for the Type of Service and Network;*/
#define FNET_ICMP_REDIRECT_TOSHOST         (3)  /*    -redirect datagrams for the Type of Service and Host;*/
#define FNET_ICMP_ECHO                     (8)  /* Echo message.*/
#define FNET_ICMP_ROUTERADVERT             (9)  /* Router advertisement.*/
#define FNET_ICMP_ROUTERSOLICIT            (10) /* Router solicitation.*/
#define FNET_ICMP_TIMXCEED                 (11) /* Time Exceeded Message:*/
#define FNET_ICMP_TIMXCEED_INTRANS         (0)  /*    -time to live exceeded in transit (ttl==0);*/
#define FNET_ICMP_TIMXCEED_REASS           (1)  /*    -fragment reassembly time exceeded (ttl==0);*/
#define FNET_ICMP_PARAMPROB                (12) /* Parameter Problem Message: */
#define FNET_ICMP_PARAMPROB_IPHEDER        (0)  /*    -IP header bad ;*/
#define FNET_ICMP_PARAMPROB_OPTABSENT      (1)  /*    -required option missing;*/
#define FNET_ICMP_TSTAMP                   (13) /* Timestamp message (request)*/
#define FNET_ICMP_TSTAMPREPLY              (14) /* Timestamp reply message*/
#define FNET_ICMP_IREQ                     (15) /* Information request message*/
#define FNET_ICMP_IREQREPLY                (16) /* Information reply message*/
#define FNET_ICMP_MASKREQ                  (17) /* Address mask request.*/
#define FNET_ICMP_MASKREPLY                (18) /* Address mask reply.*/

/* The macros returns TRUE if "type" is an ICMP request or response type and FALSE if it is an error type. */     
#define FNET_ICMP_IS_QUERY_TYPE(type)  ((type) == FNET_ICMP_ECHO || (type) == FNET_ICMP_ECHOREPLY ||     \
                                        (type) == FNET_ICMP_TSTAMP || (type) == FNET_ICMP_TSTAMPREPLY || \
                                        (type) == FNET_ICMP_IREQ || (type) == FNET_ICMP_IREQREPLY ||     \
                                        (type) == FNET_ICMP_MASKREQ || (type) == FNET_ICMP_MASKREPLY ||  \
                                        (type) == FNET_ICMP_ROUTERADVERT || (type) == FNET_ICMP_ROUTERSOLICIT )

#define FNET_ICMP_TTL                   (FNET_IP_TTL_DEFAULT)   /* The TTL of ICMP messages.*/
#define FNET_ICMP_TOS                   (IP_TOS_NORMAL)         /* The TOS of ICMP messages.*/


/**************************************************************************/ /*!
 * @internal
 * @brief    ICMP message header.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned char   type FNET_COMP_PACKED;      /* The type of the message.*/
    unsigned char   code FNET_COMP_PACKED;      /* The code of the message.*/
    unsigned short  checksum FNET_COMP_PACKED;  /* The checksum of the message.*/
} fnet_icmp_header_t;
FNET_COMP_PACKED_END

/**************************************************************************/ /*!
 * @internal
 * @brief    ICMP Echo message header.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    fnet_icmp_header_t  header FNET_COMP_PACKED;            /**< The ICMP standard header.*/
    unsigned short      identifier FNET_COMP_PACKED;        /**< Identifier.*/
    unsigned short      sequence_number FNET_COMP_PACKED;   /**< Sequence Number.*/
} fnet_icmp_echo_header_t;
FNET_COMP_PACKED_END

/**************************************************************************/ /*!
 * @internal
 * @brief    ICMP Error message header.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    fnet_icmp_header_t header FNET_COMP_PACKED;      /**< The ICMP standard header.*/

    union
    {
        unsigned long unused ;       /**< Unused.*/
        unsigned short mtu;         /**< MTU.*/
        unsigned short ptr;         /**< Pointer indicates the error.*/
    } fields FNET_COMP_PACKED;

    fnet_ip_header_t ip FNET_COMP_PACKED;            /**< IP header.*/
} fnet_icmp_err_header_t;
FNET_COMP_PACKED_END

/**************************************************************************/ /*!
 * @internal
 * @brief    ICMP Timestamp message header.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    fnet_icmp_header_t header FNET_COMP_PACKED;          /**< The ICMP standard header.*/
    unsigned short identifier FNET_COMP_PACKED;          /**< Identifier.*/
    unsigned short sequence_number FNET_COMP_PACKED;     /**< Sequence Number.*/
    unsigned long originate_timestamp FNET_COMP_PACKED;  /**< Originate timestamp.*/
    unsigned long receive_timestamp FNET_COMP_PACKED;    /**< Receive timestamp.*/
    unsigned long transmit_timestamp FNET_COMP_PACKED;   /**< Transmit timestamp.*/
} fnet_icmp_timestamp_header_t;
FNET_COMP_PACKED_END

/**************************************************************************/ /*!
 * @internal
 * @brief    ICMP Address mask message header.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    fnet_icmp_header_t header FNET_COMP_PACKED;          /**< The ICMP standard header.*/
    unsigned short identifier FNET_COMP_PACKED;          /**< Identifier.*/
    unsigned short sequence_number FNET_COMP_PACKED;     /**< Sequence Number.*/
    unsigned long mask FNET_COMP_PACKED;                 /**< Subnet mask.*/
} fnet_icmp_mask_header_t;
FNET_COMP_PACKED_END

#if FNET_CFG_IP4

extern struct fnet_prot_if fnet_icmp_prot_if;

/************************************************************************
*     Function Prototypes
*************************************************************************/
void fnet_icmp_error( fnet_netif_t *netif, unsigned char type, unsigned char code, fnet_netbuf_t *nb );

#endif /* FNET_CFG_IP4 */

#endif /* _FNET_ICMP_H_ */

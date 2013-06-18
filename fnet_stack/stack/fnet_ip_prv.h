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
**********************************************************************/
/*!
*
* @file fnet_ip_prv.h
*
* @author Andrey Butok
*
* @date Mar-4-2013
*
* @version 0.1.32.0
*
* @brief Private. IP protocol API.
*
***************************************************************************/

#ifndef _FNET_IP_PRV_H_

#define _FNET_IP_PRV_H_

#include "fnet.h"

#include "fnet_ip.h"
#include "fnet_netif.h"
#include "fnet_netif_prv.h"

/************************************************************************
*    Definitions.
*************************************************************************/
#define FNET_IP_MAX_PACKET     (FNET_CFG_IP_MAX_PACKET)

/* Check max. values.*/
#if (FNET_IP_MAX_PACKET > 65535)
    #undef FNET_IP_MAX_PACKET
    #define FNET_IP_MAX_PACKET      (65535)
#endif

#if (FNET_IP_MAX_PACKET < 200)
    #undef FNET_IP_MAX_PACKET
    #define FNET_IP_MAX_PACKET      (200)
#endif

#define FNET_IP_MAX_OPTIONS     (40) /* Maximum option field length */

/************************************************************************
*    IP implementation parameters.
*************************************************************************/
#define FNET_IP_VERSION         (4)   /* IP version */
#define FNET_IP_TTL_MAX         (255) /* maximum time to live */
#define FNET_IP_TTL_DEFAULT     (64)  /* default ttl, from RFC 1340 */

/************************************************************************
*    Supported protocols.
*************************************************************************/
#define FNET_IP_PROTOCOL_ICMP   (1)
#define FNET_IP_PROTOCOL_IGMP   (2)
#define FNET_IP_PROTOCOL_UDP    (17)
#define FNET_IP_PROTOCOL_TCP    (6)
#define FNET_IP_PROTOCOL_ICMP6  (58)

#define FNET_IP_DF              (0x4000)    /* dont fragment flag */
#define FNET_IP_MF              (0x2000)    /* more fragments flag */
#define FNET_IP_FLAG_MASK       (0xE000)    /* mask for fragmenting bits */
#define FNET_IP_OFFSET_MASK     (0x1fff)    /* mask for fragmenting bits */

#define FNET_IP_TIMER_PERIOD    (500)
#define FNET_IP_FRAG_TTL        (10000/FNET_IP_TIMER_PERIOD) /* TTL for fragments to complete a datagram (10sec)*/

/* Maximum size of IP input queue.*/
#define FNET_IP_QUEUE_COUNT_MAX (FNET_CFG_IP_MAX_PACKET/2)

/************************************************************************
*    Timestamp option
*************************************************************************/

/**************************************************************************/ /*!
 * @internal
 * @brief    timestamp.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned char code      FNET_COMP_PACKED;       /**< = IPOPT_TS */
    unsigned char length    FNET_COMP_PACKED;       /**< The number of bytes in the option.*/
    unsigned char pointer   FNET_COMP_PACKED;       /**< The number of bytes from the beginning of this option to the end of timestamps plus one.*/
    unsigned char overflow__flag FNET_COMP_PACKED;  /**< overflow counter & flag */
    union
    {
        unsigned long time[1];
        struct
        {
            fnet_ip4_addr_t address  FNET_COMP_PACKED;
            unsigned long time[1]   FNET_COMP_PACKED;
        } record    FNET_COMP_PACKED;
    } timestamp     FNET_COMP_PACKED;
} fnet_ip_timestamp_t;
FNET_COMP_PACKED_END

#define FNET_IP_TIMESTAMP_GET_OVERFLOW(x)           ((x->overflow__flag & 0xF0)>>4)
#define FNET_IP_TIMESTAMP_SET_OVERFLOW(x, overflow) (x->overflow__flag = (unsigned char)((x->overflow__flag & 0x0F)|(((overflow)&0x0F)<<4)))
#define FNET_IP_TIMESTAMP_GET_FLAG(x)               (x->overflow__flag & 0x0F)
#define FNET_IP_TIMESTAMP_SET_FLAG(x, flag)         (x->overflow__flag = (unsigned char)((x->overflow__flag & 0xF0)|((flag)&0x0F)))


/**************************************************************************/ /*!
 * @internal
 * @brief    IPv4 layer socket options.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned char       ttl       FNET_COMP_PACKED;       /**< TTL.*/
    unsigned char       tos       FNET_COMP_PACKED;       /**< TOS.*/
#if FNET_CFG_MULTICAST   
    unsigned char       ttl_multicast FNET_COMP_PACKED;   /**< TTL.*/
#endif
} fnet_ip_sockopt_t;
FNET_COMP_PACKED_END

/**************************************************************************/ /*!
 * @internal
 * @brief    Structure of IP header.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct
{
    unsigned char version__header_length    FNET_COMP_PACKED;   /**< version =4 & header length (x4) (>=5)*/     
    unsigned char tos                       FNET_COMP_PACKED;   /**< type of service */
    unsigned short total_length             FNET_COMP_PACKED;   /**< total length */
    unsigned short id                       FNET_COMP_PACKED;   /**< identification */
    unsigned short flags_fragment_offset    FNET_COMP_PACKED;   /**< flags & fragment offset field (measured in 8-byte order).*/
    unsigned char ttl                       FNET_COMP_PACKED;   /**< time to live */
    unsigned char protocol                  FNET_COMP_PACKED;   /**< protocol */
    unsigned short checksum                 FNET_COMP_PACKED;   /**< checksum */
    fnet_ip4_addr_t source_addr             FNET_COMP_PACKED;   /**< source address */
    fnet_ip4_addr_t desination_addr         FNET_COMP_PACKED;   /**< destination address */
} fnet_ip_header_t;
FNET_COMP_PACKED_END

/**************************************************************************/ /*!
 * @internal
 * @brief    Structure of IP fragment header.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct fnet_ip_frag_header
{
    unsigned char version__header_length    FNET_COMP_PACKED;   /**< version =4 & header length (x4) (>=5)*/    
    unsigned char mf                        FNET_COMP_PACKED;
    unsigned short total_length             FNET_COMP_PACKED;   /**< data-payload total length (Host endian)*/
    unsigned short id                       FNET_COMP_PACKED;   /**< identification*/
    unsigned short offset                   FNET_COMP_PACKED;   /**< offset field (measured in 8-byte order). (Host endian)*/
    fnet_netbuf_t *nb                       FNET_COMP_PACKED;
    struct fnet_ip_frag_header *next        FNET_COMP_PACKED;   /**< Pointer to the next fragment.*/
    struct fnet_ip_frag_header *prev        FNET_COMP_PACKED;   /**< Pointer to the previous fragment.*/
} fnet_ip_frag_header_t;
FNET_COMP_PACKED_END

#define FNET_IP_HEADER_GET_FLAG(x)                      (x->flags_fragment_offset & FNET_HTONS(FNET_IP_FLAG_MASK))
#define FNET_IP_HEADER_GET_OFFSET(x)                    (x->flags_fragment_offset & FNET_HTONS(FNET_IP_OFFSET_MASK))
#define FNET_IP_HEADER_GET_VERSION(x)                   ((x->version__header_length & 0xF0)>>4)
#define FNET_IP_HEADER_SET_VERSION(x, version)          (x->version__header_length = (unsigned char)((x->version__header_length & 0x0F)|(((version)&0x0F)<<4)))
#define FNET_IP_HEADER_GET_HEADER_LENGTH(x)             (x->version__header_length & 0x0F)
#define FNET_IP_HEADER_SET_HEADER_LENGTH(x, length)     (x->version__header_length = (unsigned char)((x->version__header_length & 0xF0)|((length)&0x0F)))

/**************************************************************************/ /*!
 * @internal
 * @brief    Structure of the head of each reassembly list.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN
typedef struct fnet_ip_frag_list
{
    struct fnet_ip_frag_list *next  FNET_COMP_PACKED;   /**< Pointer to the next reassembly list.*/
    struct fnet_ip_frag_list *prev  FNET_COMP_PACKED;   /**< Pointer to the previous reassembly list.*/
    unsigned char ttl               FNET_COMP_PACKED;   /**< TTL for reassembly.*/
    unsigned char protocol          FNET_COMP_PACKED;   /**< protocol.*/
    unsigned short id               FNET_COMP_PACKED;   /**< identification.*/
    fnet_ip4_addr_t source_addr     FNET_COMP_PACKED;   /**< source address.*/
    fnet_ip4_addr_t desination_addr FNET_COMP_PACKED;   /**< destination address.*/
    fnet_ip_frag_header_t *frag_ptr FNET_COMP_PACKED;   /**< Pointer to the first fragment of the list.*/
} fnet_ip_frag_list_t;
FNET_COMP_PACKED_END


/******************************************************************************
 * Multicast related structures.
 ******************************************************************************/
#if FNET_CFG_MULTICAST
    /* Entry of the multicast group list.*/
    typedef struct fnet_ip_multicast_list_entry
    {
        fnet_netif_t *netif;        /* Interface to join on. */
        fnet_ip4_addr_t group_addr; /* IP address of joined multicast group. */
        int user_counter;           /* User counter. Keeps a reference count of the number 
                                     * of requests to join a particular host group. */
    } fnet_ip_multicast_list_entry_t;

    /* Global multicast list.*/
    extern fnet_ip_multicast_list_entry_t fnet_ip_multicast_list[FNET_CFG_MULTICAST_MAX];

#endif /* FNET_CFG_MULTICAST */

typedef struct                            /* IP input queue.*/
{
    fnet_netbuf_t *head;                  /* Pointer to the queue head.*/
    unsigned long count;                  /* Number of data in buffer.*/
} fnet_ip_queue_t;

/************************************************************************
*     Function Prototypes
*************************************************************************/
int fnet_ip_init( void );
void fnet_ip_release( void );
int fnet_ip_addr_is_broadcast( fnet_ip4_addr_t addr, fnet_netif_t *netif );
int fnet_ip_output( fnet_netif_t *netif,    fnet_ip4_addr_t src_ip, fnet_ip4_addr_t dest_ip,
                    unsigned char protocol, unsigned char tos,     unsigned char ttl,
                    fnet_netbuf_t *nb,      int DF,  int do_not_route,
                    FNET_COMP_PACKED_VAR unsigned short *checksum );
void fnet_ip_input( fnet_netif_t *netif, fnet_netbuf_t *nb );
fnet_netif_t *fnet_ip_route( fnet_ip4_addr_t dest_ip );
unsigned long fnet_ip_maximum_packet( fnet_ip4_addr_t dest_ip );
void fnet_ip_drain( void );

int fnet_ip_queue_append( fnet_ip_queue_t *queue, fnet_netif_t *netif, fnet_netbuf_t *nb );
fnet_netbuf_t *fnet_ip_queue_read( fnet_ip_queue_t *queue, fnet_netif_t ** netif );
int fnet_ip_will_fragment( fnet_netif_t *netif, unsigned long protocol_message_size);

#if FNET_CFG_MULTICAST
    fnet_ip_multicast_list_entry_t *fnet_ip_multicast_join( fnet_netif_t *netif, fnet_ip4_addr_t group_addr );
    void fnet_ip_multicast_leave( fnet_ip_multicast_list_entry_t *multicastentry );
#endif /* FNET_CFG_MULTICAST */

#endif /* _FNET_IP_PRV_H_ */

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
* @file fnet_ip6.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.12.0
*
* @brief IPv6 protocol API.
*
***************************************************************************/

#ifndef _FNET_IP6_H_

#define _FNET_IP6_H_

#include "fnet_comp.h"

/*! @addtogroup fnet_socket */
/*! @{ */



/**************************************************************************/ /*!
 * @brief 128-bit IPv6 address type.
 ******************************************************************************/
FNET_COMP_PACKED_BEGIN 
typedef struct 
{
    union 
    {
        unsigned char  addr[16];
        unsigned short addr16[8];
        unsigned long  addr32[4];
    };  /* 128-bit IP6 address */
} fnet_ip6_addr_t;
FNET_COMP_PACKED_END

/**************************************************************************/ /*!
 * @brief   Size of the string buffer that will contain 
 *          null-terminated ASCII string of an IPv6 address
 *          in standard ":" notation.
 * @see fnet_inet_ntop
 * @showinitializer 
 ******************************************************************************/
#define FNET_IP6_ADDR_STR_SIZE       sizeof("abcd:abcd:abcd:abcd:abcd:abcd:abcd:abcd")

/*! @} */

#if FNET_CFG_IP6

/******************************************************************
* Constants
*******************************************************************/
#define FNET_IP6_HEADSIZE        40     /*                 */

#define FNET_IP6_DEFAULT_MTU     1280   /* Minimum IPv6 datagram size which    
                                         * must be supported by all IPv6 hosts */

/****************************************************************
 *
 * Helpful macros.
 *
 *****************************************************************/
#define FNET_IP6_ADDR_INIT(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16)      \
                            { (a1), (a2), (a3), (a4), (a5), (a6), (a7), (a8),           \
                              (a9), (a10), (a11), (a12), (a13), (a14), (a15), (a16) }

/*
 * Definition of some useful macros to handle IP6 addresses (BSD-like)
 */
#define FNET_IP6_ADDR_ANY_INIT                                  \
        {{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}
#define FNET_IP6_ADDR_LOOPBACK_INIT                             \
        {{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define FNET_IP6_ADDR_NODELOCAL_ALLNODES_INIT                   \
        {{{ 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define FNET_IP6_ADDR_INTFACELOCAL_ALLNODES_INIT                \
        {{{ 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define FNET_IP6_ADDR_LINKLOCAL_ALLNODES_INIT                   \
        {{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define FNET_IP6_ADDR_LINKLOCAL_ALLROUTERS_INIT                 \
        {{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 }}}
#define FNET_IP6_ADDR_LINKLOCAL_ALLV2ROUTERS_INIT               \
        {{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16 }}}
#define FNET_IP6_ADDR_LINKLOCAL_PREFIX_INIT                      \
        {{{ 0xFE,0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}            
            

extern const fnet_ip6_addr_t fnet_ip6_addr_any;
extern const fnet_ip6_addr_t fnet_ip6_addr_loopback;
extern const fnet_ip6_addr_t fnet_ip6_addr_nodelocal_allnodes;
extern const fnet_ip6_addr_t fnet_ip6_addr_linklocal_allnodes;
extern const fnet_ip6_addr_t fnet_ip6_addr_linklocal_allrouters;
extern const fnet_ip6_addr_t fnet_ip6_addr_linklocal_allv2routers;
extern const fnet_ip6_addr_t fnet_ip6_addr_linklocal_prefix;

/* Equality. */
#define FNET_IP6_ADDR_EQUAL(a, b)			\
        (fnet_memcmp(&(a)->addr[0], &(b)->addr[0], sizeof(fnet_ip6_addr_t)) == 0)

/* Copying address. */
#define FNET_IP6_ADDR_COPY(from_addr, to_addr)  \
        (fnet_memcpy(&(to_addr)->addr[0], &(from_addr)->addr[0], sizeof(fnet_ip6_addr_t)))


/* Unspecified.*/
#define FNET_IP6_ADDR_IS_UNSPECIFIED(a)	                                \
    	((*(const unsigned long *)(const void *)(&(a)->addr[0]) == 0) &&	\
    	 (*(const unsigned long *)(const void *)(&(a)->addr[4]) == 0) &&	\
    	 (*(const unsigned long *)(const void *)(&(a)->addr[8]) == 0) &&	\
    	 (*(const unsigned long *)(const void *)(&(a)->addr[12]) == 0))

/* Loopback.*/
#define FNET_IP6_ADDR_IS_LOOPBACK(a)	                                	\
    	((*(const unsigned long *)(const void *)(&(a)->addr[0]) == 0) &&	\
    	 (*(const unsigned long *)(const void *)(&(a)->addr[4]) == 0) &&	\
    	 (*(const unsigned long *)(const void *)(&(a)->addr[8]) == 0) &&	\
    	 (*(const unsigned long *)(const void *)(&(a)->addr[12]) == FNET_NTOHL(1)))

/* Multicast. */
#define FNET_IP6_ADDR_IS_MULTICAST(a)	((a)->addr[0] == 0xff)

/* Unicast Scope.*/
#define FNET_IP6_ADDR_IS_LINKLOCAL(a)	\
    	(((a)->addr[0] == 0xfe) && (((a)->addr[1] & 0xc0) == 0x80))
#define FNET_IP6_ADDR_IS_SITELOCAL(a)	\
    	(((a)->addr[0] == 0xfe) && (((a)->addr[1] & 0xc0) == 0xc0))


#endif /* FNET_CFG_IP6 */



#endif

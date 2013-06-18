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
* @file fnet_eth.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.9.0
*
* @brief Ethernet platform independent API definitions.
*
***************************************************************************/

#ifndef _FNET_ETH_H_

#define _FNET_ETH_H_

#include "fnet_config.h"


/**************************************************************************
*     Definitions
***************************************************************************/

/*! @addtogroup fnet_netif
*/
/*! @{ */

/**************************************************************************/ /*!
 * @def     FNET_MAC_ADDR_STR_SIZE
 * @brief   Size of the string buffer that will contain 
 *          null-terminated ASCII string of MAC address, represented
 *          by six groups of two hexadecimal digits, separated by colons (:).
 * @see fnet_mac_to_str
 * @showinitializer 
 ******************************************************************************/
#define FNET_MAC_ADDR_STR_SIZE       (18)

/**************************************************************************/ /*!
 * @brief Media Access Control (MAC) address  type.
 ******************************************************************************/
typedef unsigned char fnet_mac_addr_t[6]; /* MAC address type.*/


/******************************************************************************
*     Function Prototypes
*******************************************************************************/

/***************************************************************************/ /*!
 *
 * @brief    Converts a 6 byte MAC address into a null terminated string.
 *
 *
 * @param addr       MAC address.
 *
 * @param str_mac    Pointer to a character buffer will contain the resulting 
 *                   text address in standard ":" notation. @n 
 *                   The @c str_mac buffer must be at least 18 bytes long 
 *                   (@ref FNET_MAC_ADDR_STR_SIZE). 
 *
 * @see fnet_str_to_mac()
 ******************************************************************************
 * This function takes an MAC-48 address, specified by the @c addr 
 * parameter, and returns a null-terminated ASCII string, represented
 * by six groups of two hexadecimal digits, separated by colons (:), 
 * in transmission order (e.g. 01:23:45:67:89:ab ), into buffer pointed to by the
 * @c str_mac.
 ******************************************************************************/
void fnet_mac_to_str( fnet_mac_addr_t addr, char *str_mac );

/***************************************************************************/ /*!
 *
 * @brief    Converts a null terminated string to a 6 byte MAC address 
 *
 * @param str_mac    Null-terminated MAC address string as pairs of 
 *                   hexadecimal numbers separated by colons.
 *
 * @param addr       Buffer will contain a suitable 
 *                   binary representation of the @c str_mac MAC address .
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if the string in the @c str_mac parameter does not contain 
 *     a legitimate MAC address.
 *
 * @see fnet_mac_to_str()
 ******************************************************************************
 * This function interprets the character string specified by the @c str_mac 
 * parameter. This string represents a numeric MAC address expressed 
 * in six groups of two hexadecimal digits, separated by colons (:), 
 * in transmission order. The value returned, pointed to by the @c addr,
 * is a number suitable for use as an MAC address.
 ******************************************************************************/
int fnet_str_to_mac( char *str_mac, fnet_mac_addr_t addr );

/*! @} */

#endif

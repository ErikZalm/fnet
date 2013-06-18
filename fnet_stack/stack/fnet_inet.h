/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2011 by Andrey Butok. Freescale Semiconductor, Inc.
* Copyright 2003 by Andrey Butok. Motorola SPS.
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
* @file fnet_inet.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.2.0
*
* @brief Address-conversion functions API.
*
***************************************************************************/

#ifndef _FNET_INET_H_

#define _FNET_INET_H_

#include "fnet.h"

/*! @addtogroup fnet_socket 
*/
/*! @{ */

/***************************************************************************/ /*!
 *
 * @brief    Converts an IPv4 address into a string in Internet 
 *           standard dotted-decimal format.
 *
 *
 * @param addr       Structure that represents an Internet address.
 *
 * @param res_str    Pointer to a character buffer will contain the resulting 
 *                   text address in standard "." notation.@n 
 *                   The @c res_str buffer must be at least 16 bytes long 
 *                   (@ref FNET_IP4_ADDR_STR_SIZE). 
 *              
 *
 *
 * @return This function always returns the @c res_str.
 *
 * @see fnet_inet_aton(), fnet_inet_ntop(), fnet_inet_pton()
 *
 ******************************************************************************
 *
 * This function takes an Internet address structure, specified by the @c addr 
 * parameter, and returns a null-terminated ASCII string, representing the 
 * address in "." (dot) notation as in "a.b.c.d" into buffer pointed to by the
 * @c res_str.
 *
 * @note 
 * fnet_inet_ntop() extends the fnet_inet_ntoa() function to support multiple 
 * address families. @n
 * fnet_inet_ntoa() is now considered to be deprecated.
 *
 ******************************************************************************/
char *fnet_inet_ntoa( struct in_addr addr, char *res_str );

/***************************************************************************/ /*!
 *
 * @brief    Converts the string in the standard dotted-decimal notation 
 *           to an integer value, suitable for use as an IPv4 address.
 *
 *
 * @param cp         Null-terminated character string representing a number 
 *                   expressed in the Internet standard "." (dotted) notation.
 *
 * @param addr       Pointer to an integer will contain a suitable 
 *                   binary representation of the Internet address @c cp.
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if the string in the @c cp parameter does not contain 
 *     a legitimate Internet address.
 *
 * @see fnet_inet_aton(), fnet_inet_ntop(), fnet_inet_pton()
 *
 ******************************************************************************
 *
 * This function interprets the character string specified by the @c cp 
 * parameter. This string represents a numeric Internet address expressed 
 * in the Internet standard "." notation. The value returned, pointed to by the @c addr,
 * is a number suitable for use as an Internet address.@n
 * @note 
 * fnet_inet_pton() extends the fnet_inet_aton() function to support multiple 
 * address families. @n
 * fnet_inet_aton() is now considered to be deprecated. 
 *
 ******************************************************************************/
int fnet_inet_aton( char *cp, struct in_addr *addr );

/***************************************************************************/ /*!
 *
 * @brief   Converts IPv4 or IPv6 address from binary to text form.
 *
 *
 * @param family   The address family (@ref AF_INET or @ref AF_INET6).
 *
 * @param addr  Pointer to the IP address in network-byte order.
 *
 * @param str   Pointer to a buffer in which to store the NULL-terminated 
 *              string representation of the IP address..@n 
 *              For an IPv4 address, the @c str buffer must be at least 16 bytes long 
 *                   (@ref FNET_IP4_ADDR_STR_SIZE).@n
 *              For an IPv6 address, the @c str buffer must be at least 46 bytes long 
 *                   (@ref FNET_IP6_ADDR_STR_SIZE).@n 
 *
 * @param str_len  Length of the @c str buffer.
 *
 *
 * @return This function returns:
 *   - pointer to a buffer containing the string representation of IP 
 *     address (the @c str), if no error occurs, 
 *   - @ref FNET_NULL if an error occurs.
 *
 * @see fnet_inet_pton()
 *
 ******************************************************************************
 *
 * This function converts the network address structure, specified by the @c addr 
 * parameter, in the @c addr_family address family into a character string.
 * The resulting string is copied to the buffer pointed to by @c str.
 *
 * @note 
 * fnet_inet_ntop() extends the fnet_inet_ntoa() function to support multiple 
 * address families. @n
 * fnet_inet_ntoa() is now considered to be deprecated.
 *
 ******************************************************************************/
char *fnet_inet_ntop(fnet_address_family_t family, const void *addr, char *str, int str_len);

/***************************************************************************/ /*!
 *
 * @brief    Converts IPv4 and IPv6 addresses from text to binary form.
 *
 *
 * @param family     The address family (@ref AF_INET or @ref AF_INET6).
 *
 * @param str        Null-terminated character string that contains the text 
 *                   representation of the IP address to convert to numeric 
 *                   binary form.
 *
 * @param addr       Pointer to a buffer in which to store the numeric binary 
 *                   representation of the IP address @c str.
 *
 * @param addr_len   Length of the @c addr buffer. 
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if the string in the @c str parameter does not contain 
 *     a legitimate Internet address.
 *
 * @see fnet_inet_ntop()
 *
 ******************************************************************************
 *
 * This function converts the character string @c src into a network address
 * structure in the @c addr_family address family, then copies the network 
 * address structure to the @c addr buffer.
 *
 * @note 
 * fnet_inet_pton() extends the fnet_inet_aton() function to support multiple 
 * address families. @n
 * fnet_inet_aton() is now considered to be deprecated. 
 *
 ******************************************************************************/
int fnet_inet_pton (fnet_address_family_t family, const char *str, void *addr, int addr_len);


/************************************************************************
* NAME: inet_ptos
*
* DESCRIPTION:The function converts from presentation format (string)
*	        to struct sockaddr.
*************************************************************************/
int fnet_inet_ptos (char *str, struct sockaddr *addr);


/*! @} */

#endif /* _FNET_INET_H_ */

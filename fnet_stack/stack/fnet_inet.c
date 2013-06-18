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
* @file fnet_inet.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.6.0
*
* @brief Address-conversion functions.
*
***************************************************************************/

#include "fnet_inet.h"

/************************************************************************
*     Function Prototypes
*************************************************************************/
static char *fnet_inet_ntop_ip4(fnet_ip4_addr_t *addr, char *str, int size);
static int fnet_inet_pton_ip4(const char *str, fnet_ip4_addr_t *addr);

#if FNET_CFG_IP6
static char *fnet_inet_ntop_ip6(fnet_ip6_addr_t *addr, char *str, int size);
static int fnet_inet_pton_ip6(const char *str, fnet_ip6_addr_t *addr);
#endif

/************************************************************************
* NAME: fnet_inet_ntoa
*
* DESCRIPTION:The function converts an (IPv4) Internet network address 
*             into a string in Internet standard dotted format.
*************************************************************************/
char *fnet_inet_ntoa( struct in_addr addr, char *res_str )
{
    return  fnet_inet_ntop_ip4(&addr.s_addr, res_str, FNET_IP4_ADDR_STR_SIZE);
}

/************************************************************************
* NAME: fnet_inet_aton
*
* DESCRIPTION:The function converts a string containing an (IPv4) 
*             Internet Protocol dotted address into a suitable binary 
*             representation of the Internet address.
*************************************************************************/
int fnet_inet_aton( char *cp, struct in_addr *addr )
{
    return fnet_inet_pton_ip4(cp, &addr->s_addr);
}

/************************************************************************
* NAME: fnet_inet_ntop
*
* DESCRIPTION:The function converts network format IPv4 and IPv6 address 
*               to presentation/text format (string).
*************************************************************************/ 
char *fnet_inet_ntop(fnet_address_family_t family, const void *addr, char *str, int str_len)
{
	switch (family)
	{
    #if FNET_CFG_IP4         
    	case AF_INET:
    		return (fnet_inet_ntop_ip4((fnet_ip4_addr_t *)addr, str, str_len));
    #endif  
    #if FNET_CFG_IP6 
    	case AF_INET6:
    		return (fnet_inet_ntop_ip6((fnet_ip6_addr_t *)addr, str, str_len));
    #endif 
    	default:
    		return (FNET_NULL);
	}
}

/************************************************************************
* NAME: fnet_inet_pton
*
* DESCRIPTION:The function converts from presentation format (string)
*	        to network format.
*************************************************************************/
int fnet_inet_pton(fnet_address_family_t family, const char *str, void *addr, int addr_len)
{   
    switch (family)
    {
#if FNET_CFG_IP4 
        case AF_INET:
            if(addr_len < sizeof(fnet_ip4_addr_t))
            {
                return FNET_ERR;
            }
            return fnet_inet_pton_ip4(str, addr);
#endif /* FNET_CFG_IP4 */ 
#if FNET_CFG_IP6            
        case AF_INET6:
            if(addr_len < sizeof(fnet_ip6_addr_t))
            {
                return FNET_ERR;   	
            }
            return fnet_inet_pton_ip6(str, addr);
#endif /* FNET_CFG_IP6 */             
        default:
            return FNET_ERR;
    }
}

/************************************************************************
* NAME: fnet_inet_ptos
*
* DESCRIPTION:The function converts from presentation format (string)
*	        to struct sockaddr.
*************************************************************************/
int fnet_inet_ptos (char *str, struct sockaddr *addr)
{   
#if FNET_CFG_IP4    
    if(fnet_inet_pton(AF_INET, str, addr->sa_data, sizeof(addr->sa_data)) == FNET_OK)
        addr->sa_family = AF_INET;
    else 
#endif /* FNET_CFG_IP4 */
#if FNET_CFG_IP6 
    if(fnet_inet_pton(AF_INET6, str, addr->sa_data, sizeof(addr->sa_data)) == FNET_OK)
        addr->sa_family = AF_INET6;
    else    
#endif /* FNET_CFG_IP6 */ 
        return FNET_ERR;
        
    return FNET_OK;    
}

/************************************************************************
* NAME: fnet_inet_pton_ip4
*
* DESCRIPTION:The function converts from presentation format (string)
*	        to IPv4 address.
*************************************************************************/ 
static int fnet_inet_pton_ip4( const char *str, fnet_ip4_addr_t *addr)
{
	int                 is_digit = 0;
	int                 octets = 0;
	int                 ch;
	unsigned char       tmp[sizeof(*addr)];
	unsigned char       *tp;
	unsigned int        val;

	*(tp = tmp) = 0;
	
	while ((ch = *str++) != '\0')
	{
	    /* If "c" is digit. */
        if((ch >= '0') && (ch <= '9'))
		{
	        val = (unsigned int)(*tp * 10 + (ch - '0'));		    

			if (is_digit && *tp == 0)
                goto ERROR;
				
			if (val > 255)
                goto ERROR;
				
			*tp = (unsigned char)val;
			
			if (!is_digit)
			{
				if (++octets > 4)
                    goto ERROR;
					
				is_digit = 1;
			}
		} 
		else if (ch == '.' && is_digit)
		{
			if (octets == 4)
                goto ERROR;
				
			*++tp = 0;
			is_digit = 0;
		} 
		else
		    goto ERROR;
	}
	if (octets < 4)
		goto ERROR;
		
	fnet_memcpy(addr, tmp, sizeof(*addr));
	
	return (FNET_OK);
ERROR:	
    return (FNET_ERR);	
}

/************************************************************************
* NAME: fnet_inet_pton_ip6
*
* DESCRIPTION:The function converts from presentation format (string)
*	        to IPv6 address.
*************************************************************************/  
#if FNET_CFG_IP6 
static int fnet_inet_pton_ip6( const char *str, fnet_ip6_addr_t *addr )
{
    const char      xdigits_l[] = "0123456789abcdef";
    const char      xdigits_u[] = "0123456789ABCDEF";
    unsigned char   tmp[sizeof(*addr)]; 
    unsigned char   *tp;
    unsigned char   *endp;
    unsigned char   *colonp;
    const char      *xdigits;
    int             ch;
    int             seen_xdigits;
    unsigned int    val;

    fnet_memset_zero((tp = tmp), sizeof(*addr));
    endp = tp + sizeof(*addr);
    colonp = FNET_NULL;
	
    /* Leading :: */
    if (*str == ':')
        if (*++str != ':')
            goto ERROR;
            
    seen_xdigits = 0;
    val = 0;
    
    while (((ch = *str++) != '\0') && (ch != '%'))
    {
        const char *pch;

        if ((pch = fnet_strchr((xdigits = xdigits_l), ch)) == FNET_NULL)
            pch = fnet_strchr((xdigits = xdigits_u), ch);
			
        if (pch != FNET_NULL)
        {
            val <<= 4;
            val |= (pch - xdigits);
			if (++seen_xdigits > 4)
                goto ERROR;
                
            continue;
        }
        if (ch == ':')
        {
            if (!seen_xdigits)
            {
                if (colonp)
                    goto ERROR;
                    
                colonp = tp;
                continue;
            } 
            else if (*str == '\0')
            {
                goto ERROR;
			}
			
            if (tp + 2 > endp)
                goto ERROR;
                
            *tp++ = (unsigned char) ((val >> 8) & 0xff);
            *tp++ = (unsigned char) (val & 0xff);
            seen_xdigits = 0;
            val = 0;
            continue;
        }
		goto ERROR;
    }
    
    if (seen_xdigits)
    {
        if (tp + 2 > endp)
            goto ERROR;
            
        *tp++ = (unsigned char) ((val >> 8) & 0xff);
        *tp++ = (unsigned char) (val & 0xff);
    }
    
    if (colonp != FNET_NULL)
    {
        /*
         *  Shift.
         */
        const int n = tp - colonp;
        int i;

        if (tp == endp)
            goto ERROR;
        for (i = 1; i <= n; i++)
        {
            endp[- i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        goto ERROR;
		
    fnet_memcpy(addr, tmp, sizeof(*addr));
    return (FNET_OK);
ERROR:	
    return (FNET_ERR);    
}

#endif /* FNET_CFG_IP6 */

/************************************************************************
* NAME: fnet_inet_ntop_ip4
*
* DESCRIPTION:The function converts IPv4 address 
*               to presentation format (string).
*************************************************************************/  
static char *fnet_inet_ntop_ip4 ( fnet_ip4_addr_t *addr, char *str, int str_len)
{
	char                tmp[FNET_IP4_ADDR_STR_SIZE];
	int                 length;
	unsigned char       *ptr = (unsigned char *) addr;

    length=fnet_snprintf(tmp, sizeof(tmp), "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
    
	if ((length <= 0) || (length >= str_len))
	{
		return (FNET_NULL);
	}
	else
	{
	    fnet_strncpy(str, tmp, (unsigned long)str_len);

	    return (str);
	}
}

/************************************************************************
* NAME: fnet_inet_ntop_ip6
*
* DESCRIPTION:The function converts IPv6 binary address into 
*               presentation (printable) format.
*************************************************************************/  
#if FNET_CFG_IP6 
static char *fnet_inet_ntop_ip6 (fnet_ip6_addr_t *addr, char *str, int str_len)
{
    char    tmp[FNET_IP6_ADDR_STR_SIZE];
    char    *tp;
    struct { int base, len; } best, cur;
	
    unsigned long words[16 / 2];
    int i;

    /*
     *	Copy the input (bytewise) array into a wordwise array.
     *	Find the longest run of 0x00's in addr[] for :: shorthanding.
     */
    fnet_memset_zero(words, sizeof(words));
    for (i = 0; i < 16; i++)
        words[i / 2] |= (addr->addr[i] << ((1 - (i % 2)) << 3));
        
    best.base = -1;
    best.len = 0;
    cur.base = -1;
    cur.len = 0;
    for (i = 0; i < (16 / 2); i++)
    {
        if (words[i] == 0)
        {
            if (cur.base == -1)
                cur.base = i, cur.len = 1;
            else
                cur.len++;
        } 
        else
        {
            if (cur.base != -1)
            {
                if (best.base == -1 || cur.len > best.len)
                    best = cur;
                cur.base = -1;
            }
        }
    }
    if (cur.base != -1)
    {
        if (best.base == -1 || cur.len > best.len)
        best = cur;
    }
    if (best.base != -1 && best.len < 2)
        best.base = -1;

    /* Format the result. */
    tp = tmp;
    for (i = 0; i < (16 / 2); i++)
    {
        /* Are we inside the best run of 0x00's? */
        if (best.base != -1 && i >= best.base &&
            i < (best.base + best.len))
        {
            if (i == best.base)
                *tp++ = ':';
            continue;
        }
        /* Are we following an initial run of 0x00s or any real hex? */
        if (i != 0)
            *tp++ = ':';
		
        tp += fnet_sprintf(tp, "%x", words[i]);
    }
    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) ==  (16 / 2))
		*tp++ = ':';
    
    *tp++ = '\0';

	/* Check for overflow, copy, and we're done. */
    if ((int)(tp - tmp) > str_len)
    {
        return (FNET_NULL);
    }
    fnet_strncpy(str, tmp, (unsigned long)str_len);
    return (str);
}
#endif /* FNET_CFG_IP6 */



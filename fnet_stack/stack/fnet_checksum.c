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
* @file fnet_checksum.c
*
* @date Dec-19-2012
*
* @version 0.1.25.0
*
* @brief Internet checksum implementation.
*
***************************************************************************/

#include "fnet.h"
#include "fnet_config.h"
#include "fnet_checksum.h"
#include "fnet_ip.h"

static unsigned long fnet_checksum_nb(fnet_netbuf_t * nb, int len);

/*RFC:
    The checksum field is the 16 bit one's complement of the one's
    complement sum of all 16 bit words in the header and text.  If a
    segment contains an odd number of header and text octets to be
    checksummed, the last octet is padded on the right with zeros to
    form a 16 bit word for checksum purposes.  The pad is not
    transmitted as part of the segment.  While computing the checksum,
    the checksum field itself is replaced with zeros.
*/


/************************************************************************
* NAME: fnet_checksum_low
*
* DESCRIPTION: Calculates Internet checksum of nb chain.
*
*************************************************************************/
#if !FNET_CFG_OVERLOAD_CHECKSUM_LOW

static unsigned long fnet_checksum_low(unsigned long sum, int current_length, unsigned short *d_ptr)
{
        unsigned short p_byte1;
        
        while((current_length -= 32) >= 0)
        {
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
        }
        current_length += 32;

        while((current_length -= 8) >= 0)
        {
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
            sum += *d_ptr++;
        }
        current_length += 8;

        while((current_length -= 2) >= 0)
            sum += *d_ptr++;  
            
        if(current_length += 2)
        {
            p_byte1 = (unsigned short)((*((unsigned short *)d_ptr)) & FNET_NTOHS(0xFF00));
           
            sum += (unsigned short)p_byte1;
        }
        return sum;
} 
#else

extern unsigned long fnet_checksum_low(unsigned long sum, int current_length, unsigned short *d_ptr);

#endif

static unsigned long fnet_checksum_nb(fnet_netbuf_t * nb, int len)
{
    fnet_netbuf_t   *tmp_nb;
    unsigned short  *d_ptr;
    unsigned short  p_byte2;
    unsigned long   sum = 0;
    int             current_length;

    tmp_nb = nb;

    d_ptr = tmp_nb->data_ptr;        /* Store the data pointer of the 1st net_buf.*/

    if(nb->length > len)
        current_length = len;        /* If no more net_bufs to proceed.*/
    else
        current_length = (int)nb->length; /* Or full first net_buf.*/

    while(len)
    {
        len -= current_length;       /* current_length bytes should be proceeded.*/

        sum = fnet_checksum_low(sum, current_length, d_ptr); 
        
        
        tmp_nb = tmp_nb->next;
        d_ptr = tmp_nb->data_ptr;
        
        if(current_length & 1)
        {
            if(len)
            {
                /* If previous fragment was odd, add in first byte in lower 8 bits. */
                p_byte2 = fnet_ntohs((unsigned short)((*((unsigned char *)d_ptr)) & 0x00FF));
                d_ptr = (unsigned short *)((unsigned char *)d_ptr + 1);
                
                sum += (unsigned short)p_byte2;
                len--;
                current_length = -1;
            }
            else
                current_length = 0;
        }
        else
        {
            current_length = 0;
        }
          

        if(tmp_nb->length > len)
            current_length = len;
        else
            current_length += tmp_nb->length;
    }

    return sum;
}

/************************************************************************
* NAME: fnet_checksum
*
* DESCRIPTION: Calculates one's complement (Internet) checksum.
*
*************************************************************************/
unsigned short fnet_checksum(fnet_netbuf_t * nb, int len)
{
    unsigned long sum = fnet_checksum_nb(nb, len);

    /* Add potential carries - no branches. */

    sum = (sum >> 16) + (sum & 0xffff); /* Add in accumulated carries */
    sum += sum >> 16;                   /* Add potential last carry   */

    return (unsigned short)(0xffff & ~sum);
}

/************************************************************************
* NAME: fnet_checksum_buf
*
* DESCRIPTION: Calculates one's complement (Internet) checksum 
*              for a buffer.
*
*************************************************************************/
unsigned short fnet_checksum_buf(char *buf, int buf_len)
{
    unsigned long sum = fnet_checksum_low(0, buf_len, (unsigned short *)buf); 

    sum = (sum >> 16) + (sum & 0xffff); /* Add in accumulated carries */
    sum += sum >> 16;                   /* Add potential last carry   */

    return (unsigned short)(0xffff & ~sum);
}

/************************************************************************
* NAME: fnet_checksum_pseudo_buf
*
* DESCRIPTION: Calculates one's complement (Internet) checksum of 
*              the IP pseudo header, for a buffer.
*
*************************************************************************/
unsigned short fnet_checksum_pseudo_buf(char *buf, unsigned short buf_len, unsigned short protocol, char *ip_src, char *ip_dest, int addr_size )
{
    unsigned long sum = fnet_checksum_low(0, buf_len, (unsigned short *)buf); 
    sum += protocol + fnet_htons(buf_len);
    sum = fnet_checksum_low(sum, addr_size, (unsigned short *)ip_src); 
    sum = fnet_checksum_low(sum, addr_size, (unsigned short *)ip_dest);

    sum = (sum >> 16) + (sum & 0xffff); /* Add in accumulated carries */
    sum += sum >> 16;                   /* Add potential last carry   */

    return (unsigned short)(0xffff & ~sum);
}

/************************************************************************
* NAME: fnet_checksum_pseudo_start
*
* DESCRIPTION: Calculates  one's complement (Internet) checksum of 
*              the IP pseudo header
*
*************************************************************************/
unsigned short fnet_checksum_pseudo_start( fnet_netbuf_t *nb,
                                           unsigned short protocol, unsigned short protocol_len )
{
    unsigned long sum = fnet_checksum_nb(nb, protocol_len);
    sum += protocol + fnet_htons(protocol_len);

    sum = (sum >> 16) + (sum & 0xffff); /* Add in accumulated carries */
    sum += sum >> 16;                   /* Add potential last carry   */

    return (unsigned short)(sum);
}

/************************************************************************
* NAME: fnet_checksum_pseudo_end
*
* DESCRIPTION: Calculates  one's complement (Internet) checksum of 
*              the IP pseudo header
*
*************************************************************************/
unsigned short fnet_checksum_pseudo_end( unsigned short sum_s, char *ip_src, char *ip_dest, int addr_size )
{
    unsigned long sum = 0;
    
    sum = sum_s;

    sum = fnet_checksum_low(sum, addr_size, (unsigned short *)ip_src); 
    sum = fnet_checksum_low(sum, addr_size, (unsigned short *)ip_dest);

    sum = (sum >> 16) + (sum & 0xffff); /* Add in accumulated carries */
    sum += sum >> 16;                   /* Add potential last carry   */

    return (unsigned short)(0xffff & ~sum);
}



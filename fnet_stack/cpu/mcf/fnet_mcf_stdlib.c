/**************************************************************************
* 
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
* @file fnet_mcf_stdlib.c
*
* @author Andrey Butok
*
* @date Aug-2-2012
*
* @version 0.1.14.0
*
* @brief ColdFire optimized/specific standard functions.
*
***************************************************************************/
#include "fnet_mcf.h"

#if FNET_MCF

#include "fnet_stdlib.h"

/****************************************************************/
#if FNET_CFG_OVERLOAD_MEMCPY
void *fnet_memcpy (void *dest, const void *src, unsigned n)
{
	int longs, bytes;
	fnet_uint32 *dpl = (fnet_uint32 *)dest;
	fnet_uint32 *spl = (fnet_uint32 *)src;
	fnet_uint8  *dpb, *spb;

    bytes = (n & 0x3);
    longs = ((n - bytes) >> 2);

    for (longs++; --longs;)
        *dpl++ = *spl++;
		
    dpb = (fnet_uint8 *)dpl;
    spb = (fnet_uint8 *)spl;

    for (bytes++; --bytes;)
        *dpb++ = *spb++;

	return dest;
}
#endif

#endif /*FNET_MCF*/
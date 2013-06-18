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
* @file fnet_netbuf.c
*
* @author Andrey Butok
*
* @date Mar-4-2013
*
* @version 0.1.29.0
*
* @brief FNET Buffer and memory management implementation.
*
***************************************************************************/

#include "fnet_config.h"
#include "fnet_netbuf.h"
#include "fnet_prot.h"
#include "fnet_isr.h"
#include "fnet_stdlib.h"
#include "fnet_debug.h"
#include "fnet_mempool.h"


#define FNET_HEAP_SPLIT     (0) /* If 1 the main heap will be splitted to two parts. 
                                 * (have issue so to be tested later).*/

static fnet_mempool_desc_t fnet_mempool_main = 0; /* Main memory pool. */

#if FNET_HEAP_SPLIT
    #define FNET_NETBUF_MEMPOOL_SIZE(size)      (((size)*4)/5)
    static fnet_mempool_desc_t fnet_mempool_netbuf = 0; /* Netbuf memory pool. */
#else
    #define fnet_mempool_netbuf fnet_mempool_main
#endif

fnet_netbuf_t *dm_nb;

/************************************************************************
* NAME: fnet_netbuf_new
*
* DESCRIPTION: Creates a new net_buf and allocates memory
*              for a new data buffer. 
*************************************************************************/
fnet_netbuf_t *fnet_netbuf_new( int len, int drain )
{
    fnet_netbuf_t   *nb;
    void            *nb_d;

    if(len < 0)
        return (fnet_netbuf_t *)0;


    nb = (fnet_netbuf_t *)fnet_malloc_netbuf(sizeof(fnet_netbuf_t));

    if((nb == 0) && drain)
    {
        fnet_prot_drain();
        nb = (fnet_netbuf_t *)fnet_malloc_netbuf(sizeof(fnet_netbuf_t));
    }

    if(nb == 0) /* If FNET_NETBUF_MALLOC_NOWAIT and no free memory for a new net_buf*/
    {
        return (fnet_netbuf_t *)0;
    }


    nb_d = fnet_malloc_netbuf((unsigned int)len + sizeof(int)/* For reference_counter */);

    if((nb_d == 0) && drain )
    {
        fnet_prot_drain();
        nb_d = fnet_malloc_netbuf((unsigned int)len + sizeof(int)/* For reference_counter */);
    }


    if(nb_d == 0) /* If FNET_NETBUF_MALLOC_NOWAIT and no free memory for data.*/
    {
        fnet_free_netbuf(nb);
        return (fnet_netbuf_t *)0;
    }

    nb->next = (fnet_netbuf_t *)0;
    nb->next_chain = (fnet_netbuf_t *)0;

    /* In memory net_buf's data is after the descriptor "data".*/
    
    ((int *)nb_d)[0] = 1; /* First element is used by the reference_counter.*/
    nb->data = &((int *)nb_d)[0];
    nb->data_ptr = &((int *)nb_d)[1];
    nb->length = (unsigned long)len;
    nb->total_length = (unsigned long)len;
    nb->flags = 0;

    return (nb);
}

/************************************************************************
* NAME: fnet_netbuf_copy
*
* DESCRIPTION: Creates a new net_buf using data buffer, 
*              which was created before for another net_buf. 
*************************************************************************/
fnet_netbuf_t *fnet_netbuf_copy( fnet_netbuf_t *nb, int offset, int len, int drain )
{
    fnet_netbuf_t *loc_nb, *loc_nb_head, *tmp_nb;
    long tot_len = 0, tot_offset = 0;

    tmp_nb = nb;

    do /* Calculate the total length of the buf for current net_buf chain.*/
    {
        tot_len += tmp_nb->length;
        tmp_nb = tmp_nb->next;
    } while (tmp_nb);

    if(len == FNET_NETBUF_COPYALL)
    {
        tot_len -= offset;
        len = tot_len;
    }
    else
    {
        if((len > tot_len - offset) || (tot_len - offset < 0))
        {
            return (fnet_netbuf_t *)0;
        }    
    }
    /* In tot_len finally - the size of required net_buf data*/

    if(offset < 0)
    {
        return (fnet_netbuf_t *)0;
    }

    tmp_nb = nb;


    loc_nb = (fnet_netbuf_t *)fnet_malloc_netbuf(sizeof(fnet_netbuf_t));

    if((loc_nb == 0) && drain)
    {
        fnet_prot_drain();
        loc_nb = (fnet_netbuf_t *)fnet_malloc_netbuf(sizeof(fnet_netbuf_t));
    }

    if(loc_nb == 0)
    {
        return (fnet_netbuf_t *)0;
    }

    loc_nb_head = loc_nb; /* Save the head of net_buf chain.*/
    loc_nb->next_chain = (fnet_netbuf_t *)0;
    loc_nb->total_length = (unsigned long)len;
    loc_nb->flags = nb->flags;

    if(tmp_nb->length > offset) /* If offset less than size of 1st net_buf.*/
    {
        tot_offset = offset;
    }
    else /* Find corresponding net_buf and calculate the offset in it.*/
    {
        while((tot_offset += tmp_nb->length) <= offset)
            tmp_nb = tmp_nb->next;

        tot_offset = (long)(tmp_nb->length + offset - tot_offset);
    }

    loc_nb->data = tmp_nb->data;

    loc_nb->data_ptr = (unsigned char *)tmp_nb->data_ptr + tot_offset;
    
    ((int *)loc_nb->data)[0] = ((int *)loc_nb->data)[0] + 1;    /* Increment the the reference_counter.*/
    
    tot_len = (long)(len - (tmp_nb->length - tot_offset));

    if(tot_len <= 0) /* If only one net_buf required.*/
        loc_nb->length = (unsigned long)len;
    else
    {
        loc_nb->length = tmp_nb->length - tot_offset;

        do
        {
            loc_nb->next = (fnet_netbuf_t *)fnet_malloc_netbuf(sizeof(fnet_netbuf_t));

            if((loc_nb->next == 0) && drain )
            {
                fnet_prot_drain();
                loc_nb->next = (fnet_netbuf_t *)fnet_malloc_netbuf(sizeof(fnet_netbuf_t));
            }

            if(loc_nb->next == 0) /* there is a need to erase all buffers,*/
            {                     /* which were created earlier.*/
                loc_nb_head = fnet_netbuf_free(loc_nb_head);

                while(loc_nb_head != loc_nb->next)
                {
                    tmp_nb = loc_nb_head->next;
                    fnet_free_netbuf(loc_nb_head);
                    loc_nb_head = tmp_nb;
                }
                
                return (fnet_netbuf_t *)0;
            }

            loc_nb = loc_nb->next;
            loc_nb->next_chain = (fnet_netbuf_t *)0;

            tmp_nb = tmp_nb->next;

            loc_nb->data = tmp_nb->data;
            loc_nb->flags = tmp_nb->flags; 

            ((int *)loc_nb->data)[0] = ((int *)loc_nb->data)[0] + 1; /* Increment the the reference_counter.*/

            loc_nb->data_ptr = tmp_nb->data_ptr;

            tot_len -= tmp_nb->length;

            if(tot_len < 0) /* for correct calculation of length */
                loc_nb->length = tot_len + tmp_nb->length;
            else
                loc_nb->length = tmp_nb->length;
        } 
        while (tot_len > 0);
    }

    loc_nb->next = (fnet_netbuf_t *)0;
    
    return (loc_nb_head);
}

/************************************************************************
* NAME: fnet_netbuf_from_buf
*
* DESCRIPTION: Creates a new net_buf and fills it by a content of 
*              the external data buffer. 
*************************************************************************/
fnet_netbuf_t *fnet_netbuf_from_buf( void *data_ptr, int len, int drain )
{
    fnet_netbuf_t *nb;
    
    nb = fnet_netbuf_new(len, drain);

    if(nb)
        fnet_memcpy(nb->data_ptr, data_ptr, (unsigned int)len);

    return (nb);
}

/************************************************************************
* NAME: fnet_netbuf_to_buf
*
* DESCRIPTION: Creates a new continuous buffer, which contains 
*              info from all net buffers of the current chain. 
*************************************************************************/

void fnet_netbuf_to_buf( fnet_netbuf_t *nb, int offset, int len, void *data_ptr )
{
    unsigned char *u_buf;
    fnet_netbuf_t *tmp_nb;
    long tot_len = 0, tot_offset = 0, cur_len;

    u_buf = data_ptr;

    tmp_nb = nb;

    /* This part is similar to the corresponding part in fnet_netbuf_copy */
    do
    {
        tot_len += tmp_nb->length;
        tmp_nb = tmp_nb->next;
    } while (tmp_nb);

    if(len == FNET_NETBUF_COPYALL) /* If buffer should contain info from all net buffers */
    {
        tot_len -= offset;
        len = tot_len;
    }
    else
    {
        if((len > tot_len - offset) || (tot_len - offset < 0))
        {
       
            return; /* if input params aren't correct. */
        }
    }

    if((offset < 0) || (len == 0))
    {
    
        return; /* if input params aren't correct. */
    }

    tmp_nb = nb;

    if(tmp_nb->length > offset)
    {
        tot_offset = offset;
    }
    else
    {
        while((tot_offset += tmp_nb->length) <= offset)
          tmp_nb = tmp_nb->next;

        tot_offset = (long)(tmp_nb->length + offset - tot_offset);
    }

    tot_len = len;

    do
    {
        /* Calculate the quantity of bytes we copy from the current net_buf*/
        cur_len = (long)((tmp_nb->length - tot_offset > tot_len) ? tot_len : tmp_nb->length - tot_offset);
        /* and substract from the total quantity of bytes we copy */
        tot_len -= cur_len;

        fnet_memcpy(u_buf, (unsigned char *)tmp_nb->data_ptr + tot_offset, (unsigned int)cur_len);
        tot_offset = 0;          /* offset is only for the first peace of data*/

        u_buf = u_buf + cur_len; /* move the pointer for the next copy */
        tmp_nb = tmp_nb->next;   /* go to the next net_buf in the chain */     
    } while (tot_len > 0);
    
}

/************************************************************************
* NAME: fnet_netbuf_free
*
* DESCRIPTION: Frees the memory, which was allocated by net_buf 'nb' and  
*              returns pointer to the next net_buf. 
*************************************************************************/
fnet_netbuf_t *fnet_netbuf_free( fnet_netbuf_t *nb )
{
    fnet_netbuf_t *tmp_nb;
   
    if(nb != 0)
    {
        
        if(((int *)nb->data)[0] == 1)   /* If nobody uses this data buffer. */
            fnet_free_netbuf(nb->data);
        else                           /* else decrement reference counter */
            ((int *)nb->data)[0] = ((int *)nb->data)[0] - 1;

        tmp_nb = nb->next;

        fnet_free_netbuf(nb);

        return (tmp_nb);
    }
    else
    {
        return (0);
    }
}

/************************************************************************
* NAME: fnet_netbuf_free_chain
*
* DESCRIPTION: Frees the memory, which was allocated by all net_bufs
*              in the chain beginning from the buffer 'nb'
*************************************************************************/
void fnet_netbuf_free_chain( fnet_netbuf_t *nb )
{
    fnet_netbuf_t *tmp_nb;

    while(nb != 0)
    {
        tmp_nb = nb->next;
        
        if(((int *)nb->data)[0] == 1)   /* If nobody uses this data buffer. */
            fnet_free_netbuf(nb->data);
        else                            /* Else decrement reference counter */
            ((int *)nb->data)[0] = ((int *)nb->data)[0] - 1;


        fnet_free_netbuf(nb);

        nb = tmp_nb;
    }
}

/************************************************************************
* NAME: fnet_netbuf_pullup
*
* DESCRIPTION: Create a data buffer for the first net_buf with the 
*              length len 
*************************************************************************/
fnet_netbuf_t *fnet_netbuf_pullup( fnet_netbuf_t *nb, int len)
{
    unsigned long   tot_len = 0;
    unsigned long   offset;
    fnet_netbuf_t   *tmp_nb;
    fnet_netbuf_t   *nb_run;
    void            *new_buf;
    
    /* Check length*/
    if(nb->total_length < len)
        return FNET_NULL;
    
    if((nb->length >= len) || (len <= 0) || (nb == 0))
        /* if function shouldn't do anything*/
        return (nb);

    tmp_nb = nb;

    tot_len += tmp_nb->length;

    /* search of the last buffer, from which the data have to be copied*/
    while(tot_len < len && tmp_nb)
    {
        tmp_nb = tmp_nb->next;
        tot_len += tmp_nb->length;
    }

    new_buf = (struct net_buf_data *)fnet_malloc_netbuf((unsigned int)len + sizeof(int)/* For reference_counter */);

    if(new_buf == 0)
    {
        return (FNET_NULL);
    }
       
    ((int *)new_buf)[0] = 1; /* First element is used by the reference_counter.*/

    /* Copy into it the contents of first data buffer. Skip the reference counter (placed in the first bytes). */
    fnet_memcpy(&((int *)new_buf)[1], nb->data_ptr, nb->length);
    offset = nb->length;

    /* Free old data buffer (for the first net_buf) */
    if(((int *)nb->data)[0] == 1)   /* If nobody uses this data buffer. */
        fnet_free_netbuf(nb->data);
    else                            /* Else decrement reference counter */
        ((int *)nb->data)[0] = ((int *)nb->data)[0] - 1;

    /* Currently data buffer contains the contents of the first buffer */
    nb->data = &((int *)new_buf)[0];
    nb->data_ptr = &((int *)new_buf)[1];

    nb_run = nb->next;      /* Let's start from the next buffer */

    while(nb_run != tmp_nb) /* Copy full data buffers */
    {
        fnet_memcpy((unsigned char *)nb->data_ptr + offset, nb_run->data_ptr, nb_run->length);

        if(nb_run != tmp_nb)
        {
            offset += nb_run->length;
            nb_run = fnet_netbuf_free(nb_run);
        }
    }

    tot_len = len - offset;

    /* Copy the remaining part and change data pointer and length of the 
     * last net_buf, which is the source for the first net_buf */

    fnet_memcpy((unsigned char *)nb->data_ptr + offset, nb_run->data_ptr, tot_len);

    nb_run->length -= tot_len;

    if(nb_run->length == 0)
    {
        nb_run = fnet_netbuf_free(nb_run);
    }
    else
        nb_run->data_ptr = (unsigned char *)nb_run->data_ptr + tot_len;

    /* Setting up the params of the first net_buf;*/
    nb->next = nb_run;

    nb->length = (unsigned long)len;

    return (nb);
}


/************************************************************************
* NAME: fnet_netbuf_trim
*
* DESCRIPTION: Trims len bytes from the begin of the net_buf data if len 
*              is positive. Otherwise len bytes should be trimmed from the
*              end of net_buf buffer. If len=0 - do nothing
*************************************************************************/
void fnet_netbuf_trim( fnet_netbuf_t **nb_ptr, int len )
{
    fnet_netbuf_t   *head_nb;
    fnet_netbuf_t   *nb;
    long            tot_len;
    long            total_rem;
    fnet_netbuf_t   *tmp_nb;
    

    if(len == 0)
        return;
    
    tmp_nb = (fnet_netbuf_t *) *nb_ptr;
    nb = (fnet_netbuf_t *) *nb_ptr;
    head_nb = nb;

    /* If the quantity of trimmed bytes is greater than net_buf size - do nothing.*/
    if((nb->total_length < (len > 0 ? len : -len)) || nb == 0)
        return;

    tot_len = (long)nb->length;
    total_rem = (long)nb->total_length;

    if(len > 0) /* Trim len bytes from the begin of the buffer.*/
    {
        while(nb != 0 && len >= tot_len)
        {
            *nb_ptr = nb->next;
            
            nb = fnet_netbuf_free(nb); /* In some cases we delete some net_bufs.*/
            tot_len += nb->length;
        }

        if(nb != 0)
        {
            nb->data_ptr = (unsigned char *)nb->data_ptr + /* Or change pointer. */
                            nb->length - (tot_len - len);
            nb->length = (unsigned long)(tot_len - len);
            nb->flags = tmp_nb->flags; 
        }
    }
    else /* Trim len bytes from the end of the buffer. */
    {
        while(nb != 0 && (total_rem + len > tot_len))
        {
            nb = nb->next;         /* Run up th the first net_buf, which points */
            tot_len += nb->length; /* to the data, which should be erased.*/
        }

        /* Cut the part of the first net_buf, which should be modified.*/
        if(nb != 0)
        {
            nb->length += (len + total_rem - tot_len);

            while(nb->next != 0) /* Cut the redundant net_bufs. */
              nb->next = fnet_netbuf_free(nb->next);

            if(nb->length == 0)  /* if |len| == total_length */
                head_nb = fnet_netbuf_free(nb);

            nb = head_nb;
        }
    }

    if(nb != 0)
    {
        nb->total_length = (unsigned long)(total_rem - (len > 0 ? len : -len));
    }

    *nb_ptr = nb;
    
}

/************************************************************************
* NAME: fnet_netbuf_cut_center
*
* DESCRIPTION: Cuts len bytes in net_buf queue starting from offset "offset"
*
*************************************************************************/
fnet_netbuf_t *fnet_netbuf_cut_center( fnet_netbuf_t ** nb_ptr, int offset, int len )
{
    fnet_netbuf_t *head_nb, *tmp_nb, *nb;
    long tot_len;

    if(len == 0)
        return (0);

    nb = (fnet_netbuf_t *) *nb_ptr;

    if((nb->total_length < (len + offset)) || (nb == 0))
        return (0);

    if(offset == 0) /* The first case - when we cut from the begin of buffer.*/
    {
        fnet_netbuf_trim(&nb, len);
        *nb_ptr = nb;
        return (nb);
    }

    head_nb = nb;

    tmp_nb = nb;

    tot_len = (long)nb->length;

    while(nb != 0 && (offset >= tot_len))
    {
        nb = nb->next;                             /* Run up th the first net_buf, which points */
        tot_len += nb->length;                     /* to the data, which should be erased.*/ 

        if((nb != 0) && (offset >= tot_len))
            tmp_nb = nb;                          /* To store previous pointer. */
    }

    if(tot_len - nb->length == offset)            /* If we start cut from the begin of buffer. */
    {
        nb->total_length = head_nb->total_length; /* For correct fnet_netbuf_trim execution.*/
        fnet_netbuf_trim(&tmp_nb->next, len);
        head_nb->total_length -= len;
        
        return ((fnet_netbuf_t *) *nb_ptr);
    }

    /* If only the middle of one net_buf should be cut.*/
    if(tot_len - offset > len)
    {
        head_nb->total_length -= len;

        /* Split one net_uf into two.*/
        if(((int *)nb->data)[0] == 1) /* If we can simply erase them (reference_counter == 1).*/
        {
            fnet_memcpy((unsigned char *)nb->data_ptr + nb->length - tot_len + offset,
                        (unsigned char *)nb->data_ptr + nb->length - tot_len + offset + len,
                        (unsigned long)(tot_len - offset - len));
            nb->length -= len;
        }
        else
        {
            head_nb = fnet_netbuf_new(tot_len - offset - len, FNET_FALSE);

            if(head_nb == 0) /* If no free memory.*/
            {
                nb = (fnet_netbuf_t *) *nb_ptr;
                nb->total_length += len;
                return (0);
            }

            fnet_memcpy(head_nb->data_ptr,
                        (unsigned char *)nb->data_ptr + nb->length - tot_len + offset + len,
                        (unsigned long)(tot_len - offset - len));

            head_nb->next = nb->next;

            nb->next = head_nb;

            nb->length -= tot_len - offset;
        }

        return ((fnet_netbuf_t *) *nb_ptr);
    }

    if(tot_len - offset == len) /* If we cut from the middle of buffer to the end only.*/
    {
        nb->length -= len;

        head_nb->total_length -= len;

        return ((fnet_netbuf_t *) *nb_ptr);
    }
    else                                /* Cut from the middle of net_buf to the end and trim remaining info*/
    {                                   /* (tot_len-offset < len)*/
        nb->length -= tot_len - offset;

        nb->next->total_length = head_nb->total_length; /* For correct fnet_netbuf_trim execution. */
        fnet_netbuf_trim(&nb->next, len - (tot_len - offset));

        head_nb->total_length -= len;

        return ((fnet_netbuf_t *) *nb_ptr);
    }
}

/************************************************************************
* NAME: fnet_netbuf_concat
*
* DESCRIPTION: Makes one net_buf from two. If the first or second pointer
*              is 0, the pointer to the second or first buffer
*              correspondingly is returned
*************************************************************************/
fnet_netbuf_t *fnet_netbuf_concat( fnet_netbuf_t *nb1, fnet_netbuf_t *nb2 )
{
    fnet_netbuf_t *head_nb;

    if(nb1 == 0)
        return nb2;

    if(nb2 == 0)
        return nb1;

    head_nb = nb1;

    while(nb1->next != 0)
      nb1 = nb1->next;

    nb1->next = nb2;
    
    head_nb->flags |= nb2->flags;
    head_nb->total_length += nb2->total_length;

    return head_nb;
}

/************************************************************************
* NAME: fnet_netbuf_add_chain
*
* DESCRIPTION: Adds chain nb_chain into the queue of chains pointed by 
*              nb_ptr
*              
*************************************************************************/
void fnet_netbuf_add_chain( fnet_netbuf_t ** nb_ptr, fnet_netbuf_t *nb_chain )
{
    fnet_netbuf_t *nb;

    nb = (fnet_netbuf_t *) *nb_ptr;

    if(nb == 0)
    {
        *nb_ptr = nb_chain;
    }
    else
    {
        while(nb->next_chain)
          nb = nb->next_chain;

        nb->next_chain = nb_chain;
    }

}

/************************************************************************
* NAME: fnet_netbuf_del_chain
*
* DESCRIPTION: Deletes chain nb_chain, which is in the queue pointed by
*              nb_ptr
*              
*************************************************************************/
void fnet_netbuf_del_chain( fnet_netbuf_t ** nb_ptr, fnet_netbuf_t *nb_chain )
{
    fnet_netbuf_t *nb_current, *nb;
    
    if(!((nb_ptr == 0) || ((fnet_netbuf_t *) *nb_ptr == 0)))
    {
        nb_current = (fnet_netbuf_t *) *nb_ptr;

        if(nb_current == nb_chain)
        {
            nb = nb_current->next_chain;
            fnet_netbuf_free_chain(nb_current);
            *nb_ptr = nb;
            return;
        }

        while((nb_current->next_chain != nb_chain) && (nb_current != 0))
        {
            nb_current = nb_current->next_chain;
        }

        nb = nb_current->next_chain->next_chain;

        fnet_netbuf_free_chain(nb_current->next_chain);

        nb_current->next_chain = nb;
    }
}



/************************************************************************
* NAME: fnet_heap_init
*
* DESCRIPTION: Heap init
*              
*************************************************************************/
int fnet_heap_init( unsigned char *heap_ptr, unsigned long heap_size )
{
    int result;


/* Init memory pools. */
#if FNET_HEAP_SPLIT
    if(((fnet_mempool_main = fnet_mempool_init( heap_ptr, heap_size, FNET_MEMPOOL_ALIGN_8 )) != 0) &&
       ((fnet_mempool_netbuf = fnet_mempool_init( (void*)((unsigned long)fnet_malloc( FNET_NETBUF_MEMPOOL_SIZE(heap_size))), 
                                                    FNET_NETBUF_MEMPOOL_SIZE(heap_size), FNET_MEMPOOL_ALIGN_8 )) != 0) )

#else
     if((fnet_mempool_main = fnet_mempool_init( heap_ptr, heap_size, FNET_MEMPOOL_ALIGN_8 )) != 0)
#endif
        result = FNET_OK;
     else
        result = FNET_ERR;  
                                                             
   
    return result;
}


/************************************************************************
* NAME: fnet_free
*
* DESCRIPTION: Frees memory in heap for TCP/IP
*              
*************************************************************************/
void fnet_free_netbuf( void *ap )
{
    fnet_mempool_free(fnet_mempool_netbuf, ap);
}

/************************************************************************
* NAME: fnet_malloc
*
* DESCRIPTION: Allocates memory in heap for TCP/IP
*              
*************************************************************************/
void *fnet_malloc_netbuf( unsigned nbytes )
{
    return fnet_mempool_malloc( fnet_mempool_netbuf, nbytes );
}

/************************************************************************
* NAME: fnet_free_mem_status
*
* DESCRIPTION: Returns a quantity of free memory (for debug needs)
*              
*************************************************************************/
unsigned long fnet_free_mem_status_netbuf( void )
{
    return fnet_mempool_free_mem_status( fnet_mempool_netbuf );
}

/************************************************************************
* NAME: fnet_malloc_max
*
* DESCRIPTION: Returns a maximum size of posible allocated memory chunk.
*              
*************************************************************************/
unsigned long fnet_malloc_max_netbuf( void )
{
    return fnet_mempool_malloc_max( fnet_mempool_netbuf  );
}

/************************************************************************
* NAME: fnet_mem_release
*
* DESCRIPTION: Free all Net Memory
*              
*************************************************************************/
void fnet_mem_release_netbuf( void )
{
    fnet_mempool_release(fnet_mempool_netbuf);
}


/************************************************************************
* NAME: fnet_free
*
* DESCRIPTION: Frees memory in heap for TCP/IP
*              
*************************************************************************/
void fnet_free( void *ap )
{
    fnet_mempool_free(fnet_mempool_main, ap);
}

/************************************************************************
* NAME: fnet_malloc
*
* DESCRIPTION: Allocates memory in heap for TCP/IP
*              
*************************************************************************/
void *fnet_malloc( unsigned nbytes )
{
    return fnet_mempool_malloc( fnet_mempool_main, nbytes );
}

/************************************************************************
* NAME: fnet_free_mem_status
*
* DESCRIPTION: Returns a quantity of free memory (for debug needs)
*              
*************************************************************************/
unsigned long fnet_free_mem_status( void )
{
    return fnet_mempool_free_mem_status( fnet_mempool_main );
}

/************************************************************************
* NAME: fnet_malloc_max
*
* DESCRIPTION: Returns a maximum size of posible allocated memory chunk.
*              
*************************************************************************/
unsigned long fnet_malloc_max( void )
{
    return fnet_mempool_malloc_max( fnet_mempool_main  );
}

/************************************************************************
* NAME: fnet_mem_release
*
* DESCRIPTION: Free all Net Memory
*              
*************************************************************************/
void fnet_mem_release( void )
{
    fnet_mempool_release(fnet_mempool_main);
}

#if 0 /* For Debug needs.*/
int fnet_netbuf_mempool_check( void ) 
{
    return fnet_mempool_check(fnet_mempool_netbuf);
}

void FNET_DEBUG_NETBUF_print_chain( fnet_netbuf_t *nb, char *str, int max)
{
    int i = 0;
    fnet_println("== %s nb = 0x%08X ==", str, (unsigned long)nb);
    
    while(nb->next && i<max)
    {
          nb = nb->next;
          i++;
          fnet_println("\t(%d) next = 0x%08X", i, (unsigned long)nb);
    }
}
#endif





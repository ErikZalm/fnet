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
* @file fnet_serial.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.20.0
*
* @brief FNET Serial Input and Output Library implementation.
*
***************************************************************************/

#include "fnet.h"
#include "fnet_serial.h"

static int fnet_serial_printk_mknumstr( char *numstr, void *nump, int neg, int radix );
static void fnet_serial_printk_pad( char c, fnet_serial_stream_t stream, int curlen, int field_width, int *count );
static void fnet_serial_buffer_putchar( long p_dest, int character );


/******************************************************************************
 * Stream descriptors associated with the serial ports.
 ******************************************************************************/
const struct fnet_serial_stream fnet_serial_stream_port0 =
{
    0,
    fnet_cpu_serial_putchar,
    fnet_cpu_serial_getchar
};

const struct fnet_serial_stream fnet_serial_stream_port1 =
{
    1,
    fnet_cpu_serial_putchar,
    fnet_cpu_serial_getchar
};

const struct fnet_serial_stream fnet_serial_stream_port2 =
{
    2,
    fnet_cpu_serial_putchar,
    fnet_cpu_serial_getchar
};

const struct fnet_serial_stream fnet_serial_stream_port3 =
{
    3,
    fnet_cpu_serial_putchar,
    fnet_cpu_serial_getchar
};

const struct fnet_serial_stream fnet_serial_stream_port4 =
{
    4,
    fnet_cpu_serial_putchar,
    fnet_cpu_serial_getchar
};

const struct fnet_serial_stream fnet_serial_stream_port5 =
{
    5,
    fnet_cpu_serial_putchar,
    fnet_cpu_serial_getchar
};

/********************************************************************/
void fnet_serial_putchar(fnet_serial_stream_t stream, int character)
{
    stream->putchar(stream->id, character);
}

/********************************************************************/
int fnet_serial_getchar(fnet_serial_stream_t stream)
{
    return stream->getchar(stream->id);
}

/********************************************************************/
void fnet_serial_flush(fnet_serial_stream_t stream)
{
    if(stream->flush)
        stream->flush(stream->id);
}

/*********************************************************************
 * fnet_prinf & fnet_sprintf staff
 * 
 ********************************************************************/

/********************************************************************/

#define FNET_SERIAL_FLAGS_MINUS         (0x01)
#define FNET_SERIAL_FLAGS_PLUS          (0x02)
#define FNET_SERIAL_FLAGS_SPACE         (0x04)
#define FNET_SERIAL_FLAGS_ZERO          (0x08)
#define FNET_SERIAL_FLAGS_POUND         (0x10)

#define FNET_SERIAL_IS_FLAG_MINUS(a)    (a & FNET_SERIAL_FLAGS_MINUS)
#define FNET_SERIAL_IS_FLAG_PLUS(a)     (a & FNET_SERIAL_FLAGS_PLUS)
#define FNET_SERIAL_IS_FLAG_SPACE(a)    (a & FNET_SERIAL_FLAGS_SPACE)
#define FNET_SERIAL_IS_FLAG_ZERO(a)     (a & FNET_SERIAL_FLAGS_ZERO)
#define FNET_SERIAL_IS_FLAG_POUND(a)    (a & FNET_SERIAL_FLAGS_POUND)

/********************************************************************/
static int fnet_serial_printk_mknumstr( char *numstr, void *nump, int neg, int radix )
{
    int a, b, c;
    unsigned int ua, ub, uc;

    int nlen;
    char *nstrp;

    nlen = 0;
    nstrp = numstr;
    *nstrp++ = '\0';

    if(neg)
    {
        a = *(int *)nump;

        if(a == 0)
        {
            *nstrp = '0';
            ++nlen;
            goto done;
        }

        while(a != 0)
        {
            b = (int)a / (int)radix;
            c = (int)a - ((int)b * (int)radix);

            if(c < 0)
            {
                c = ~c + 1 + '0';
            }
            else
            {
                c = c + '0';
            }

            a = b;
            *nstrp++ = (char)c;
            ++nlen;
        }
    }
    else
    {
        ua = *(unsigned int *)nump;

        if(ua == 0)
        {
            *nstrp = '0';
            ++nlen;
            goto done;
        }

        while(ua != 0)
        {
            ub = (unsigned int)ua / (unsigned int)radix;
            uc = (unsigned int)ua - ((unsigned int)ub * (unsigned int)radix);

            if(uc < 10)
            {
                uc = uc + '0';
            }
            else
            {
                uc = uc - 10 + 'a';
            }

            ua = ub;
            *nstrp++ = (char)uc;
            ++nlen;
        }
    }

    done:
    return nlen;
}

/********************************************************************/
static void fnet_serial_printk_pad(char c, fnet_serial_stream_t stream, int curlen, int field_width, int *count )

{
    int i;

    for (i = curlen; i < field_width; i++)
    {
        (*count)++;
        fnet_serial_putchar(stream, c);
    }
}

/********************************************************************/
int fnet_serial_vprintf(fnet_serial_stream_t stream, const char *format, fnet_va_list ap )
{
    char *p;
    int c;

    char vstr[33];
    char *vstrp;
    int vlen;

    int done;
    int count = 0;

    int flags_used;
    int field_width;

#if 0

    int precision_used;
    int precision_width;
    int length_modifier;

#endif

    int ival;
    int schar, dschar;
    int *ivalp;
    char *sval;
    int cval;
    unsigned int uval;
    
    /*
     * Start parsing apart the format string and display appropriate
     * formats and data.
     */
    for (p = (char *)format; (c = *p) != 0; p++)
    {
        /*
         * All formats begin with a '%' marker.  Special chars like
         * '\n' or '\t' are normally converted to the appropriate
         * character by the __compiler__.  Thus, no need for this
         * routine to account for the '\' character.
         */
        if(c != '%')
        {
        
#if FNET_CFG_SERIAL_PRINTF_N_TO_RN
            if(c == '\n') /* LF.*/
            {
                count++;
                fnet_serial_putchar(stream, '\r' /* CR */);
            }
#endif            
            
            count++;
            fnet_serial_putchar(stream, c);

            continue;
        }

        /*
         * First check for specification modifier flags.
         */
        flags_used = 0;
        done = 0;

        while(!done)
        {
            switch( /* c = */*++p)
            {
                case '-':
                  flags_used |= FNET_SERIAL_FLAGS_MINUS;
                  break;

                case '+':
                  flags_used |= FNET_SERIAL_FLAGS_PLUS;
                  break;

                case ' ':
                  flags_used |= FNET_SERIAL_FLAGS_SPACE;
                  break;

                case '0':
                  flags_used |= FNET_SERIAL_FLAGS_ZERO;
                  break;

                case '#':
                  flags_used |= FNET_SERIAL_FLAGS_POUND;
                  break;

                default:
                  /* we've gone one char too far */
                  --p;
                  done = 1;
                  break;
            }
        }

        /*
         * Next check for minimum field width.
         */
        field_width = 0;
        done = 0;

        while(!done)
        {
#if 0
            switch(c = *++p)
            {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                  field_width = (field_width * 10) + (c - '0');
                  break;

                default:
                  /* we've gone one char too far */
                  --p;
                  done = 1;
                  break;
            }
#else 
            c = *++p;
            if(c >= '0' && c <= '9')
            {
                field_width = (field_width * 10) + (c - '0');
            }
            else
            {
                /* we've gone one char too far */
                --p;
                done = 1;
            }
#endif            

           
        }

        /*
         * Next check for the width and precision field separator.
         */
        if( /* (c = *++p) */*++p == '.')
        {
            /* precision_used = TRUE; */

            /*
             * Must get precision field width, if present.
             */
            /* precision_width = 0; */
            done = 0;

            while(!done)
            {

#if 0

                switch( /* c = uncomment if used below */*++p)
                {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':

    #if 0

                          precision_width = (precision_width * 10) + (c - '0');

    #endif

                      break;

                    default:
                      /* we've gone one char too far */
                      --p;
                      done = 1;
                      break;
                }
#else
                c = *++p;
                if(c >= '0' && c <= '9')
                {
                    #if 0
                      precision_width = (precision_width * 10) + (c - '0');
                    #endif
                }
                else
                {
                    /* we've gone one char too far */
                    --p;
                    done = 1;
                }


#endif

                
            }
        }
        else
        {
            /* we've gone one char too far */
            --p;

#if 0

            precision_used = FALSE;
            precision_width = 0;

#endif

        }

        /*
         * Check for the length modifier.
         */
        /* length_modifier = 0; */
        switch( /* c = */*++p)
        {
            case 'h':
              /* length_modifier |= LENMOD_h; */
              break;

            case 'l':
              /* length_modifier |= LENMOD_l; */
              break;

            case 'L':
              /* length_modifier |= LENMOD_L; */
              break;

            default:
              /* we've gone one char too far */
              --p;
              break;
        }

        /*
         * Now we're ready to examine the format.
         */
        switch(c = *++p)
        {
            case 'd':
            case 'i':
                ival = (int)fnet_va_arg(ap, int);
                vlen = fnet_serial_printk_mknumstr(vstr, &ival, 1, 10);
                vstrp = &vstr[vlen];

                if(ival < 0)
                {
                    schar = '-';
                    ++vlen;
                }
                else
                {
                    if(FNET_SERIAL_IS_FLAG_PLUS(flags_used))
                    {
                        schar = '+';
                        ++vlen;
                    }
                    else
                    {
                        if(FNET_SERIAL_IS_FLAG_SPACE(flags_used))
                        {
                            schar = ' ';
                            ++vlen;
                        }
                        else
                        {
                            schar = 0;
                        }
                    }
                }

                dschar = 0;

                /*
                * do the ZERO pad.
                */
                if(FNET_SERIAL_IS_FLAG_ZERO(flags_used))
                {
                    if(schar)
                    {
                        count++;
                        fnet_serial_putchar(stream, schar);
                    }

                    dschar = 1;

                    fnet_serial_printk_pad('0', stream, vlen, field_width, &count);
                    
                    vlen = field_width;
                }
                else
                {
                    if(!FNET_SERIAL_IS_FLAG_MINUS(flags_used))
                    {
                        fnet_serial_printk_pad(' ', stream, vlen, field_width, &count);

                        if(schar)
                        {
                            count++;
                            fnet_serial_putchar(stream, schar);
                        }

                        dschar = 1;
                    }
                }

              /* the string was built in reverse order, now display in */
              /* correct order */
              if(!dschar && schar)
              {
                  count++;
                  fnet_serial_putchar(stream, schar);
              }

              goto cont_xd;

            case 'x':
            case 'X':
                uval = (unsigned int)fnet_va_arg(ap, unsigned int);
                vlen = fnet_serial_printk_mknumstr(vstr, &uval, 0, 16);
                vstrp = &vstr[vlen];

                dschar = 0;

                if(FNET_SERIAL_IS_FLAG_ZERO(flags_used))
                {
                    if(FNET_SERIAL_IS_FLAG_POUND(flags_used))
                    {
                        count+=2;
                        fnet_serial_putchar(stream, '0');
                        fnet_serial_putchar(stream, 'x');
                      
                        /*vlen += 2;*/
                        dschar = 1;
                    }

                    fnet_serial_printk_pad('0', stream, vlen, field_width, &count);
                    vlen = field_width;
                }
                else
                {
                    if(!FNET_SERIAL_IS_FLAG_MINUS(flags_used))
                    {
                        if(FNET_SERIAL_IS_FLAG_POUND(flags_used))
                        {
                            vlen += 2;
                        }

                        fnet_serial_printk_pad(' ', stream, vlen, field_width, &count);

                        if(FNET_SERIAL_IS_FLAG_POUND(flags_used))
                        {
                            count+=2;
                            fnet_serial_putchar(stream, '0');
                            fnet_serial_putchar(stream, 'x');
                            dschar = 1;
                        }
                    }
                }
    
                if((FNET_SERIAL_IS_FLAG_POUND(flags_used)) && !dschar)
                {
                    count+=2;
                    fnet_serial_putchar(stream, '0');
                    fnet_serial_putchar(stream, 'x');
                    
                    vlen += 2;
                }

                goto cont_xd;

            case 'o':
              uval = (unsigned int)fnet_va_arg(ap, unsigned int);
              vlen = fnet_serial_printk_mknumstr(vstr, &uval, 0, 8);
              goto cont_u;

            case 'b':
              uval = (unsigned int)fnet_va_arg(ap, unsigned int);
              vlen = fnet_serial_printk_mknumstr(vstr, &uval, 0, 2);
              goto cont_u;

            case 'p':
              uval = (unsigned int)fnet_va_arg(ap, void *);
              vlen = fnet_serial_printk_mknumstr(vstr, &uval, 0, 16);
              goto cont_u;

            case 'u':
                uval = (unsigned int)fnet_va_arg(ap, unsigned int);
                vlen = fnet_serial_printk_mknumstr(vstr, &uval, 0, 10);

                cont_u:
                vstrp = &vstr[vlen];

                if(FNET_SERIAL_IS_FLAG_ZERO(flags_used))
                {
                    fnet_serial_printk_pad('0', stream, vlen, field_width, &count);
                    vlen = field_width;
                }
                else
                {
                    if(!FNET_SERIAL_IS_FLAG_MINUS(flags_used))
                    {
                        fnet_serial_printk_pad(' ', stream, vlen, field_width, &count);
                    }
                }

                cont_xd:
                while(*vstrp)
                {
                    count++;
                    fnet_serial_putchar(stream, *vstrp--);
                }
                if(FNET_SERIAL_IS_FLAG_MINUS(flags_used))
                {
                    fnet_serial_printk_pad(' ', stream, vlen, field_width, &count);
                }

                break;

            case 'c':
                cval = (char)fnet_va_arg(ap, unsigned int);
                count++;
                fnet_serial_putchar(stream, cval);
                break;

            case 's':
                sval = (char *)fnet_va_arg(ap, char *);

                if(sval)
                {
                    vlen = (int)fnet_strlen(sval);

                    if(!FNET_SERIAL_IS_FLAG_MINUS(flags_used))
                    {
                        fnet_serial_printk_pad(' ', stream, vlen, field_width, &count);
                    }

                    while(*sval)
                    {
                        count++;
                        fnet_serial_putchar(stream, *sval++);
                    }

                    if(FNET_SERIAL_IS_FLAG_MINUS(flags_used))
                    {
                        fnet_serial_printk_pad(' ', stream, vlen, field_width, &count);
                    }
                }

                break;

            case 'n':
                ivalp = (int *)fnet_va_arg(ap, int *);
                *ivalp = count;
                break;

            default:
                count++;
                fnet_serial_putchar(stream, c);
                break;
        }
    }

    return count;
}


/********************************************************************/
int fnet_serial_printf(fnet_serial_stream_t stream, const char *format, ... )
{
    fnet_va_list ap;
    /*
     * Initialize the pointer to the variable length argument list.
     */
    fnet_va_start(ap, format);
    return fnet_serial_vprintf(stream, format, ap);

}

/********************************************************************/
int fnet_printf(const char *format, ... )
{
    fnet_va_list ap;
    /*
     * Initialize the pointer to the variable length argument list.
     */
    fnet_va_start(ap, format);
    return fnet_serial_vprintf(FNET_SERIAL_STREAM_DEFAULT, format, ap);  
}


/************************************************************************
* NAME: fnet_println
*
* DESCRIPTION:
************************************************************************/
int fnet_println(const char *format, ... )
{
    fnet_va_list ap;
    int result = 0;
    
    /*
     * Initialize the pointer to the variable length argument list.
     */
    fnet_va_start(ap, format);
    result = fnet_serial_vprintf(FNET_SERIAL_STREAM_DEFAULT, format, ap);
    result += fnet_printf("\n");
    
    return result;
}


/******************************************************************************
 * Control structure associated with the data buffer stream.
 ******************************************************************************/
struct fnet_serial_buffer_id
{
    char *dest;             /* Pointer to the destination buffer.*/ 
    unsigned int dest_size; /* Maximum number of characters to be written to the buffer.*/
};



/********************************************************************/
static void fnet_serial_buffer_putchar(long p_dest, int character)
{
    struct fnet_serial_buffer_id *buffer_id = (struct fnet_serial_buffer_id*)p_dest;

    if(buffer_id->dest_size)
    {
        *(buffer_id->dest) = (char)character;
        buffer_id->dest++;
        
        --buffer_id->dest_size;
    }
}

/********************************************************************/
int fnet_sprintf( char *str, const char *format, ... )
{
    fnet_va_list ap;
    int result = 0;
   
    struct fnet_serial_stream buffer_stream;
    struct fnet_serial_buffer_id buffer_id;
    
    if(str != 0)
    {
        buffer_id.dest = str;
        buffer_id.dest_size = (unsigned int)-1; /* No limit.*/

        buffer_stream.id = (long)&buffer_id;//(long)&str;
        buffer_stream.putchar = fnet_serial_buffer_putchar;
        
        /*
         * Initialize the pointer to the variable length argument list.
         */
        fnet_va_start(ap, format);
        result = fnet_serial_vprintf(&buffer_stream, format, ap);
        *buffer_id.dest = '\0'; /* Trailing null character.*/
    }
    
    return result;
}

/********************************************************************/
int fnet_snprintf( char *str, unsigned int size, const char *format, ... )
{
    fnet_va_list ap;
    int result = 0;
   
    struct fnet_serial_stream buffer_stream;
    struct fnet_serial_buffer_id buffer_id;
    
    
    if((str != 0) && (size != 0))
    {
        --size; /* Space for the trailing null character.*/
        buffer_id.dest = str;
        buffer_id.dest_size = size; 
        
        buffer_stream.id = (long)&buffer_id;
        buffer_stream.putchar = fnet_serial_buffer_putchar;
        
        /*
         * Initialize the pointer to the variable length argument list.
         */
        fnet_va_start(ap, format);
        result = fnet_serial_vprintf(&buffer_stream, format, ap);
        *buffer_id.dest = '\0'; /* Trailing null character.*/
        if(result > size)
            result = (int)size;
    } 
    return result;
}

/********************************************************************/
void fnet_putchar( int character )
{
    fnet_cpu_serial_putchar( FNET_CFG_CPU_SERIAL_PORT_DEFAULT, character);
}

/********************************************************************/
int fnet_getchar( void )
{
    return fnet_cpu_serial_getchar(FNET_CFG_CPU_SERIAL_PORT_DEFAULT);    
}


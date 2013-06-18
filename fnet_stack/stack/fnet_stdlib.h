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
* @file fnet_stdlib.h
*
* @date Dec-19-2012
*
* @version 0.1.27.0
*
* @brief Standard functions API.
*
***************************************************************************/

#ifndef _FNET_STDLIB_H_

#define _FNET_STDLIB_H_

#include "fnet_config.h"
#include "fnet_comp.h"

/*! @addtogroup fnet_stdlib
* The standard library defines general purpose functions, including string converting, searching and other data manipulations.
*/

/*! @{ */

#include <stdarg.h>
/**************************************************************************/ /*!
 * @brief Type to hold information about variable arguments.
 * @showinitializer
 ******************************************************************************/
#define fnet_va_list                va_list                             
#define fnet_va_start(ap, parm)     va_start(ap, parm)  /* Initialize a variable argument list.*/
#define fnet_va_end(ap)             va_end(ap)          /* End using variable argument list.*/
#define fnet_va_arg(ap, type)       va_arg(ap, type)    /* Retrieve next argument.*/

#if 0 /* To be deleted as it is fully compiler dependant */ /* For MCF CW */
    /* Type to hold information about variable arguments. */
    typedef char *fnet_va_list;
    /* Initialize a variable argument list.*/
    #define fnet_va_start(ap, parm) ap = (char *)(&parm + 1)
    /* End using variable argument list.*/
    #define fnet_va_end(ap)         ((void)0)
    /* Retrieve next argument.*/
    #define fnet_va_arg(ap, type)   (*(type *)((ap += (sizeof(type) + 3U) & ~3U) - (sizeof(type))))
#endif

/**************************************************************************/ /*!
 * @def FNET_TRUE
 * @brief TRUE Boolean value.
 * @showinitializer 
 ******************************************************************************/
#define FNET_TRUE       (1)

/**************************************************************************/ /*!
 * @def FNET_FALSE
 * @brief FALSE Boolean value.
 * @showinitializer 
 ******************************************************************************/
#define FNET_FALSE      (0)

/**************************************************************************/ /*!
 * @def FNET_NULL
 * @brief NULL pointer.
 * @showinitializer 
 ******************************************************************************/
#define FNET_NULL       (0)


/***************************************************************************/ /*!
 *
 * @brief    Copies bytes in memory.
 *
 * @param dest      Pointer to the memory location to copy to.
 *
 * @param src       Pointer to the memory location to copy from. 
 *
 * @param n         Number of bytes to copy.
 *
 ******************************************************************************
 *
 * This function copies @c number_of_bytes bytes from 
 * memory area pointed by @c from_ptr to memory area pointed by @c to_ptr.
 *
 ******************************************************************************/
void fnet_memcpy(FNET_COMP_PACKED_VAR void *dest, FNET_COMP_PACKED_VAR const void *src, unsigned n);

/***************************************************************************/ /*!
 *
 * @brief           Sets bytes in memory.
 *
 * @param dest      Pointer to the buffer to be set.
 *
 * @param value     Value to be set. @n
 *                  The value is passed as an @c int, but the function converts 
 *                  it to an @c unsigned @c char.
 *
 * @param n         Number of bytes to be set.
 *
 * @see fnet_memset_zero()
 *
 ******************************************************************************
 *
 * This function sets the first @c n bytes of the memory area pointed to by
 * @c dest with the constant byte @c value.
 *
 ******************************************************************************/
void fnet_memset(void *dest, int value, unsigned int n);

/***************************************************************************/ /*!
 *
 * @brief           Sets bytes in memory to zeros.
 *
 * @param dest      Pointer to the buffer to be set.
 *
 * @param n         Number of bytes to be set.
 *
 * @see fnet_memset()
 *
 ******************************************************************************
 *
 * This function sets the first @c n bytes of the memory area pointed to by
 * @c dest with zeros.
 *
 ******************************************************************************/
void fnet_memset_zero(void *dest, unsigned n );

/***************************************************************************/ /*!
 *
 * @brief           Compares memory areas.
 *
 * @param src1      Pointer to the memory buffer to compare.
 *
 * @param src2      Pointer to the memory buffer to compare.
 *
 * @param n         Number of bytes to compare.
 *
 * @return          This function returns zero if two buffers are identical,
 *                  otherwise returns @c 1.
 *
 ******************************************************************************
 *
 * This function compares the first @c n bytes of the memory buffer pointed 
 * by @c src1 to the first @c n bytes pointed by @c src2, returning zero if 
 * they all match, otherwise returns @c 1.
 *
 ******************************************************************************/
int fnet_memcmp(const void *src1, const void *src2, int n );

/***************************************************************************/ /*!
 *
 * @brief           Calculates the length of a string.
 *
 * @param str       Pointer to the null-terminated string to be examined.  
 *
 * @return          This function returns the number of characters in @c str.
 *
 ******************************************************************************
 *
 * This function computes the number of bytes in the string to which @c str 
 * points, not including the terminating null byte.
 *
 ******************************************************************************/
unsigned long fnet_strlen (const char *str);

/***************************************************************************/ /*!
 *
 * @brief           Concatenates two strings.
 *
 * @param dest      Pointer to the null-terminated string to append to.  
 *
 * @param src       Pointer to the null-terminated string to copy to 
 *                  the end of @c dest. 
 *
 ******************************************************************************
 *
 * This function appends a copy of the string pointed to by @c src to the end 
 * of the string pointed to by  @c dest. @n
 * The resulting string is null-terminated. 
 *
 ******************************************************************************/
void fnet_strcat (char *dest, const char *src);

/***************************************************************************/ /*!
 *
 * @brief           Concatenates a string with part of another.
 *
 * @param dest      Pointer to the null-terminated string to append to.  
 *
 * @param src       Pointer to the null-terminated string to copy to 
 *                  the end of @c dest. 
 *
 * @param n         Maximum number of characters to be appended.
 *
 ******************************************************************************
 *
 * This function appends a copy of the string pointed to by @c src to the end 
 * of the string pointed to by  @c dest. If the length of the @c src string 
 * is less than @c n, only the content up to the terminating null-character 
 * is copied. @n
 * The resulting string is null-terminated. 
 *
 ******************************************************************************/
void fnet_strncat (char *dest, const char *src, int n);

/***************************************************************************/ /*!
 *
 * @brief           Copies a string.
 *
 * @param dest      Pointer to the destination buffer where the content is 
 *                  to be copied.  
 *
 * @param src       Pointer to the null-terminated string to be copied.
 *
 ******************************************************************************
 *
 * This function copies the string pointed by @c src into the buffer pointed 
 * by @c dest, including the terminating null character.
 *
 ******************************************************************************/
void fnet_strcpy (char *dest, const char *src);

/***************************************************************************/ /*!
 *
 * @brief           Copies part of a string.
 *
 * @param dest      Pointer to the destination buffer where the content is 
 *                  to be copied.  
 *
 * @param src       Pointer to the null-terminated string to be copied.
 *
 * @param n         Maximum number of characters to be copied. 
 *
 ******************************************************************************
 *
 * This function copies the first @c n characters of the string pointed 
 * by @c src into the buffer pointed by @c dest.@n
 * The result string is null-terminated.
 *
 ******************************************************************************/
void fnet_strncpy( char *dest, const char *src, unsigned long n );

/***************************************************************************/ /*!
 *
 * @brief           Locates last occurrence of a character in a string.
 *
 * @param str       Pointer to the null-terminated string to be analyzed.
 * 
 * @param chr       Character to search for.  
 *
 * @return          This function returns the pointer to the found character 
 *                  in @c str, or @ref FNET_NULL if no such character is found.
 *
 ******************************************************************************
 *
 * This function finds the last occurrence of the character @n chr
 * in the string pointed to by @c str..
 *
 ******************************************************************************/
char *fnet_strrchr( const char *str, int chr );

/***************************************************************************/ /*!
 *
 * @brief           Locates first occurrence of a character in a string.
 *
 * @param str       Pointer to the null-terminated string to be analyzed.
 * 
 * @param chr       Character to search for.  
 *
 * @return          This function returns the pointer to the found character
 *                  in @c str, or @ref FNET_NULL if no such character is found.
 *
 ******************************************************************************
 *
 * This function finds the first occurrence of the character @n chr
 * in the string pointed to by @c str.
 *
 ******************************************************************************/
char *fnet_strchr( const char *str, int chr );

/***************************************************************************/ /*!
 *
 * @brief           Locates a substring.
 *
 * @param str       Pointer to the null-terminated string to be analyzed.
 * 
 * @param substr    Pointer to the null-terminated string to search for.  
 *
 * @return          This function returns pointer to the first occurrence 
 *                  in @c str of the entire sequence of characters 
 *                  specified in @c substr.
 *
 ******************************************************************************
 *
 * This function finds the first occurrence of the substring @c substr 
 * (excluding the terminating null byte) in the string @c str. 
 *
 ******************************************************************************/
char *fnet_strstr( const char *str, const char *substr );

/***************************************************************************/ /*!
 *
 * @brief           Compares two strings.
 *
 * @param str1      Pointer to the null-terminated string to be compared.
 *
 * @param str2      Pointer to the null-terminated string to be compared.
 *
 * @return          This function returns zero if two strings are identical,
 *                  otherwise returns the difference between the first 
 *                  two differing characters.
 *
 ******************************************************************************
 *
 * This function compares the two strings @c str1 and @c str2, returning zero if 
 * they all match or the difference between the first two differing characters.
 *
 ******************************************************************************/
int fnet_strcmp( const char *str1, const char *str2 );

/***************************************************************************/ /*!
 *
 * @brief           Compares part of two strings.
 *
 * @param str1      Pointer to the null-terminated string to be compared.
 *
 * @param str2      Pointer to the null-terminated string to be compared.
 *
 * @param n         Maximum number of characters to compare.  
 *
 * @return          This function returns zero if @c n characters of 
 *                  two strings are identical,
 *                  otherwise returns the difference between the first 
 *                  two differing characters.
 *
 ******************************************************************************
 *
 * This function compares not more than @c n characters (characters that 
 * follow a null byte are not compared) from the two strings @c str1 and 
 * @c str2, returning zero if they all match or the difference between 
 * the first two differing characters.
 *
 ******************************************************************************/
int fnet_strncmp( const char *str1, const char *str2, unsigned int n );

/***************************************************************************/ /*!
 *
 * @brief           Compares two strings ignoring case.
 *
 * @param str1      Pointer to the null-terminated string to be compared.
 *
 * @param str2      Pointer to the null-terminated string to be compared.
 *
 * @return          This function returns an integer less than, equal to, 
 *                  or greater than zero if @c str1 is found, 
 *                  respectively, to be less than, to match, or be greater 
 *                  than @c str2.
 *
 ******************************************************************************
 *
 * This function compares the two strings @c str1 
 * and @c str2, ignoring the case of the characters. It returns an 
 * integer less than, equal to, or greater than zero if @c str1 is found, 
 * respectively, to be less than, to match, or be greater than @c str2.
 *
 ******************************************************************************/
int fnet_strcasecmp( const char *str1, const char *str2 );

/***************************************************************************/ /*!
 *
 * @brief           Compares two strings, with additional terminator.
 *
 * @param str1      Pointer to the null or @c splitter terminated string to be compared.
 *
 * @param str2      Pointer to the null-terminated string to be compared.
 *
 * @param splitter  Additional terminator that can be used in @c str. 
 *
 * @return          This function returns zero if two strings are identical,
 *                  otherwise returns the difference between the first 
 *                  two differing characters.
 *
 ******************************************************************************
 *
 * This function compares the two strings @c str1 and @c str2, returning zero if 
 * they all match or the difference between the first two differing characters.@n
 * The @c str1 string can be terminated by null or @c splitter character.
 *
 ******************************************************************************/
int fnet_strcmp_splitter( const char *str1, const char *str2, char splitter);

/***************************************************************************/ /*!
 *
 * @brief           Converts string to unsigned long integer.
 *
 * @param str       Pointer to the null-terminated string to be interpreted .
 *
 * @param ptr       Pointer to a pointer to character that will be set to 
 *                  the character immediately following the unsigned long 
 *                  integer in the string.  If no integer can be formed, 
 *                  it points to the first wrong character.
 *
 * @param base      Base of the interpreted integer value. If it equals to 0,
 *                  it will be set to the default value 10.
 *
 * @return          This function returns integer value corresponding to the 
 *                  contents of @c str on success. If no conversion can be 
 *                  performed, @c 0 is returned.
 *
 ******************************************************************************
 *
 * This function converts a string to an unsigned long integer.@n
 * The string to be converted can contain the digits '0' to '9'. 
 * Depending on the base, the string can also contain letters representing 
 * digits greater than 9. 
 *
 ******************************************************************************/
unsigned long fnet_strtoul (char *str, char **ptr, int base);

/***************************************************************************/ /*!
 *
 * @brief    Breaks a string into a sequence of tokens. 
 *
 * @param str           The string from which to extract tokens (null-terminated).
 *
 * @param delimiter     The string that contains a set of delimiter 
 *                      characters (null-terminated).
 *
 * @param last          Return parameter used by @ref fnet_strtok_r() to record 
 *                      its progress through @c str. @n
 *                      This pointer, must be the same while parsing the 
 *                      same string.
 *
 * @return       This function returns a pointer to the next token in @c str. 
 *               If there are no remaining tokens, it returns a @ref FNET_NULL
 *               pointer.
 *
 ******************************************************************************
 *
 * This function parses the string  @c str  into  tokens. @n
 * The  first function call should have @c str as its first argument. Subse-
 * quent calls should have the first  argument  set  to  @ref FNET_NULL.  Each  call
 * returns  a  pointer  to the next token, or @ref FNET_NULL when no more tokens are
 * found. @n
 * The function also updates the @c last parameter with the starting address 
 * of the token following the first occurrence of the @c delimiter parameter. @n
 * The @c delimiter string may be different for each call.
 *
 ******************************************************************************/
char * fnet_strtok_r(char *str, const char *delimiter, char **last);


#include "fnet_serial.h"

/*! @} */

#endif /* _FNET_STDLIB_H_ */

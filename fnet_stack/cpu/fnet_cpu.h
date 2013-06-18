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
* @file fnet_cpu.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.25.0
*
* @brief CPU-specific library API. 
*
***************************************************************************/

#ifndef _FNET_CPU_H_

#define _FNET_CPU_H_

#include "fnet_config.h"

#if FNET_MCF    /* ColdFire.*/ 
    #include "fnet_mcf.h"
#endif    

#if FNET_MK     /* Kinetis.*/
    #include "fnet_mk.h"
#endif    

#if FNET_MPC     /* MPC.*/
    #include "fnet_mpc.h"
#endif  

/*! @addtogroup fnet_socket */
/*! @{ */

/* native Flash controller align */
#if FNET_CFG_CPU_FLASH
	#if (FNET_CFG_CPU_FLASH_PROGRAM_SIZE == 8)
	  #define FNET_COMP_PACKED_NATIVE         FNET_COMP_PACKED_8
	#elif (FNET_CFG_CPU_FLASH_PROGRAM_SIZE == 4)
	  #define FNET_COMP_PACKED_NATIVE         FNET_COMP_PACKED_4
	#else
	  #error The macro FNET_CFG_CPU_FLASH_PROGRAM_SIZE must be set to correct value.
	#endif
#endif

/************************************************************************
*     For doing byte order conversions between CPU 
*     and the independent "network" order.
*     For Freescale CPUs they just return the variable passed.
*************************************************************************/

#if FNET_CFG_CPU_LITTLE_ENDIAN /* Little endian.*/ || defined(__DOXYGEN__)
  
/**************************************************************************/ /*!
 * @def FNET_HTONS
 * @param short_var A 16-bit number in host byte order.
 * @hideinitializer
 * @see FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl()
 * @brief Macros which converts the unsigned short integer from host byte order to 
 * network byte order.
 ******************************************************************************/
    #define FNET_HTONS(short_var)   ((((short_var) & 0x00FF) << 8) | (((short_var) & 0xFF00) >> 8))
/**************************************************************************/ /*!
 * @def FNET_NTOHS
 * @param short_var A 16-bit number in network byte order.
 * @hideinitializer
 * @see FNET_HTONS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl() 
 * @brief Macros which converts the unsigned short integer from network byte order to 
 * host byte order.
 ******************************************************************************/      
    #define FNET_NTOHS(short_var)   FNET_HTONS(short_var) 
/**************************************************************************/ /*!
 * @def FNET_HTONL
 * @param long_var A 32-bit number in host byte order.
 * @hideinitializer
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl() 
 * @brief Macros which converts the unsigned long integer from host byte order to 
 * network byte order.
 ******************************************************************************/      
    #define FNET_HTONL(long_var)    ((((long_var) & 0x000000FF) << 24) | (((long_var) & 0x0000FF00) << 8) | (((long_var) & 0x00FF0000) >> 8) | (((long_var) & 0xFF000000) >> 24))   
/**************************************************************************/ /*!
 * @def FNET_NTOHL
 * @param long_var A 32-bit number in network byte order.
 * @hideinitializer
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), fnet_htons(), fnet_ntohs(), fnet_htonl(), fnet_ntohl() 
 * @brief Macros which converts the unsigned long integer from network byte order to 
 * host byte order.
 ******************************************************************************/     
    #define FNET_NTOHL(long_var)    FNET_HTONL(long_var)  

/***************************************************************************/ /*!
 *
 * @brief    Converts 16-bit value from host to network byte order.
 *
 *
 * @param short_var A 16-bit number in host byte order.
 *
 *
 * @return This function returns the network byte-ordered @c short_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_ntohs(), fnet_htonl(), fnet_ntohl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned short integer from host byte order to 
 * network byte order.
 *
 ******************************************************************************/
    unsigned short fnet_htons(unsigned short short_var);
 
    #define fnet_ntohs   fnet_htons
/***************************************************************************/ /*!
 *
 * @brief    Converts 16-bit value from network to host byte order.
 *
 *
 * @param short_var A 16-bit number in network byte order.
 *
 *
 * @return This function returns the host byte-ordered @c short_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_htonl(), fnet_ntohl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned short integer from network byte order to 
 * host byte order.
 *
 ******************************************************************************/    
     unsigned short fnet_ntohs(unsigned short short_var);
     
/***************************************************************************/ /*!
 *
 * @brief    Converts 32-bit value from host to network byte order.
 *
 *
 * @param long_var A 32-bit number in host byte order.
 *
 *
 * @return This function returns the network byte-ordered @c long_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_ntohs(), fnet_htons(), fnet_ntohl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned short integer from host byte order to 
 * network byte order.
 *
 ******************************************************************************/
    unsigned long fnet_htonl(unsigned long long_var);

#define fnet_ntohl    fnet_htonl
/***************************************************************************/ /*!
 *
 * @brief    Converts 32-bit value from network to host byte order.
 *
 *
 * @param long_var A 32-bit number in network byte order.
 *
 *
 * @return This function returns the host byte-ordered @c long_var.
 *
 * @see FNET_HTONS(), FNET_NTOHS(), FNET_HTONL(), FNET_NTOHL(), fnet_htons(), fnet_ntohs(), fnet_htonl()
 *
 ******************************************************************************
 *
 * The function converts the unsigned short integer from network byte order to 
 * host byte order.
 *
 ******************************************************************************/       
     unsigned long fnet_ntohl(unsigned long long_var);


#else /* Big endian. */
    #define FNET_HTONS(short_var)   (short_var) 
    #define FNET_NTOHS(short_var)   (short_var)
    #define FNET_HTONL(long_var)    (long_var)
    #define FNET_NTOHL(long_var)    (long_var)
    
    #define fnet_htons(short_var)   (short_var) /* Convert short from host- to network byte order.*/
    #define fnet_ntohs(short_var)   (short_var) /* Convert short from network - to host byte order.*/
    #define fnet_htonl(long_var)    (long_var)  /* Convert long from host- to network byte order.*/
    #define fnet_ntohl(long_var)    (long_var)  /* Convert long from network - to host byte order.*/
#endif

/*! @} */

/////////////////////////////////////////////////////////////////////////////////

/*! @addtogroup fnet_cpu
* The CPU-specific library provides commonly used platform dependent functions.
* Most of these functions are not used by the FNET, but can be useful for
* an application.
*/
/*! @{ */


/**************************************************************************/ /*!
 * @brief IRQ status descriptor returned by the @ref fnet_cpu_irq_disable() function.
 * Actually it corresponds to the interrupt level mask value.
 * @see fnet_cpu_irq_disable(), fnet_cpu_irq_enable()
 ******************************************************************************/
typedef unsigned long fnet_cpu_irq_desc_t;

/***************************************************************************/ /*!
 *
 * @brief    Initiates software reset.
 *
 ******************************************************************************
 *
 * This function performs software reset and asserts the external reset 
 * (RSTO) pin.
 *
 ******************************************************************************/
void fnet_cpu_reset (void);

/***************************************************************************/ /*!
 *
 * @brief    Disables all interrupts.
 *
 * @return This function returns the current IRQ status defined 
 * by @ref fnet_cpu_irq_desc_t.
 *
 * @see fnet_cpu_irq_enable()
 *
 ******************************************************************************
 *
 * This function disables all interrupts. @n
 * The interrupts can be enabled again by the @ref fnet_cpu_irq_enable() function.
 *
 ******************************************************************************/ 
fnet_cpu_irq_desc_t fnet_cpu_irq_disable(void);

/***************************************************************************/ /*!
 *
 * @brief    Enables interrupts.
 *
 * @param desc     IRQ status descriptor returned by the 
 *                 @ref fnet_cpu_irq_disable() function.@n
 *                 Pass @c 0 value to enable all interrupts.
 *
 * @see fnet_cpu_irq_disable()
 *
 ******************************************************************************
 *
 * This function enables interrupts that were disabled by the 
 * @ref fnet_cpu_irq_disable function. @n
 * The functions can enable all interrupts by passing into it the @c 0 value.
 *
 ******************************************************************************/
void fnet_cpu_irq_enable(fnet_cpu_irq_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Writes character to the serial port.
 *
 * @param port_number     Serial port number.
 *
 * @param character       Character to be written to the serial port.
 *
 * @see fnet_cpu_serial_getchar(), fnet_cpu_serial_init()
 *
 ******************************************************************************
 *
 * This function writes @c character to the serial port defined 
 * by @c port_number. @n
 *
 ******************************************************************************/ 
void fnet_cpu_serial_putchar( long port_number, int character );

/***************************************************************************/ /*!
 *
 * @brief    Reads character from the serial port.
 *
 * @param port_number     Serial port number.
 *
 * @return This function returns:
 *   - character received by the serial port.
 *   - @ref FNET_ERR if no character is available.
 *
 * @see fnet_cpu_serial_putchar(), fnet_cpu_serial_init()
 *
 ******************************************************************************
 *
 * This function reads character from the serial port defined 
 * by @c port_number. @n
 *
 ******************************************************************************/ 
int fnet_cpu_serial_getchar( long port_number );

/***************************************************************************/ /*!
 *
 * @brief    Initializes the serial port.
 *
 * @param port_number     Serial port number.
 * 
 * @param baud_rate       Baud rate to be set to the serial port.
 *
 * @see fnet_cpu_serial_putchar(), fnet_cpu_serial_getchar()
 *
 ******************************************************************************
 *
 * This function executes the  HW initialization of the serial port defined 
 * by the @c port_number.
 *
 ******************************************************************************/
void fnet_cpu_serial_init(long port_number, unsigned long baud_rate);


/***************************************************************************/ /*!
 *
 * @brief    Invalidates CPU-cache memory.
 *
 ******************************************************************************
 *
 * If the CPU has cache memory, this function invalidates it.
 *
 ******************************************************************************/
void fnet_cpu_cache_invalidate(void);

/***************************************************************************/ /*!
 *
 * @brief    Erases the specified page of the on-chip Flash memory.
 *
 * @param flash_page_addr      Address of the page in the Flash to erase.
 *
 * @see fnet_cpu_flash_write()
 *
 ******************************************************************************
 *
 * This function erases the  whole Flash page pointed by @c flash_page_addr.@n
 * Erase page size is defined by @ref FNET_CFG_CPU_FLASH_PAGE_SIZE.
 *
 ******************************************************************************/
void fnet_cpu_flash_erase(void *flash_page_addr);

/***************************************************************************/ /*!
 *
 * @brief    Writes the specified data to the Flash memory.
 *
 * @param dest            Destination address in the Flash to write to.
 *
 * @param data            Pointer to the block of data to write to the Flash memory
 *                        Size of the data block must be equal to 
 *                        @ref FNET_CFG_CPU_FLASH_PROGRAM_SIZE.
 *
 * @see fnet_cpu_flash_erase()
 *
 ******************************************************************************
 *
 * This function copies @ref FNET_CFG_CPU_FLASH_PROGRAM_SIZE bytes pointed by @c data
 * to the Flash memory pointed by @c dest.
 *
 ******************************************************************************/
void fnet_cpu_flash_write(unsigned char *dest, unsigned char *data);

/***************************************************************************/ /*!
 *
 * @brief    CPU-specific FNET interrupt service routine.
 *
 ******************************************************************************
 *
 * This is CPU-specific ISR which is executed on every FNET interrupt 
 * (from Ethernet and timer module).@n
 * It extracts vector number and calls fnet_isr_handler().
 *
 ******************************************************************************/
void fnet_cpu_isr(void);

/***************************************************************************/ /*!
 * @def FNET_CPU_INSTRUCTION_ADDR
 *
 * @brief           Adjust value of the instruction address.
 *
 * @param addr      Instruction address to be adjusted.
 *
 ******************************************************************************
 *
 * This is CPU-specific macro that adjusts the instruction address. @n
 * If the current platform is Kinetis, it sets the Thumb bit (bit 0) of 
 * the address to 1. @n
 * If the current platform is ColdFire or MPC, it does nothing.
 *
 ******************************************************************************/
#ifndef FNET_CPU_INSTRUCTION_ADDR
    #define FNET_CPU_INSTRUCTION_ADDR(addr)    (addr)
#endif

/*! @} */

#endif /* _FNET_CPU_H_ */

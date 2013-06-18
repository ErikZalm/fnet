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
* @file fnet_serial_config.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.4.0
*
* @brief FNET Serial lib. configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_SERIAL_CONFIG_H_

#define _FNET_SERIAL_CONFIG_H_

/*! @addtogroup fnet_serial_config */
/*! @{ */

/**************************************************************************/ /*!
 * @def     FNET_CFG_SERIAL_PRINTF_N_TO_RN 
 * @brief   Automatic replacing of all line feeds ("\n") inside the format 
 *          string to CR LF ("\r\n") by fnet_(s)printf() functions:
 *               - @b @c 1 = is enabled (Deafault value, to save some ROM).
 *               - @c 0 = is disabled.@n
 *           @n
 * To save some bytes from all the hard coded strings the fnet_(s)printf() functions will 
 * expand all line feeds ("\n") inside the format string to CR LF ("\r\n"). 
 * So do not use "\r\n" in	the format string - it will be expanded to 
 * "\r\r\n". It is save to add it via a parameter though, e.g. 
 * fnet_printf("%s", "\r\n");
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_SERIAL_PRINTF_N_TO_RN
    #define FNET_CFG_SERIAL_PRINTF_N_TO_RN     (1)
#endif

/*! @} */

#endif

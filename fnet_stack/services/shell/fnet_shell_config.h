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
* @file fnet_shell_config.h
*
* @author Andrey Butok
*
* @date Feb-5-2013
*
* @version 0.0.22.0
*
* @brief FNET Shell Library configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_SHELL_CONFIG_H_

#define _FNET_SHELL_CONFIG_H_

/*! @addtogroup fnet_shell_config */
/*! @{ */

/**************************************************************************/ /*!
 * @def     FNET_CFG_SHELL_MAX
 * @brief   Maximum number of the Shell services that can be run simultaneously.@n
 *          Default value is @b @c 1.
 ******************************************************************************/
#ifndef FNET_CFG_SHELL_MAX

    #if FNET_CFG_TELNET
        #define FNET_CFG_SHELL_MAX          (1+(FNET_CFG_TELNET_MAX*FNET_CFG_TELNET_SESSION_MAX))
    #else
        #define FNET_CFG_SHELL_MAX          (1)
    #endif
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_SHELL_ARGS_MAX 
 * @brief Maximum number of arguments that can be handled by the 
 *        shell command parser. @n
 *        Arguments must be split by the space symbol. Note that the 
 *        quote @ref FNET_SHELL_QUOTE_SYMBOL is used to combine multiple 
 *        words to one argument. @n
 *        Default value is @b @c 16.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_SHELL_ARGS_MAX
    #define FNET_CFG_SHELL_ARGS_MAX         (16)
#endif

/**************************************************************************/ /*!
 * @def     FNET_CFG_SHELL_HELP_FORMAT 
 * @brief Format of the command-shell @c help message,
 *        that is used by the @ref fnet_shell_help() function.@n
 *        For example it can set to the @c ">%7s %-32s- %s" value , where the first string is 
 *        the command name, second one is the brief description of the command 
 *        and the third one is the syntax of the command.
 * @see fnet_shell_command, fnet_shell_help()
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_SHELL_HELP_FORMAT
    #define FNET_CFG_SHELL_HELP_FORMAT      (">%7s %-32s- %s")
#endif

/*! @} */

#endif

/**************************************************************************
* 
* Copyright 2012-2013 by Andrey Butok. FNET Community.
* Copyright 2005-2009 by Andrey Butok. Freescale Semiconductor, Inc.
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
* @file fnet_fs_config.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.10.0
*
* @brief FNET File System configuration file.
*
***************************************************************************/

/**************************************************************************
 * !!!DO NOT MODIFY THIS FILE!!!
 **************************************************************************/

#ifndef _FNET_FS_CONFIG_H_

#define _FNET_FS_CONFIG_H_

/** @addtogroup fnet_fs_config */
/** @{ */

/**************************************************************************/ /*!
 * @def      FNET_CFG_FS
 * @brief    File System Interface support:
 *               - @c 1 = is enabled.
 *               - @b @c 0 = is disabled (Default value).
 ******************************************************************************/
#ifndef FNET_CFG_FS
    #define FNET_CFG_FS         (0) 
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_FS_MOUNT_MAX
 * @brief    Maximum number of mount points. @n
 *           Default value is @b @c 2.
 * @showinitializer
 ******************************************************************************/
#ifndef FNET_CFG_FS_MOUNT_MAX
    #define FNET_CFG_FS_MOUNT_MAX           (2)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_FS_MOUNT_NAME_MAX
 * @brief    Maximum size of a mount-point name. @n
 *           For example, for mount point named "rom" the name size is 3.@n
 *           Default value is @b @c 10.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_FS_MOUNT_NAME_MAX
    #define FNET_CFG_FS_MOUNT_NAME_MAX      (10)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_FS_DESC_MAX
 * @brief    Maximum number of file and directory descriptors opened 
 *           simultaneously.@n
 *           Default value is @b @c 5.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_FS_DESC_MAX
    #define FNET_CFG_FS_DESC_MAX            (5)
#endif

/**************************************************************************/ /*!
 * @def      FNET_CFG_FS_ROM
 * @brief    FNET ROM File System support:
 *               - @b @c 1 = is enabled (Default value) .
 *               - @c 0 = is disabled.
 * @showinitializer 
 ******************************************************************************/
#ifndef FNET_CFG_FS_ROM
    #define FNET_CFG_FS_ROM                 (1) 
#endif

/** @} */

#endif /* _FNET_FS_CONFIG_H_ */

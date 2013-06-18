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
* @file fnet_fs_rom.h
*
* @author Andrey Butok
*
* @date Jan-28-2013
*
* @version 0.0.19.0
*
* @brief FNET ROM FS API.
*
***************************************************************************/
#ifndef _FNET_FS_ROM_H_

#define _FNET_FS_ROM_H_

#include "fnet_config.h"

#if (FNET_CFG_FS && FNET_CFG_FS_ROM) || defined(__DOXYGEN__)

#include "fnet_fs.h"

/*! @addtogroup fnet_fs_rom
* The FNET ROM File System provides read-only access to files 
* and directories. @n
* The FNET project has the @ref how_to_generate_rom_fs "special GUI PC tool"
* that is used to create the ROM file-system image files. 
*
* After the FS is initialized by calling the @ref fnet_fs_init() 
* function, a user application should call the  @ref fnet_fs_rom_register() 
* function  and finally mount a ROM FS Image by passing the @ref FNET_FS_ROM_NAME 
* and the @ref fnet_fs_rom_image structure as arguments to the 
* @ref fnet_fs_mount() function. @n
* Thereafter an application has access to files and directories on the mounted 
* FNET ROM FS image.
*
* For example:
* @code
* ...
* // FNET FS initialization.
* if( fnet_fs_init( ) == FNET_OK)
* {
*    // Register FNET ROM FS.
*    fnet_fs_rom_register( );
* 
*    // Mount FNET ROM FS image.
*    if( fnet_fs_mount( FNET_FS_ROM_NAME, "rom", (void *)&fnet_fs_image ) == FNET_ERR )
*           fnet_println("ERROR: FS image mount is failed!");
*    ...
*
*    // Print directory content.
*    {
*       struct fnet_fs_dirent ep;
*       FNET_FS_DIR dir;
*       char name[FNET_CFG_FS_MOUNT_NAME_MAX+1];
*
*       // Open dir.
*       dir = fnet_fs_opendir("rom");
*     
*       if (dir)
*       {
*           fnet_memset_zero(&ep, sizeof(struct fnet_fs_dirent) ); 
*           ep.d_name = name;
*           ep.d_name_size = sizeof(name);
*           
*           // Print the dir content.
*           while ((fnet_fs_readdir (dir, &ep))==FNET_OK)
*               fnet_println ("%7s  -  %s", (ep.d_type == DT_DIR)?"<DIR>":"<FILE>",ep.d_name);
*               
*           // Close dir.
*           fnet_fs_closedir(dir);    
*       }
*       else
*           fnet_println("ERROR: The directory is failed!");
*    }
*    ...
* }
* else
* {
*    fnet_println("ERROR: FNET FS initialization is failed!");
* } 
* ...
* @endcode
* @n
* Configuration parameters:
* - @ref FNET_CFG_FS_ROM  
*/

/*! @{ */


/**************************************************************************/ /*!
 * @brief   FNET ROM file-system node.@n
 *          The node represents a file or a directory. 
 * @see     fnet_fs_readdir()
 ******************************************************************************/  
struct fnet_fs_rom_node
{ 
	const char *name;           /**< @brief Name of a file or directory 
	                             * (null-terminated string). */
	unsigned char *data;        /**< @brief Pointer to a file-content buffer. @n
	                             * For a directory this field must be set to @c 0.*/
	unsigned long data_size;    /**< @brief Size of the file buffer pointed to
	                             * by the @c data field. @n
	                             * For a directory this field must be set to @c 0.*/
	const struct fnet_fs_rom_node * parent_node;    /**< @brief Pointer to the
	                                                 * parent directory. @n
	                                                 * For the root directory this field must be 
	                                                 * set to @c 0.*/
};

/**************************************************************************/ /*!
 * @brief FNET ROM file-system image 
 ******************************************************************************/ 
struct fnet_fs_rom_image
{
	const char *name;               /**< @brief File-system name (null-terminated string).@n
	                                 * Should be set to the @ref FNET_FS_ROM_NAME. */  
	unsigned long version;          /**< @brief File-system version the FS image 
	                                 * was generated for.*/
	const struct fnet_fs_rom_node *nodes;   /**< @brief Array of file-system nodes.@n
	                                         * The last node element must have 
	                                         * all fields set to zero 
	                                         * as the end-of-array mark.
	                                         */
};

/**************************************************************************/ /*!
 * @brief FNET ROM file-system name string.
 ******************************************************************************/ 
#define FNET_FS_ROM_NAME     "fnet_rom"

/**************************************************************************/ /*!
 * @brief FNET ROM file-system current version.
 ******************************************************************************/ 
#define FNET_FS_ROM_VERSION  (2)

/***************************************************************************/ /*!
 *
 * @brief    Registers the FNET ROM file system.
 *
 * @see fnet_fs_rom_unregister()
 * 
 ******************************************************************************
 *
 * This function registers the FNET ROM file system within the FNET FS interface. 
 *
 ******************************************************************************/
void fnet_fs_rom_register(void);

/***************************************************************************/ /*!
 *
 * @brief    Unregisters the FNET ROM file system.
 *
 * @see fnet_fs_rom_register()
 *
 ******************************************************************************
 *
 * This function unregisters the FNET ROM file system from the FNET FS interface. 
 *
 ******************************************************************************/
void fnet_fs_rom_unregister(void);

/*! @} */

#endif /* FNET_CFG_FS && FNET_CFG_FS_ROM */


#endif /* _FNET_FS_ROM_H_ */

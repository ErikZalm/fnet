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
* @file fnet_fs_prv.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.10.0
*
* @brief Private. File System API.
*
***************************************************************************/

#ifndef _FNET_FS_PRV_H_

#define _FNET_FS_PRV_H_

#include "fnet_config.h"

#if FNET_CFG_FS


/* Type of access and method for the file. */
typedef enum
{
    FNET_FS_MODE_READ               = (1<<0), /* Data can be read from the file.*/
    FNET_FS_MODE_WRITE              = (1<<1), /* Data can be written to the file.*/
    FNET_FS_MODE_END                = (1<<2), /* Move to the end of the file.*/
    FNET_FS_MODE_OPEN_EXISTING      = (1<<3), /* Opens the file. The function fails if the file is not existing. (Default) */
    FNET_FS_MODE_OPEN_ALWAYS        = (1<<4), /* Opens the file, if it is existing. If not, the function creates the new file. */
    FNET_FS_MODE_OPEN_TRUNC         = (1<<5)  /* If the file is existing, it is truncated and overwritten. */
} 
fnet_fs_open_mode_t;


/* Mount point. */
struct fnet_fs_mount_point
{
    char name[FNET_CFG_FS_MOUNT_NAME_MAX+1];
    struct fnet_fs * fs;
    void * arg; /* Argument passed by mount(). */
};

/* Descriptor structure. */
struct fnet_fs_desc
{
    unsigned long id;   /* Owner FS Directory ID. */
    unsigned long pos;  /* Current position. */
    struct fnet_fs_mount_point * mount; /* Pointer to the mount point. */
}; 

/* File operations. */
struct fnet_fs_file_operations
{
    int (*fopen)( struct fnet_fs_desc *file, const char *name, char mode, struct fnet_fs_desc * re_dir );
    unsigned long (*fread) (struct fnet_fs_desc *file, char * buf, unsigned long bytes); 
    int (*fseek) (struct fnet_fs_desc *file, long offset, fnet_fs_seek_origin_t origin);
    int (*finfo) (struct fnet_fs_desc *file, struct fnet_fs_dirent *dirent);
};

/* Dir operations. */
struct fnet_fs_dir_operations
{
    int (*opendir)( struct fnet_fs_desc *dir, const char *name );
    int (*readdir)( struct fnet_fs_desc *dir, struct fnet_fs_dirent* dirent );
};

/* FS operations. */
struct fnet_fs_operations
{
    int (*mount)( void *arg );
    void (*unmount)( void *arg );
};

/* FS control structure (for every fs).*/
struct fnet_fs
{
    const char * name;                                          /* FS uniqe name.*/
    const struct fnet_fs_operations *         operations;       /* FS operations.*/
    const struct fnet_fs_file_operations *    file_operations;  /* FS file operations.*/
    const struct fnet_fs_dir_operations *     dir_operations;   /* FS directory operations.*/
    struct fnet_fs *                    _next;                  /* Next FS in the list.*/
    struct fnet_fs *                    _prev;                  /* Previous FS in the list.*/
};

extern struct fnet_fs_mount_point fnet_fs_mount_list[FNET_CFG_FS_MOUNT_MAX];

void fnet_fs_register( struct fnet_fs *fs );
void fnet_fs_unregister( struct fnet_fs *fs );
struct fnet_fs * fnet_fs_find_name( char *name );
int fnet_fs_path_cmp( const char **path, const char *name);

#endif /* FNET_CFG_FS */

#endif

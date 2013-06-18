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
* @file fnet_fs_root.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.17.0
*
* @brief Root FS Implementation.
*
***************************************************************************/
#include "fnet_config.h"

#if FNET_CFG_FS

#include "fnet_stdlib.h"
#include "fnet_fs.h"
#include "fnet_fs_prv.h"
#include "fnet.h"
#include "fnet_fs_root.h"


#define FNET_FS_ROOTDIR_ID          ((unsigned long)(-1))   /* Root dir ID */

/* Root FS */

int fnet_fs_root_opendir( struct fnet_fs_desc *dir, const char *name);
int fnet_fs_root_readdir(struct fnet_fs_desc *dir, struct fnet_fs_dirent* dirent);

static const struct fnet_fs_dir_operations fnet_fs_root_dir_operations =
{
    fnet_fs_root_opendir,
    fnet_fs_root_readdir
};



static struct fnet_fs fnet_fs_root =
{
    FNET_FS_ROOT_NAME,
    0,
    0,
    &fnet_fs_root_dir_operations,   /* fs directory operations.*/
    0,
    0
};


static int fnet_fs_root_registered;  /* Flag that ROM FS is registered or not.*/

/************************************************************************
* NAME: fnet_fs_root_register
*
* DESCRIPTION:
*************************************************************************/
void fnet_fs_root_register( void )
{
    if(fnet_fs_root_registered == 0)
    {
        fnet_fs_register(&fnet_fs_root);
        fnet_fs_root_registered = 1;
    }
}

/************************************************************************
* NAME: fnet_fs_root_unregister
*
* DESCRIPTION: 
*************************************************************************/
void fnet_fs_root_unregister( void )
{
    if(fnet_fs_root_registered == 1)
    {
        fnet_fs_unregister(&fnet_fs_root);
        fnet_fs_root_registered = 0;
    }
}

/************************************************************************
* NAME: fnet_fs_root_opendir
*
* DESCRIPTION: Open DIR stream for the ROOT FS.
*************************************************************************/
int fnet_fs_root_opendir( struct fnet_fs_desc *dir, const char *name)
{
    int result = FNET_ERR;

    FNET_COMP_UNUSED_ARG(name);
    
    if(dir)
    {
        dir->id = FNET_FS_ROOTDIR_ID;
        result = FNET_OK;
    }
    else
        result = FNET_ERR;    
        
    return result;
}

/************************************************************************
* NAME: fnet_fs_root_readdir
*
* DESCRIPTION: Read DIR stream for the ROOT FS.
*************************************************************************/
int fnet_fs_root_readdir(struct fnet_fs_desc *dir, struct fnet_fs_dirent* dirent)
{
    int result = FNET_ERR;
    unsigned long i;
    struct fnet_fs_mount_point *tmp;
    
    if(dir && (dir->id == FNET_FS_ROOTDIR_ID) && (dir->pos != (unsigned long)FNET_FS_EOF) && dirent)
    {
        for(i=dir->pos; i<FNET_CFG_FS_MOUNT_MAX; i++)
        {
            tmp = &fnet_fs_mount_list[i];
            if(tmp->fs) /* Found next mount - dir */
            {
                dir->pos = i+1; /* incriment next dir index */
                if(fnet_strcmp(tmp->fs->name, FNET_FS_ROOT_NAME ) )/* It's not ROOT FS mount. */
                {
                    /*fill ident */
                    dirent->d_ino = (unsigned long) tmp; /* File serial number. */
                    dirent->d_type = DT_DIR;
                    dirent->d_name = tmp->name;
                    dirent->d_size = 0;
                    result = FNET_OK;
                    break;
                }
            }
        }
        if (result == FNET_ERR)
             dir->pos = (unsigned long)FNET_FS_EOF; /* End of the directory is encountered */
    
    }
    
    return result;
}

#endif /* FNET_CFG_FS */

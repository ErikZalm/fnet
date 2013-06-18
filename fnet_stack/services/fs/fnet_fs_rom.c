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
* @file fnet_fs_rom.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.17.0
*
* @brief ROM FS Implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_FS && FNET_CFG_FS_ROM

#include "fnet_fs_rom.h"
#include "fnet_fs_prv.h"
#include "fnet.h"

/* Supported fopen mode = read-only */
#define FNET_FS_ROM_OPENMODE     (FNET_FS_MODE_READ|FNET_FS_MODE_OPEN_EXISTING)

int fnet_fs_rom_opendir( struct fnet_fs_desc *dir, const char *name);
int fnet_fs_rom_readdir(struct fnet_fs_desc *dir, struct fnet_fs_dirent* dirent);
int fnet_fs_rom_fopen( struct fnet_fs_desc *file, const char *name, char mode, struct fnet_fs_desc * re_dir);
unsigned long fnet_fs_rom_fread (struct fnet_fs_desc *file, char * buf, unsigned long bytes);
int fnet_fs_rom_mount( void *arg );
int fnet_fs_rom_fseek (struct fnet_fs_desc *file, long offset, fnet_fs_seek_origin_t origin) ;
int fnet_fs_rom_finfo (struct fnet_fs_desc *file, struct fnet_fs_dirent *info);
static const struct fnet_fs_rom_node * fnet_fs_rom_find(const struct fnet_fs_rom_node * file_table, const char *name);
static void fnet_fs_rom_fill_dirent(struct fnet_fs_rom_node * node, struct fnet_fs_dirent* dirent);

/* FS  directory operations */
static const struct fnet_fs_dir_operations fnet_fs_rom_dir_operations =
{
    fnet_fs_rom_opendir,
    fnet_fs_rom_readdir
};

/* FS  file operations */
static const struct fnet_fs_file_operations fnet_fs_rom_file_operations =
{
    fnet_fs_rom_fopen,
    fnet_fs_rom_fread,
    fnet_fs_rom_fseek,
    fnet_fs_rom_finfo
};

/* FS operations */
static const struct fnet_fs_operations fnet_fs_rom_operations =
{
    fnet_fs_rom_mount
};

/* FS interface structure */
static struct fnet_fs fnet_fs_rom =
{
    FNET_FS_ROM_NAME,
    &fnet_fs_rom_operations,
    &fnet_fs_rom_file_operations,
    &fnet_fs_rom_dir_operations,
    0,
    0
};

static int fnet_fs_rom_registered;  /* Flag that ROM FS is registered or not.*/

/************************************************************************
* NAME: fnet_fs_rom_register
*
* DESCRIPTION: This function registers the ROM FS.
*************************************************************************/
void fnet_fs_rom_register( void )
{
    if(fnet_fs_rom_registered == 0)
    {
        fnet_fs_register(&fnet_fs_rom);
        fnet_fs_rom_registered = 1;
    }
}

/************************************************************************
* NAME: fnet_fs_rom_unregister
*
* DESCRIPTION: This function unregisters the ROM FS.
*************************************************************************/
void fnet_fs_rom_unregister( void )
{
    if(fnet_fs_rom_registered == 1)
    {
        fnet_fs_unregister(&fnet_fs_rom);
        fnet_fs_rom_registered = 0;
    }
}

/************************************************************************
* NAME: fnet_fs_rom_mount
*
* DESCRIPTION:
*************************************************************************/
int fnet_fs_rom_mount( void *arg )
{
    int result = FNET_ERR;
    struct fnet_fs_rom_image * image;
    
    if(arg)
    {
        /* Check if the image is ROM FS image and version number*/
        image = ((struct fnet_fs_rom_image * )arg);
        if( (fnet_strcmp( FNET_FS_ROM_NAME, image->name )==0)
            && (image->version == FNET_FS_ROM_VERSION))
        {
            result = FNET_OK;
        }
    }
    
    return result;
}

/************************************************************************
* NAME: fnet_fs_rom_find
*
* DESCRIPTION:
*************************************************************************/
static const struct fnet_fs_rom_node * fnet_fs_rom_find(const struct fnet_fs_rom_node * file_table, const char *name)
{
    const struct fnet_fs_rom_node * result = 0;
    const struct fnet_fs_rom_node * parent = 0;
    const struct fnet_fs_rom_node * current;
    
    if (file_table && name)
    {	
        current = file_table; /* root is always first */       
        
        if (current->name) 
		{
			while (*name == ' ') name++;	        /* Strip leading spaces */
	        if (*name == FNET_FS_SPLITTER) name++;	/* Strip heading slash */

			if (*name == '\0') /* Find root */
			{
			    result = current;
			}
			else
			{
			    parent = current;
                current++;

  		        while (current->name) 
		        {
			        if ((current->parent_node == parent)
			        && (fnet_fs_path_cmp( &name, current->name) == 0))
			        {
			            parent = current;
			        }

			        if (*name == '\0')
			        {
			            result = current;
				        break;
			        }
			
			        current++; 
		        }
		    }
		}
	}
    
    return result;
}

/************************************************************************
* NAME: fnet_fs_rom_opendir
*
* DESCRIPTION: Open DIR stream for the ROM FS.
*************************************************************************/
int fnet_fs_rom_opendir( struct fnet_fs_desc *dir, const char *name)
{
    int result = FNET_ERR;
    const struct fnet_fs_rom_node * file_table;
    const struct fnet_fs_rom_node * node;
    
    if(dir && name)
    {
        /* Find dir */ 
        file_table = ((struct fnet_fs_rom_image * )dir->mount->arg)->nodes;
  
        node = fnet_fs_rom_find(file_table, name);
        
        if(node && (node->data == 0) /* Is dir (not file)? */)
        {
            dir->id = (unsigned long) node; /* save pointer to found dir */
            dir->pos = 0;
            result = FNET_OK;
        }
    }
        
    return result;
}

/************************************************************************
* NAME: fnet_fs_rom_fill_dirent
*
* DESCRIPTION: 
*************************************************************************/
static void fnet_fs_rom_fill_dirent(struct fnet_fs_rom_node * node, struct fnet_fs_dirent* dirent)
{
    dirent->d_ino = (unsigned long) node; /*  File serial number. */
    dirent->d_type = (node->data == 0)? DT_DIR : DT_REG;
    dirent->d_name = node->name;
    dirent->d_size = node->data_size;
}

/************************************************************************
* NAME: fnet_fs_rom_readdir
*
* DESCRIPTION: Read DIR stream for the ROM FS.
*************************************************************************/
int fnet_fs_rom_readdir(struct fnet_fs_desc *dir, struct fnet_fs_dirent* dirent)
{
    int result = FNET_ERR;
    struct fnet_fs_rom_node * current;
    struct fnet_fs_rom_node * parent;
    
    if(dir && dir->id && (dir->pos != (unsigned long)FNET_FS_EOF) && dirent)
    {
        if(dir->pos == 0)
            current = (struct fnet_fs_rom_node *)(dir->id) + 1; 
        else
            current = (struct fnet_fs_rom_node *)dir->pos;    
        
        parent = (struct fnet_fs_rom_node *)(dir->id);
        
        while (current->name) 
		{
		    if(current->parent_node == parent) /* Next node is found */
		    {
		               
		        dir->pos = (unsigned long) (current+1); /* Save position */
                fnet_fs_rom_fill_dirent(current, dirent);
                result = FNET_OK;
                break;
		    }
		    
		    current++;
		}

		if (result == FNET_ERR)
             dir->pos = (unsigned long) FNET_FS_EOF; /* end of the directory is encountered */
   
    }
    
    return result;
}

/************************************************************************
* NAME: fnet_fs_rom_fopen
*
* DESCRIPTION: Open FILE stream for the ROM FS.
*************************************************************************/
int fnet_fs_rom_fopen( struct fnet_fs_desc *file, const char *name, char mode, struct fnet_fs_desc * re_dir )
{
    int result = FNET_ERR;
    const struct fnet_fs_rom_node * file_table;
    const struct fnet_fs_rom_node * node;
    
    if(file && name && (mode == FNET_FS_ROM_OPENMODE))
    {
        /* Find dir */
        if(re_dir && re_dir->id) 
            file_table = (struct fnet_fs_rom_node *) re_dir->id;
        else
            file_table = ((struct fnet_fs_rom_image * )file->mount->arg)->nodes;
  
        node = fnet_fs_rom_find(file_table, name);
        
        if(node && node->data /* Is file (not dir)? */)
        {
            file->id = (unsigned long) node; /* save pointer to found dir */
            file->pos = 0;
            result = FNET_OK;
        }
    }
        
    return result;
}

/************************************************************************
* NAME: fnet_fs_rom_fread
*
* DESCRIPTION: 
*************************************************************************/
unsigned long fnet_fs_rom_fread (struct fnet_fs_desc *file, char * buf, unsigned long bytes) 
{
    unsigned long result = 0;
    struct fnet_fs_rom_node * current;
    unsigned long size;
    unsigned long pos;
    
    if(file && file->id && (file->pos != (unsigned long)FNET_FS_EOF) && buf)
    {

        current = (struct fnet_fs_rom_node *)(file->id); 
        if(current && current->data_size && current->data)
        {
            size = current->data_size;
            pos = file->pos;
        
            if((pos + bytes) > size)
            {
                bytes = size - pos;
                file->pos = (unsigned long)FNET_FS_EOF;
            }
            else
            {
                file->pos += bytes;
            }
            
            fnet_memcpy( buf, &current->data[pos], bytes );
            result = bytes;
        }
   
    }
    
    return result;
}

/************************************************************************
* NAME: fnet_fs_rom_fseek
*
* DESCRIPTION:
*************************************************************************/
int fnet_fs_rom_fseek (struct fnet_fs_desc *file, long offset, fnet_fs_seek_origin_t origin) 
{
    int result = FNET_ERR;
    struct fnet_fs_rom_node * current;
    unsigned long size;
    unsigned long pos;
    long new_pos;
    
    if(file && file->id)
    {
        current = (struct fnet_fs_rom_node *)(file->id); 
        if(current && current->data_size)
        {
            size = current->data_size;
            pos = file->pos;
            
            switch( origin)
            {
                case FNET_FS_SEEK_SET:
                    new_pos = offset;
                    break;
                case FNET_FS_SEEK_CUR:
                    new_pos = (long)pos + offset;
                    break;
                case FNET_FS_SEEK_END:
                    new_pos = ((long)size - 1) - offset;
                    break;
                default:
                    new_pos = -1;
            }
            
            if((new_pos > 0) && (new_pos < size))
            {
                file->pos = (unsigned long)new_pos;
                result = FNET_OK;
            }
        }
    }
    
    return result;
}

/************************************************************************
* NAME: fnet_fs_rom_finfo
*
* DESCRIPTION:
*************************************************************************/
int fnet_fs_rom_finfo (struct fnet_fs_desc *file, struct fnet_fs_dirent *dirent)
{
    int result = FNET_ERR;
    if(file && file->id && dirent)
    {
        fnet_fs_rom_fill_dirent((struct fnet_fs_rom_node *)(file->id), dirent);
        result = FNET_OK;
    }
    
    return result;
}

#endif

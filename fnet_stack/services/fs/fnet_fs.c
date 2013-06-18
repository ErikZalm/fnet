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
* @file fnet_fs.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.0.21.0
*
* @brief File System API Implementation.
*
***************************************************************************/

#include "fnet_config.h"

#if FNET_CFG_FS

#include "fnet_stdlib.h"
#include "fnet_fs.h"
#include "fnet_fs_prv.h"
#include "fnet.h"
#include "fnet_fs_root.h"

/* Mount point list */
struct fnet_fs_mount_point fnet_fs_mount_list[FNET_CFG_FS_MOUNT_MAX];

/* File&dir descriptor list */
static struct fnet_fs_desc fnet_fs_desc_list[FNET_CFG_FS_DESC_MAX];

/* The list of registered FSs. */
static struct fnet_fs *fnet_fs_list;           

static struct fnet_fs_mount_point * fnet_fs_find_mount( const char **name );

/************************************************************************
* NAME: fnet_fs_init
*
* DESCRIPTION: This function initializes FNET FS interface.
*************************************************************************/
int fnet_fs_init( void )
{
    int result = FNET_ERR;
    
    if(fnet_fs_list == 0) /* If no init before. */
    {
        fnet_fs_root_register();
        result = fnet_fs_mount( FNET_FS_ROOT_NAME, "", 0 );
    }
    
    return result;
}

/************************************************************************
* NAME: fnet_fs_release
*
* DESCRIPTION: This function releases FNET FS interface.
*************************************************************************/
void fnet_fs_release( void )
{
    fnet_fs_unmount("");
    fnet_fs_root_unregister();
    
    /* Clear the rest. */
    fnet_memset_zero( fnet_fs_mount_list, sizeof(struct fnet_fs_mount_point)*FNET_CFG_FS_MOUNT_MAX);
    fnet_memset_zero( fnet_fs_desc_list, sizeof(struct fnet_fs_desc)*FNET_CFG_FS_DESC_MAX);
    fnet_fs_list = 0;
}

/************************************************************************
* NAME: fnet_fs_register
*
* DESCRIPTION: This function registers a FS.
*************************************************************************/
void fnet_fs_register( struct fnet_fs *fs )
{
    if(fs)
    {
        fnet_os_mutex_lock();
        fs->_next = fnet_fs_list;

        if(fs->_next != 0)
            fs->_next->_prev = fs;

        fs->_prev = 0;
        fnet_fs_list = fs;
        fnet_os_mutex_unlock();
    }
}

/************************************************************************
* NAME: fnet_fs_unregister
*
* DESCRIPTION: This function unregisters a FS.
*************************************************************************/
void fnet_fs_unregister( struct fnet_fs *fs )
{
    if(fs)
    {
        fnet_os_mutex_lock();

        if(fs->_prev == 0)
            fnet_fs_list = fs->_next;
        else
            fs->_prev->_next = fs->_next;

        if(fs->_next != 0)
            fs->_next->_prev = fs->_prev;

        fnet_os_mutex_unlock();
    }
}

/************************************************************************
* NAME: fnet_fs_find_name
*
* DESCRIPTION: Returns a FS given its name.
*************************************************************************/
struct fnet_fs * fnet_fs_find_name( char *name )
{
    struct fnet_fs *fs;
    struct fnet_fs *result = 0;

    fnet_os_mutex_lock();

    if(name)
        for (fs = fnet_fs_list; fs != 0; fs = fs->_next)
        {
            if(fnet_strcmp(name, fs->name) == 0)
            {
                result = fs;
                break;
            }
        }

    fnet_os_mutex_unlock();
    return result;
}

/************************************************************************
* NAME: fnet_fs_mount
*
* DESCRIPTION: Mounts a FS.
*************************************************************************/
int fnet_fs_mount( char *fs_name, const char *mount_name, void *arg )
{
    int result = FNET_ERR;
    struct fnet_fs_mount_point *mount_point = 0;
    struct fnet_fs * fs;
    int i;
        
    if(fs_name && mount_name)
    {
        fnet_os_mutex_lock();
        fs = fnet_fs_find_name(fs_name);
        if(fs)
        {
            for(i=0; i< FNET_CFG_FS_MOUNT_MAX; i++)
            {
                if(fnet_fs_mount_list[i].fs == 0)
                {
                    mount_point = &fnet_fs_mount_list[i]; /* found free mount point */
                    break;
                }
            }
            
            if(mount_point) 
            {
                if(fs->operations && fs->operations->mount)
                    result = fs->operations->mount(arg);
                else
                    result = FNET_OK;
                
                if(result == FNET_OK) /* Mount is successful */
                {
                    mount_point->arg = arg; /* Fill mount info structure. */
                    mount_point->fs = fs;
                    fnet_strncpy( mount_point->name, mount_name, FNET_CFG_FS_MOUNT_NAME_MAX+1 );    
                }  
            }
        }
        fnet_os_mutex_unlock();
    }  
    
    return result; 
}

/****************************************************************/
int fnet_fs_path_cmp( const char **path, const char *name)
{
    int result;
    
    /* No checks for 0 */
    char *s1p = (char *)*path;
    char *s2p = (char *)name;

    while (*s1p == ' ') s1p++;	            /* Strip leading spaces */
	while (*s1p == FNET_FS_SPLITTER) s1p++;	/* Strip heading slash */

    while(*s2p != '\0')
    {
        if(*s1p != *s2p)
            break;

        ++s1p;
        ++s2p;
        
        if (*s1p == FNET_FS_SPLITTER)
            break; /* next path element */
    }
    
    if(*s1p == FNET_FS_SPLITTER)
    {
        result = 0;
    }
    else
    {
        result = (*s1p - *s2p);
    }
   
    if(result == 0) /* Save end of compare */ 
       *path = s1p; 

    return result;
}

/************************************************************************
* NAME: fnet_fs_find_mount
*
* DESCRIPTION: Find mount point given its name and remove mount name from 
*               the path.
*************************************************************************/
static struct fnet_fs_mount_point * fnet_fs_find_mount( const char **name )
{
    struct fnet_fs_mount_point *result = 0;
    struct fnet_fs_mount_point *tmp;
    int i;

    if(name && *name)
        for(i=0; i<FNET_CFG_FS_MOUNT_MAX; i++)
        {
            tmp = &fnet_fs_mount_list[i];
            if(tmp->fs && fnet_fs_path_cmp(name, tmp->name) == 0)
            {
                result = tmp;
                break;
            }
        }

    return result;
}

/************************************************************************
* NAME: fnet_fs_unmount
*
* DESCRIPTION: Unmounts a FS.
*************************************************************************/
int fnet_fs_unmount( const char *mount_name )
{
    int result = FNET_ERR;
    struct fnet_fs_mount_point *mount_point;
    struct fnet_fs * fs;
        
    if(mount_name)
    {
        fnet_os_mutex_lock();
        mount_point = fnet_fs_find_mount(&mount_name);
        if(mount_point)
        {
            fs = mount_point->fs;
            
            if(fs && fs->operations && fs->operations->unmount )
            {
                fs->operations->unmount(mount_point->arg);    
            }
            
            fnet_memset_zero( mount_point, sizeof(struct fnet_fs_mount_point) );
            result = FNET_OK;
        }
        fnet_os_mutex_unlock();
    }  
    
    return result; 
}

/************************************************************************
* NAME: fnet_fs_opendir
*
* DESCRIPTION: Open DIR stream.
*************************************************************************/
FNET_FS_DIR fnet_fs_opendir( const char *dirname)
{
    FNET_FS_DIR result = 0;
    int i; 
    struct fnet_fs_desc *dir = 0;
    struct fnet_fs_mount_point *mount_point;

    if(dirname)
    {
        fnet_os_mutex_lock();
        for(i=0; i < FNET_CFG_FS_DESC_MAX; i++) /* Free descriptor? */
        {
            if(fnet_fs_desc_list[i].id == 0)
            {
                    dir = &fnet_fs_desc_list[i]; /* found free DIR descriptor */
                    break;
            }
        }
        
        if(dir) /* Found free descriptor. */
        {
            mount_point = fnet_fs_find_mount(&dirname);
            if(mount_point && mount_point->fs && mount_point->fs->dir_operations
               && mount_point->fs->dir_operations->opendir)
            {
                dir->mount = mount_point;
                if(mount_point->fs->dir_operations->opendir(dir, dirname) == FNET_OK)
                {
                   result = dir;
                }
                else
                {
                    fnet_memset_zero( dir, sizeof(struct fnet_fs_desc) ); /* clear dir structure */  
                }
            }
	    }
	    fnet_os_mutex_unlock();	
	
	}
	return result;
}

/************************************************************************
* NAME: fnet_fs_closedir
*
* DESCRIPTION: Close DIR stream.
*************************************************************************/
int fnet_fs_closedir( FNET_FS_DIR dir)
{
    int result = FNET_ERR;
    struct fnet_fs_desc *dirp = (struct fnet_fs_desc *)dir;

    if(dirp)
    {
        fnet_os_mutex_lock();
        fnet_memset_zero( dirp, sizeof(struct fnet_fs_desc) ); /* clear dir structure */
        fnet_os_mutex_unlock();	
        result = FNET_OK;	
	}
	return result;
}


/************************************************************************
* NAME: fnet_fs_readdir
*
* DESCRIPTION: Returns a pointer to a structure representing the directory 
* entry. 
*************************************************************************/
int fnet_fs_readdir(FNET_FS_DIR dir, struct fnet_fs_dirent* dirent)
{
    int result = FNET_ERR;
    struct fnet_fs_desc * dirp = (struct fnet_fs_desc *) dir;
    
    if(dirp)
    {
        fnet_os_mutex_lock();
        if(dirp->mount && dirp->mount->fs 
            && dirp->mount->fs->dir_operations
            && dirp->mount->fs->dir_operations->readdir)
        {
            result = dirp->mount->fs->dir_operations->readdir(dirp, dirent);
        }
        fnet_os_mutex_unlock();	
    }
    return result;   
}

/************************************************************************
* NAME: fnet_fs_rewinddir
*
* DESCRIPTION: Resets the position of the directory stream.
*************************************************************************/
void fnet_fs_rewinddir( FNET_FS_DIR dir )
{
    struct fnet_fs_desc * dirp = (struct fnet_fs_desc *) dir;
    
    if(dirp)
    {
        fnet_os_mutex_lock();
        /* Reset current index. */
        dirp->pos = 0; 
        fnet_os_mutex_unlock();	
    }
}

/************************************************************************
* NAME: fnet_fs_fopen
*
* DESCRIPTION: Opens the specified file.
*************************************************************************/
FNET_FS_FILE fnet_fs_fopen(const char *filename, const char *mode)
{
    return fnet_fs_fopen_re(filename, mode, 0 );
}

/************************************************************************
* NAME: fnet_fs_fopen
*
* DESCRIPTION: Opens the specified file relative to.
*************************************************************************/
FNET_FS_FILE fnet_fs_fopen_re(const char *filename, const char *mode, FNET_FS_FILE dir )
{
    char mode_in = 0;
    FNET_FS_FILE result = 0;
    int i;
    struct fnet_fs_desc *file = 0;
    struct fnet_fs_mount_point *mount_point;
    struct fnet_fs_desc * cur_dir = (struct fnet_fs_desc *) dir;    
    
    if(filename && mode)
    {
        /* Parse the file mode. */
        
        switch(*mode)
        {
            case 'r':
                mode_in = FNET_FS_MODE_READ|FNET_FS_MODE_OPEN_EXISTING;
                break;
            case 'w':
                mode_in = FNET_FS_MODE_WRITE|FNET_FS_MODE_OPEN_ALWAYS|FNET_FS_MODE_OPEN_TRUNC;
                break;
            case 'a':
                mode_in = FNET_FS_MODE_WRITE|FNET_FS_MODE_OPEN_ALWAYS|FNET_FS_MODE_END;
                break;
            default:
                break;                
        }
        
        if(mode_in)         /* Is mode correct? */
        {
            mode++;        
            
            if(*mode == 'b')    /* Text and binary is the same for us. */
                mode++;
            
            if(*mode == '+')
                mode_in |= FNET_FS_MODE_READ|FNET_FS_MODE_WRITE;
        


            fnet_os_mutex_lock();
            for(i=0; i < FNET_CFG_FS_DESC_MAX; i++) /* Free descriptor? */
            {
                if(fnet_fs_desc_list[i].id == 0)
                {
                    file = &fnet_fs_desc_list[i]; /* found free DIR descriptor */
                    break;
                }
            }
        
            if(file) /* Found free descriptor. */
            {
                if(cur_dir)
                    mount_point = cur_dir->mount;
                else
                    mount_point = fnet_fs_find_mount(&filename);
                    
                if(mount_point && mount_point->fs && mount_point->fs->file_operations
                    && mount_point->fs->file_operations->fopen)
                {
                    file->mount = mount_point;
                    if(mount_point->fs->file_operations->fopen(file, filename, mode_in, cur_dir) == FNET_OK)
                    {
                        result = file;
                    }
                    else
                    {
                        fnet_memset_zero( file, sizeof(struct fnet_fs_desc) ); /* clear file structure */  
                    }
                }
	        }
	        fnet_os_mutex_unlock();    
        }
    }

    return result;
}

/************************************************************************
* NAME: fnet_fs_closedir
*
* DESCRIPTION: Close DIR stream.
*************************************************************************/
int fnet_fs_fclose( FNET_FS_FILE file)
{
    int result = FNET_ERR;
    struct fnet_fs_desc *filep = (struct fnet_fs_desc *)file;

    if(filep)
    {
        fnet_os_mutex_lock();
        fnet_memset_zero( filep, sizeof(struct fnet_fs_desc) ); /* clear file structure */
        fnet_os_mutex_unlock();	
        result = FNET_OK;	
	}
	return result;
}

/************************************************************************
* NAME: fnet_fs_rewind
*
* DESCRIPTION: Resets the position of the file stream.
*************************************************************************/
void fnet_fs_rewind(FNET_FS_FILE file)
{
    struct fnet_fs_desc * filep = (struct fnet_fs_desc *) file;
    
    if(filep)
    {
        fnet_os_mutex_lock();
        /* Reset current pos. */
        filep->pos = 0; 
        fnet_os_mutex_unlock();	
    }
}

/************************************************************************
* NAME: get_fs_fread
*
* DESCRIPTION: 
*************************************************************************/
unsigned long fnet_fs_fread(void * buf, unsigned long size, FNET_FS_FILE file)
{
    unsigned long result = 0;
    struct fnet_fs_desc * filep = (struct fnet_fs_desc *) file;
    unsigned long bytes = size;
    
    if(filep && bytes && buf)
    {
        fnet_os_mutex_lock();
        if(filep->mount && filep->mount->fs 
            && filep->mount->fs->file_operations
            && filep->mount->fs->file_operations->fread)
        {
            result = filep->mount->fs->file_operations->fread(filep, buf, bytes);    
        }
        fnet_os_mutex_unlock();	
    }
    return result;   
}

/************************************************************************
* NAME: fnet_fs_feof
*
* DESCRIPTION: A nonzero value signifies that the end of the file has been 
*               reached; a value of zero signifies that it has not.
* In virtually all cases, there's no need to use feof at all. 
*(feof, or more likely ferror, may be useful after a stdio call has 
* returned EOF or NULL, to distinguish between an end-of-file condition 
* and a read error.
*************************************************************************/
int fnet_fs_feof(FNET_FS_FILE file)
{
    int result = 0;
    struct fnet_fs_desc * filep = (struct fnet_fs_desc *) file;
    
    if(filep && (filep->pos == (unsigned long)FNET_FS_EOF))
    {
        result = FNET_FS_EOF;
    }
    return result;   
}

/************************************************************************
* NAME: fnet_fs_fgetc
*
* DESCRIPTION: If successful, fgetc returns the next byte or character from the stream 
*  If unsuccessful, fgetc returns EOF.
*************************************************************************/
int fnet_fs_fgetc(FNET_FS_FILE file)
{
    int result = FNET_FS_EOF;
    struct fnet_fs_desc * filep = (struct fnet_fs_desc *) file;
    char buf;
    
    if(filep)
    {
        fnet_os_mutex_lock();
        if(filep->mount && filep->mount->fs 
            && filep->mount->fs->file_operations
            && filep->mount->fs->file_operations->fread)
        {
            if(filep->mount->fs->file_operations->fread(filep, &buf, 1) != 0)
                result = (unsigned char)buf;
        }
        fnet_os_mutex_unlock();	
    }
    return result;   
}

/************************************************************************
* NAME: fnet_fs_fseek
*
* DESCRIPTION: Change the file position indicator for the specified file.
*************************************************************************/
int fnet_fs_fseek (FNET_FS_FILE file, long offset, int origin)
{
    int result = FNET_ERR;
    struct fnet_fs_desc * filep = (struct fnet_fs_desc *) file;
    
    if(filep)
    {
        fnet_os_mutex_lock();
        if(filep->mount && filep->mount->fs 
            && filep->mount->fs->file_operations
            && filep->mount->fs->file_operations->fseek)
        {
            result = filep->mount->fs->file_operations->fseek(filep, offset, (fnet_fs_seek_origin_t)origin);    
        }
        fnet_os_mutex_unlock();	
    }
    return result;  
}

/************************************************************************
* NAME: fnet_fs_ftell
*
* DESCRIPTION: Returns the current offset in a stream in relation to the first byte.
*************************************************************************/
long fnet_fs_ftell (FNET_FS_FILE file)
{
    int result = FNET_ERR;
    struct fnet_fs_desc * filep = (struct fnet_fs_desc *) file;
    
    if(filep)
    {
        result = (long) filep->pos; 
    }
    return result;  
}

/************************************************************************
* NAME: fnet_fs_finfo
*
* DESCRIPTION: Reads file info data.
*************************************************************************/
int fnet_fs_finfo (FNET_FS_FILE file, struct fnet_fs_dirent *info)
{
    int result = FNET_ERR;
    struct fnet_fs_desc *filep = (struct fnet_fs_desc *) file;
    
    if(filep)
    {
        fnet_os_mutex_lock();
        if(filep->mount && filep->mount->fs 
            && filep->mount->fs->file_operations
            && filep->mount->fs->file_operations->finfo)
        {
            result = filep->mount->fs->file_operations->finfo(filep, info);    
        }
        fnet_os_mutex_unlock();	
    }
    return result;
}

#endif /* FNET_CFG_FS */


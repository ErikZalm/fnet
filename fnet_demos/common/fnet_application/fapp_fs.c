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
* @file fapp_fs.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.23.0
*
* @brief FNET Shell Demo implementation.
*
***************************************************************************/

#include "fapp.h"
#include "fapp_fs.h"
#include "fapp_prv.h"


#if FAPP_CFG_EXP_CMD || FAPP_CFG_HTTP_CMD

/************************************************************************
*     File System Image
*************************************************************************/
extern const struct fnet_fs_rom_image fnet_fs_image;


/************************************************************************
*     Definitions.
*************************************************************************/
#define FAPP_FS_FSMOUNT_ERR  ("Error: FS mount is failed!\n")
#define FAPP_FS_FSINIT_ERR   ("Error: FS init is failed!\n")

#endif

#if FAPP_CFG_EXP_CMD && FNET_CFG_FS

/* Error mesages */
#define FAPP_FS_DIR_ERR      ("Error: DIR is failed!")
#define FAPP_FS_CD_ERR       ("Error: CD %s is failed!")
#define FAPP_FS_VIEW_ERR     ("Error: VIEW %s is failed!")

#define FAPP_FS_DIR_STR      "%7s %6d Bytes  -  %s"

/************************************************************************
*     Function Prototypes
*************************************************************************/
void fapp_fs_init( fnet_shell_desc_t desc );
void fapp_fs_dir_cmd( fnet_shell_desc_t desc, int argc, char ** argv );
void fapp_fs_cd_cmd( fnet_shell_desc_t desc, int argc, char ** argv );
void fapp_fs_view_cmd( fnet_shell_desc_t desc, int argc, char ** argv );


/************************************************************************
*     File Explorer definitions.
*************************************************************************/
/* maximum path length */ 
#define FAPP_FS_DIR_PATH_MAX     (50) 

/* Explorer shell prompt */
static const char FAPP_FS_PROMPT_STR_HEADER[]="EXP:";
static const char FAPP_FS_PROMPT_STR_TRAILER[]="> ";
static char FAPP_FS_PROMPT_STR [FAPP_FS_DIR_PATH_MAX+
                                   sizeof(FAPP_FS_PROMPT_STR_HEADER)+
                                   sizeof(FAPP_FS_PROMPT_STR_TRAILER)];


/* Current path */
static char fapp_fs_current_path[FAPP_FS_DIR_PATH_MAX+1] = {FNET_FS_SPLITTER,'\0'};

/************************************************************************
*     The table of the File Explorer commands.
*************************************************************************/
static const struct fnet_shell_command fapp_fs_cmd_tab [] =
{
    { FNET_SHELL_CMD_TYPE_NORMAL, "help", 0, 0, (void *)fapp_help_cmd,  "Display this help message.", ""},
    { FNET_SHELL_CMD_TYPE_NORMAL, "dir", 0, 0, (void *)fapp_fs_dir_cmd,     "Display a list of files & directories.", ""},
    { FNET_SHELL_CMD_TYPE_NORMAL, "cd", 1, 1, (void *)fapp_fs_cd_cmd,       "Change the current directory.", "<directory>"},
    { FNET_SHELL_CMD_TYPE_NORMAL, "view", 1, 1, (void *)fapp_fs_view_cmd,   "View a text file.", "<file>"},  
    { FNET_SHELL_CMD_TYPE_QUIT, "exit", 0, 0, 0,    "Exit from the file explorer.", ""},
    { FNET_SHELL_CMD_TYPE_END, 0, 0, 0, 0, 0, 0}    
};



/************************************************************************
*     The File Explorer control data structure.
*************************************************************************/
struct fnet_shell fapp_fs_shell =
{
  fapp_fs_cmd_tab,                   /* cmd_table */
  FAPP_FS_PROMPT_STR,                /* prompt_str */
  fapp_fs_init,                      /* shell_init */
};

/************************************************************************
* NAME: fapp_fs_init
*
* DESCRIPTION: File Explorer initialization function.
************************************************************************/
void fapp_fs_init( fnet_shell_desc_t desc )
{
    /* Format exp shell prompt */
    fnet_snprintf( FAPP_FS_PROMPT_STR, sizeof(FAPP_FS_PROMPT_STR), "%s%s%s",
                  &FAPP_FS_PROMPT_STR_HEADER[0],
                  &fapp_fs_current_path[0],
                  &FAPP_FS_PROMPT_STR_TRAILER[0]);
        
    fnet_shell_println(desc, "\n%s", FAPP_DELIMITER_STR);
    fnet_shell_println(desc, "  File Explorer started.");
    fnet_shell_println(desc, "  Enter 'help' for command list.");
    fnet_shell_println(desc, "%s\n", FAPP_DELIMITER_STR);
}


/************************************************************************
* NAME: fapp_fs_dir_cmd
*
* DESCRIPTION: Displays a list of files and subdirectories in a directory.
*************************************************************************/
void fapp_fs_dir_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    struct fnet_fs_dirent ep;
    FNET_FS_DIR dir;
    FNET_COMP_UNUSED_ARG(argc);
    FNET_COMP_UNUSED_ARG(argv);

    /* Open dir */
    dir = fnet_fs_opendir(fapp_fs_current_path);
 
    if (dir)
    {
        /* Print the dir content. */
        while ((fnet_fs_readdir (dir, &ep))==FNET_OK)
            fnet_shell_println(desc, FAPP_FS_DIR_STR, (ep.d_type == DT_DIR)?"<DIR>":"<FILE>", ep.d_size, ep.d_name);
            
        /* Close dir */
        fnet_fs_closedir(dir);    
    }
    else
        fnet_shell_println(desc, FAPP_FS_DIR_ERR);
}


/************************************************************************
* NAME: fapp_fs_cd_cmd
*
* DESCRIPTION: Change the current directory.
*************************************************************************/
void fapp_fs_cd_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    FNET_FS_DIR dir;
    char * path = argv[1];
    char * path_end;
    unsigned long size_cd = fnet_strlen (fapp_fs_current_path);
    unsigned long size_path;
    char splitter[] = {FNET_FS_SPLITTER,'\0'};    

    FNET_COMP_UNUSED_ARG(argc);
        
	if (*path != FNET_FS_SPLITTER) /* Realative path.*/
	{
	    /* Add splitter if not yet.*/
	    if(fapp_fs_current_path[size_cd-1] != FNET_FS_SPLITTER) 
	        fnet_strncat( &fapp_fs_current_path[0], splitter, FAPP_FS_DIR_PATH_MAX);
	        
	    fnet_strncat( &fapp_fs_current_path[0], path, FAPP_FS_DIR_PATH_MAX);
	    path = fapp_fs_current_path; 
	}    
    else /* Full path. */
    {
        /* Strip possible repetitive leading slashes. */
        while ((path[0] == FNET_FS_SPLITTER) && (path[1] == FNET_FS_SPLITTER)) 
            path++;	        
    }
    
    /* Strip possible ending slashes. */
    if((size_path = fnet_strlen(path))>0)
    {
        path_end = &path[size_path-1]; 
        while(*path_end == FNET_FS_SPLITTER)
        {
            *path_end = '\0';
            path_end--;	        
        }
    }
    
    /* Open dir. */
    dir = fnet_fs_opendir(path);
    
    if (dir)
    {
        /* Update cur path. */
        fnet_strncpy( &fapp_fs_current_path[0], path, FAPP_FS_DIR_PATH_MAX);
        if(fapp_fs_current_path[0] == '\0') /* root dir */
            fnet_strncat( &fapp_fs_current_path[0], splitter, FAPP_FS_DIR_PATH_MAX);
    
        /* Change shell prompt. */
        fnet_sprintf( FAPP_FS_PROMPT_STR, "%s%s%s", 
                  &FAPP_FS_PROMPT_STR_HEADER[0],
                  &fapp_fs_current_path[0],
                  &FAPP_FS_PROMPT_STR_TRAILER[0]);
                  
        /* Close dir. */    
        fnet_fs_closedir(dir);                  
    }
    else
    {
        /* Restore cur path. */
        fapp_fs_current_path[size_cd] = '\0'; 
        fnet_shell_println(desc, FAPP_FS_CD_ERR, argv[1]);
    }
                 
}

/************************************************************************
* NAME: fapp_fs_view_cmd
*
* DESCRIPTION: 
*************************************************************************/
void fapp_fs_view_cmd( fnet_shell_desc_t desc, int argc, char ** argv )
{
    FNET_FS_FILE file;
    char * path = argv[1];
    char * path_end;
    unsigned long size_cd = fnet_strlen (fapp_fs_current_path);
    unsigned long size_path;
    char splitter[] = {FNET_FS_SPLITTER,'\0'};
    char data;
    struct fnet_fs_dirent dirent;    

    FNET_COMP_UNUSED_ARG(desc);
    FNET_COMP_UNUSED_ARG(argc);
        
	if (*path != FNET_FS_SPLITTER) /* Realative path.*/
	{
	    /* Add splitter if not yet.*/
	    if(fapp_fs_current_path[size_cd-1] != FNET_FS_SPLITTER) 
	        fnet_strncat( &fapp_fs_current_path[0], splitter, FAPP_FS_DIR_PATH_MAX);
	        
	    fnet_strncat( &fapp_fs_current_path[0], path, FAPP_FS_DIR_PATH_MAX);
	    path = fapp_fs_current_path; 
	}    
    else /* Full path. */
    {
        /* Strip possible repetitive leading slashes. */
        while ((path[0] == FNET_FS_SPLITTER) && (path[1] == FNET_FS_SPLITTER)) 
            path++;	        
    }
    
    /* Strip possible ending slashes. */
    if((size_path = fnet_strlen(path))>0)
    {
        path_end = &path[size_path-1]; 
        while(*path_end == FNET_FS_SPLITTER)
        {
            *path_end = '\0';
            path_end--;	        
        }
    }
    
    /* Open file. */
    file = fnet_fs_fopen(path,"r");
    
    if (file)
    {
        /* Print file info.*/
        fnet_fs_finfo (file, &dirent);
        
        fnet_shell_println(desc, FAPP_FS_DIR_STR, "Content of:", dirent.d_size, dirent.d_name);
        
        while(fnet_fs_fread(&data, sizeof(data), file))
            fnet_shell_putchar(desc, data);
                  
        /* Close file. */    
        fnet_fs_fclose(file);                  
    }
    else
    {
        fnet_shell_println(desc, FAPP_FS_VIEW_ERR, argv[1]);
    }
    
    /* Restore cur path. */
    fapp_fs_current_path[size_cd] = '\0'; 
                 
}
#endif /* FAPP_CFG_EXP_CMD */



#if (FAPP_CFG_EXP_CMD || FAPP_CFG_HTTP_CMD) && FNET_CFG_FS
/************************************************************************
* NAME: fapp_fs_mount
*
* DESCRIPTION: Mount FS image.
************************************************************************/
void fapp_fs_mount()
{
    if( fnet_fs_init( ) == FNET_OK)
    {
        /* Register the FNET ROM FS. */
        fnet_fs_rom_register( );
 
        /* Mount the FNET ROM FS image. */
        if( fnet_fs_mount( FNET_FS_ROM_NAME, FAPP_FS_MOUNT_NAME, (void *)&fnet_fs_image ) == FNET_ERR )
            fnet_printf(FAPP_FS_FSMOUNT_ERR);
    }
    else
    {
        fnet_printf(FAPP_FS_FSINIT_ERR);
    } 
}
/************************************************************************
* NAME: fapp_fs_unmount
*
* DESCRIPTION: Unmount FS image.
************************************************************************/
void fapp_fs_unmount()
{
    fnet_fs_rom_unregister();   /* Unregister ROM FS. */
    fnet_fs_release();          /* Release FS. */ 
}

#endif 


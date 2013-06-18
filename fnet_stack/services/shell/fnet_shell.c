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
* @file fnet_shell.c
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.34.0
*
* @brief Shell service implementation.
*
***************************************************************************/

#include "fnet.h"
#include "fnet_shell.h"
#include "fnet_serial.h"

#define FNET_SHELL_ERR_SYNTAX   ("Error: Invalid syntax for: %s")
#define FNET_SHELL_ERR_CMD      ("Error: No such command: %s")

#define FNET_SHELL_BACKSPACE    (0x08)  /* Backspace. */
#define FNET_SHELL_DELETE       (0x7F)  /* Delete. */
#define FNET_SHELL_CTRLC        (0x03)  /* Ctrl + C. */
#define FNET_SHELL_CR           (0x0D)  /* CR. */
#define FNET_SHELL_LF           (0x0A)  /* LF. */
#define FNET_SHELL_ESC          (0x1B)  /* Esc. */
#define FNET_SHELL_SPACE        (0x20)  /* Space. */

#if FNET_CFG_DEBUG_SHELL  
    #define FNET_DEBUG_SHELL   FNET_DEBUG
#else
    #define FNET_DEBUG_SHELL(...)
#endif

#if FNET_CFG_DEBUG_MEMPOOL  
    #include "fnet_netbuf.h"  
    #define FNET_DEBUG_MEMPOOL   FNET_DEBUG
#else
    #define FNET_DEBUG_MEMPOOL(...)
#endif

#if FNET_CFG_DEBUG_STACK  
    #define FNET_DEBUG_STACK   FNET_DEBUG
    extern unsigned long fnet_dbg_stack_max;
#else
    #define FNET_DEBUG_STACK(...)
#endif

int fnet_shell_ctrlc (fnet_shell_desc_t desc);


/************************************************************************
*    Shell interface control structure.
*************************************************************************/
struct fnet_shell_if
{
    
    const struct fnet_shell *shell;
    const struct fnet_shell *top_shell;         /* Pointer to the top shell. */
    fnet_shell_state_t state;                   /* Current state.*/
    fnet_poll_desc_t service_descriptor;        /* Descriptor of polling service.*/
    int pos; 
    char *cmd_line_begin;
    char *cmd_line_end;
    char *cmd_line;                             /* Command line buffer.*/
    unsigned int cmd_line_size;
    
    char _blocked;                              /* Flag that current command is blocked. */
    void (*_exit_blocked)(fnet_shell_desc_t shl_desc);/* Pointer to the callback function, 
                                                 * occurring on exit from command blocked 
                                                 * state. It happens when a user press
                                                 * [Ctrl+C] button all fnet_shell_unblock() called.
                                                 */
    fnet_serial_stream_t stream;
    int echo;   
};

/* The Shell interface structure list */
static struct fnet_shell_if shell_if_list[FNET_CFG_SHELL_MAX];


static void fnet_shell_echo( struct fnet_shell_if *shell_if, int character );
static int fnet_shell_make_argv( char *cmdline, char *argv [] );
static void fnet_shell_state_machine( void *shell_if_p );
static void fnet_shell_esc_clear(char * str);


/************************************************************************
* NAME: fnet_shell_echo
*
* DESCRIPTION:
************************************************************************/
static void fnet_shell_echo( struct fnet_shell_if *shell_if, int character )
{
    if(shell_if->echo)  
        fnet_serial_putchar(shell_if->stream, character);  
}

/************************************************************************
* NAME: fnet_shell_state_machine
*
* DESCRIPTION: Shell state machine.
************************************************************************/
static void fnet_shell_state_machine( void *shell_if_p )
{
    struct fnet_shell_if * shell_if = (struct fnet_shell_if *)shell_if_p;
    const struct fnet_shell *shell = ((struct fnet_shell_if *)shell_if_p)->shell;

    int ch;
    int argc;
    char *argv[FNET_CFG_SHELL_ARGS_MAX + 1]; /* One extra for 0 terminator.*/
   
    switch(shell_if->state)
    {

        case FNET_SHELL_STATE_INIT:
            /* Print the shell prompt. */
            fnet_shell_printf((fnet_shell_desc_t)shell_if_p, "%s", shell->prompt_str);
                
                
            /* Debug: Prints mempool free memery (max posible allocated chunk) on every enter. */
            FNET_DEBUG_MEMPOOL("MAIN pool = %d (%d), FNET pool = %d (%d)", fnet_free_mem_status(), fnet_malloc_max(), fnet_free_mem_status_netbuf(), fnet_malloc_max_netbuf());
            /* Debug: Prints maximum stack usage. */
            FNET_DEBUG_STACK("Max Stack usage = %d", fnet_dbg_stack_max);

            shell_if->state = FNET_SHELL_STATE_GET_USER_INPUT;
        /*-------------------------------------*/    
        case FNET_SHELL_STATE_GET_USER_INPUT:
            if( (ch = fnet_serial_getchar(shell_if->stream)) != FNET_ERR) 
            {
                /* CR or Buffer is full. */
                if((ch != FNET_SHELL_CR) && (shell_if->pos < shell_if->cmd_line_size))
                {
                    switch(ch)
                    {
                        case FNET_SHELL_BACKSPACE:
                        case FNET_SHELL_DELETE: 
                            if(shell_if->pos > 0)
                            {
                                shell_if->pos -= 1;
                                fnet_shell_echo(shell_if, FNET_SHELL_BACKSPACE);
                                fnet_serial_putchar(shell_if->stream, ' ');
                                fnet_serial_putchar(shell_if->stream, FNET_SHELL_BACKSPACE);
                            }
                            break;
                        default:
                            if((shell_if->pos + 1) < shell_if->cmd_line_size)
                            {
                                /* Only printable characters. */
                                if((ch >= FNET_SHELL_SPACE) && (ch <= FNET_SHELL_DELETE))
                                {
                                    shell_if->cmd_line[shell_if->pos++] = (char)ch;
                                    fnet_shell_echo(shell_if, ch);
                                }
                            }
                            break;
                    }
                }
                else
                {
                    shell_if->cmd_line[shell_if->pos] = '\0';
                 
                    fnet_shell_echo(shell_if, FNET_SHELL_CR);
                    fnet_shell_echo(shell_if, FNET_SHELL_LF);
                        
                    shell_if->state = FNET_SHELL_STATE_EXEC_CMD;
                    /* Reset pointers */
                    shell_if->cmd_line_begin = shell_if->cmd_line;
                    shell_if->cmd_line_end = shell_if->cmd_line;
                    }
            }
            break;
        /*-------------------------------------------*/    
        case FNET_SHELL_STATE_EXEC_CMD:
  
            shell_if->cmd_line_begin = shell_if->cmd_line_end; 
                   
            do
            {
                if(*shell_if->cmd_line_end!= '\0')
                    shell_if->cmd_line_end ++;
                            
                shell_if->cmd_line_end = fnet_strchr( shell_if->cmd_line_end, FNET_SHELL_COMMAND_SPLITTER );
                        
                if((shell_if->cmd_line_end != 0) &&(shell_if->cmd_line_begin != shell_if->cmd_line_end) && (shell_if->cmd_line_end[-1] != FNET_SHELL_ESCAPE_SYMBOL)) /* Found new command symbol.*/
                {
                    *shell_if->cmd_line_end++ = '\0'; /* Set of end of line */
                        
                }
            }
            while(shell_if->cmd_line_end && (shell_if->cmd_line_end[-1] != '\0'));
                    
                    
            fnet_strncpy(shell_if->cmd_line, shell_if->cmd_line_begin, shell_if->cmd_line_size);

            argc = fnet_shell_make_argv(shell_if->cmd_line, argv);

            if(argc)
            {
                const struct fnet_shell_command *cur_command = shell->cmd_table;

                while(cur_command->type)
                {
                    if(fnet_strcasecmp(cur_command->name, argv[0]) == 0) /* Command is found. */
                    {
                        if(((argc - 1) >= cur_command->min_args)
                                       && ((argc - 1) <= cur_command->max_args))
                        {
                            /* Shell. */
                            if(cur_command->type == FNET_SHELL_CMD_TYPE_SHELL) /* Command. */
                            {
                                shell_if->cmd_line[0] = '\0'; /* Clear command line. */
                                        
                                shell = (struct fnet_shell *)cur_command->cmd_ptr;

                                shell_if->shell = shell; /* Update current shell pointer. */

                                if(shell->shell_init)
                                    shell->shell_init((fnet_shell_desc_t)shell_if_p);

                                cur_command = shell->cmd_table;/* => to avoid wrong command message. */
                            }
                            else /* Command. */
                            {
                                if(cur_command->cmd_ptr)
                                            ((void(*)(fnet_shell_desc_t desc, int, char **))(cur_command->cmd_ptr))((fnet_shell_desc_t)shell_if, argc, argv);
                                
                                /* Check if the shell was released during command execution.*/
                                if(shell_if->state == FNET_SHELL_STATE_DISABLED)
                                    return;
                                    
                                /* If user wants to exit from current shell.*/
                                if(cur_command->type == FNET_SHELL_CMD_TYPE_QUIT)
                                {
                                    shell_if->cmd_line[0] = '\0';
                                    shell_if->shell = shell_if->top_shell;
                                }
                            }
                        }
                        else /* Wrong command syntax. */
                        {
                                fnet_shell_println((fnet_shell_desc_t)shell_if_p, FNET_SHELL_ERR_SYNTAX, argv[0]);
                        }
                                
                        break;
                    }
                    cur_command++; 
                }

                if(cur_command->type == 0)
                {
                    fnet_shell_println((fnet_shell_desc_t)shell_if_p, FNET_SHELL_ERR_CMD, argv[0]);
                }

            }
             
            if(shell_if->_blocked)
                shell_if->state = FNET_SHELL_STATE_BLOCKED;    
            else if(shell_if->cmd_line_end == 0)
                shell_if->state = FNET_SHELL_STATE_END_CMD;

            break;
        /*----------------------------------*/    
        case FNET_SHELL_STATE_BLOCKED:
            if(shell_if->_blocked)
            {
                if(fnet_shell_ctrlc ((fnet_shell_desc_t)shell_if_p))
                {
                    shell_if->_exit_blocked((fnet_shell_desc_t) shell_if);
                    shell_if->_blocked = 0;    
                }    
            }
            else if(shell_if->cmd_line_end == 0)
                shell_if->state = FNET_SHELL_STATE_END_CMD;
            else
                shell_if->state = FNET_SHELL_STATE_EXEC_CMD;
            break;        
        /*----------------------------------*/    
        case FNET_SHELL_STATE_END_CMD:
            shell_if->state = FNET_SHELL_STATE_INIT;
            shell_if->pos = 0;
            shell_if->cmd_line[0] = 0;
            break;
        default:
            break;           
    }                
}

/************************************************************************
* NAME: fnet_shell_init
*
* DESCRIPTION: 
************************************************************************/
fnet_shell_desc_t fnet_shell_init( struct fnet_shell_params * params)
{
    int i;

    struct fnet_shell_if *shell_if = 0;
    
    /* Check input parameters. */
    if((params == 0) || (params->cmd_line_buffer == 0 ) || (params->cmd_line_buffer_size == 0 ) )
    {
        FNET_DEBUG_SHELL("Shell: Wrong init parameters.");
        goto ERROR;
    }

    /* Try to find free Shell service. */
    for(i=0; i<FNET_CFG_SHELL_MAX; i++)
    {
        if(shell_if_list[i].state == FNET_SHELL_STATE_DISABLED)
        {
            shell_if = &shell_if_list[i];
            break; 
        }
    }

    /* Is Shell already initialized. */
    if(shell_if == 0)
    {
        FNET_DEBUG_SHELL("SHELL: No free Sheell service.");
        goto ERROR;
    }

    shell_if->shell = params->shell;
    shell_if->top_shell = shell_if->shell;
    shell_if->cmd_line = params->cmd_line_buffer;
    shell_if->cmd_line_size = params->cmd_line_buffer_size;
    shell_if->echo = params->echo;
     
    if(params->stream)
        shell_if->stream = params->stream;
    else
        shell_if->stream = FNET_SERIAL_STREAM_DEFAULT;
     
    shell_if->service_descriptor = fnet_poll_service_register(fnet_shell_state_machine, (void *) shell_if);
    if(shell_if->service_descriptor == (fnet_poll_desc_t)FNET_ERR)
    {
        FNET_DEBUG_SHELL("Shell: Service registration error.");
        goto ERROR;
    }
  
     /* Reset parameters. */
    shell_if->state = FNET_SHELL_STATE_INIT;
    shell_if->pos = 0;
    shell_if->cmd_line[0] = 0;

    if(shell_if->shell->shell_init)
        shell_if->shell->shell_init((fnet_shell_desc_t)shell_if); 
    
    return (fnet_shell_desc_t)shell_if;
ERROR:
    return FNET_ERR;     
}

/************************************************************************
* NAME: fnet_shell_release
*
* DESCRIPTION: 
************************************************************************/
void fnet_shell_release(fnet_shell_desc_t desc)
{
    struct fnet_shell_if *shell_if = (struct fnet_shell_if *) desc;
    if(shell_if && (shell_if->state != FNET_SHELL_STATE_DISABLED))
    {
        fnet_poll_service_unregister(shell_if->service_descriptor); /* Delete service.*/
        shell_if->state = FNET_SHELL_STATE_DISABLED;    
    }
}


/************************************************************************
* NAME: fnet_shell_esc_clear
*
* DESCRIPTION: Eliminate escape symbols.
************************************************************************/
static void fnet_shell_esc_clear(char * str)
{
    char * dest = str ;
    char * src = str;

    while(*src != 0)
    {
        if((*src == FNET_SHELL_ESCAPE_SYMBOL) && (src[1] != FNET_SHELL_ESCAPE_SYMBOL))
        {
            src++; 
        }
        else
            *dest++ = *src++;
    }
    *dest= '\0';
}

/************************************************************************
* NAME: fnet_shell_make_argv
*
* DESCRIPTION: Calculates the quantity of arguments and splits them array
*              argv[]
************************************************************************/
static int fnet_shell_make_argv( char *cmdline, char *argv [] )
{
    int argc;
    int i;
    int in_text_flag;
    int qouted;

    /* Break cmdline into strings.*/
    argc = 0;
    in_text_flag = 0;
    qouted = 0;

    if( cmdline && argv)
    { 
    
        for(i=0; cmdline[i] != '\0'; i++)
        {
            if(((qouted == 0) && ((cmdline[i] == ' ') || (cmdline[i] == '\t')))
             ||((qouted == 1) && ((cmdline[i] == FNET_SHELL_QUOTE_SYMBOL)&&((i==0)||(cmdline[i-1] != FNET_SHELL_ESCAPE_SYMBOL))  )   ) )
            {
                if(in_text_flag)
                {
                    /* Set end of command line argument.*/
                    cmdline[i] = '\0';
                    in_text_flag = 0;
                    qouted = 0;
                    fnet_shell_esc_clear(argv[argc-1]); /* Clear escape symbols. */
                }
            }
            else
            {
                /* Got non-whitespace character. */
                if(!in_text_flag)
                {
                    /* Start of an argument. */
                    in_text_flag = 1;

                    if(argc < FNET_CFG_SHELL_ARGS_MAX)
                    {
                        if((cmdline[i] == FNET_SHELL_QUOTE_SYMBOL)&&((i==0)||(cmdline[i-1] != FNET_SHELL_ESCAPE_SYMBOL)))
                            qouted = 1;    

                        argv[argc] = &cmdline[i+qouted];

                        argc++;
                    }
                    else /* Return argc.*/
                        break;
                }
            }
        }

        argv[argc] = 0;
    }
    
    return argc;
}



/************************************************************************
* NAME: fnet_shell_printf
*
* DESCRIPTION:
************************************************************************/
int fnet_shell_printf(fnet_shell_desc_t desc, const char *format, ... )
{
    fnet_va_list ap;
    int result = 0;
    struct fnet_shell_if *shell_if = (struct fnet_shell_if *) desc;
    
    if(shell_if)
    {
        /* Initialize the pointer to the variable length argument list. */
        fnet_va_start(ap, format);
        result = fnet_serial_vprintf(shell_if->stream, format, ap);
        
        fnet_serial_flush(shell_if->stream);
    }
    
    return result;

}

/************************************************************************
* NAME: fnet_shell_println
*
* DESCRIPTION:
************************************************************************/
int fnet_shell_println(fnet_shell_desc_t desc, const char *format, ... )
{
    fnet_va_list ap;
    int result = 0;
    struct fnet_shell_if *shell_if = (struct fnet_shell_if *) desc;
    
    if(shell_if)
    {
        /* Initialize the pointer to the variable length argument list. */
        fnet_va_start(ap, format);
        result = fnet_serial_vprintf(shell_if->stream, format, ap);
        result += fnet_shell_printf(desc, "\n"); /* Add new line.*/
        
        fnet_serial_flush(shell_if->stream);
    }
    
    return result;

}

/************************************************************************
* NAME: fnet_shell_putchar
*
* DESCRIPTION:
************************************************************************/
void fnet_shell_putchar(fnet_shell_desc_t desc, int character)
{
    struct fnet_shell_if *shell_if = (struct fnet_shell_if *) desc;
    
    if(shell_if)
    {
        fnet_serial_putchar(shell_if->stream, character);
        fnet_serial_flush(shell_if->stream);
    }
}

/************************************************************************
* NAME: fnet_shell_getchar
*
* DESCRIPTION:
************************************************************************/
int fnet_shell_getchar(fnet_shell_desc_t desc)
{
    int result;
    struct fnet_shell_if *shell_if = (struct fnet_shell_if *) desc;
    
    if(shell_if)
        result = fnet_serial_getchar(shell_if->stream);
    else
        result = FNET_ERR;
    
    return result;
}

/************************************************************************
* NAME: fnet_shell_script
*
* DESCRIPTION: Executes command-line script.
************************************************************************/
void fnet_shell_script(fnet_shell_desc_t desc, char *script )
{
    struct fnet_shell_if *shell_if = (struct fnet_shell_if *) desc;
    
    if(shell_if)
    {
    
        fnet_strncpy( shell_if->cmd_line, script, shell_if->cmd_line_size );
        shell_if->state = FNET_SHELL_STATE_EXEC_CMD;
        /* Reset pointers */
        shell_if->cmd_line_begin = shell_if->cmd_line;
        shell_if->cmd_line_end = shell_if->cmd_line;
    }                        
}

/************************************************************************
* NAME: fnet_shell_script_release
*
* DESCRIPTION: 
************************************************************************/
void fnet_shell_script_release( fnet_shell_desc_t desc)
{
    struct fnet_shell_if *shell_if = (struct fnet_shell_if *) desc;
    
    if(shell_if)
    {
       fnet_memset_zero(shell_if->cmd_line, shell_if->cmd_line_size );
    }
}

/************************************************************************
* NAME: fnet_shell_help
*
* DESCRIPTION: Shows command shell help.
************************************************************************/
void fnet_shell_help( fnet_shell_desc_t desc)
{
    const struct fnet_shell *shell = ((struct fnet_shell_if *)desc)->shell;
    const struct fnet_shell_command *cur_command = shell->cmd_table;

    while(cur_command->type)
    {
        fnet_shell_println(desc, FNET_CFG_SHELL_HELP_FORMAT,
                        cur_command->name,
                        cur_command->syntax,
                        cur_command->description);
        cur_command++;                 
    }
}

/***************************************************************************/ /*!
 * @internal
 * @brief    Detects if the [Ctrl]+[c] is received.
 *
 *
 * @return This function returns:
 *   - @c 0 if [Ctrl]+[c] is not received/pressed.
 *   - @c 1 if [Ctrl]+[c] is received/pressed.
 * 
 ******************************************************************************
 *
 * This function detects if the [Ctrl]+[c] command is received.@n 
 * It can be used by blocking shell commands to detect that a user wants to 
 * terminate the command activity.
 *
 ******************************************************************************/
int fnet_shell_ctrlc (fnet_shell_desc_t desc)
{
    int                     res;
    struct fnet_shell_if    *shell_if = (struct fnet_shell_if *) desc;
    int                     ch = fnet_serial_getchar(shell_if->stream);
    
    if(ch == FNET_SHELL_CTRLC)
        res = 1;
    else
        res = 0;

    return res;
}

/************************************************************************
* NAME: fnet_shell_block
*
* DESCRIPTION: 
************************************************************************/
int fnet_shell_block( fnet_shell_desc_t desc, void (*on_ctrlc)(fnet_shell_desc_t shl_desc))
{
    struct fnet_shell_if *shell_if = (struct fnet_shell_if *) desc;
    int res;

    if(shell_if && on_ctrlc )
    {
        shell_if->_blocked = 1;
        shell_if->_exit_blocked = on_ctrlc;
        res = FNET_OK;
    }
    else
        res = FNET_ERR;
   
    return res;
}

/************************************************************************
* NAME: fnet_shell_unblock
*
* DESCRIPTION: 
************************************************************************/
void fnet_shell_unblock( fnet_shell_desc_t desc)
{
    struct fnet_shell_if *shell_if = (struct fnet_shell_if *) desc;
    
    if(shell_if)
    {
        shell_if->_blocked = 0;
    }
}



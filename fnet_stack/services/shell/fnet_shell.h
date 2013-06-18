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
**********************************************************************/
/*!
*
* @file fnet_shell.h
*
* @author Andrey Butok
*
* @date Dec-19-2012
*
* @version 0.1.33.0
*
* @brief FNET Shell service API.
*
***************************************************************************/
#ifndef _FNET_SHELL_H_

#define _FNET_SHELL_H_

#include "fnet_config.h"

#include "fnet_serial.h"

/*! @addtogroup fnet_shell
* The shell service provides application-specific command-line interface.@n
* It runs on asigned serial stream, parses the input text, decodes the command 
* name and arguments and calls back user-specified functions assigned to 
* each command. @n
* @n
* After the Shell service is initialized by calling the @ref fnet_shell_init() 
* function, the user application should call the main service polling function  
* @ref fnet_poll_services() periodically in background. @n
* @n
* For example:
* @code
* ...
* // ************************************************************************
* // *     The table of the main shell commands.
* // *************************************************************************
*  static const struct fnet_shell_command fapp_cmd_tab [] =
*  {
*      { FNET_SHELL_CMD_TYPE_NORMAL, "help", 0, 0, fnet_shell_help_cmd,"Display this help message.", ""},
*      { FNET_SHELL_CMD_TYPE_NORMAL, "set", 0, 2, fapp_set_cmd, "Set parameter.", "[<parameter> <value>]"},
*      { FNET_SHELL_CMD_TYPE_NORMAL, "show", 0, 0, fapp_show, "Show parameters.", "" },
*      { FNET_SHELL_CMD_TYPE_NORMAL, "info", 0, 0, fapp_info_cmd, "Show detailed status.", ""},
*      { FNET_SHELL_CMD_TYPE_NORMAL, "dhcp", 0, 1, fapp_dhcp_cmd, "Start DHCP client.", "[release|reboot]"},
*      { FNET_SHELL_CMD_TYPE_NORMAL, "http", 0, 1, fapp_http_cmd, "Start HTTP Server.", "[release]"},
*      { FNET_SHELL_CMD_TYPE_SHELL, "exp", 0, 1, &fapp_fs_shell, "File Explorer submenu...", ""},
*      { 0, 0, 0, 0, 0, 0, 0},
*  };
* // ************************************************************************
* // *     The main shell control data structure.
* // *************************************************************************
* struct fnet_shell fapp_shell =
* {
*   fapp_cmd_tab,                                         
*   "PROMT> ",                                
*   fapp_shell_init,                                              
* };
* fnet_shell_desc_t fapp_shell_desc = 0; // Shell descriptor.
* ...
* ...
* main()
* {
*   struct fnet_shell_params shell_params;
*   ...
*   shell_params.shell = &fapp_shell;
*   shell_params.cmd_line_buffer = fapp_cmd_line_buffer;
*   shell_params.cmd_line_buffer_size = sizeof(fapp_cmd_line_buffer);
*   shell_params.stream = FNET_SERIAL_STREAM_DEFAULT; // Use default stream.
*   shell_params.echo = 1; // Enable echo.
*   ...
*   if((fapp_shell_desc = fnet_shell_init(&shell_params)) != FNET_ERR)
*   {
*       fnet_printf("\n Shell started.\n");
*       while(1)
*       {
*           // Do something.
*           ...
*           fnet_poll_services();
*       }
*    }
*    else
*    {
*        fnet_printf("\n Shell initialization is failed.\n");
*    }
*   ...
* }
* @endcode
*
* For Shell service usage example, refer to FNET demo application source code.@n
* @n
* Configuration parameters:
* - @ref FNET_CFG_SHELL_MAX
* - @ref FNET_CFG_SHELL_ARGS_MAX
* - @ref FNET_CFG_SHELL_HELP_FORMAT  
*/
/*! @{ */

/**************************************************************************/ /*!
 * @brief The quote (@c ') symbol is used to combine multiple words 
 *        to one argument.
 * @showinitializer 
 ******************************************************************************/
#define FNET_SHELL_QUOTE_SYMBOL      '\''    

/**************************************************************************/ /*!
 * @brief The escape symbol (@c \).
 *
 * The escape character (@c \) provides the way to include the 
 * @ref FNET_SHELL_QUOTE_SYMBOL quote symbol and 
 * the @ref FNET_SHELL_COMMAND_SPLITTER split symbol 
 * inside a string argument.@n
 * For example, to set the @c bootscript parameter to the value: 
 * @code
 * dhcp ; tftp; set bootscript 'tftp\; go 0x20000'
 * @endcode
 * In the shell prompt, you should enter:
 * @code
 * FNET> set bootscript `dhcp \; info\; set bootscrpt \` tftp\\; go 0x20000\``
 * @endcode
 * @showinitializer 
 ******************************************************************************/
#define FNET_SHELL_ESCAPE_SYMBOL      '\\'    

/**************************************************************************/ /*!
 * @brief Split symbol used to split shell commands. @n
 *        User may enter several commands in one command line, but they must 
 *        be split by this symbol.
 * @showinitializer 
 ******************************************************************************/
#define FNET_SHELL_COMMAND_SPLITTER          ';'   

/**************************************************************************/ /*!
 * @brief  Shell command type.
 ******************************************************************************/
typedef enum
{
    FNET_SHELL_CMD_TYPE_END = 0,    /**< @brief End of command command. @n 
                                    */
    FNET_SHELL_CMD_TYPE_NORMAL ,    /**< @brief Normal command. @n 
                                    * This command invokes a function call,
                                    * pointed to by @ref fnet_shell_command.cmd_ptr.
                                    */
    FNET_SHELL_CMD_TYPE_QUIT ,      /**< @brief Shell exit command. @n 
                                    * If the "exit" command is entered, the shell calls
                                    * a function pointed by the @ref fnet_shell_command.cmd_ptr
                                    * and tries to switch to the root shell, in case the current
                                    * shell is not a root shell.
                                    */                                 
    FNET_SHELL_CMD_TYPE_SHELL       /**< @brief Shell. @n 
                                    * This command invokes another shell,
                                    * pointed to by @ref fnet_shell_command.cmd_ptr.
                                    */
} fnet_shell_cmd_type_t;


/**************************************************************************/ /*!
 * @brief Shell states.@n
 * Used mainly for debugging purposes.
 ******************************************************************************/
typedef enum
{
    FNET_SHELL_STATE_DISABLED = 0,      /**< @brief The Shell service is not 
                                         * initialized.
                                         */
    FNET_SHELL_STATE_INIT,              /**< @brief The Shell service is not 
                                         * initialized.
                                         */                                     
    FNET_SHELL_STATE_GET_USER_INPUT,    /**< @brief The Shell service is accepting user commnads.
                                         */
    FNET_SHELL_STATE_EXEC_CMD,          /**< @brief The Shell service is executing user commnads.
                                         */
    FNET_SHELL_STATE_BLOCKED,           /**< @brief The Shell service is blocked and 
                                         * ignores user commnads.
                                         */
    FNET_SHELL_STATE_END_CMD            /**< @brief The Shell service finished command execution.
                                         */                         
} fnet_shell_state_t;

/**************************************************************************/ /*!
 * @brief Shell service descriptor.
 * @see fnet_shell_init()
 ******************************************************************************/
typedef long fnet_shell_desc_t;

/**************************************************************************/ /*!
 * @brief Shell command control structure.
 *
 * This structure is used to define properties of a command that will be 
 * supported by the shell.@n
 * An application should define the command table and pass it to the 
 * @c fnet_shell structure. @n
 * The last table element must have all fields 
 * set to zero as the end-of-table mark.
 * The good example of @ref fnet_shell_command usage is in the FNET Shell application. 
 *
 * @see fnet_shell
 ******************************************************************************/
struct fnet_shell_command
{
    fnet_shell_cmd_type_t type; /**< @brief Flag, defining the type of the command.*/
    const char *name;   /**< @brief Command name (null-terminated string). */
    int min_args;       /**< @brief Minimum number of arguments the command accepts.*/
    int max_args;       /**< @brief Maximum number of arguments the command accepts.*/
    void *cmd_ptr;      /**< @brief The pointer depending on the @c type flag: 
                         * - Pointer to the actual command function defined by
                         *   the @ref fnet_shell_cmd_function_t type,
                         *   in case the @c type flag is equal to @ref FNET_SHELL_CMD_TYPE_NORMAL
                         *   or @ref FNET_SHELL_CMD_TYPE_QUIT. 
                         * - Pointer to the  @c fnet_shell data structure,
                         *   in case the @c type flag is equal to @ref FNET_SHELL_CMD_TYPE_SHELL.
                         */
    char *description;  /**< @brief Brief description of the command (null-terminated string). @n
                         * This field is used by the @ref fnet_shell_help() function.@n@n
                         */
    char *syntax;       /**< @brief Syntax of the command (null-terminated string). @n
                         * This field is used by the @ref fnet_shell_help() function. 
                         * The standard command line syntax information 
                         * which will be helpful to describe the possible command 
                         * line parameters in a help display is:
                         * - @c [] = When a parameter is surrounded with square 
                         * brackets, this means the parameter is optional. 
                         * - @c <> = When a parameter is surrounded with angle 
                         * brackets, this means the parameter is required for 
                         * normal operations of command.
                         * - @c | = The vertical bar means a choice between 
                         * parameter value is acceptable.
                         */
};

/**************************************************************************/ /*!
 * @brief Shell main control structure.
 *
 * This structure defines shell-specific parameters.@n
 * The good example of @ref fnet_shell usage is in the FNET Shell application. 
 *
 * @see fnet_shell_params
 ******************************************************************************/
struct fnet_shell
{
    const struct fnet_shell_command *cmd_table; /**< @brief The pointer to the command table.@n 
                                                 * The last table element must have all fields 
                                                 * set to zero as the end-of-table mark.
                                                 */
    char *prompt_str;                           /**< @brief Shell prompt (null-terminated string).
                                                 */
    void (*shell_init)( fnet_shell_desc_t shell_desc );/**< @brief Routine called during the shell initialization. 
                                                 * It's called by the @ref fnet_shell_init() function.@n
                                                 * This parameter is optional and can be set to @c 0.
                                                 */
 };


/**************************************************************************/ /*!
 * @brief Input parameters for @ref fnet_shell_init().
 ******************************************************************************/
struct fnet_shell_params
{
    const struct fnet_shell *shell;     /**< @brief Root-shell structure. */
    char *cmd_line_buffer;              /**< @brief Command-line buffer. */
    unsigned int cmd_line_buffer_size;  /**< @brief Size of the command-line buffer. 
                                         * It defines the maximum length of the 
                                         * entered input-command line. */
    fnet_serial_stream_t stream;        /**< @brief Serial stream assigned to the shell. */
    int echo;                           /**< @brief Enable/disable terminal echo flag.
                                         * When set to @c 1 the echo is enabled, 
                                         * characters received by the shell are echoed 
                                         * back to the terminal.@n
                                         * When set to @c 0 the echo is disabled,
                                         * characters are transferred to the terminal 
                                         * without echoing them to the terminal display. 
                                         */
};

/***************************************************************************/ /*!
 *
 * @brief    Initializes the Shell service.
 *
 * @param params     Initialization parameters defined by @ref fnet_shell_params.
 *
 * @return This function returns:
 *   - Shell service descriptor if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_shell_release()
 *
 ******************************************************************************
 *
 * This function executes the initialization of the shell service according to 
 * settings pointed to by @c params parameters. It allocates all
 * resources needed, and registers the shell service in the polling list.@n
 * After the initialization, the user application should call the main polling 
 * function  @ref fnet_poll_services() periodically to run the Shell service in background.@n
 * The shell service runs on asigned serial stream, executes parsing of 
 * the user-entered commands, and calls user-defined command functions, 
 * if the parsing was successful.
 *
 ******************************************************************************/
fnet_shell_desc_t fnet_shell_init( struct fnet_shell_params * params);

/***************************************************************************/ /*!
 *
 * @brief    Releases the Shell service.
 *
 * @param desc     Shell service descriptor to be unregistered.
 *
 * @see fnet_shell_init()
 *
 ******************************************************************************
 *
 * This function releases the Shell service assigned to the @c desc 
 * descriptor.@n 
 * It releases all occupied resources, and unregisters the Shell service from 
 * the polling list.
 *
 ******************************************************************************/
void fnet_shell_release(fnet_shell_desc_t desc);


/**************************************************************************/ /*!
 * @brief Command callback function prototype.
 *
 * @param desc      Shell descriptor.
 *
 * @param srgc      This parameter is a count of the arguments supplied to 
 *                  the command function. @n
 *                  It is equal to @c 1 when the command was entered without
 *                  any argument.
 *
 * @param srgv      This parameter is an array of pointers to the strings 
 *                  which are the command arguments.
 *                  The first array element points to the command name.
 *                        
 ******************************************************************************/
typedef void(*fnet_shell_cmd_function_t)( fnet_shell_desc_t desc, int argc, char ** argv );


/***************************************************************************/ /*!
 *
 * @brief    Prints the command-shell help message.
 *
 * @param desc   Shell service descriptor.
 *
 * @see fnet_shell_cmd_function_t
 *
 ******************************************************************************
 *
 * This function prints the list of commands supported by the shell,
 * defined by the @c desc descriptor.
 * For example:
 * @code
 * ...
 *  static const struct fnet_shell_command fapp_cmd_table [] =
 *  {
 *      { FNET_SHELL_CMD_TYPE_NORMAL, "help", 0, 0, fapp_help_cmd,"Display this help message.", ""},
 *      { FNET_SHELL_CMD_TYPE_NORMAL, "set", 0, 2, fapp_set_cmd,      "Set parameter.", "[<parameter> <value>]"},
 *      { FNET_SHELL_CMD_TYPE_NORMAL, "get", 0, 0, fapp_show,    "Get parameters.", "[<parameter>]" },
 *      { FNET_SHELL_CMD_TYPE_NORMAL, "info", 0, 0, fapp_info_cmd,    "Show detailed status.", ""},
 *      { FNET_SHELL_CMD_TYPE_NORMAL, "dhcp", 0, 1, fapp_dhcp_cmd,    "Start DHCP client.", "[release|reboot]"},
 *      { FNET_SHELL_CMD_TYPE_NORMAL, "http", 0, 1, fapp_http_cmd,    "Start HTTP Server.", "[release]"},
 *      { FNET_SHELL_CMD_TYPE_SHELL, "exp", 0, 1, &fapp_fs_shell,   "File Explorer submenu...", ""},
 *  };
 * ...
 * @endcode
 * Calling of the @ref fnet_shell_help() for the @c fapp_cmd_table structure prints:
 * @verbatim
>   help                                 - Display this help message
>    set [<parameter> <value>]           - Set parameter
>    get [<parameter>]                   - Get parameters
>   info                                 - Show detailed status
>   dhcp [release]                       - Start DHCP client
>   http [release]                       - Start HTTP Server
>    exp                                 - File Explorer submenu...
 @endverbatim
 *
 ******************************************************************************/
void fnet_shell_help( fnet_shell_desc_t desc);


/***************************************************************************/ /*!
 *
 * @brief    Executes the command line script.
 *
 * @param desc      Shell service descriptor.
 *
 * @param script    Command line string (null-terminated) to be executed by the shell.
 *
 * @see fnet_shell_script_release
 *
 ******************************************************************************
 *
 * This function executes a predefined command-line script pointed by the @c script. @n
 * The script string may have several commands but they must be split by 
 * the @ref FNET_SHELL_COMMAND_SPLITTER symbol.
 *
 ******************************************************************************/
void fnet_shell_script(fnet_shell_desc_t desc, char *script );

/***************************************************************************/ /*!
 *
 * @brief    Stops execution of the shell command line.
 *
 * @param desc      Shell service descriptor.
 *
 ******************************************************************************
 *
 * This function stops execution of the all shell commands that are waiting 
 * for execution. @n
 * This function is usually called when the execution of a critical command is 
 * failed and no sense to execute the rest of the commands that were entered 
 * by a user in one command line or executed by the @ref fnet_shell_script() 
 * function.
 *
 ******************************************************************************/
void fnet_shell_script_release( fnet_shell_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Blocks the shell to ignore user commands.
 *
 * @param desc      Shell service descriptor.
 *
 * @param on_ctrlc  Pointer to the callback function called on the [Ctrl]+[c]
 *                  button pressing.
 *
 * @return This function returns:
 *   - @ref FNET_OK if no error occurs.
 *   - @ref FNET_ERR if an error occurs.
 *
 * @see fnet_shell_unblock
 *
 ******************************************************************************
 *
 * This function moves the shell to the blocking state.
 * In this state the shell ignore any commands entered by a user into a terminal
 * console. @n
 * The shell can be unblocked by the @ref fnet_shell_unblock() function.
 * Also the shell may be unblocked by pressing the [Ctrl]+[c] button combination 
 * in a terminal console, after that the callback function,
 * pointed by the @c on_ctrlc() parameter, will be called.
 *
 ******************************************************************************/
int fnet_shell_block( fnet_shell_desc_t desc, void (*on_ctrlc)(fnet_shell_desc_t shl_desc));

/***************************************************************************/ /*!
 *
 * @brief    Unblocks the shell to accept user commands.
 *
 * @param desc      Shell service descriptor.
 *
 * @see fnet_shell_block
 *
 ******************************************************************************
 *
 * This function moves the shell from the blocked state to the normal state.
 * In this state the shell accepts all supported commands.@n
 * @n
 * NOTE: Also the shell may be unblocked by a user by pressing the [Ctrl]+[c] 
 * buttons in a terminal console.
 *
 ******************************************************************************/
void fnet_shell_unblock( fnet_shell_desc_t desc);

/***************************************************************************/ /*!
 *
 * @brief    Prints formatted text to the shell stream.
 *
 * @param desc      Shell service descriptor.
 *
 * @param format       Format string.
 *
 * @return This function returns the number of characters that were 
 *         successfully output.
 *
 * @see fnet_shell_putchar(), fnet_shell_getchar(), fnet_shell_println()
 *
 ******************************************************************************
 *
 * This function outputs formatted text to the shell stream. @n
 * The function takes one or more arguments. The first argument is a string parameter 
 * called the "format string".
 * The optional arguments following @c format are items
 * (integers, characters or strings) that are to be converted to character strings 
 * and inserted into the output of @c format at specified points.@n
 * The syntax for a format placeholder is @a "%[Flags][Width][Length]Type".
 * - @a Flasgs can be omitted or be any of:
 *    - @c - : Left justify.
 *    - @c + : Right justify.
 *    - @c 0 : Pad with leading zeros.
 *    - @a space  : Print space if no sign.
 * - @a Width is minimum field width. It can be omitted.
 * - @a Length is conversion character. It can be omitted or by any of:
 *    - @c h : Short integer.
 *    - @c l : Long integer.
 * - @a Type can by any of:
 *    - @c d, @c i : Integer.
 *    - @c u : Unsigned.
 *    - @c x, @c X : Hexadecimal.
 *    - @c o : Octal.
 *    - @c b : Binary.
 *    - @c p : Pointer.
 *    - @c c : Single char.
 *    - @c s : Char string.
 *    - @c n : Nothing.
 * @note
 * To save some bytes from all the hard coded strings the (s)printf() functions will 
 * expand all line feeds ("\n") inside the format string to CR LF ("\r\n"). 
 * So do not use "\r\n" in	the format string - it will be expanded to 
 * "\r\r\n". It is save to add it via a parameter though, e.g. 
 * fnet_printf("%s", "\r\n");@n
 * This feature can be disable/enabled by the @ref FNET_CFG_SERIAL_PRINTF_N_TO_RN
 * configuration parameter.
 ******************************************************************************/
int fnet_shell_printf(fnet_shell_desc_t desc, const char *format, ... );

/***************************************************************************/ /*!
 *
 * @brief    Prints formatted text to the shell stream and terminates the 
 *           printed text by the line separator string.
 *
 * @param desc      Shell service descriptor.
 *
 * @param format       Format string.
 *
 * @return This function returns the number of characters that were 
 *         successfully output.
 *
 * @see fnet_shell_putchar(), fnet_shell_getchar(), fnet_shell_printf()
 *
 ******************************************************************************
 *
 * This function outputs formatted text to the shell stream and terminates the 
 * printed text by the line separator string. @n
 * The function takes one or more arguments. The first argument is a string parameter 
 * called the "format string".
 * The optional arguments following @c format are items
 * (integers, characters or strings) that are to be converted to character strings 
 * and inserted into the output of @c format at specified points.@n
 * The syntax for a format placeholder is @a "%[Flags][Width][Length]Type".
 * - @a Flasgs can be omitted or be any of:
 *    - @c - : Left justify.
 *    - @c + : Right justify.
 *    - @c 0 : Pad with leading zeros.
 *    - @a space  : Print space if no sign.
 * - @a Width is minimum field width. It can be omitted.
 * - @a Length is conversion character. It can be omitted or by any of:
 *    - @c h : Short integer.
 *    - @c l : Long integer.
 * - @a Type can by any of:
 *    - @c d, @c i : Integer.
 *    - @c u : Unsigned.
 *    - @c x, @c X : Hexadecimal.
 *    - @c o : Octal.
 *    - @c b : Binary.
 *    - @c p : Pointer.
 *    - @c c : Single char.
 *    - @c s : Char string.
 *    - @c n : Nothing.
 * @note
 * To save some bytes from all the hard coded strings the (s)printf() functions will 
 * expand all line feeds ("\n") inside the format string to CR LF ("\r\n"). 
 * So do not use "\r\n" in	the format string - it will be expanded to 
 * "\r\r\n". It is save to add it via a parameter though, e.g. 
 * fnet_printf("%s", "\r\n");@n
 * This feature can be disable/enabled by the @ref FNET_CFG_SERIAL_PRINTF_N_TO_RN
 * configuration parameter.
 ******************************************************************************/
int fnet_shell_println(fnet_shell_desc_t desc, const char *format, ... );

/***************************************************************************/ /*!
 *
 * @brief    Writes character to the shell stream.
 *
 * @param desc          Shell service descriptor.
 *
 * @param character     Character to be written.
 *
 * @see fnet_shell_getchar()
 *
 ******************************************************************************
 *
 * This function writes a single @c character to the shell stream.
 *
 ******************************************************************************/
void fnet_shell_putchar(fnet_shell_desc_t desc, int character);


/***************************************************************************/ /*!
 *
 * @brief    Reads character from the shell stream.
 *
 * @param desc          Shell service descriptor.
 *
 * @return This function returns:
 *          - character from the shell stream.
 *          - FNET_ERR if no charcter is available in the shell stream.
 *
 * @see fnet_shell_putchar()
 *
 ******************************************************************************
 *
 * This function reads a single character from the shell stream. @n
 * If no character is availble in the shell stream the FNET_ERR is returned.
 *
 ******************************************************************************/
int fnet_shell_getchar(fnet_shell_desc_t desc);

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
int fnet_shell_ctrlc (fnet_shell_desc_t desc);
/*! @} */

#endif

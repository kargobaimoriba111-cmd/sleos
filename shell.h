/*
 * =============================================================================
 * SLeOS - Interactive Shell (shell.h)
 *
 * Provides a command-line interface running on top of the kernel.
 * Supported commands:
 *   help      - list all commands
 *   clear     - clear screen
 *   ps        - show process table
 *   sched     - run scheduling simulation (fcfs / rr)
 *   mem       - show memory map
 *   memtest   - run memory allocation demo
 *   ls        - list files
 *   cat <f>   - read file
 *   touch <f> - create file
 *   write <f> <data> - append data to file
 *   rm <f>    - delete file
 *   stat <f>  - file info
 *   fsinfo    - filesystem summary
 *   uname     - OS info
 *   about     - SLeOS mission statement
 *   halt      - display shutdown message
 * =============================================================================
 */

#ifndef SHELL_H
#define SHELL_H

void shell_init(void);
void shell_run(void);    /* Main REPL loop — does not return */

#endif /* SHELL_H */

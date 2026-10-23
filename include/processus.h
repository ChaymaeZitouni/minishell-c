/** @file processus.h
 *  @brief Gestion des processus et contrôle de flux
 * @author zitouni chaymae
 * 
 * @author Moujane basma
 * @date 2025-26
 *  @date 2025-26
 */

 #ifndef PROCESSUS_H
 #define PROCESSUS_H
 
 #include <unistd.h>
 #include <sys/types.h>
 #include <stdint.h>
 #include <time.h>
 
 #define MAX_ARGS 128
 #define MAX_ENV 128
 #define MAX_CMDS 128
 #define MAX_CMD_LINE 4096
 
 typedef enum {
     UNCONDITIONAL,
     ON_SUCCESS,
     ON_FAILURE
 } control_flow_mode_t;
 
 typedef enum {
     NONE,
     AND,
     OR
 } operator_t;
 
 struct control_flow;
 struct command_line;
 
 typedef struct {
     pid_t pid;
     char* argv[MAX_ARGS];
     char* envp[MAX_ENV];
     char* path;
 
     int stdin_fd;
     int stdout_fd;
     int stderr_fd;
     int status;
     uint8_t is_background;
     uint8_t invert;
 
     struct timespec start_time;
     struct timespec end_time;
 
     struct control_flow* cf;
 
     operator_t prev_operator; // NONE, AND, OR
     int argc;                 // Nombre d'arguments
 } processus_t;
 
 typedef struct control_flow {
     processus_t* proc;
     struct control_flow* unconditionnal_next;
     struct control_flow* on_success_next;
     struct control_flow* on_failure_next;
     struct command_line* cmdl;
 } control_flow_t;
 
 typedef struct command_line {
     char command_line[MAX_CMD_LINE];
     char* tokens[MAX_CMD_LINE / 2 + 1];
     processus_t commands[MAX_CMDS];
     control_flow_t flow[MAX_CMDS];
     unsigned int num_commands;
     int opened_descriptors[MAX_CMDS * 3 + 1];
 } command_line_t;
 
 /* Fonctions */
 int init_processus(processus_t* proc);
 int init_control_flow(control_flow_t* cf);
 int init_command_line(command_line_t* cmdl);
 
 int add_fd(command_line_t* cmdl, int fd);
 int close_fds(command_line_t* cmdl);
 
 processus_t* add_processus(command_line_t* cmdl, control_flow_mode_t mode);
 processus_t* next_processus(command_line_t* cmdl);
 
 int launch_processus(processus_t* p);
 int launch_command_line(command_line_t* cmdl);
 
 #endif
 
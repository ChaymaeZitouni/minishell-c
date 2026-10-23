/**

 * @author zitouni chaymae
 * 
 * @author Moujane basma
 * @date 2025-26
 * @date 2025-26
 */
#include "processus.h"
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

/* ----- Initialisation ----- */
int init_processus(processus_t* proc) {
    if (!proc) return -1;
    memset(proc, 0, sizeof(processus_t));
    proc->stdin_fd  = STDIN_FILENO;
    proc->stdout_fd = STDOUT_FILENO;
    proc->stderr_fd = STDERR_FILENO;
    proc->prev_operator = NONE;
    return 0;
}

int init_control_flow(control_flow_t* cf) {
    if (!cf) return -1;
    memset(cf, 0, sizeof(control_flow_t));
    return 0;
}

int init_command_line(command_line_t* cmdl) {
    if (!cmdl) return -1;
    memset(cmdl, 0, sizeof(command_line_t));
    for (int i=0; i<MAX_CMDS*3+1; i++) cmdl->opened_descriptors[i]=-1;
    for (unsigned int i=0; i<MAX_CMDS; i++) init_processus(&cmdl->commands[i]);
    for (unsigned int i=0; i<MAX_CMDS; i++) init_control_flow(&cmdl->flow[i]);
    cmdl->num_commands=0;
    return 0;
}

/* ----- Gestion des descripteurs ----- */
int add_fd(command_line_t* cmdl,int fd) {
    if (!cmdl || fd<0) return -1;
    for (int i=0;i<MAX_CMDS*3+1;i++) {
        if (cmdl->opened_descriptors[i]==-1){
            cmdl->opened_descriptors[i]=fd;
            return 0;
        }
    }
    return -1;
}

int close_fds(command_line_t* cmdl){
    if(!cmdl) return -1;
    for (int i=0;i<MAX_CMDS*3+1;i++){
        if(cmdl->opened_descriptors[i]>=0){
            close(cmdl->opened_descriptors[i]);
            cmdl->opened_descriptors[i]=-1;
        }
    }
    return 0;
}

/* ----- Ajout de processus ----- */
processus_t* add_processus(command_line_t* cmdl, control_flow_mode_t mode){
    if(!cmdl || cmdl->num_commands>=MAX_CMDS) return NULL;

    processus_t* p = &cmdl->commands[cmdl->num_commands];
    init_processus(p);

    p->cf = &cmdl->flow[cmdl->num_commands];
    p->cf->proc = p;
    p->cf->cmdl = cmdl;
    if(cmdl->num_commands>0){
        processus_t* prev = &cmdl->commands[cmdl->num_commands-1];
        switch(mode){
            case UNCONDITIONAL: prev->cf->unconditionnal_next=p->cf; break;
            case ON_SUCCESS:    prev->cf->on_success_next=p->cf; break;
            case ON_FAILURE:    prev->cf->on_failure_next=p->cf; break;
        }
    }
    // Définir l'opérateur pour le processus courant, pas le précédent
    switch(mode){
        case UNCONDITIONAL: p->prev_operator = NONE; break;
        case ON_SUCCESS:    p->prev_operator = AND; break;
        case ON_FAILURE:    p->prev_operator = OR; break;
    }
    
    
   

    cmdl->num_commands++;
    return p;
}

processus_t* next_processus(command_line_t* cmdl){
    if(!cmdl || cmdl->num_commands>=MAX_CMDS) return NULL;
    return &cmdl->commands[cmdl->num_commands];
}

/* ----- Redirections ----- */
static void handle_redirections(processus_t* p){
    for(int i=0;i<p->argc;i++){
        if(!p->argv[i]) continue;
        if(strcmp(p->argv[i],">")==0 && i+1<p->argc){
            int fd=open(p->argv[i+1],O_WRONLY|O_CREAT|O_TRUNC,0644);
            if(fd>=0){ dup2(fd,STDOUT_FILENO); close(fd); p->argv[i]=NULL; } else perror("open");
        } else if(strcmp(p->argv[i],">>")==0 && i+1<p->argc){
            int fd=open(p->argv[i+1],O_WRONLY|O_CREAT|O_APPEND,0644);
            if(fd>=0){ dup2(fd,STDOUT_FILENO); close(fd); p->argv[i]=NULL; } else perror("open");
        } else if(strcmp(p->argv[i],"<")==0 && i+1<p->argc){
            int fd=open(p->argv[i+1],O_RDONLY);
            if(fd>=0){ dup2(fd,STDIN_FILENO); close(fd); p->argv[i]=NULL; } else perror("open");
        } else if(strcmp(p->argv[i],"2>")==0 && i+1<p->argc){
            int fd=open(p->argv[i+1],O_WRONLY|O_CREAT|O_TRUNC,0644);
            if(fd>=0){ dup2(fd,STDERR_FILENO); close(fd); p->argv[i]=NULL; } else perror("open");
        } else if(strcmp(p->argv[i],"2>&1")==0){
            dup2(STDOUT_FILENO,STDERR_FILENO); p->argv[i]=NULL;
        }
    }
}

/* ----- Lancement d’un processus simple ----- */
int launch_processus(processus_t* p){
    if(!p || !p->argv[0]) return -1;

    if(is_builtin(p)){ exec_builtin(p); return 0; }

    int pipefd[2]={-1,-1};
    int use_pipe=0;

    for(int i=0;i<p->argc;i++)
        if(p->argv[i] && strcmp(p->argv[i],"|")==0){ use_pipe=1; break; }

    if(use_pipe && pipe(pipefd)<0){ perror("pipe"); return -1; }

    pid_t pid=fork();
    if(pid<0){ perror("fork"); return -1; }

    if(pid==0){
        handle_redirections(p);
        if(use_pipe){
            dup2(pipefd[1],STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
        }
        if(p->cf && p->cf->cmdl) close_fds(p->cf->cmdl);
        execvp(p->argv[0],p->argv);
        perror("execvp");
        exit(1);
    } else {
        p->pid=pid;
        if(!p->is_background) waitpid(pid,&p->status,0);
        if(use_pipe){
            close(pipefd[1]);
            dup2(pipefd[0],STDIN_FILENO);
            close(pipefd[0]);
        }
    }
    return 0;
}
int launch_command_line(command_line_t* cmdl) {
    if (!cmdl) return -1;

    int last_status = 0;
    int fd_in = STDIN_FILENO;
    int pipefd[2];

    for (unsigned int i = 0; i < cmdl->num_commands; i++) {
        processus_t* p = &cmdl->commands[i];
        if (!p->argv[0]) continue;

        // --- Vérifier AND/OR avant exécution ---
        if (i > 0) {
            if ((p->prev_operator == AND && last_status != 0) ||
                (p->prev_operator == OR  && last_status == 0)) {
                continue; // sauter cette commande
            }
        }

        int use_pipe = (i < cmdl->num_commands - 1 && cmdl->commands[i+1].prev_operator == NONE);
        if (use_pipe && pipe(pipefd) < 0) { perror("pipe"); return -1; }

        // --- Si builtin qui modifie l'environnement, exécuter directement ---
        int is_env_builtin = 0;
        if (is_builtin(p)) {
            if (strcmp(p->argv[0], "export") == 0 ||
                strcmp(p->argv[0], "unset") == 0 ||
                strcmp(p->argv[0], "cd") == 0 ||
                strcmp(p->argv[0], "exit") == 0) {

                last_status = exec_builtin(p);
                continue;
            }
        }

        // --- Fork pour les autres commandes (externes ou builtins “normaux”) ---
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return -1; }

        if (pid == 0) {
            // --- Dans le fils ---
            dup2(fd_in, STDIN_FILENO);
            if (use_pipe) dup2(pipefd[1], STDOUT_FILENO);
            handle_redirections(p);
            if (p->cf && p->cf->cmdl) close_fds(p->cf->cmdl);

            if (is_builtin(p)) exit(exec_builtin(p) ? 1 : 0);
            execvp(p->argv[0], p->argv);
            perror("execvp");
            exit(1);
        }

        // --- Dans le parent ---
        if (fd_in != STDIN_FILENO) close(fd_in);
        if (use_pipe) {
            close(pipefd[1]);
            fd_in = pipefd[0];
        } else fd_in = STDIN_FILENO;

        if (!p->is_background) {
            waitpid(pid, &p->status, 0);
            last_status = WIFEXITED(p->status) ? WEXITSTATUS(p->status) : 1;
        } else last_status = 0;

        if (p->invert) last_status = (last_status == 0) ? 1 : 0;
    }

    return 0;
}

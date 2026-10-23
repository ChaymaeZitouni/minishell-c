/**

 * @author zitouni chaymae
 * 
 * @author Moujane basma
 * @date 2025-26
 * @date 2025-26
 */

#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int is_builtin(const processus_t* cmd) {
    if (!cmd || !cmd->argv[0]) return 0;
    return strcmp(cmd->argv[0], "cd") == 0 ||
           strcmp(cmd->argv[0], "exit") == 0 ||
           strcmp(cmd->argv[0], "pwd") == 0 ||
           strcmp(cmd->argv[0], "export") == 0 ||
           strcmp(cmd->argv[0], "unset") == 0;
}

int exec_builtin(processus_t* cmd) {
    if (!cmd || !cmd->argv[0]) return -1;
    if (strcmp(cmd->argv[0], "cd") == 0) return builtin_cd(cmd);
    if (strcmp(cmd->argv[0], "exit") == 0) return builtin_exit(cmd);
    if (strcmp(cmd->argv[0], "pwd") == 0) return builtin_pwd(cmd);
    if (strcmp(cmd->argv[0], "export") == 0) return builtin_export(cmd);
    if (strcmp(cmd->argv[0], "unset") == 0) return builtin_unset(cmd);
    return -1;
}

int builtin_cd(processus_t* cmd) {
    char* path = cmd->argv[1] ? cmd->argv[1] : getenv("HOME");
    if (chdir(path) != 0) { perror("cd"); return -1; }
    return 0;
}

int builtin_exit(processus_t* cmd) {
    int code = 0;
    if (cmd->argv[1]) code = atoi(cmd->argv[1]);
    exit(code);
}

int builtin_pwd(processus_t* cmd) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    } else { perror("pwd"); return -1; }
}

int builtin_export(processus_t *cmd) {
    if (cmd->argv[1] == NULL) {
        // Afficher toutes les variables d'environnement
        extern char **environ;
        for (char **env = environ; *env != NULL; env++) {
            dprintf(cmd->stdout_fd, "export %s\n", *env);
        }
        return 0;
    }
    
    for (int i = 1; cmd->argv[i] != NULL; i++) {
        char *equal_sign = strchr(cmd->argv[i], '=');
        if (equal_sign) {
            *equal_sign = '\0';
            char *var_name = cmd->argv[i];
            char *var_value = equal_sign + 1;
            
            if (setenv(var_name, var_value, 1) != 0) {
                dprintf(cmd->stderr_fd, "export: %s: %s\n", var_name, strerror(errno));
                return 1;
            }
            *equal_sign = '='; // Restaurer pour l'affichage
        }
    }
    
    return 0;
}

/**
 * @brief Fonction d'exécution de la commande "unset"
 */
int builtin_unset(processus_t *cmd) {
    if (cmd->argv[1] == NULL) {
        dprintf(cmd->stderr_fd, "unset: missing argument\n");
        return 1;
    }
    
    for (int i = 1; cmd->argv[i] != NULL; i++) {
        if (unsetenv(cmd->argv[i]) != 0) {
            dprintf(cmd->stderr_fd, "unset: %s: %s\n", cmd->argv[i], strerror(errno));
            return 1;
        }
    }
    
    return 0;
}

char* expand_var(const char* varname) {
    char* val = getenv(varname); // lit l'environnement du process
    if (!val) return NULL;
    return strdup(val); // retourne une copie
}

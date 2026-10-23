#include "parser.h"
#include "processus.h"
#include "builtins.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern char* expand_var(const char* token); 

int parse_command_line(command_line_t* cmdl, const char* line) {
    if (!cmdl || !line) return -1;

    char buffer[MAX_CMD_LINE];
    strncpy(buffer, line, MAX_CMD_LINE - 1);
    buffer[MAX_CMD_LINE - 1] = '\0';

    processus_t* p = add_processus(cmdl, UNCONDITIONAL);
    int argc = 0;

    char* ptr = buffer;
    while (*ptr) {
        while (*ptr == ' ' || *ptr == '\t') ptr++; // ignorer les espaces

        if (*ptr == '\0') break;

        char token[MAX_CMD_LINE];
        int token_len = 0;

        // --- Gérer les guillemets doubles ou simples ---
        if (*ptr == '"' || *ptr == '\'') {
            char quote = *ptr++;
            while (*ptr && *ptr != quote) {
                token[token_len++] = *ptr++;
            }
            if (*ptr == quote) ptr++; // sauter le guillemet fermant
        }
        // --- Gérer les opérateurs spéciaux ---
        else if ((*ptr == '&' && *(ptr+1) == '&') || (*ptr == '|' && *(ptr+1) == '|')) {
            token[token_len++] = *ptr++;
            token[token_len++] = *ptr++;
        }
        else if (*ptr == ';' || *ptr == '|' || *ptr == '&' || *ptr == '!') {
            token[token_len++] = *ptr++;
        }
        // --- Token normal ---
        else {
            while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != ';' &&
                   *ptr != '|' && *ptr != '&' && *ptr != '!') {
                token[token_len++] = *ptr++;
            }
        }

        token[token_len] = '\0';
        if (token_len == 0) continue;

        // --- Expansion des variables ---
        char* expanded = NULL;
        if (token[0] == '$') {
            expanded = expand_var(token + 1); // "$VAR" → "VAR"
            if (!expanded) expanded = strdup(""); // variable non définie
        } else {
            expanded = strdup(token);
        }

        // --- Ajouter le token au processus ou gérer les opérateurs ---
        if (strcmp(expanded, ";") == 0) {
            p = add_processus(cmdl, UNCONDITIONAL);
            argc = 0;
        } else if (strcmp(expanded, "&&") == 0) {
            p = add_processus(cmdl, ON_SUCCESS);
            argc = 0;
        } else if (strcmp(expanded, "||") == 0) {
            p = add_processus(cmdl, ON_FAILURE);
            argc = 0;
        } else if (strcmp(expanded, "!") == 0) {
            p = add_processus(cmdl, UNCONDITIONAL);
            p->invert = 1;
            argc = 0;
        } else if (strcmp(expanded, "|") == 0) {
            p = add_processus(cmdl, UNCONDITIONAL);
            argc = 0;
        } else if (strcmp(expanded, "&") == 0) {
            if (argc > 0) p->is_background = 1;
        } else {
            if (argc < MAX_ARGS) {
                p->argv[argc++] = expanded;
                p->argc = argc;
            } else {
                free(expanded); // éviter fuite mémoire
            }
        }
    }

    return 0;
}

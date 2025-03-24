/* NaGNU/nshell */
/* ruzen42, mkfs */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <readline/readline.h>  
#include <readline/history.h>

#define MAX_INPUT 1488
#define MAX_ARGS 64
#define MAX_PATH 1024

struct env_var {
    char *name;
    char *value;
};

struct env_var *env_vars = NULL;
int env_count = 0;

void parse_input(char *input, char **args);
void execute_command(char **args);
void set_env_var(char *name, char *value);
char *get_env_var(char *name);
void handle_internal_commands(char **args);

void init_readline() {
    rl_bind_key('\t', rl_complete);  }

void parse_input(char *input, char **args) {
    int i = 0;
    args[i] = strtok(input, " \t\n");
    while (args[i] != NULL && i < MAX_ARGS - 1) {
        args[++i] = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

void set_env_var(char *name, char *value) {
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].name, name) == 0) {
            free(env_vars[i].value);
            env_vars[i].value = strdup(value);
            return;
        }
    }
    
    env_vars = realloc(env_vars, (env_count + 1) * sizeof(struct env_var));
    env_vars[env_count].name = strdup(name);
    env_vars[env_count].value = strdup(value);
    env_count++;
}

char *get_env_var(char *name) {
    for (int i = 0; i < env_count; i++) {
        if (strcmp(env_vars[i].name, name) == 0) {
            return env_vars[i].value;
        }
    }
    return getenv(name);  }

void handle_internal_commands(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            chdir(get_env_var("HOME"));
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
    } else if (strcmp(args[0], "set") == 0) {
        if (args[1] && args[2]) {
            set_env_var(args[1], args[2]);
        }
    }
}

void execute_command(char **args) {
    if (args[0] == NULL) return;

    
    if (strcmp(args[0], "exit") == 0 || 
        strcmp(args[0], "cd") == 0 || 
        strcmp(args[0], "set") == 0) {
        handle_internal_commands(args);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
               for (int i = 0; args[i] != NULL; i++) {
            if (args[i][0] == '$') {
                char *var_value = get_env_var(args[i] + 1);
                if (var_value) {
                    args[i] = var_value;
                }
            }
        }
        
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork");
    }
}

int main() {
    char *username = getlogin();
    char host[256];
    char cwd[MAX_PATH];
    char prompt[1024];
    
    gethostname(host, sizeof(host));
    init_readline();
    
    printf("NShell v0.3 - Enhanced shell for NaGNU/Linux\n");
    printf("Made for NaGNU project\n\n");

        set_env_var("HOME", getenv("HOME"));
    set_env_var("PATH", getenv("PATH"));

    while (1) {
        getcwd(cwd, sizeof(cwd));
        snprintf(prompt, sizeof(prompt), "%s@%s:%s$ ", username, host, cwd);
        
        char *input = readline(prompt);
        if (!input) {
            printf("\n");
            break;
        }
        
        if (strlen(input) > 0) {
            add_history(input);  
        }

        char *args[MAX_ARGS];
        parse_input(input, args);
        
        if (args[0] != NULL) {
            execute_command(args);
        }
        
        free(input);
    }

        for (int i = 0; i < env_count; i++) {
        free(env_vars[i].name);
        free(env_vars[i].value);
    }
    free(env_vars);
    
    return 0;
}

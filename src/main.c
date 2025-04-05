/* NaGNU/nshell 
 * 
 * Developers:
 * - ruzen42    (Developer)
 * - mkfs       (Developer)
 * - AnmiTaliDev (Pull-request creator)
 * 
 */

 #include "../include/nshell.h"

 /* Глобальные переменные */
 struct env_var *env_vars = NULL;
 int env_count = 0;
 
 /* Встроенные команды */
 char *builtin_commands[] = {
     "cd",
     "set",
     "exit",
     NULL
 };
 
 /* Генератор автодополнения команд */
 char *command_generator(const char *text, int state) {
     static int list_index, len;
     const char *name;
 
     if (!state) {
         list_index = 0;
         len = strlen(text);
     }
 
     while ((name = builtin_commands[list_index++])) {
         if (strncmp(name, text, len) == 0) {
             return strdup(name);
         }
     }
     return NULL;
 }
 
 /* Функция автодополнения */
 char **custom_completion(const char *text, int start __attribute__((unused)), 
                         int end __attribute__((unused))) {
     rl_attempted_completion_over = 1;
     return rl_completion_matches(text, command_generator);
 }
 
 /* Инициализация readline */
 void init_readline() {
     rl_bind_key('\t', rl_complete);
     rl_attempted_completion_function = custom_completion;
 }
 
 /* Обработчик сигналов */
 void signal_handler(int signo) {
     if (signo == SIGINT) {
         printf("\n");
         rl_on_new_line();
         rl_replace_line("", 0);
         rl_redisplay();
     }
 }
 
 /* Загрузка истории команд */
 void load_history() {
     char *home = getenv("HOME");
     if (!home) return;
     
     char history_path[MAX_PATH];
     snprintf(history_path, sizeof(history_path), "%s/%s", home, HISTORY_FILE);
     read_history(history_path);
 }
 
 /* Сохранение истории команд */
 void save_history() {
     char *home = getenv("HOME");
     if (!home) return;
     
     char history_path[MAX_PATH];
     snprintf(history_path, sizeof(history_path), "%s/%s", home, HISTORY_FILE);
     write_history(history_path);
 }
 
 /* Разбор входной строки на аргументы */
 void parse_input(char *input, char **args) {
     if (!input || !args) {
         return;
     }
 
     int i = 0;
     char *token = strtok(input, " \t\n");
     
     while (token != NULL && i < MAX_ARGS - 1) {
         if (strlen(token) >= MAX_INPUT) {
             fprintf(stderr, "Ошибка: слишком длинный аргумент\n");
             args[i] = NULL;
             return;
         }
         args[i++] = token;
         token = strtok(NULL, " \t\n");
     }
     args[i] = NULL;
 }
 
 /* Установка переменной окружения */
 void set_env_var(char *name, char *value, char *owner) {
     if (!name || !value || !owner) {
         fprintf(stderr, "Ошибка: неверные аргументы для set_env_var\n");
         return;
     }
 
     for (int i = 0; i < env_count; i++) {
         if (strcmp(env_vars[i].name, name) == 0) {
             char *new_value = strdup(value);
             if (!new_value) {
                 perror("strdup");
                 return;
             }
             free(env_vars[i].value);
             env_vars[i].value = new_value;
             return;
         }
     }
     
     struct env_var *new_vars = realloc(env_vars, (env_count + 1) * sizeof(struct env_var));
     if (!new_vars) {
         perror("realloc");
         return;
     }
     env_vars = new_vars;
 
     if (!(env_vars[env_count].name = strdup(name)) ||
         !(env_vars[env_count].value = strdup(value)) ||
         !(env_vars[env_count].owner = strdup(owner))) {
         perror("strdup");
         free(env_vars[env_count].name);
         free(env_vars[env_count].value);
         free(env_vars[env_count].owner);
         return;
     }
     env_count++;
 }
 
 /* Получение значения переменной окружения */
 char *get_env_var(char *name) {
     if (!name) return NULL;
     
     for (int i = 0; i < env_count; i++) {
         if (strcmp(env_vars[i].name, name) == 0) {
             return env_vars[i].value;
         }
     }
     return getenv(name);
 }
 
 /* Обработка встроенных команд */
 int handle_internal_commands(char **args) {
     if (!args || !args[0]) return 1;
 
     if (strcmp(args[0], "exit") == 0) {
         cleanup();
         exit(0);
     } else if (strcmp(args[0], "cd") == 0) {
         if (args[1] == NULL) {
             char *home = get_env_var("HOME");
             if (home && chdir(home) != 0) {
                 perror("cd");
                 return 1;
             }
         } else {
             if (chdir(args[1]) != 0) {
                 perror("cd");
                 return 1;
             }
         }
         return 0;
     } else if (strcmp(args[0], "set") == 0) {
         if (args[1] && args[2] && args[3]) {
             set_env_var(args[1], args[2], args[3]);
             return 0;
         } else {
             fprintf(stderr, "Использование: set <имя> <значение> <владелец>\n");
             return 1;
         }
     }
     return 1;  // Команда не найдена
 }
 
 /* Выполнение команды */
 void execute_command(char **args) {
     if (!args || !args[0]) return;
 
     if (strcmp(args[0], "exit") == 0 || 
         strcmp(args[0], "cd") == 0 || 
         strcmp(args[0], "set") == 0) {
         handle_internal_commands(args);
         return;
     }
 
     pid_t pid = fork();
     if (pid == 0) {
         // Дочерний процесс
         for (int i = 0; args[i] != NULL; i++) {
             if (args[i][0] == '$') {
                 char *var_value = get_env_var(args[i] + 1);
                 if (var_value) {
                     args[i] = var_value;
                 }
             }
         }
         
         execvp(args[0], args);
         perror("nshell");
         exit(1);
     } else if (pid > 0) {
         // Родительский процесс
         int status;
         waitpid(pid, &status, 0);
     } else {
         perror("fork");
     }
 }
 
 /* Очистка ресурсов */
 void cleanup() {
     for (int i = 0; i < env_count; i++) {
         free(env_vars[i].name);
         free(env_vars[i].value);
         free(env_vars[i].owner);
     }
     free(env_vars);
     clear_history();
     save_history();
 }
 
 int main() {
     char *username = getlogin();
     char host[256];
     char cwd[MAX_PATH];
     char prompt[1024];
     
     // Установка обработчика сигнала
     signal(SIGINT, signal_handler);
     
     if (gethostname(host, sizeof(host)) != 0) {
         strncpy(host, "unknown", sizeof(host) - 1);
         host[sizeof(host) - 1] = '\0';
     }
     
     init_readline();
     load_history();
     
     printf("NShell v%s \n", NSHELL_VERSION);
 
     // Инициализация переменных окружения
     set_env_var("HOME", getenv("HOME"), "user");
     set_env_var("PATH", getenv("PATH"), "user");
 
     while (1) {
         if (getcwd(cwd, sizeof(cwd)) == NULL) {
             perror("getcwd");
             strncpy(cwd, "(ошибка)", sizeof(cwd) - 1);
             cwd[sizeof(cwd) - 1] = '\0';
         }
         
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
 
     cleanup();
     return 0;
 }
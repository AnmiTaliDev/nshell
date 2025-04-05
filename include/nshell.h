/* NaGNU/nshell - Improved shell for NaGNU/Linux
 * 
 * Developers:
 * - ruzen42    (Developer)
 * - mkfs       (Developer)
 * - AnmiTaliDev (Pull-request creator)
 */

 #ifndef NSHELL_H
 #define NSHELL_H
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/wait.h>
 #include <termios.h>
 #include <signal.h>
 #include <readline/readline.h>  
 #include <readline/history.h>
 
 /* Основные константы */
 #define MAX_INPUT 1488       // Максимальная длина входной строки
 #define MAX_ARGS 64         // Максимальное количество аргументов
 #define MAX_PATH 1024       // Максимальная длина пути
 #define HISTORY_FILE ".nshell_history"  // Файл истории команд
 
 /* Версия */
 #define NSHELL_VERSION "0.4"
 
 /* Коды возврата */
 #define NSHELL_SUCCESS 0
 #define NSHELL_ERROR -1
 #define NSHELL_ERROR_MEMORY -2
 #define NSHELL_ERROR_ARGS -3
 #define NSHELL_ERROR_EXEC -4
 #define NSHELL_ERROR_FORK -5
 #define NSHELL_ERROR_PATH -6
 
 /* Цветовые коды для вывода */
 #define NSHELL_COLOR_RED     "\x1b[31m"
 #define NSHELL_COLOR_GREEN   "\x1b[32m"
 #define NSHELL_COLOR_YELLOW  "\x1b[33m"
 #define NSHELL_COLOR_BLUE    "\x1b[34m"
 #define NSHELL_COLOR_MAGENTA "\x1b[35m"
 #define NSHELL_COLOR_CYAN    "\x1b[36m"
 #define NSHELL_COLOR_RESET   "\x1b[0m"
 
 /* Структура для хранения переменных окружения */
 struct env_var {
     char *name;    // Имя переменной
     char *value;   // Значение
     char *owner;   // Владелец
 };
 
 /* Структура для хранения истории команд */
 struct command_history {
     char *command;           // Текст команды
     time_t timestamp;        // Время выполнения
     int exit_status;        // Код возврата
     struct command_history *next;
 };
 
 /* Структура для конфигурации шелла */
 struct shell_config {
     int max_history_size;    // Максимальный размер истории
     int save_history;        // Флаг сохранения истории
     int color_prompt;        // Флаг цветного приглашения
     int verbose_mode;        // Флаг подробного вывода
     char *config_file;       // Путь к файлу конфигурации
     char *history_file;      // Путь к файлу истории
 };
 
 /* Функции для работы с переменными окружения */
 void set_env_var(char *name, char *value, char *owner);
 char *get_env_var(char *name);
 void unset_env_var(char *name);
 void list_env_vars(void);
 
 /* Функции для работы с командами */
 void parse_input(char *input, char **args);
 void execute_command(char **args);
 int handle_internal_commands(char **args);
 void execute_external_command(char **args);
 
 /* Функции для работы с историей */
 void load_history(void);
 void save_history(void);
 void add_to_history(const char *command, int exit_status);
 void clear_history(void);
 void show_history(void);
 
 /* Функции для работы с readline */
 void init_readline(void);
 char *command_generator(const char *text, int state);
 char **custom_completion(const char *text, int start, int end);
 
 /* Функции для обработки сигналов */
 void signal_handler(int signo);
 void setup_signals(void);
 
 /* Функции для работы с конфигурацией */
 void load_config(void);
 void save_config(void);
 void set_config_value(const char *key, const char *value);
 char *get_config_value(const char *key);
 
 /* Функции для работы с путями */
 char *get_full_path(const char *cmd);
 int is_executable(const char *path);
 void add_path(const char *path);
 void remove_path(const char *path);
 
 /* Вспомогательные функции */
 void cleanup(void);
 void print_error(const char *msg);
 void print_warning(const char *msg);
 void print_info(const char *msg);
 char *trim_string(char *str);
 int is_empty_string(const char *str);
 char *strdup_safe(const char *str);
 void *malloc_safe(size_t size);
 void free_safe(void *ptr);
 
 /* Функции для работы с файлами */
 int file_exists(const char *path);
 int is_directory(const char *path);
 int create_file(const char *path);
 int remove_file(const char *path);
 char *read_file_content(const char *path);
 int write_file_content(const char *path, const char *content);
 
 /* Функции для работы с процессами */
 pid_t create_process(void);
 int wait_for_process(pid_t pid);
 void kill_process(pid_t pid);
 int get_process_status(pid_t pid);
 
 /* Функции для работы со строками */
 char *string_replace(const char *str, const char *old, const char *new);
 char **string_split(const char *str, const char *delim);
 void free_string_array(char **arr);
 char *string_join(char **arr, const char *delim);
 int string_to_int(const char *str, int *result);
 
 /* Макросы для отладки */
 #ifdef DEBUG
     #define DEBUG_PRINT(fmt, ...) fprintf(stderr, "DEBUG: %s:%d: " fmt "\n", \
                                         __FILE__, __LINE__, ##__VA_ARGS__)
 #else
     #define DEBUG_PRINT(fmt, ...) ((void)0)
 #endif
 
 /* Макросы для проверки ошибок */
 #define CHECK_NULL(ptr) \
     do { \
         if ((ptr) == NULL) { \
             print_error("Ошибка выделения памяти"); \
             return NSHELL_ERROR_MEMORY; \
         } \
     } while (0)
 
 #define CHECK_ERROR(expr) \
     do { \
         if ((expr) < 0) { \
             print_error("Ошибка выполнения операции"); \
             return NSHELL_ERROR; \
         } \
     } while (0)
 
 /* Внешние переменные */
 extern struct env_var *env_vars;
 extern int env_count;
 extern struct shell_config config;
 extern struct command_history *history_head;
 extern char *builtin_commands[];
 
 #endif /* NSHELL_H */
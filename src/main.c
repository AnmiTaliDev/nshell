#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 64

void
parse_input(
		char *input,
		char **args
		)
{
	int i = 0;
	args[i] = strtok (input, " \t\n");

	while (args[i] != NULL)
	{
		args[++i] = strtok (NULL, " \t\n");
	}
}

void 
execute_command(char **args)
{
	pid_t pid = fork ();

	if (pid == 0) 
	{
		execvp (args[0], args);
		perror ("execvp");
		exit (1488);
	}
	else if (pid > 0) 
	{
		waitpid (pid, NULL, 0);
	}
	else
	{
		perror ("fork");
	}
}

int
main()
{
	char input[MAX_INPUT];
	char *args[MAX_INPUT / 2 + 1];
	char *username = getlogin ();
	char host[256];
	gethostname (host, sizeof (host));

	while (1)
	{
		printf ("%s&%s+>", username, host);
		if (fgets (input, MAX_INPUT, stdin) == NULL)
		{
			break;
		}

		parse_input (input, args);

		if (args[0] != NULL)
		{
			if (strcmp (args[0], "exit") == 0)
			{
				break;
			}

			execute_command (args);
		}
	}
}

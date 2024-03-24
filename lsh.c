#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/history.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

extern char **environ;

char *lsh_read_line(void) {
	int bufsize = LSH_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	//c must be int, not char, so that it can take in EOF
	int c;

	if (!buffer) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		c = getchar();

		if (c == EOF || c == '\n') {
			buffer[position] = '\0';
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;

		if (position >= bufsize) {
			bufsize += LSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

char **lsh_split_line(char *line) {
	int bufsize = LSH_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, LSH_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {
			bufsize += LSH_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, LSH_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

int lsh_launch(char **args) {
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		perror("lsh");
	} else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_export(char **args);
int lsh_prev(char **args);
int lsh_next(char **args);

char *builtin_str[] = {
	"cd",
	"help",
	"exit",
	"export",
	"prev",
	"next"
};

int (*builtin_func[]) (char **) = {
	&lsh_cd,
	&lsh_help,
	&lsh_exit,
	&lsh_export,
	&lsh_prev,
	&lsh_next
};

int lsh_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args) {
	if (args[1] == NULL) {
		fprintf(stderr, "lsh: expected argument to \"cd\"\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("lsh");
		}
	}
	return 1;
}

int lsh_help(char **args) {
	int i;
	printf("Stephen Brennan's LSH\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");

	for (i = 0; i < lsh_num_builtins(); i++) {
		printf("  %s\n", builtin_str[i]);
	}

	printf("Use the man command for information on other programs.\n");
	return 1;
}

int lsh_exit(char **args) {
	return 0;
}

int lsh_export(char **args) {
	char **env = environ;

	if (args[1] == NULL) {
		while (*env) {
			printf("%s\n", *env);
			env++;
		}
	} else if (args[2] == NULL) {
		char *pair = args[1];
		char *key = strtok(pair, "=");
		char *val = strtok(NULL, "=");
		if (val == NULL) {
			fprintf(stderr, "lsh: export takes format \"export key=val\"\n");
		} else if (setenv(key, val, 1) != 0) {
			perror("lsh");
		}
	} else {
		fprintf(stderr, "lsh: expected one argument to \"export\"\n");
	}
	return 1;
}

int lsh_execute(char **args) {
	int i;

	if (args[0] == NULL) {
		return 1;
	}

	for (i = 0; i < lsh_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}

	return lsh_launch(args);
}

int history_index = -1;

int lsh_prev(char **args) {
	if (args[1] != NULL) {
		fprintf(stderr, "lsh: prev taqkes in no argument\n");
	} else {
		history_index++;
		int position = history_length - history_index;
		if (position <= 0) {
			printf("No history available.\n");
			history_index = -1;
		} else { 
			HIST_ENTRY *entry = history_get(position);
			printf("%s\n", entry->line);
		}
	}
	return 1;
}

int lsh_next(char **args) {
	if (args[1] != NULL) {
		fprintf(stderr, "lsh: next taqkes in no argument\n");
	} else {
		history_index--;
		int position = history_length - history_index;
		if (position > history_length) {
			printf("No history available.\n");
			history_index = -1;
		} else {
			HIST_ENTRY *entry = history_get(position);
			printf("%s\n", entry->line);
		}
	}
	return 1;
}

void lsh_loop(void) {
	char *line;
	char **args;
	int status;

	do {
		printf("> ");
		line = lsh_read_line();
		if (*line && strcmp(line, "prev") != 0 && strcmp(line, "next") != 0) {
			add_history(line);
		}
		args = lsh_split_line(line);
		status = lsh_execute(args);

		free(line);
		free(args);
	} while (status);
}

int main(int argc, char **argv) {
	using_history();
	lsh_loop();

	return EXIT_SUCCESS;
}

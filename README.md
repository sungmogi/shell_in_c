# shell_in_c

## Introduction
This is my implementation of the shell in C. I followed Stephen Brennan's [Tutorial](https://brennan.io/2015/01/16/write-a-shell-in-c/) for the most part and added my own functionalities (command history and setting/getting environment variables). You can check out the link above to see how to implement reading the command line, tokenizing it, launching a process by forking, etc. Below I will explain how I implemented navigating through command history and export. 

## Command History
```
int history_index = -1;

int lsh_prev(char **args) {
	if (args[1] != NULL) {
		fprintf(stderr, "lsh: prev takes in no argument\n");
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
		fprintf(stderr, "lsh: next takes in no argument\n");
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
```
For history navigation, I created two functions, lsh_prev and lsh_next. They work basically the same as the up and down arrow keys in Terminal. I imported the '<readline/history.h>' library to access previous commands. I declared a global history 'history_index' and used it to navigate through the history with the 'history_get' function. I also made sure the index never goes over the entire length of the history or below 0, and wraps around the array.

## Export Functionality
```
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
```
In the export function, I declare the variable 'env', which is a pointer to an array of strings (environment variables) managed by the operating system. If the user does not give an argument, we want to display the entire array of environment variables, so we iterate over the array of environment variables and print out each of them. If the user give an argument in the format [key]=[val], we add the variable pair to the array with the 'setenv' function. The third argument to 'setenv' allows us to update the variable when there already exists a variable pair with the same key. 

## Conclusion
This project made me practice C programming (pointers and memory allocation, especially) and explore the different useful libraries in C. What was interesting to me during this project was that I could use some commands such as 'ls' and 'pwd' without implementing them myself, while I had to implement 'cd', 'export (setting env variables)', etc. to use them. I did some research and it turns out the former ones are external commands while the latter ones are built-in commands. 

### External Commands vs Built-in Commands
External commands are standalone programs stored on the file system. Built-in commands are not stored on the file system, but are part of the shell itself. The 'cd' command had to be implemented because if the parent process created a fork and waited for the child process to run 'cd', the parent process would not be affected. Similarly, commands such as 'export' also manipulate the environment or processes of the current shell process, therefore need to be built-in. 

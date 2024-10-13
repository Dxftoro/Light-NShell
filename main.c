#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/wait.h"
#include "linux/limits.h"
#include "dirent.h"
#include "signal.h"

#include "defcoms.h"

enum ERR {
	BUFF_MEM_ALLOC_ERROR,
	BUFF_ERROR
};

void AllocCheck(char* buffer) {
	if (!buffer) {
		fprintf(stderr, "BUFF_MEM_ALLOC_ERROR");
		exit(1);
	}
}

char* CheckDir() {
	char dirname[PATH_MAX];
	
	if (getcwd(dirname, sizeof(dirname)) == nullptr) {
		perror(SHELL_NAME);
		return "?";
	}

	return dirname;
}

char* ReadLine(void) {
	char* line = nullptr;
	size_t buffsize = 0;
	getline(&line, &buffsize, stdin);	
	return line;
}

char** Tokenise(char* line) {
	size_t buffsize = TOKEN_BUFFSIZE;
	int pos = 0;
	char **tokens = malloc(buffsize * sizeof(char*));
	char* token;

	AllocCheck(token);

	const char* strippers = " \t\n\a\r";
	token = strtok(line, strippers);
	while (token != nullptr) {
		tokens[pos] = token;
		pos++;

		if (pos >= buffsize) {
			buffsize += TOKEN_BUFFSIZE;

			tokens = realloc(tokens, buffsize * sizeof(char*));
			AllocCheck(tokens[0]);
		}

		token = strtok(nullptr, strippers);
	}

	tokens[pos] = nullptr;
	return tokens;
}

int LinkProcess(char** args) {
	pid_t pid, wpid;
	int status;

	pid = fork();
	if (pid < 0) perror(SHELL_NAME);
	else if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			perror(SHELL_NAME);
			exit(1);
		}
	}
	else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

int Execute(char** args) {
	if (args[0] == nullptr) return 1;

	for (int i = 0; i < DefNum(); i++) {
		if (strcmp(args[0], defCmd[i]) == 0) {
			return (*defFuncs[i])(args);
		}
	}

	return LinkProcess(args);
}

void HandleCmd(void) {
	char *line;
	char** args;
	int status = 1;

	char cwd[PATH_MAX];

	while (status) {
		printf("%s > ", getcwd(cwd, sizeof(cwd)));

		line = ReadLine();

		if (feof(stdin)) {
			printf("\n");
			return;
		}

		args = Tokenise(line);
		status = Execute(args);

		free(line);
		free(args);
	}
}

int main(int argc, char **argv) {
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != nullptr) {
		printf("Current working directory: %s\n", cwd);
	}
	else {
		perror(SHELL_NAME);
	}
	
	signal(SIGHUP, CatchSighup);
	HandleCmd();

	return 0;
}

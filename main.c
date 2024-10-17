#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/wait.h"
#include "pwd.h"
#include "sys/types.h"
#include "sys/stat.h"
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

/*void CheckDir(char* dirname) {
	if (getcwd(dirname, sizeof(dirname)) == nullptr) {
		perror(SHELL_NAME);
		dirname = "?";
	}
}*/

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

int Execute(char** args) {
	if (args[0] == nullptr) return 1;

	for (int i = 0; i < DefNum(); i++) {
		if (strcmp(args[0], defCmd[i]) == 0) {
			return (*defFuncs[i])(args);
		}
	}

	return LinkProcess(args);
}

void LineLog(char* line, char** history, int *hbuffsize, int *hindex) {
	history[*hindex] = line;
	//printf("%s", history[*hindex]);
	*hindex = *hindex + 1;

	if (*hindex >= *hbuffsize) {
		*hbuffsize += CMDHIS_BUFFSIZE;
		history = realloc(history, *hbuffsize * sizeof(char*));
		AllocCheck(history[0]);
	}


	FILE* file = fopen(getenv(CMDHIS_ENV), "a");

	if (file) {
		fputs(line, file);
		fclose(file);
	}
	else printf("%s: Can't log input!\n", SHELL_NAME);
}

void HandleCmd(char** history, int *hbuffsize, int *hindex) {
	char *line;
	char** args;
	int status = 1;

	char cwd[PATH_MAX];

	while (status) {
		printf("\033[1;36m%s\033[0m > ", getcwd(cwd, sizeof(cwd)));

		line = ReadLine();

		if (feof(stdin)) {
			printf("\n");
			return;
		}

		LineLog(line, history, hbuffsize, hindex);
		
		if (strcmp(line, defCmd[6])) {
			args = Tokenise(line);
			status = Execute(args);
		}
		else {
			status = Execute(history);
		}

		free(line); //вот из-за этого и не работает хистори
		free(args);
	}
}

int main(int argc, char **argv) {
	int hbuffsize = CMDHIS_BUFFSIZE;
	int hindex = 0;
	char** history = malloc(hbuffsize * sizeof(char*));

	//history[hindex] = "Dafsdfsdf";
	
	struct passwd* info = getpwuid(geteuid());
	//printf("%s\n", info->pw_name);
	
	int envpathLen = 8 + strlen(info->pw_name) + strlen(CMDHIS_LOG);
	char* envpath = malloc(envpathLen * sizeof(char));
	strcpy(envpath, "/home/");
	strcat(envpath, info->pw_name);
	strcat(envpath, "/");
	strcat(envpath, CMDHIS_LOG);

	if (setenv(CMDHIS_ENV, envpath, 0) < 0) {
		perror(SHELL_NAME);
		return 1;
	}
	
	signal(SIGHUP, CatchSighup);
	signal(SIGINT, CatchSigint);
	HandleCmd(history, &hbuffsize, &hindex);
	
	free(history);
	return 0;
}

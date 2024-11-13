//headers of the C standart
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

//"sys" module headers
#include "sys/wait.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/mount.h"

//"linux" module headers
#include "linux/limits.h"
#include "linux/fs.h"
#include "linux/kernel.h"
//#include "linux/init.h"
#include "linux/module.h"
//#include "linux/pagemap.h"

//other unix-system feature modules
#include "unistd.h"
#include "pwd.h"
#include "dirent.h"
#include "signal.h"

//"defcoms" is a header file that includes all built-in shell commands
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
	/*printf("\n");
	for (int i = 0; args[i] != nullptr; i++) {
		printf(" |%s| ", args[i]);
	}
	printf("\n");
	*/

	if (args[0] == nullptr) return 1;

	for (int i = 0; i < DefNum(); i++) {
		if (strcmp(args[0], cmdList[i].cmdName) == 0) {
			return (*cmdList[i].cmdFunc)(args);
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
		
		if (strcmp(line, cmdList[6].cmdName)) {
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

void StarterArt(char* filename) {
	int width = 120, height  = 15;

	FILE* file = fopen(filename, "r");

	if (!file) {
		perror(SHELL_NAME);
		return;
	}

	char sym = fgetc(file);
	int i = 0;
	while (sym != EOF && i < width * height) {
		printf("%c", sym);
		sym = fgetc(file);
		i++;
	}

	printf("\n");
	fclose(file);
}

int main(int argc, char **argv) {
	int hbuffsize = CMDHIS_BUFFSIZE;
	int hindex = 0;
	char** history = malloc(hbuffsize * sizeof(char*));

	//history[hindex] = "Dafsdfsdf";
	printf("%s process id: \033[1;36m%d\033[0m\n", SHELL_NAME, getpid());	
	
	struct passwd* info = getpwuid(geteuid());
	printf("User id: %d\n", info->pw_uid);

	int hisnameLen = strlen(CMDHIS_LOG) + 1;
	
	int envpathLen;
	if (info->pw_uid != 0) {
		envpathLen = strlen(CMDHIS_PATH) - 1 + strlen(info->pw_name) + hisnameLen;
	}
	else {
		envpathLen = strlen(CMDHIS_PATH) - 1 + hisnameLen;
	}

	char* hisname = (char*)malloc(hisnameLen * sizeof(char));
	char* envpath = (char*)malloc(envpathLen * sizeof(char));
	
	if (info->pw_uid != 0) {
		snprintf(envpath, envpathLen, CMDHIS_PATH, info->pw_name);
	}
	else {
		snprintf(envpath, envpathLen, CMDHIS_PATH, "");
	}
	strcpy(hisname, CMDHIS_LOG);
	strcat(envpath, hisname);

	if (setenv(CMDHIS_ENV, envpath, 0) < 0) {
		perror(SHELL_NAME);
		return 1;
	}
	
	signal(SIGHUP, CatchSighup);
	signal(SIGINT, CatchSigint);

	StarterArt("result.txt");

	HandleCmd(history, &hbuffsize, &hindex);
	
	free(history);
	free(hisname);
	free(envpath);
	return 0;
}

#include "stdio.h"
#include "stdlib.h"

#define CMD_BUFFSIZE 1024
#define true 1
#define false 0

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
	int buffSize = CMD_BUFFSIZE;
	int position = 0;
	char* buffer = malloc(sizeof(char) * buffSize);
	int sym;

	AllocCheck(buffer);

	while (true) {
		sym = getchar();

		if (sym == EOF || sym == '\0') {
			buffer[position] = '\0';
			return buffer;
		}
		else buffer[position] = sym;

		if (position >= buffSize) {
			buffSize += CMD_BUFFSIZE;

			buffer = realloc(buffer, buffSize);

			AllocCheck(buffer);
		}

		position++;
	}
}

void Loop(void) {
	char *line;
	char** args;
	int status = 0;

	while (status) {
		printf("> ");
		
		ReadLine();

		free(line);
		free(args);
	}
}

int main(int argc, char **argv) {

	return 0;
}

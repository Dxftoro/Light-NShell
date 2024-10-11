#include "stdio.h"
#include "stdlib.h"

#define CMD_BUFFSIZE 1024

enum ERR {
	BUFF_MEM_ALLOC_ERROR,
	BUFF_ERROR
};

char* ReadLine(void) {
	int buffSize = CMD_BUFFSIZE;
	int position = 0;
	char* buffer = malloc(sizeof(char) * buffSize);
	int c;

	if (!buffer) {
		fprintf(stderr, "BUFF_MEM_ALLOC_ERROR");
		exit(1);
	}
}

void Loop(void) {
	char *line;
	char** args;
	int status = 0;

	do { 
		printf("> ");
	} while(status);
}

int main(int argc, char **argv) {

	return 0;
}

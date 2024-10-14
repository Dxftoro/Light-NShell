#define CMD_BUFFSIZE 1024
#define TOKEN_BUFFSIZE 64
#define DIRN_BUFFSIZE 128
#define CMDHIS_BUFFSIZE 512
#define SHELL_NAME "l-nshell"
#define CMDHIS_LOG "l-nshell_cmd.log"
#define true 1
#define false 0
#define nullptr NULL

char* defCmd[] = {
	"help",
	"exit",
	"cd",
	"ls",
	"echo",
	"cyctest",
	"hsr"
};

int DefNum() {
	return sizeof(defCmd) / sizeof(char*);
}

int CmdCd(char** args) {
	if (args[1] != nullptr) {
		int cdresult = chdir(args[1]);
		if (cdresult != 0) perror(SHELL_NAME);
	}
	else {
		fprintf(stderr, "Expecting argument \"path\" for cd\n");
	}

	return 1;
}

int CmdHelp(char** args) {
	printf("List of default commands:\n");

	for (int i = 0; i < DefNum(); i++) printf("%s\n" ,defCmd[i]);

	return 1;
}

int CmdExit(char** args) {
	return 0;
}

int CmdLs(char** args) {
	char cwd[PATH_MAX];
	
	DIR* dirp = opendir(getcwd(cwd, sizeof(cwd)));
	struct dirent *dir;

	int hideHidden = 1;
	if (args[1] != nullptr) {
		hideHidden = strcmp(args[1], "-d");
	}

	if (dirp) {
		while ((dir = readdir(dirp)) != nullptr) {
			if (!hideHidden || dir->d_name[0] != '.') {
				if (dir->d_type == DT_DIR) printf("\033[1;32m%s\033[0m\t", dir->d_name);
				else printf("%s\t", dir->d_name);
			}
		}
		printf("\n");
	}
	else {
		perror(SHELL_NAME);
	}

	closedir(dirp);
	return 1;
}

int CmdEcho(char** args) {
	int i = 1;

	while (args[i] != nullptr) {
		printf("%s ", args[i]);
		i++;
	}
	printf("\n");

	return 1;
}

int CmdCyctest(char** args) {
	int echo = 0;
	while (1) {
		echo = CmdEcho(args);
	}

	return 1;
}

int CmdHsr(char** args) {
	FILE* file = fopen(CMDHIS_LOG, "r");
	
	if (file) {
		char* content;
		while (fscanf(file, "%s", content) != EOF) {
			printf("%s\n", content);
		}
	}
	else printf("%s: Can't open \"%s\"", SHELL_NAME, CMDHIS_LOG);
	return 1;
}

void CatchSighup() {
	printf("Configuration reloaded!");
}

void CatchSigint() {
	printf("\nTERMINATING %s\n", SHELL_NAME);
	exit(1);
}

int (*defFuncs[]) (char**) = {
	&CmdHelp,
	&CmdExit,
	&CmdCd,
	&CmdLs,
	&CmdEcho,
	&CmdCyctest,
	&CmdHsr
};


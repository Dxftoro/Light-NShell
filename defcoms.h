#define CMD_BUFFSIZE 	1024
#define TOKEN_BUFFSIZE 	64
#define DIRN_BUFFSIZE 	128
#define CMDHIS_BUFFSIZE 512

#define SHELL_NAME 	"l-nshell"

#define CMDHIS_PATH 	"/home/%s"
#define CMDHIS_LOG	"/l-nshell_cmdhis.log"
#define CMDHIS_ENV 	"LNSHELL_CMDHIS"

#define PROCMEM_PAT 	"/proc/%d/map_files/"
#define DUMP_PATH_PAT	"/var/l-nshell_dumps/d%d/"
#define DUMP_FILE_PAT	"%s.dump"

#define CRONFS_TARGET	"/tmp/vfs"

#define true 		1
#define false 		0
#define nullptr 	NULL

#include "blstruct.h"
#include "cronfs.h"
#include "dumper.h"

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

void Rmkdir(char* filepath) {
	size_t fpSize = strlen(filepath);
	char* path = (char*)malloc((fpSize + 1) * sizeof(char));
	
	path[fpSize] = '\0';

	for (int i = 0; i < fpSize; i++) {
		path[i] = filepath[i];

		if (path[i] == '/' || path[i] == '\0' || i == fpSize - 1) mkdir(path, 0755);
	}

	free(path);
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

int CmdExit(char** args) {
	return 0;
}

int CmdLs(char** args) {
	char cwd[PATH_MAX];
	
	DIR* dirp = nullptr;
	struct dirent *dir;

	int hideHidden = 1;
	int pathDefined = 0;

	for (int i = 1; args[i] != nullptr; i++) {
		if (args[i][0] != '-' && !pathDefined) {
			dirp = opendir(args[i]);
			pathDefined = 1;
		}
		else hideHidden = strcmp(args[i], "-d");
	}

	if (!pathDefined) {
		dirp = opendir(getcwd(cwd, sizeof(cwd)));
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
	while (true) {
		echo = CmdEcho(args);
	}

	return 1;
}

int CmdHsr(char** args) {
	char* env = getenv(CMDHIS_ENV);
	FILE* file = fopen(env, "r");
	
	if (!file) {
		printf("%s: Can't open \"%s\"\n", SHELL_NAME, env);
		return 1;
	}

	char content[256];
	while (fgets(content, 256, file) != nullptr) {
		printf("%s", content);
	}
	printf("\n");
	
	return 1;
}

int CmdE(char** args) {
	if (args[1] != nullptr) {
		char* envPath = getenv(args[1]);
		if (envPath != nullptr) {
			printf("%s: %s\n", args[1], getenv(args[1]));
		}
		else printf("There is no variable called \"%s\"\n", args[1]);
	}
	else printf("Expecting argument \"name\" for \\e\n");

	return 1;
}

int CmdL(char** args) {	
	if (args[1] == nullptr) {
		printf("Expecting argument \"path\" for \\l\n");
		return 1;
	}

	FILE* mount = fopen(args[1], "rb");
	if (!mount) { 
		printf("Unknown device \"%s\"!\n", args[1]);
		return 1;
	}
	
	MBR block;
	ReadMBR(mount, &block);
	
	printf("Signature of %s: %d\n", args[1], block.signature);

	fclose(mount);

	return 1;
}

int CmdCron(char** args) {
	char source[] = "/var/spool/cron/crontabs/";
	char target[] = CRONFS_TARGET;
	unsigned long flags = MS_BIND;

	Rmkdir(target);

	if (mount(source, target, "tmpfs", flags, "mode=0755") == -1) {
		perror(SHELL_NAME);
		return 1;
	}

	printf("%s successfuly mounted to %s!\n", source, target);

	return 1;
}

int CmdMem(char** args) {
	if (args[1] == nullptr) {
		printf("Expecting argument \"pid\" for \\l\n");
		return 1;
	}

	char* username = getenv("USER");
	if (username == nullptr) {
		perror(SHELL_NAME);
		return 1;
	}

	pid_t procid = atoi(args[1]);

	char* procp, *dumpp;
	asprintf(&procp, PROCMEM_PAT, procid);
	asprintf(&dumpp, DUMP_PATH_PAT, procid);

	printf("Reading from %s\n", procp);
	Rmkdir(dumpp);

	DIR* dirp = opendir(procp);

	if (dirp == nullptr) {
		perror(SHELL_NAME);
		return 1;
	}

	struct dirent* dir;
	while ((dir = readdir(dirp)) != nullptr) {
		if (dir->d_type != DT_DIR) {
			char *dumpf;
			asprintf(&dumpf, DUMP_FILE_PAT, dir->d_name);
			
			char *procPart = (char*)malloc((strlen(procp) + strlen(dir->d_name) + 1) * sizeof(char));
			strcpy(procPart, procp);
			strcat(procPart, dir->d_name);

			char *dumpfFull = (char*)malloc((strlen(dumpp) + strlen(dumpf) + 1) * sizeof(char));	
			strcpy(dumpfFull, dumpp);
			strcat(dumpfFull, dumpf);

			DumpPart(procPart, dumpfFull);

			free(dumpf);
			free(procPart);
			free(dumpfFull);
		}
	}
	
	printf("Dumped process %d to %s\n", procid, dumpp);
	free(procp); free(dumpp);
	return 1;
}

void CatchSighup() {
	printf("Configuration reloaded!");
}

void CatchSigint() {
	printf("\nTERMINATING %s\n", SHELL_NAME);
	exit(1);
}

typedef struct {
	char* cmdName;
	char* cmdDescr;
	int (*cmdFunc) (char**);
} DefCmd;

int CmdHelp(char** args);

DefCmd cmdList[] = {
	{ "help", "print this list", &CmdHelp },
	{ "exit", "terminate l-nshell", &CmdExit },
	{ "\\q", "do the same as \"exit\" command", &CmdExit },
	{ "cd", "[path] change working directory", &CmdCd },
	{ "ls", "[path] list all files in the specified folder", &CmdLs },
	{ "echo", "[text] prints specified text", &CmdEcho },
	{ "cyctest", "[text or nothig] run a test of terminating signal (when it runs just press Ctrl+C to terminate l-nshell)", &CmdCyctest },
	{ "hsr", "view command history", &CmdHsr },
	{ "\\e", "[envname] print a value of specified environment variable", &CmdE },
	{ "\\l", "[path to block device] print signature of specified block device", &CmdL },
	{ "\\cron", "no functionality", &CmdCron },
	{ "\\mem", "[process id] dump process memory with specified id", &CmdMem }
};

int DefNum() {
	return sizeof(cmdList) / sizeof(DefCmd);
}

int CmdHelp(char** args) {
	printf("List of default commands:\n");
	for (int i = 0; i < DefNum(); i++) printf("%s\t- %s;\n", cmdList[i].cmdName, cmdList[i].cmdDescr);
	return 1;
}

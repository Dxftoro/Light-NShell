#define CMD_BUFFSIZE 	1024
#define TOKEN_BUFFSIZE 	64
#define DIRN_BUFFSIZE 	128
#define CMDHIS_BUFFSIZE 512

#define SHELL_NAME 	"l-nshell"

#define CMDHIS_PATH 	"/home/%s"
#define CMDHIS_LOG	"/l-nshell_cmdhis.log"
#define CMDHIS_ENV 	"LNSHELL_CMDHIS"

#define PROCMEM_PAT 	"/proc/%d/map_files/"
#define DUMP_PATH_PAT	"/home/%s/l-nshell_dumps/d%d"
#define DUMP_FILE_PAT	"%s.dump"

#define true 		1
#define false 		0
#define nullptr 	NULL

#include "blstruct.h"
#include "dumper.h"

char* defCmd[] = {
	"help",
	"exit",
	"cd",
	"ls",
	"echo",
	"cyctest",
	"hsr",
	"\\e",
	"\\l",
	"\\cron",
	"\\mem"
};

int DefNum() {
	return sizeof(defCmd) / sizeof(char*);
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
	while (1) {
		echo = CmdEcho(args);
	}

	return 1;
}

int CmdHsr(char** args) {
	char* env = getenv(CMDHIS_ENV);
	FILE* file = fopen(env, "r");
	
	if (file) {
		char* content;
		while (fscanf(file, "%s", content) != EOF) {
			printf("%s\n", content);
		}
	}
	else printf("%s: Can't open \"%s\"\n", SHELL_NAME, env);
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
	//for (int i = 0; i < 4; i++) {
	//	printf("%d\n", block.partitionTable[i].id);
	//}

	fclose(mount);

	return 1;
}

int CmdCron(char** args) {
	if (mount("/etc/cron.d", "/tmp/vfs", "tmpfs", MS_BIND, nullptr)) {
		
	}
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

	int idsize = strlen(args[1]);
	pid_t procid = atoi(args[1]);

	int procpSize = strlen(PROCMEM_PAT) - 1 + idsize;
	char* procp = (char*)malloc(procpSize * sizeof(char));

	snprintf(procp, procpSize, PROCMEM_PAT, procid);

	printf("---> %s\n", procp);

	DIR* dirp = opendir(procp);

	if (dirp == nullptr) {
		perror(SHELL_NAME);
		return 1;
	}

	struct dirent* dir;
	//while ((dir = readdir(dirp)) != nullptr) {
		//int filepSize = strlen(
		//char* filepath = (char*)malloc(filepathSize * sizeof(char));
	//}


	return 1;
}

/*int CmdMemDNU(char** args) {
	if (args[1] != nullptr) {
		char* user = getenv("USER");
		if (user == nullptr) {
			perror(SHELL_NAME);
			return 1;
		}

		//gettind pid
		int idsize = strlen(args[1]);
		pid_t procid = atoi(args[1]);

		//printing to proccess path patt
		int procpSize = strlen(PROCMEM_PAT) - 1 + idsize;
		char* procp = (char*)malloc(procpSize * sizeof(char));

		snprintf(procp, procpSize, PROCMEM_PAT, procid);

		//printing to dump file name patt
		int dumpfSize = strlen(DUMP_FILE_PAT) - 1 + idsize;
		char* dumpf = (char*)malloc(dumpfSize * sizeof(char));

		snprintf(dumpf, dumpfSize, DUMP_FILE_PAT, procid);

		//printing to dump file path patt
		int dumppSize = strlen(DUMP_PATH_PAT) - 2 + strlen(user) + dumpfSize;
		char* dumpp = (char*)malloc(dumppSize * sizeof(char));

		snprintf(dumpp, dumppSize, DUMP_PATH_PAT, user);
		printf("---- %d\n", mkdir(dumpp, 0700));
		strcat(dumpp, dumpf);
		
		printf("%d: %s\n", procpSize, procp);
		printf("%d: %s\n", dumpfSize, dumpf);
		printf("%d: %s\n", dumppSize, dumpp);
		
		//getting map_files
		
		FILE* memfile = fopen(procp, "r");
		if (!memfile) {
			perror(procp);
			return 1;
		}

		

		/*FILE* dumpfile = fopen(dumpp, "w");	
		char* content;
		if (!dumpfile) {
			while (fscanf(memfile, "%s", content) != EOF) {
				printf("%s", content);
				fputs(content, dumpfile);
			}
		}
		else {
			perror(dumpp);
			return 1;
		}
		
		fclose(memfile);
		fclose(dumpfile);*/
		//free(procp);
		//free(dumpf);
		//free(dumpp);
	//}

	//return 1;
//}

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
	&CmdHsr,
	&CmdE,
	&CmdL,
	&CmdCron,
	&CmdMem
};


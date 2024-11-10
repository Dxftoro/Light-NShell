#define BYTES_PER_READ 1024

int DumpPart(char* filepath, char* dumppath) {
	int file = open(filepath, O_RDONLY);
	FILE* dump = fopen(dumppath, "wb+");

	if (file < 0) {
		perror("dumper");
		return false;
	}
	
	char rdbuff[BYTES_PER_READ];
	size_t bytesRead = read(file, rdbuff, BYTES_PER_READ);
	size_t bytesWritten = 0;

	for(; bytesRead > 0; bytesRead = read(file, rdbuff, BYTES_PER_READ)) {
		bytesWritten += fwrite(rdbuff, sizeof(char), bytesRead, dump);
	}
	
	if (dump != nullptr) fclose(dump);

	printf("%s - %ld bytes written\n", dumppath, bytesWritten);
	return true;
}

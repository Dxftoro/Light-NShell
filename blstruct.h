typedef struct {
	uint8_t active;
	uint8_t starterHead;
	uint16_t starterCns;
	uint8_t id;
	uint8_t endHead;
	uint16_t endCns;
	uint32_t prevCount;
	uint32_t sectorCount;
} MBRpartition;

typedef struct {
	char code[446];
	MBRpartition partitionTable[4];
	uint16_t signature;
} MBR;

void ReadMBRPartition(FILE* file, MBRpartition* part) {
	fread(&part->active, sizeof(part->active), 1, file);
	fread(&part->starterHead, sizeof(part->starterHead), 1, file);
	fread(&part->starterCns, sizeof(part->starterCns), 1, file);
	fread(&part->id, sizeof(part->id), 1, file);
	fread(&part->endHead, sizeof(part->endHead), 1, file);
	fread(&part->endCns, sizeof(part->endCns), 1, file);
	fread(&part->prevCount, sizeof(part->prevCount), 1, file);
	fread(&part->sectorCount, sizeof(part->sectorCount), 1, file);
}

void ReadMBR(FILE* file, MBR* table) {
	int codeSize = sizeof(table->code) / sizeof(char);
	fread(table->code, sizeof(char), codeSize, file);
	
	int ptableSize = sizeof(table->partitionTable) / sizeof(MBRpartition);
	for (int i = 0; i < ptableSize; i++) {
		ReadMBRPartition(file, &table->partitionTable[i]);
	}

	fread(&table->signature, sizeof(table->signature), 1, file);
}

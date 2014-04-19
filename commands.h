#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define ARGS 3
#define LINE 128
#define D_PTR_CNT 12
#define INDEX_LENGTH 4
#define ENTRY_SIZE (LINE - INDEX_LENGTH)


#define ONE_KB 1024
#define ONE_MB (1024*1024)
#define COUNT_GB 4
#define IMG_NAME "_VIRTUAL_DISK_IMG"

#define BLOCK_SIZE (4 * ONE_KB)
#define DISK_SIZE (100 * ONE_MB);
#define BLOCK_COUNT 250000
#define INODE_COUNT  20000
#define MAX_ENTRY_SIZE (ONE_KB+D_PTR_CNT)
#define MAX_OPEN_FILE 	128
#define BUFFER_SIZE (2 * ONE_KB)
#define SYNC_SIZE ONE_KB

#define INDEX_IN_SIN (BLOCK_SIZE / INDEX_LENGTH)
#define INDEX_IN_DIN ((BLOCK_SIZE / INDEX_LENGTH) * INDEX_IN_SIN)
#define INDEX_IN_TIN ((BLOCK_SIZE / INDEX_LENTGH) * INDEX_IN_DIN)

typedef enum
{
	FALSE = 0,
	TRUE,
}
BOOL
;

typedef enum
{
 noError,
 parseError,
 argError,
 ioError,
 exitError,
 memError,
 unInitError,
 dupError,
}
usageErrorType;

typedef enum
{
	iNode_Empty = 0,
	iNode_Dir,	
	iNode_File,
}
iNode_Type;

typedef struct
{	
int directPtr[D_PTR_CNT]; 
int s_indirectPtr;
int d_indirectPtr;
int t_indirectPtr;
char fileName[LINE];
int selfIndex;
int parentIndex;
int link;
int size;
short offset; 
iNode_Type type; 
} iNode;



typedef struct
{
int totalBlocks;
int freeBlocks;
int totalINode;
int freeINode;

int bitMapOffset;
int iNodeMapOffset;
int iNodeOffset;
int dataOffset;

int maxOpenFile;
} 
superBlock;

typedef struct
{
	iNode* oNode;
	int offset;
	char* buf;
	int bufUsed;
	char flag;
}
OpenedFile;

typedef struct
{
superBlock* sb;
char* bitMapPointer;
char* freeInodePointer;
iNode* firstiNode;
int currentDir;
void* dataPointer;
OpenedFile* openedFiles;
int openedFileCount;
iNode* curDirNode;
}
GLOBAL_Pointers;

usageErrorType Init_fs();
usageErrorType Smkfs();
usageErrorType Sopen(char* filename, char* flag);
usageErrorType Sread(int fd, int size);
usageErrorType Swrite(int fd, char* buffer);
usageErrorType Sseek(int fd, int offset);
usageErrorType Sclose(int fd);
usageErrorType Smkdir(char* dirname);
usageErrorType Srmdir(char* dirname);
usageErrorType Slink(char* src, char* dest);
usageErrorType Sunlink(char* name);
usageErrorType Sstat(char* name);
usageErrorType Sls();
usageErrorType Scd(char* dirname);
usageErrorType Scat(char* name);
usageErrorType Scp(char* src, char* dest);
usageErrorType Stree();
usageErrorType Simport(char* srcname, char* destname);
usageErrorType Sexport(char* srcname, char* destname);

usageErrorType parseCommand(char* command);
int process_arguments(char* command, char** paras);
usageErrorType batch_execute(char* script);
usageErrorType checkDuplicate(iNode* CD, char* dir);
int get_free_iNode();
int get_free_block();
void free_block(int nBlock);
usageErrorType sync_meta_to_disk();
void getAbsPath(iNode* cd);
usageErrorType changeDirectory(iNode* entry);
usageErrorType writeEntryInBlock(int blockIndex, int entryIndex, int freeNode);
void PrintEntry(iNode* entry);
void findSubEntries(int* array, iNode* CD);
usageErrorType PrintTree(iNode* entry, int level);
void PrintDash(int level);
usageErrorType mkdir_unit(char* dirname);
usageErrorType rmdir_unit(iNode* entry);
iNode* findiNodeByName(char* name, iNode* CD);
usageErrorType addSubEntry(iNode* CD, int freeNode);
usageErrorType createFile(char* filename);
int openFile(char* filename, char flag);
int addOpenFile(iNode* entry, char flag);
int checkOpened(iNode* entry);
usageErrorType PrintStat(iNode* entry);
usageErrorType sync_data_to_disk(OpenedFile* handler);
int locateIndex(int offset, int index);
int calcBlockIndex(int cBlock, iNode* entry);
int assignNewBlock(iNode* entry);
usageErrorType	writeEntryInBlockSerial(int index, int offset, int data);
usageErrorType writeWithinBlock(int cBlockIndex, int cOffset, OpenedFile* handler, int len);
usageErrorType deleteOpenedFile(OpenedFile* handler);
usageErrorType read_to_buffer(OpenedFile* handler, int size, char* buffer);
usageErrorType load_bytes_from_block(int ablock, int offset, int actualBytes,
																		 char* buffer, OpenedFile* handler);
int do_open(char* namefilename, char* flag, BOOL checkOpen);
usageErrorType do_read(int fd, char* buffer, int size);
usageErrorType releaseDataBlock(iNode* entry, int blockCount);
usageErrorType do_copy(char* src, char* dest);
usageErrorType do_copy_from_entry(iNode* sEntry, char* dest);
usageErrorType copy_data_via_block(int fd, int sblock, int dblock);

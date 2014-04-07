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

#define ONE_KB 1024
#define ONE_MB 1024*1024
#define COUNT_GB 4
#define IMG_NAME "_VIRTUAL_DISK_IMG"

#define BLOCK_SIZE 4 * ONE_KB


typedef enum
{
 noError,
 parseError,
 argError,
 ioError,
 exitError
}
usageErrorType;

typedef struct
{
void* directPtr[D_PTR_CNT];
void* s_indirectPtr;
void* d_indirectPtr;
void* t_indirectPtr;
char fileName[LINE];
} iNode;

typedef struct
{
int totalBlocks;
int freeBlocks;
iNode* firstNode;

} superBlock;


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
usageErrorType Scat(char* name);
usageErrorType Scp(char* src, char* dest);
usageErrorType Stree();
usageErrorType Simport(char* srcname, char* destname);
usageErrorType Sexport(char* srcname, char* destname);


usageErrorType parseCommand(char* command);

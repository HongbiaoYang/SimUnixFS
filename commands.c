#include "commands.h"

extern  GLOBAL_Pointers* g_pointer;
extern char path[LINE];

usageErrorType Smkfs()
{
	printf("mkfs called\n");
	
	int fd, i;
	off_t rs;
	
	fd = open(IMG_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
	
	/* write the super block */
	superBlock* sb;
	sb = (superBlock*) malloc(sizeof(superBlock));
	if (sb == NULL)
	{
		perror("Malloc Error!");
		exit(memError);
	}
	
	/* write the bitmap for free block management */
	char* bitMap;
	bitMap = (char*) malloc(BLOCK_COUNT);
	memset(bitMap, 0, BLOCK_COUNT);	
	
	/* write the iNodeBitmap for free iNodes management */
	char* iNodeMap;
	iNodeMap = (char*) malloc(INODE_COUNT);
	memset(iNodeMap, 0, INODE_COUNT);
	iNodeMap[0] = 1; // the first iNode is used by root dir "/"
	
	/* write the iNode */
	iNode* inode;
	inode = (iNode*) malloc(sizeof(iNode) * BLOCK_COUNT);
	memset(inode, 0, sizeof(iNode) * BLOCK_COUNT);
	
	// the first iNode: root directory
	inode->type = iNode_Dir;
	strcpy(inode->fileName, "/");
	inode->s_indirectPtr = 0;	// current directory: .
	inode->d_indirectPtr = 0;  // parent directory: ..
	inode->size = 0; // number of sub directory and files
	
	sb->totalBlocks = BLOCK_COUNT;
	sb->freeBlocks = BLOCK_COUNT;
	sb->totalINode = INODE_COUNT;
	sb->freeINode = INODE_COUNT - 1;
	
	sb->bitMapOffset = sizeof(superBlock);
	sb->iNodeMapOffset = sb->bitMapOffset + BLOCK_COUNT;
	sb->iNodeOffset = sb->iNodeMapOffset + INODE_COUNT;
	
	write(fd, sb, sizeof(superBlock));
	write(fd, bitMap, sb->totalBlocks);
	write(fd, iNodeMap, sb->totalINode);
	write(fd, inode, sb->totalINode * sizeof(iNode));
	
	/* format all data to empty */
			
	/* create the image with size 100MB	 */
	lseek(fd, 100 * ONE_MB - 1, SEEK_SET);
	write(fd, "\n", 1);
	close(fd);	
	
	// init the new fs
	Init_fs();
	
	return (noError);
}

usageErrorType Sopen(char* filename, char* flag)
{
	printf("open called\n");
	return (noError);
}

usageErrorType Sread(int fd, int size)
{
	printf("read called\n");
	return (noError);
}


usageErrorType Swrite(int fd, char* buffer)
{
	printf("write called\n");
	return (noError);
}


usageErrorType Sseek(int fd, int offset)
{
	printf("seek called\n");
	return (noError);
}


usageErrorType Sclose(int fd)
{
	printf("close called\n");
	return (noError);
}

usageErrorType Scd(char* dirname)
{
	iNode *CD, *entry;
	CD = g_pointer->firstiNode + g_pointer->currentDir;
	int i, entrySize;	
	
	entrySize = CD->size;
	
	for (i = 0; i < D_PTR_CNT; i++)
	{
		entry = g_pointer->firstiNode + CD->directPtr[i];
		if (CD->directPtr[i] == 0 || entry->type == iNode_Empty)
		{
			continue;
		}
		else if (entry->type == iNode_Dir)
		{

			if (strcmp(entry->fileName, dirname) == 0)
			{
				g_pointer->currentDir = entry->s_indirectPtr;
				memset(path, 0, LINE);
				getAbsPath(entry);
				return noError;
			}
			else
			{
				entrySize--;
			}
		}
		else if (entry->type == iNode_File)
		{
			entrySize--;
		}
		
		if (entrySize <= 0)
		{
			break;
		}
	
	}
	
	printf("cd: No such file or directory:%s\n", dirname);	
	
	return (ioError);
	
}

usageErrorType Smkdir(char* dirname)
{
	// get the current directory iNode
	iNode* CD = g_pointer->firstiNode + g_pointer->currentDir;
	int i, firstAvailPoint, entrySize;
	
	entrySize = CD->size;
	if (entrySize == 0)	// empty directory
	{
		firstAvailPoint = 0;
	}
	else if (0 < entrySize && entrySize < 12) // direcotory with 1~11 entries
	{
		if (checkDuplicate(CD, dirname) == dupError)
		{
			printf("Fail: File or Directory Exist!\n");
			return dupError;
		}
		
		firstAvailPoint = findAvailableEntry(CD);
		if (firstAvailPoint == -1)
		{
			printf("Entry Size error!\n");
			return ioError;
		}
		
	}
	else		// more than 12 entries
	{
		printf("Entry full (12), implement later\n");
		return noError;
	}
	
	int freeNode = get_free_iNode();
	if (freeNode == -1)
	{
		printf("Not Enough iNode available!\n");
		return ioError;
	}
	
	g_pointer->freeInodePointer[freeNode] = 1;
	iNode* newDir = g_pointer->firstiNode + freeNode;
	
	// create new directory
	newDir->s_indirectPtr = freeNode;
	newDir->d_indirectPtr = g_pointer->currentDir;
	newDir->type = iNode_Dir;
	strcpy(newDir->fileName, dirname);	
	
	CD->directPtr[firstAvailPoint] = freeNode;
	CD->size++;
	g_pointer->sb->freeINode -= 1;
	 
	
	sync_to_disk();	
	printf("mkdir succeed!");
	
	return (noError);
}


usageErrorType Srmdir(char* dirname)
{
	printf("rmdir called\n");
	return (noError);
}


usageErrorType Slink(char* src, char* dest)
{
	printf("link called\n");
	return (noError);
}


usageErrorType Sunlink(char* name)
{
	printf("unlink called\n");
	return (noError);
}


usageErrorType Sstat(char* name)
{
	printf("stat called\n");
	return (noError);
}


usageErrorType Sls()
{
	
	iNode *CD, *entry;
	CD = g_pointer->firstiNode + g_pointer->currentDir;
	int i, entrySize;	
	
	entrySize = CD->size;
	
	printf("..\n.\n");
	
	for (i = 0; i < D_PTR_CNT; i++)
	{
		entry = g_pointer->firstiNode + CD->directPtr[i];
		if (CD->directPtr[i] == 0 || entry->type == iNode_Empty)
		{
			continue;
		}
		else if (entry->type == iNode_Dir)
		{
			printf("%s/\n", entry->fileName);
			entrySize--;
		}
		else if (entry->type == iNode_File)
		{
			printf("%s\n", entry->fileName);
			entrySize--;
		}
		
		if (entrySize <= 0)
		{
			break;
		}
	
	}	
	
	return (noError);
}


usageErrorType Scat(char* name)
{
	printf("cat called\n");
	return (noError);
}


usageErrorType Scp(char* src, char* dest)
{
	printf("cp called\n");
	return (noError);
}


usageErrorType Stree()
{
	printf("tree called\n");
	return (noError);
}


usageErrorType Simport(char* srcname, char* destname)
{
	printf("import called\n");
	return (noError);
}


usageErrorType Sexport(char* srcname, char* destname)
{
	printf("mkfs called\n");
	return (noError);
}

// check if there exists a duplicated file or directory
usageErrorType checkDuplicate(iNode* CD, char* dir)
{
	int entrySize = CD->size;
	int i;
	iNode* entry;
	
	for (i = 0; i < D_PTR_CNT; i++)
	{
		entry = g_pointer->firstiNode + CD->directPtr[i];
		
		// if index is 0, this is empty
		if (CD->directPtr[i] == 0)
		{
			continue;
		}
		else if (entry->type == iNode_Empty)
		{
			continue;
		}
		else if (strcmp(entry->fileName, dir) == 0)
		{
			return dupError;
		}
		else
		{
			entrySize --;
		}
		if (entrySize <= 0)
		{
			return noError;
		}
	}	
}

// find an available entry
int findAvailableEntry(iNode* CD)
{
	int i;
	iNode* entry;
	
	for (i = 0; i < D_PTR_CNT; i++)
	{
		if (CD->directPtr[i] == 0)
		{
			return i;
		}
		
		entry = g_pointer->firstiNode + CD->directPtr[i];
		if (entry->type == iNode_Empty)
		{
			return i;
		}
	}
	
	return -1;
}

int get_free_iNode()
{
	int i;
	for (i = 1; i < INODE_COUNT; i++)
	{
		if (g_pointer->freeInodePointer[i] == 0)
		{
			return i;
		}
	}
	
	// no free inode found
	return -1;	
}

// sync the memory to disk
usageErrorType sync_to_disk()
{
	int fd, i;
	off_t rs;
	
	// overwite the origin file
	fd = open(IMG_NAME, O_RDWR | O_CREAT, S_IRWXU);
	
	lseek(fd, 0, SEEK_SET);
	
	// write the changed memory back into disk
	write(fd, g_pointer->sb, sizeof(superBlock));
	write(fd, g_pointer->bitMapPointer, g_pointer->sb->totalBlocks);
	write(fd, g_pointer->freeInodePointer, g_pointer->sb->totalINode);
	write(fd, g_pointer->firstiNode, g_pointer->sb->totalINode * sizeof(iNode));
	
	close(fd);
}

void getAbsPath(iNode* cd)
{
	if (cd->s_indirectPtr == 0)
	{
		strcpy(path, g_pointer->firstiNode->fileName);
	}
	else
	{
		iNode* parent = g_pointer->firstiNode + cd->d_indirectPtr;
		getAbsPath(parent);
		strcat(path, cd->fileName);
		strcat(path, "/");
	}
	
}

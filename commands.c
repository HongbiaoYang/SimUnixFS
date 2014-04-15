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
	sb->dataOffset = sb->iNodeOffset + sb->totalINode * sizeof(iNode);
	
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
	
	if (strcmp(dirname, ".") == 0)
	{
		return noError;
	}
	else if (strcmp(dirname, "..") == 0)
	{
		entry = g_pointer->firstiNode + CD->d_indirectPtr;
		return changeDirectory(entry);
	}	
	
	
	int* entryIndexArray;
	entryIndexArray = (int *)malloc(entrySize * sizeof(int));
	
	findSubEntries(entryIndexArray, CD);
	
	for (i = 0; i < entrySize; i++)
	{
		entry = g_pointer->firstiNode + entryIndexArray[i];
		if (entry->type == iNode_Dir && strcmp(entry->fileName, dirname) == 0)
		{
			free(entryIndexArray);
			return changeDirectory(entry);				
		}
		
	}
	
	free(entryIndexArray);	
	
	printf("cd: No such file or directory:%s\n", dirname);	
	
	return (ioError);	
}

usageErrorType Smkdir(char* dirname)
{
	char* sub;
	char 	tDir[LINE] = {0};
	
	if (strcmp(dirname, "") == 0 || strcmp(dirname, "/") == 0)
	{
		// empty dir or /
		return noError;
	}
	else if ((sub = strchr(dirname, '/')) == NULL)
	{
		// no / found
		// the deepest child		
			
		// check if there is duplicated directory or file
		iNode* CD = g_pointer->firstiNode + g_pointer->currentDir;
		if (checkDuplicate(CD, dirname) == dupError)
		{
			printf("Fail: File or Directory Exist!\n");
			return dupError;
		}	
	
		mkdir_unit(dirname);
	}
	else if (sub == dirname)
	{
		// the path name start with /, skip the first char
		Smkdir(sub+1);
	}
	else
	{
		// with /, it means there are more sub folders
		strncpy(tDir, dirname, sub - dirname);
		
		iNode* CD = g_pointer->firstiNode + g_pointer->currentDir;
		if (checkDuplicate(CD, tDir) != dupError)
		{
			mkdir_unit(tDir);
		}
		Scd(tDir);
		Smkdir(sub + 1);
		Scd("..");
	}	
	
}


usageErrorType Srmdir(char* dirname)
{
	iNode* CD = g_pointer->firstiNode + g_pointer->currentDir;
	iNode* entry;
	
	entry = findiNodeByName(dirname, CD);
	if (entry == NULL)
	{
		
		printf("rmdir: failed to remove '%s': No such file or directory\n", dirname);
		return ioError;
	}
	else if (entry->type == iNode_File)
	{
		printf("rmdir: failed to remove '%s': Not a directory\n", dirname);
		return ioError;
	}
	else if (entry->size > 0)
	{
		printf("rmdir: failed to remove '%s': Directory not empty\n", dirname);
		return ioError;
	}
	else
	{
		if (rmdir_unit(entry) == ioError)
		{
			printf("Error removing directory!\n");
			return ioError;
		}
	}

			
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
	
	int* entryIndexArray;
	entryIndexArray = (int*)malloc(sizeof(int)*entrySize);
	
	findSubEntries(entryIndexArray, CD);
	
	for (i = 0; i < entrySize; i++)
	{
		entry = g_pointer->firstiNode + entryIndexArray[i];
		PrintEntry(entry);
	}
	
	free(entryIndexArray);
	
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
	iNode* root = g_pointer->firstiNode;
	
	PrintTree(root, 0);	
	
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
	
	int* entryIndexArray;
	entryIndexArray = (int*)malloc(sizeof(int)*entrySize);
	
	findSubEntries(entryIndexArray, CD);
	
	for (i = 0; i < entrySize; i++)
	{

		// no duplicate found in all existing entries
		if (entrySize <= 0)
		{
			free(entryIndexArray);
			return noError;
		}
		
		entry = g_pointer->firstiNode + entryIndexArray[i];
		
		// check duplicated entry
		if (strcmp(entry->fileName, dir) == 0)
		{
			free(entryIndexArray);
			return dupError;
		}
	}
	
	free(entryIndexArray);
	
	return noError;
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
		
		/*
		entry = g_pointer->firstiNode + CD->directPtr[i];
		if (entry->type == iNode_Empty)
		{
			return i;
		}
		*/
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


int get_free_block()
{
	int i;
	for (i = 1; i < BLOCK_COUNT; i++)
	{
		if (g_pointer->bitMapPointer[i] == 0)
		{
			return i;
		}
	}
	
	// no free inode found
	return -1;	
}

usageErrorType writeEntryInBlock(int blockIndex, int entryIndex, int freeNode)
{
	int i, fd, rs;
	
	fd = open(IMG_NAME, O_RDWR, S_IRWXU);
	lseek(fd, g_pointer->sb->dataOffset + BLOCK_SIZE * blockIndex, SEEK_SET);
	
	int indexBuf[BLOCK_SIZE / 4] = {0};
	
	if (entryIndex == 0)
	{
		indexBuf[0] = freeNode;
	}
	else
	{		
		read(fd, indexBuf, BLOCK_SIZE);
		for (i = 1; i < BLOCK_SIZE; i++)
		{
			if (indexBuf[i] == 0)
			{
				indexBuf[i] = freeNode;
				break;
			}				
		}
	}
	
	// back to the start of the block
	lseek(fd, g_pointer->sb->dataOffset + BLOCK_SIZE * blockIndex, SEEK_SET);
	
	rs = write(fd, indexBuf, BLOCK_SIZE);
	close(fd);
	
	return noError;
}
	
	
	

// sync the memory to disk
usageErrorType sync_to_disk()
{
	int fd, i;
	off_t rs;
	
	// overwite the origin file
	fd = open(IMG_NAME, O_RDWR , S_IRWXU);
	
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

usageErrorType changeDirectory(iNode* entry)
{
	g_pointer->currentDir = entry->s_indirectPtr;
	memset(path, 0, LINE);
	getAbsPath(entry);
	return noError;
}

void PrintEntry(iNode* entry)
	{
	if (entry->type == iNode_Dir)
	{
		printf("%s/\n", entry->fileName);
	}
	else if (entry->type == iNode_File)
	{
		printf("%s\n", entry->fileName);
	}
}
			
void findSubEntries(int* array, iNode* CD)
{
	iNode* entry;
	int index, entrySize, i;
	index = 0;
	entrySize = CD->size;
	
	for (i = 0; i < D_PTR_CNT; i++)
	{
		entry = g_pointer->firstiNode + CD->directPtr[i];
		if (CD->directPtr[i] == 0 || entry->type == iNode_Empty)
		{
			continue;
		}
		else
		{
			array[index++] = CD->directPtr[i];
			entrySize--;
		}
		
		if (entrySize <= 0)
		{
			break;
		}
	
	}	
	
	if (CD->t_indirectPtr != 0)
	{
		int fd, rs;
		fd = open(IMG_NAME, O_RDONLY, S_IRWXU);
		lseek(fd, g_pointer->sb->dataOffset + BLOCK_SIZE * CD->t_indirectPtr, SEEK_SET);
	
		int indexBuf[BLOCK_SIZE / 4] = {0};
		rs = read(fd, indexBuf, BLOCK_SIZE);
		
		for (i = 0; i < BLOCK_SIZE / 4; i++)
		{
			if (indexBuf[i] == 0)
			{
				continue;
			}
			else
			{
				array[index++] = indexBuf[i];
				entrySize --;
			}
			
			if (entrySize <= 0)
			{
				break;
			}
		}
		
		close(fd);
		
	}
}
			
usageErrorType PrintTree(iNode* entry, int level)
{
	int i;
	iNode* child;
	int* entryIndexArray;
	
	entryIndexArray = (int*)malloc(sizeof(int) * entry->size);
	
	PrintDash(level);
	
	printf("%s\n", entry->fileName);
	
	findSubEntries(entryIndexArray, entry);
	
	for (i = 0; i < entry->size; i++)
	{
		child = g_pointer->firstiNode + entryIndexArray[i];
		if (child->type != iNode_Dir)
		{
			continue;
		}
		else if (child->size == 0)
		{
			PrintDash(level+1);
			printf("%s\n", child->fileName);
		}
		else
		{
			PrintTree(child, level + 1);
		}
	}
	
	free(entryIndexArray);	
	
}
			
void PrintDash(int level)
{
	int i;
	for (i = 0; i < level; i++)
	{
		printf("-");
	}
}



usageErrorType mkdir_unit(char* inputDir)
{	
	char dirname[LINE] = {0};
	strcpy(dirname, inputDir);
	
	if (strcmp(dirname, "/") == 0)
	{
		return ioError;;
	}
	else if (dirname[strlen(dirname) - 1] == '/')
	{
		dirname[strlen(dirname) - 1] = '\0';
	}
	
	// get the current directory iNode
	iNode* CD = g_pointer->firstiNode + g_pointer->currentDir;
	int i, firstAvailPoint, entrySize, blockIndex4Entry;
	
	entrySize = CD->size;
	if (entrySize >= MAX_ENTRY_SIZE)
	{
		printf("Err: Maxmum entry size (%d) exceeded!\n", entrySize);
		return ioError;
	}
		
	// assign a new iNode for this new directory entry
	int freeNode = get_free_iNode();
	if (freeNode == -1)
	{
		printf("Not Enough iNode available!\n");
		return ioError;
	}
	
	// mark free iNode number 
	g_pointer->freeInodePointer[freeNode] = 1;
	g_pointer->sb->freeINode -= 1;
	
	iNode* newDir = g_pointer->firstiNode + freeNode;	
	// create new directory
	newDir->s_indirectPtr = freeNode;
	newDir->d_indirectPtr = g_pointer->currentDir;
	newDir->type = iNode_Dir;
	strcpy(newDir->fileName, dirname);	
	
	// modify the parent directory
	if (entrySize == 0)	// empty directory
	{
		firstAvailPoint = 0;
	}
	else if (0 < entrySize && entrySize < 12) // direcotory with 1~11 entries
	{		
		
		firstAvailPoint = findAvailableEntry(CD);
		if (firstAvailPoint == -1)
		{
			printf("Entry Size error!\n");
			return ioError;
		}
		
	}
	else		// more than 12 entries
	{
		if (CD->t_indirectPtr != 0)
		{
			blockIndex4Entry = CD->t_indirectPtr;
			firstAvailPoint = -1;
		}
		else
		{
			blockIndex4Entry = get_free_block();
			CD->t_indirectPtr = blockIndex4Entry;
			firstAvailPoint = 0;
			
			g_pointer->bitMapPointer[blockIndex4Entry] = 1;
			g_pointer->sb->freeBlocks -= 1;
		}		
	}
	
	
	if (CD->t_indirectPtr  == 0)
	{
		// number of entries less than 12
		CD->directPtr[firstAvailPoint] = freeNode;
	}
	else
	{
		// number of entries more than 12	
		writeEntryInBlock(CD->t_indirectPtr, firstAvailPoint, freeNode);
	}
	
	CD->size++;		 	
	sync_to_disk();	
	
	return (noError);

}

iNode* findiNodeByName(char* name, iNode* CD)
{
	int entrySize, i;
	int* entryIndexArray;
	iNode* entry;
	
	entrySize = CD->size;
	entryIndexArray = (int *)malloc(entrySize * sizeof(int));
	
	findSubEntries(entryIndexArray, CD);
	
	for (i = 0; i < entrySize; i++)
	{
		entry = g_pointer->firstiNode + entryIndexArray[i];
		if (strcmp(entry->fileName, name) == 0)
		{
			free(entryIndexArray);
			return entry;				
		}		
	}
	
	
	free(entryIndexArray);	
	
	return NULL;
}

usageErrorType rmdir_unit(iNode* entry)
{
	iNode* parent;
	parent = g_pointer->firstiNode + entry->d_indirectPtr;
	int i, entrySize, directEntryCount;
	
	
	entrySize = parent->size;
	directEntryCount = 0;
	
	for (i = 0; i < D_PTR_CNT; i++)
	{
		if (parent->directPtr[i] == entry->s_indirectPtr)
		{
			parent->directPtr[i] = 0;
			parent->size --;
			
			g_pointer->freeInodePointer[entry->s_indirectPtr] = 0;
			g_pointer->sb->freeINode ++;
			
			return noError;
		}
		else
		{
			entrySize--;
			directEntryCount++;
		}
		
		if (entrySize <= 0)
		{
			return ioError;
		}	
	}	
	
	if (parent->t_indirectPtr != 0)
	{
		int fd, rs;
		fd = open(IMG_NAME, O_RDWR, S_IRWXU);
		lseek(fd, g_pointer->sb->dataOffset + BLOCK_SIZE * parent->t_indirectPtr, SEEK_SET);
	
		int indexBuf[BLOCK_SIZE / 4] = {0};
		rs = read(fd, indexBuf, BLOCK_SIZE);
		
		for (i = 0; i < BLOCK_SIZE / 4; i++)
		{
			if (indexBuf[i] == 0)
			{
				continue;
			}
			else if (indexBuf[i] == entry->s_indirectPtr)
			{
				indexBuf[i] = 0;
				parent->size --;
				
				// if the entry size equals to number of entries in 
				// direct pointers, means the block is empty
				if (parent->size == directEntryCount)
				{					
					g_pointer->bitMapPointer[parent->t_indirectPtr] = 0;
					g_pointer->sb->freeBlocks += 1;
					
					parent->t_indirectPtr = 0;
				}
				
				// free the iNode table and stat
				g_pointer->freeInodePointer[entry->s_indirectPtr] = 0;
				g_pointer->sb->freeINode ++;
				
				lseek(fd, g_pointer->sb->dataOffset + BLOCK_SIZE * parent->t_indirectPtr, SEEK_SET);
				rs = write(fd, indexBuf, BLOCK_SIZE);
				close(fd);				
				
				return noError;
			}
			else
			{
				entrySize --;
			}
			
			if (entrySize <= 0)
			{
				return ioError;
			}
		}
	}
	
	return ioError;
	
}


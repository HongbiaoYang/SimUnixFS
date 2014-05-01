#include "commands.h"

extern  GLOBAL_Pointers* g_pointer;
extern char path[LINE];
extern  FILE* outputStream;
extern  BOOL  outputFlag;

usageErrorType Smkfs()
{
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
	inode->selfIndex = 0;	// current directory: .
	inode->parentIndex = 0;  // parent directory: ..
	inode->size = 0; // number of sub directory and files
	inode->link = 1;
	
	sb->totalBlocks = BLOCK_COUNT;
	sb->freeBlocks = BLOCK_COUNT;
	sb->totalINode = INODE_COUNT;
	sb->freeINode = INODE_COUNT - 1;
	sb->maxOpenFile = MAX_OPEN_FILE;
	
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
	int openFd = do_open(filename, flag, TRUE);
	if (openFd != -1)
	{
		PrintOutPut("SUCCEES, File: %s opened, fd = %d\n", filename, openFd);
	}
}

usageErrorType Sread(int fd, int size)
{
	char* buffer;
	buffer = (char*)malloc(size + 1);
	memset(buffer, 0, size + 1);
	
	do_read(fd, buffer, size);
	PrintOutPut("%s\n", buffer);
	free(buffer);	
	
	return (noError);		
}

usageErrorType do_read(int fd, char* buffer, int size)
{
	OpenedFile* handler;
	
	if (g_pointer->openedFiles[fd].oNode == NULL)
	{
		PrintOutPut("read: No opened file found!\n");
		return argError;
	}
	else
	{
		handler = g_pointer->openedFiles + fd;
	}
	
	if (handler->flag != 'r' && handler->flag != '+')
	{
		PrintOutPut("Read failed! The file is write only!\n");
		return ioError;
	}
	
	if (size <= 0)
	{
		PrintOutPut("Illegal read bytes:%d\n", size);
		return argError;
	}
	
	if (handler->offset >= handler->oNode->size)
	{
		PrintOutPut("Read: end of file reached!\n");
		return ioError;
	}
	
	read_to_buffer(handler, size, buffer);
	
	return (noError);
}


usageErrorType Swrite(int fd, char* buffer)
{
	OpenedFile* handler;

	if (g_pointer->openedFiles[fd].oNode == NULL)
	{
		PrintOutPut("write: No opened file found!\n");
		return argError;
	}
	else
	{
		handler = g_pointer->openedFiles + fd;
	}
	
	if (handler->flag != 'w' && handler->flag != '+')
	{
		PrintOutPut("Write failed! The file is read only!\n");
		return ioError;
	}

	strcat(handler->buf, buffer);
	handler->bufUsed += strlen(buffer);
	
	if (handler->bufUsed >= SYNC_SIZE)
	{
		sync_data_to_disk(handler);
		sync_meta_to_disk();		
	}
	
	return (noError);
}


usageErrorType Sseek(int fd, int offset)
{
	OpenedFile* handler;
	
	if (g_pointer->openedFiles[fd].oNode == NULL)
	{
		PrintOutPut("seek: No opened file found!\n");
		return argError;
	}
	else
	{
		handler = g_pointer->openedFiles + fd;
	}
	
	if (offset < 0)
	{
		PrintOutPut("seek: Illegal offset! %d\n", offset);
		return argError;
	}
	
	handler->offset = offset;
	
	
	return (noError);
}


usageErrorType Sclose(int fd)
{	
	if (g_pointer->openedFiles[fd].oNode == NULL)
	{
		PrintOutPut("No such file opened!\n");
		return ioError;
	}
	else
	{
		OpenedFile* handler;
		handler = g_pointer->openedFiles + fd;
		
		sync_data_to_disk(handler);
		sync_meta_to_disk();
		
		deleteOpenedFile(handler);
	}		
		
	return (noError);
}

usageErrorType Scd(char* dirname)
{
	iNode *CD, *entry;
	CD = g_pointer->curDirNode;
	int i, entrySize;	
	char* sub;
	
	entrySize = CD->size;	
	
	if (strcmp(dirname, "/") == 0 ||
		  strcmp(dirname, "") == 0)
	{
		return changeDirectory(g_pointer->firstiNode);
	}
	else if ((sub = strchr(dirname, '/')) == NULL)
	{
	
		entry = findiNodeByName(dirname, CD);
		
		if (entry == NULL)
		{
			PrintOutPut("cd: No such file or directory:%s\n", dirname);	
			return ioError;
		}
		else if (entry->type != iNode_Dir)
		{
			PrintOutPut("cd: not a directory: %s\n", entry->fileName);
			return ioError;
		}
		
		return changeDirectory(entry);
	}
	else
	{
		*sub = '\0';
		Scd(dirname);
		Scd(sub + 1);
	}
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
		iNode* CD = g_pointer->curDirNode;
		if (checkDuplicate(CD, dirname) == dupError)
		{
			PrintOutPut("Fail: File or Directory Exist!\n");
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
		
		iNode* CD = g_pointer->curDirNode;
		if (checkDuplicate(CD, tDir) != dupError)
		{
			mkdir_unit(tDir);
		}
		
		Scd(tDir);
		Smkdir(sub + 1);
		Scd("..");
	}	
	
	return noError;
}


usageErrorType Srmdir(char* dirname)
{
	iNode* CD = g_pointer->curDirNode;
	iNode* entry;
	
	entry = findiNodeByName(dirname, CD);
	if (entry == NULL)
	{
		
		PrintOutPut("rmdir: failed to remove '%s': No such file or directory\n", dirname);
		return ioError;
	}
	else if (entry->type == iNode_File)
	{
		PrintOutPut("rmdir: failed to remove '%s': Not a directory\n", dirname);
		return ioError;
	}
	else if (entry->size > 0)
	{
		PrintOutPut("rmdir: failed to remove '%s': Directory not empty\n", dirname);
		return ioError;
	}
	else
	{
		if (rmdir_unit(entry) == ioError)
		{
			PrintOutPut("Error removing directory!\n");
			return ioError;
		}
		
		sync_meta_to_disk();
	}

			
	return (noError);
}


usageErrorType Slink(char* src, char* dest)
{
	iNode *CD, *sEntry, *dEntry;
	
	CD = g_pointer->curDirNode;
	
	sEntry = findiNodeByName(src, CD);
	if (sEntry == NULL)
	{
		PrintOutPut("link: cannot create link '%s' to '%s': "\
			 		 "No such file or directory\n", dest, src);	
		return argError;
	}
	else if (sEntry->type == iNode_Dir)
	{
		PrintOutPut("link: cannot create link '%s' to '%s': "\
					 "Operation not permitted\n", dest, src);	
		return argError;
	}
	
	dEntry = findiNodeByName(dest, CD);
	if (dEntry != NULL)
	{
		PrintOutPut("link: cannot create link '%s' to '%s': "\
					 "File exists", dest, src);	
		return argError;
	}
	
	do_copy(src, dest);
		
	return (noError);
}


usageErrorType Sunlink(char* name)
{
	iNode *entry, *CD;
	CD = g_pointer->curDirNode;
	
	entry = findiNodeByName(name, CD);
	
	if (entry == NULL)
	{
		PrintOutPut("unlink: cannot unlink '%s': No such file or directory\n", name);
		return argError;
	}
	else if (entry->type == iNode_Dir)
	{
		PrintOutPut("unlink: cannot unlink '%s': Is a directory\n", name);
		return argError;
	}
	
	int blockCount = entry->size / BLOCK_SIZE + 1;
	
	releaseDataBlock(entry, blockCount);
	
	rmdir_unit(entry);	
	sync_meta_to_disk();	
	
	return (noError);
}


usageErrorType Sstat(char* name)
{
	
	iNode *entry, *CD;
	CD = g_pointer->curDirNode;
	
	entry = findiNodeByName(name, CD);
	if (entry == NULL)
	{
		PrintOutPut("stat: cannot stat '%s': No such file or directory\n", name);
	}
	else
	{
		PrintStat(entry);		
	}
	
	return (noError);
}


usageErrorType Sls()
{
	
	iNode *CD, *entry;
	CD = g_pointer->curDirNode;
	int i, entrySize;	
	
	entrySize = CD->size;
	
	PrintOutPut("..\n.\n");
	
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
	iNode* CD;
	CD = g_pointer->curDirNode;
	
	iNode* entry = findiNodeByName(name, CD);
	if (entry == NULL)
	{
		PrintOutPut("cat: %s: No such file or directory\n", name);
		return ioError;
	}
	else if (entry->type == iNode_Dir)
	{
		PrintOutPut("cat: %s: Is a directory\n", name);
		return ioError;
	}
	
	// read the whole file	
	if (entry->size > 0)
	{
		int fd = do_open(name, "r", FALSE);
		Sread(fd, entry->size);
		Sclose(fd);
	}
	
	return (noError);
}


usageErrorType Scp(char* src, char* dest)
{
	iNode *CD, *sEntry, *dEntry;
	CD = g_pointer->curDirNode;
	
	sEntry = findiNodeByName(src, CD);
	if (sEntry == NULL)
	{
		PrintOutPut("cp: %s: No such file or directory\n", src);
		return ioError;
	}
	else if (sEntry->type == iNode_Dir)
	{
		PrintOutPut("cp: %s: Is a directory\n", src);
		return ioError;
	}
	
	dEntry = findiNodeByName(dest, CD);
	if (dEntry != NULL)
	{
		if (dEntry->type == iNode_Dir)
		{
			changeDirectory(dEntry);
			do_copy_from_entry(sEntry, src);
			changeDirectory(CD);
		}
		else if (dEntry->type == iNode_File)
		{
			Sunlink(dest);
			do_copy(src, dest);
		}
	}
	else 
	{
		do_copy(src, dest);
	}
	

	return (noError);
}


usageErrorType Stree()
{
	PrintTree(g_pointer->curDirNode, 0);	
	return (noError);
}


usageErrorType Simport(char* srcname, char* destname)
{
	int ifd, efd, rs;	
	iNode* dEntry;
	char buf[BUFFER_SIZE] = {0};
	
	dEntry = findiNodeByName(destname, g_pointer->curDirNode);
	
	// check if the dest file already exist
	if (dEntry != NULL)
	{
		PrintOutPut("import: File already exist!\n");
		return ioError;
	}
	
	// check if the source file open fail
	ifd = open(srcname, O_RDONLY, S_IRWXU);
	if (ifd == -1)
	{
		perror("Fail open file!\n");
		return ioError;
	}
	
	// open and create dest file. Don't have to check if opened
	efd = do_open(destname, "w", FALSE);
	
	// each time read in at most buffer_size - 1 bytes, so the last
	// byte is 0 and the whole buffer is a valid string 
	while ((rs = read(ifd, buf, BUFFER_SIZE - 1)) > 0)
	{
		Swrite(efd, buf);
		memset(buf, 0, BUFFER_SIZE);
	}
	
	// close both file
	close(ifd);
	Sclose(efd);
	
	
	return (noError);
}


usageErrorType Sexport(char* srcname, char* destname)
{
	int efd, ifd, fsize, rs;
	iNode *sEntry, *dEntry;
	char* buffer;
	
	sEntry = findiNodeByName(srcname, g_pointer->curDirNode);
	
	// check if no source file found
	if (sEntry == NULL)
	{
		PrintOutPut("export: No file found!\n");
		return ioError;
	}
	
	// check if the dest file open fail
	efd = open(destname, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
	if (efd == -1)
	{
		perror("Fail open file!\n");
		return ioError;
	}
	
	// open and create src file. Don't have to check if opened
	ifd = do_open(srcname, "r", FALSE);
	
	fsize = sEntry->size;
	buffer = (char*) malloc(fsize);
	memset(buffer, 0, fsize);
	
	do_read(ifd, buffer, fsize);
	
	rs = write(efd, buffer, fsize);
	
	// close both file
	close(efd);
	Sclose(ifd);
	
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
			// mark free iNode number 
			g_pointer->freeInodePointer[i] = 1;
			g_pointer->sb->freeINode -= 1;
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
			g_pointer->bitMapPointer[i] = 1;
			g_pointer->sb->freeBlocks -= 1;
			
			return i;
		}
	}
	
	// no free inode found
	return -1;	
}

void free_block(int nBlock)
{
	g_pointer->bitMapPointer[nBlock] = 0;
	g_pointer->sb->freeBlocks += 1;
}

usageErrorType writeEntryInBlock(int blockIndex, int entryIndex, int freeNode)
{
	int i, fd, rs;
	
	fd = open(IMG_NAME, O_RDWR, S_IRWXU);
	lseek(fd, g_pointer->sb->dataOffset + BLOCK_SIZE * blockIndex, SEEK_SET);
	
	int indexBuf[BLOCK_SIZE / INDEX_LENGTH] = {0};
	
	if (entryIndex == 0)
	{
		indexBuf[0] = freeNode;
	}
	else
	{		
		read(fd, indexBuf, BLOCK_SIZE);
		
		for (i = 1; i < BLOCK_SIZE /INDEX_LENGTH ; i++)
		{
			if (indexBuf[i] == 0)
			{
				indexBuf[i] = freeNode;
				break;
			}				
		}
		
		if (i  == BLOCK_SIZE /INDEX_LENGTH)
		{
			PrintOutPut("Entry number exceed maxima!\n");
			close(fd);
			return ioError;
		}
	}
	
	// back to the start of the block
	lseek(fd, g_pointer->sb->dataOffset + BLOCK_SIZE * blockIndex, SEEK_SET);
	
	rs = write(fd, indexBuf, BLOCK_SIZE);
	close(fd);
	
	return noError;
}
	
usageErrorType	writeEntryInBlockSerial(int index, int offset, int data)
{
	int i, fd, rs;
	
	fd = open(IMG_NAME, O_WRONLY, S_IRWXU);
	lseek(fd, g_pointer->sb->dataOffset + BLOCK_SIZE * index + offset * INDEX_LENGTH, SEEK_SET);
	
	rs = write(fd, &data, INDEX_LENGTH);
	close(fd);
	
	return noError;
}
	
	

// sync the memory to disk
usageErrorType sync_meta_to_disk()
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
	if (cd->selfIndex == 0)
	{
		strcpy(path, g_pointer->firstiNode->fileName);
	}
	else
	{
		iNode* parent = g_pointer->firstiNode + cd->parentIndex;
		getAbsPath(parent);
		strcat(path, cd->fileName);
		strcat(path, "/");
	}
	
}

usageErrorType changeDirectory(iNode* entry)
{
	g_pointer->currentDir = entry->selfIndex;
	g_pointer->curDirNode = g_pointer->firstiNode + g_pointer->currentDir;
	memset(path, 0, LINE);
	getAbsPath(entry);
	return noError;
}

void PrintEntry(iNode* entry)
	{
	if (entry->type == iNode_Dir)
	{
		PrintOutPut("%s/\n", entry->fileName);
	}
	else if (entry->type == iNode_File)
	{
		PrintOutPut("%s\n", entry->fileName);
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
	
		int indexBuf[BLOCK_SIZE / INDEX_LENGTH] = {0};
		rs = read(fd, indexBuf, BLOCK_SIZE);
		
		for (i = 0; i < BLOCK_SIZE / INDEX_LENGTH; i++)
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

	
	
	findSubEntries(entryIndexArray, entry);

	
	for (i = 0; i < entry->size; i++)
	{
		child = g_pointer->firstiNode + entryIndexArray[i];
		if (child->type == iNode_File)
		{				
			PrintDash(level);	
			PrintOutPut("%s (%d bytes)\n", child->fileName, child->size);
		}
		else if (child->type == iNode_Dir && child->size == 0)
		{
			PrintDash(level);
			PrintOutPut("%s\n", child->fileName);
		}
		else
		{			
			PrintDash(level);	
			PrintOutPut("%s\n", child->fileName);
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
		PrintOutPut("-");
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
	iNode* CD = g_pointer->curDirNode;
	int i, firstAvailPoint, entrySize, blockIndex4Entry;
	
	entrySize = CD->size;
	if (entrySize >= MAX_ENTRY_SIZE)
	{
		PrintOutPut("Err: Maxmum entry size (%d) exceeded!\n", entrySize);
		return ioError;
	}
		
	// assign a new iNode for this new directory entry
	int freeNode = get_free_iNode();
	if (freeNode == -1)
	{
		PrintOutPut("Not Enough iNode available!\n");
		return ioError;
	}
	
	iNode* newDir = g_pointer->firstiNode + freeNode;	
	// create new directory
	newDir->selfIndex = freeNode;
	newDir->parentIndex = g_pointer->currentDir;
	newDir->type = iNode_Dir;
	strcpy(newDir->fileName, dirname);	
	newDir->link = 1;
	
	addSubEntry(CD, freeNode);
	
	return (noError);

}

int findiNodeIndexByName(char* name, iNode* CD)
{
	int entrySize, i, index;
	int* entryIndexArray;
	iNode* entry;
	
	if (strcmp(name, ".") == 0)
	{
		return CD->selfIndex;
	}
	else if (strcmp(name, "..") == 0)
	{
		return CD->parentIndex;
	}
	
	entrySize = CD->size;
	entryIndexArray = (int *)malloc(entrySize * sizeof(int));
	
	findSubEntries(entryIndexArray, CD);
	
	for (i = 0; i < entrySize; i++)
	{
		entry = g_pointer->firstiNode + entryIndexArray[i];
		if (strcmp(entry->fileName, name) == 0)
		{
			index = entryIndexArray[i];
			free(entryIndexArray);
			return index;				
		}		
	}	
	
	free(entryIndexArray);	
	
	return -1;
}

iNode* findiNodeByName(char* name, iNode* CD)
{
	int index = findiNodeIndexByName(name, CD);
	if (index == -1)
	{
		return NULL;
	}
	else
	{
		return (g_pointer->firstiNode + index);
	}
}

usageErrorType rmdir_unit(iNode* entry)
{
	iNode* parent;
	parent = g_pointer->firstiNode + entry->parentIndex;
	int i, entrySize, directEntryCount;
	
	
	entrySize = parent->size;
	directEntryCount = 0;
	
	for (i = 0; i < D_PTR_CNT; i++)
	{
		if (parent->directPtr[i] == entry->selfIndex)
		{
			parent->directPtr[i] = 0;
			parent->size --;
			
			g_pointer->freeInodePointer[entry->selfIndex] = 0;
			g_pointer->sb->freeINode ++;
			
			return noError;
		}
		else if (parent->directPtr[i] == 0)  // empty entry
		{
			continue;
		}
		else	// other file or directory
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
	
		int indexBuf[BLOCK_SIZE / INDEX_LENGTH] = {0};
		rs = read(fd, indexBuf, BLOCK_SIZE);
		
		for (i = 0; i < BLOCK_SIZE / INDEX_LENGTH; i++)
		{
			if (indexBuf[i] == 0)
			{
				continue;
			}
			else if (indexBuf[i] == entry->selfIndex)
			{
				indexBuf[i] = 0;
				parent->size --;
				
				// if the entry size equals to number of entries in 
				// direct pointers, means the block is empty
				if (parent->size == directEntryCount)
				{					
					free_block(parent->t_indirectPtr);
					
					parent->t_indirectPtr = 0;
				}
				
				// free the iNode table and stat
				g_pointer->freeInodePointer[entry->selfIndex] = 0;
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

usageErrorType createFile(char* filename)
{
	int entrySize, freeNode, firstAvailPoint;
	iNode *CD, *entry;
	
	CD = g_pointer->curDirNode;
	entrySize = CD->size;
	
	freeNode = get_free_iNode();
	if (freeNode == -1)
	{
		PrintOutPut("Not Enough iNode available!\n");
		return ioError;
	}
	
	entry = g_pointer->firstiNode + freeNode;
	entry->type = iNode_File;
	entry->selfIndex = freeNode;
	entry->parentIndex = CD->selfIndex;
	entry->size = 0;
	strcpy(entry->fileName, filename);
	entry->link = 1;
	entry->offset = 0;
	entry->link = 1;
	
	// assign a new block as data block
	int block4Data = get_free_block();
	if (block4Data == -1)
	{
		PrintOutPut("Not Enough free Block available!\n");
		return -1;
	}	
	entry->directPtr[0] = block4Data;
	
	
	addSubEntry(CD, freeNode);
}

// add sub entry into directory 
// input: the iNode of the directory to be added, the new entry to add
// output: no
// comments: depending on the exsiting entry #, block assignment might occur
// 					 sync meta data will occur; sync block data might occur
usageErrorType addSubEntry(iNode* CD, int freeNode)
{
	int entrySize, firstAvailPoint, blockIndex4Entry;
	entrySize = CD->size;
	
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
			PrintOutPut("Entry Size error!\n");
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
	sync_meta_to_disk();	
}

int openFile(char* filename, char* flag)
{
	iNode *CD, *entry;
	int fd;

	CD = g_pointer->curDirNode;
	
	
	entry = findiNodeByName(filename, CD);
	
	return addOpenFile(entry, flag);	
}

// add an opened file into the global pointer
// input: iNode of the file, open flag(w/r)
// output: the index of the open-file global pointer
// error: return -1 if the opened file reaches the maxima
int addOpenFile(iNode* entry, char* flag)
{
	int i, openCnt;
		
	// find a unused slot in openedFiles array	
	for (i = 0; i < g_pointer->sb->maxOpenFile; i++)
	{
		if (g_pointer->openedFiles[i].oNode == NULL)
		{	
			g_pointer->openedFiles[i].oNode = entry;
			
			// offset always at the begining
			g_pointer->openedFiles[i].offset = 0;
			g_pointer->openedFiles[i].buf = (char*)malloc(BUFFER_SIZE);
			memset(g_pointer->openedFiles[i].buf, 0, BUFFER_SIZE);
			g_pointer->openedFiles[i].bufUsed = 0;			
			g_pointer->openedFileCount ++;
			
			// assign the flag bit
			if (strcmp(flag, "w") == 0)
			{
				g_pointer->openedFiles[i].flag = 'w';
			}
			else if (strcmp(flag, "r") == 0)
			{
				g_pointer->openedFiles[i].flag = 'r';
			}
			else 
			{
				g_pointer->openedFiles[i].flag = '+';
			}
			
			return i;
		}
	}
	
	// no unused slot found, array full!
	return -1;	
}

// delete the opened-file entry from the global pointer
// input: the opened file handler
// output: no
usageErrorType deleteOpenedFile(OpenedFile* handler)
{
	free(handler->buf);
	memset(handler, 0, sizeof(OpenedFile));	
	g_pointer->openedFileCount--;
		
	return noError;
}

// check if the file is already opened
// input: the iNode of the file
// output: index of the opened file global array
// error: return -1 if the file already exists in this opened file global array
int checkOpened(iNode* entry)
{
	int i, openCount;
	openCount = g_pointer->openedFileCount;
	
	if (openCount <= 0)
	{
		return -1;
	}
	
	for (i = 0; i < g_pointer->sb->maxOpenFile; i++)
	{
		if (g_pointer->openedFiles[i].oNode != NULL)
		{	
			if (g_pointer->openedFiles[i].oNode == entry)
			{
				return i;
			}
			else 
			{
				openCount --;
			}
	
			if (openCount <= 0)
			{
				return -1;
			}	
		}
	}
	
	return -1;
}


// print the status of the file
// input: iNode of the file/directory
// output: no
usageErrorType PrintStat(iNode* entry)
{
	int nBlock;
	
	if (entry->type == iNode_Dir)
	{
		nBlock = (entry->size - 12) / INDEX_IN_SIN + 1;
	}
	else if (entry->type == iNode_File)
	{
		nBlock = entry->size / BLOCK_SIZE + 1;
	}
	
	PrintOutPut("File: '%s'\n", entry->fileName);
	PrintOutPut("Size: %d\n", entry->size);
	PrintOutPut("Blocks: %d\n", nBlock);
	PrintOutPut("Link: %d\n", entry->link);
	PrintOutPut("Type: %s\n", entry->type == iNode_Dir ?
										  "directory" : "regular file");
	
	return noError;	
}

// sync the data block into disk
// input: the opened file handler
// output: none
// comments: 2 situations: might write data into only one data block or two data block
usageErrorType sync_data_to_disk(OpenedFile* handler)
{
	int fsize, cBlock, cOffset, cBlockIndex, nBlockIndex;

	// fsize = handler->oNode->size;
	fsize = handler->offset;
	cBlock = fsize / BLOCK_SIZE;
	cOffset = fsize - BLOCK_SIZE * cBlock;
	
	cBlockIndex = calcBlockIndex(cBlock, handler->oNode);	
	
	if (handler->bufUsed < BLOCK_SIZE - cOffset)
	{
		// all bytes can be written within this same block
		writeWithinBlock(cBlockIndex, cOffset, handler, handler->bufUsed);
	}
	else if (handler->bufUsed == BLOCK_SIZE - cOffset)
	{
		writeWithinBlock(cBlockIndex, cOffset, handler, handler->bufUsed);
		nBlockIndex = assignNewBlock(handler->oNode);
	}
	else
	{
		// not enough room, can only write part of bytes and write the rest 
		// in a new assigned block. Since buf size < Block size, only one new 
		// block is enough at a time
		writeWithinBlock(cBlockIndex, cOffset, handler, BLOCK_SIZE - cOffset);
		strcpy(handler->buf, handler->buf + BLOCK_SIZE - cOffset);
		
		nBlockIndex = assignNewBlock(handler->oNode);		
		writeWithinBlock(nBlockIndex, 0, handler, handler->bufUsed - (BLOCK_SIZE - cOffset));
	}

	memset(handler->buf, 0, BUFFER_SIZE);
	handler->bufUsed = 0;

	return noError;	
}

// calculate the actual block index given the relative block index
// input: relative index, iNode* entry
// output: actual block index in disk
int calcBlockIndex(int cBlock, iNode* entry)
{
	if (cBlock < D_PTR_CNT)	
	{
		// 0 ~ 11 blocks, -> (0, 48K)
		return entry->directPtr[cBlock];
	}
	else if (cBlock >= D_PTR_CNT &&  
					 cBlock < INDEX_IN_SIN + D_PTR_CNT)
	{
		// 12 ~ 1024 + 12 - 1 blocks, ->(12, 1035) -> (48K, 4M)
		// eg. offset = 50 - 12 = 38
		int offset = cBlock - D_PTR_CNT;
		return locateIndex(offset, entry->s_indirectPtr);
	}
	else if (cBlock >= INDEX_IN_SIN + D_PTR_CNT &&
					 cBlock <  INDEX_IN_DIN + INDEX_IN_SIN + D_PTR_CNT) 
	{
		// 1036 ~ 1048576 + 1024 + 12 - 1 blocks, ->(1036, 1049611) -> (4M, 4G)
		// eg. sBlock = (5000 - 1036) / 1024 = 3.87 = 3
		// 		 offset = 5000 - 1036 - 3 * 1024 = 892
		int sBlock = (cBlock - (INDEX_IN_SIN + D_PTR_CNT)) / INDEX_IN_SIN;
		int offset = (cBlock - (INDEX_IN_SIN + D_PTR_CNT) - sBlock * D_PTR_CNT);
		
		int s_index = locateIndex(sBlock, entry->d_indirectPtr);
		return locateIndex(offset, s_index);
	}
	else
	{
		// too many, not considered for now ()
		PrintOutPut("Too many blocks for this system to handle!\n");
		return -1;
	}
}

// read the block index from block in disk
// input: the offset of the to-be-read value in block, the actual block index in disk
// output: the value of the index read from disk
int locateIndex(int offset, int index)
{
	int buf4Index = -1;
	int fd = open(IMG_NAME, O_RDONLY, S_IRWXU);
	lseek(fd, g_pointer->sb->dataOffset + index * BLOCK_SIZE + offset * INDEX_LENGTH, SEEK_SET);
	read(fd, &buf4Index, INDEX_LENGTH);
	
	close(fd);
	
	return buf4Index;
}

// assign a new block for data write
// input: the iNode of the file
// output: the actual block index
// comments: depending on the file size, 1 ~ 3 assign-block operation might occur
int assignNewBlock(iNode* entry)
{
	// assign a new block as data block
	int block4Data = get_free_block();
	if (block4Data == -1)
	{
		PrintOutPut("Not Enough free Block available!\n");
		return -1;
	}	
	
	// calculate the current Block count
	int currentBlock = entry->size / BLOCK_SIZE;
	
	if (currentBlock < D_PTR_CNT)
	{
		// if less than 12, the direct pointer is enough
		entry->directPtr[currentBlock] = block4Data;
	}
	else if (currentBlock == D_PTR_CNT)
	{
		// direct pointers just full, need assign new block to store new direct pointers
		int block4sIndex = get_free_block();
		entry->s_indirectPtr = block4sIndex;
		writeEntryInBlockSerial(entry->s_indirectPtr, 0, block4Data);

	}
	else if (currentBlock > D_PTR_CNT &&
					 currentBlock < D_PTR_CNT + D_PTR_CNT)
	{
		// single indirect block not full, only need to add a direct pointer in the block
		writeEntryInBlockSerial(entry->s_indirectPtr, currentBlock - D_PTR_CNT, block4Data);
	}
	else if (currentBlock == INDEX_IN_SIN + D_PTR_CNT)
	{
		// assign a new block as single-indirect pointer inside the double-indirect pointer
		int block4sIndex = get_free_block();
		writeEntryInBlockSerial(block4sIndex, 0, block4Data);		
		
		// assign a new block as double-indirect pointer
		int block4dIndex = get_free_block();
		entry->d_indirectPtr = block4dIndex;
		writeEntryInBlockSerial(entry->d_indirectPtr, 0, block4dIndex);		
		
	}
	else if (currentBlock > INDEX_IN_SIN + D_PTR_CNT &&
					 currentBlock <  INDEX_IN_DIN + INDEX_IN_SIN + D_PTR_CNT) 
	{
		int s_index = (currentBlock - (D_PTR_CNT + INDEX_IN_SIN)) / INDEX_IN_SIN;
		int offset = (currentBlock - (D_PTR_CNT + INDEX_IN_SIN)) % INDEX_IN_SIN;
		
		if (offset == 0)
		{
			int direct = get_free_block();
			writeEntryInBlockSerial(entry->d_indirectPtr, s_index, direct);		
			
			writeEntryInBlockSerial(direct, 0, block4Data);			
		}
		else
		{
			int direct = locateIndex(s_index, entry->d_indirectPtr);
			writeEntryInBlockSerial(direct, offset + 1, block4Data);			
		}		
	}
	 
	return block4Data;	
}


// write buffer into data block. All bytes are with in the same block
// input: block index, offset in block, the opened file handler, # bytes to be written
// output: noError
usageErrorType writeWithinBlock(int cBlockIndex, int cOffset, OpenedFile* handler, int len)
{
	int fd; 
	
	fd = open(IMG_NAME, O_WRONLY, S_IRWXU);
	lseek(fd, g_pointer->sb->dataOffset + cBlockIndex * BLOCK_SIZE + cOffset, SEEK_SET);
	write(fd, handler->buf, len);
	handler->offset += len;
	handler->oNode->size += len;
	
	close(fd);
	
	return noError;
}

usageErrorType read_to_buffer(OpenedFile* handler, int size, char* buffer)
{
	int remain, actualBytes, rblock, offset, ablock;
	
	remain = size;
	
	
	while (remain > 0)
	{
		rblock = handler->offset / BLOCK_SIZE;
		offset = handler->offset % BLOCK_SIZE;
		
		// the actual block in disk
		ablock = calcBlockIndex(rblock, handler->oNode);
		
		actualBytes = BLOCK_SIZE - offset > remain ? remain : BLOCK_SIZE - offset;
		
		load_bytes_from_block(ablock, offset, actualBytes, buffer, handler);
		
		remain -= (BLOCK_SIZE - offset);
	}
	
	return noError;
}

// read bytes from block to buffer
// input: actual block index, current offset in block, actual bytes # to read,
//			  buffer pointer to hold the bytes, the openfile handler
usageErrorType load_bytes_from_block(int ablock, int offset, int actualBytes,
																		 char* buffer, OpenedFile* handler)
{
	int fd, rs;
	char newBytes[BLOCK_SIZE + 1] = {0};
	
	fd = open(IMG_NAME, O_RDONLY, S_IRWXU);
	lseek(fd, g_pointer->sb->dataOffset + ablock * BLOCK_SIZE + offset, SEEK_SET);
	rs = read(fd, newBytes, actualBytes);
	
	strncat(buffer, newBytes, rs);
	handler->offset += actualBytes;
	
	close(fd);
	
	return noError;
	
}

int do_open(char* filename, char* flag, BOOL checkOpen)
{
	if (strlen(flag) > 2 || ((strcmp(flag, "r") != 0)
											 &&  (strcmp(flag, "w") != 0)
											 &&  (strcmp(flag, "rw") != 0)
											 &&  (strcmp(flag, "wr") != 0	)))
	{
		PrintOutPut("Open flag error! Please must contain 'w' or 'r' only!\n");
		return -1;
	}
	
	iNode *CD, *entry;
	CD = g_pointer->curDirNode;
	int openFd;
	
	entry = findiNodeByName(filename, CD);
	
	if (entry == NULL)
	{
		createFile(filename);
		openFd = openFile(filename, flag);		
	}	
	else if (entry->type == iNode_Dir)
	{
		PrintOutPut("Open: '%s' is not a file!\n", filename);
		return ioError;
	}
	else if (checkOpen  && (openFd = checkOpened(entry)) != -1)
	{
		PrintOutPut("File already opened, fd = %d\n", openFd);
		return -1;
	}
	else
	{
		openFd = openFile(filename, flag);
	}	
	
	if (openFd == -1)
	{
		PrintOutPut("File open error!\n");
	}
		
	return (openFd);
}

usageErrorType releaseDataBlock(iNode* entry, int blockCount)
{
	int i, ablock;	

	for (i = 0; i < blockCount; i++)
	{
		
		ablock = calcBlockIndex(i, entry);
		g_pointer->bitMapPointer[ablock] = 0;
		g_pointer->sb->freeBlocks ++;
	}
	
	return noError;
	
}

// copy file : create an empty file, and copy all block of data into this 
// created dest file
// input: the iNode of the source file, the name of the dest file
// command: the dest file need to be created first, so no iNode is 
//          available at first
usageErrorType do_copy_from_entry(iNode* sEntry, char* dest)
{
	iNode *dEntry, *CD;
	int rblock, sblock, dblock, i, fd;
	
	CD = g_pointer->curDirNode;
	
	// create the destination file
	createFile(dest);
	dEntry = findiNodeByName(dest, CD);
	
	// block size
	rblock = sEntry->size / BLOCK_SIZE;
	if (sEntry->size % BLOCK_SIZE != 0)
	{
		rblock += 1;
	}
	
	fd = open(IMG_NAME, O_RDWR, S_IRWXU);
	
	// copy the first block
	copy_data_via_block(fd, sEntry->directPtr[0], dEntry->directPtr[0]);
	
	// if the data is less than or equal to a block 
	if (rblock == 1)
	{
		dEntry->size = sEntry->size;	
		close(fd);
		return noError;
	}
	else
	{
		dEntry->size = BLOCK_SIZE;
	}
	
	// copy the rest of the block
	for (i = 1; i < rblock; i++)
	{
		sblock = calcBlockIndex(i, sEntry);
		
		dblock = assignNewBlock(dEntry);
		
		copy_data_via_block(fd, sblock, dblock);		
	}
	
	dEntry->size = sEntry->size;
	
	close(fd);
}

// wrapper of the do_copy_from_entry() function
// the only difference is: the source file is the file name instead of iNode
// input: file name of the source, file name of the dest
// output: no
usageErrorType do_copy(char* src, char* dest)
{
	iNode *CD, *sEntry;
	CD = g_pointer->curDirNode;

	sEntry = findiNodeByName(src, CD);

	// call the inner function
	do_copy_from_entry(sEntry, dest);
}


usageErrorType copy_data_via_block(int fd, int sblock, int dblock)
{
	int rs;
	char buf[BLOCK_SIZE] = {0};
	
	lseek(fd, g_pointer->sb->dataOffset + sblock * BLOCK_SIZE, SEEK_SET);
	rs = read(fd, buf, BLOCK_SIZE);
	
	lseek(fd, g_pointer->sb->dataOffset + dblock * BLOCK_SIZE, SEEK_SET);
	rs = write(fd, buf, BLOCK_SIZE);
	
	return noError;
}

void PrintOutPut(const char *format, ...)
{
	 va_list args;
   va_start(args, format);
    
    
	if (outputFlag == FALSE)
	{
		vprintf(format, args);
	}
	else
	{
		vfprintf(outputStream, format, args);
	}
	
	va_end(args);
}
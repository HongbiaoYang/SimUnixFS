/* 
 * File:   simfs.c
 * Author: Bill
 *
 * Created on April 3, 2014, 4:15 PM
 */

#include "commands.h"


/*
 * 
 */
GLOBAL_Pointers* g_pointer;
char path[LINE];

int main(int argc, char** argv) 
{
    strcpy(path, "/");
    char command[LINE];
    
    if (Init_fs() == unInitError)
    {   
    	printf("Alert! Disk not formated; Use mkfs to format first!\n");
    }
  	else
  	{
  		
  		printf("Welcome to simulated file system!\nType HELP for help\n");
  	}

    printf("$%s$>",path);        
    while (gets(command) != NULL)
    {
    	if (parseCommand(command) == exitError)
  		{
  			break;
  		}    	
    	
			// printf("%s\n", command);
			printf("$%s$>",path);        
        
    }
    
    return (EXIT_SUCCESS);
}

// parse the command
usageErrorType parseCommand(char* command)
{
	char* cmds[ARGS] = {NULL};
	char* p;
	int args = 0;
	
	// empty command
	if (strcmp(command, "") == 0)
	{
		return noError;
	}
	
	cmds[args++] = strtok(command, " ");
	
	while (args < ARGS)
	{
		
		p = strtok(NULL, " ");
		if (p != NULL)
		{
			cmds[args++] = p;
			// printf("cmds=%s #=%d\n", cmds[args-1], args);
		}		
		else
		{
			break;
		}
	}
	
	if (strcmp(cmds[0], "exit") == 0)
	{
		printf("Thanks, Bye!\n");
		return (exitError);
	}
	

	if (strcmp(cmds[0], "mkfs") == 0)
	{
		Smkfs();
		return noError;
	}
	
	if (strcmp(cmds[0], "open") == 0)
	{
		if (args < 3)
		{
			printf("open: missing operand\n");
		}
		else
		{
			Sopen(cmds[1], cmds[2]);
		}
		return noError;
	}
	
	if (strcmp(cmds[0], "read") == 0)
	{
		if (args < 3)
		{
			printf("read: missing operand\n");
		}
		else
		{
			Sread(atoi(cmds[1]), atoi(cmds[2]));			
		}		
		
		return noError;
	}
	
	if (strcmp(cmds[0], "write") == 0)
	{
		if (args < 3)
		{
			printf("write: missing operand\n");
		}
		else
		{
				Swrite(atoi(cmds[1]), cmds[2]);
		}						
		
		return noError;
	}
	
	if (strcmp(cmds[0], "seek") == 0)
	{
		if (args < 3)
		{
			printf("seek: missing operand\n");
		}
		else
		{
			Sseek(atoi(cmds[1]), atoi(cmds[2]));
		}				
		
		return noError;
	}
	
	if (strcmp(cmds[0], "close") == 0)
	{		
		if (args < 2)
		{
			printf("close: missing operand\n");
		}
		else
		{
				Sclose(atoi(cmds[1]));
		}		
		
		return noError;
	}
	
	if (strcmp(cmds[0], "mkdir") == 0)
	{
		if (args < 2)
		{
			printf("mkdir: missing operand\n");
		}
		else
		{
			Smkdir(cmds[1]);
		}
		return noError;
	}
	
	if (strcmp(cmds[0], "cd") == 0)
	{
		if (args < 2)
		{
			Scd("/");
		}
		else
		{
				Scd(cmds[1]);
		}
		return noError;
	}
	
	if (strcmp(cmds[0], "rmdir") == 0)
	{
		if (args < 2)
		{
			printf("rmdir: missing operand\n");
		}
		else
		{
			Srmdir(cmds[1]);
		}		
		
		return noError;
	}
	
	if (strcmp(cmds[0], "link") == 0)
	{
		if (args < 3)
		{
			printf("link: missing operand\n");
		}
		else
		{
			Slink(cmds[1], cmds[2]);
		}			
		
		return noError;
	}
	
	if (strcmp(cmds[0], "unlink") == 0)
	{
		
		if (args < 2)
		{
			printf("unlink: missing operand\n");
		}
		else
		{
			Sunlink(cmds[1]);
		}			

		return noError;
	}
	
	if (strcmp(cmds[0], "stat") == 0)
	{
		if (args < 2)
		{
			printf("stat: missing operand\n");
		}
		else
		{
				Sstat(cmds[1]);
		}		
		return noError;
	}
	
	if (strcmp(cmds[0], "ls") == 0)
	{
		Sls();
		return noError;
	}
	if (strcmp(cmds[0], "cat") == 0)
	{
		if (args < 2)
		{
			printf("cat: missing operand\n");
		}
		else
		{
			Scat(cmds[1]);
		}		

		return noError;
	}
	
	if (strcmp(cmds[0], "cp") == 0)
	{
		if (args < 3)
		{
			printf("cp: missing operand\n");
		}
		else
		{
			Scp(cmds[1], cmds[2]);
		}		

		return noError;
	}
	
	if (strcmp(cmds[0], "tree") == 0)
	{
		Stree();
		return noError;
	}
	
	if (strcmp(cmds[0], "import") == 0)
	{
		Simport(cmds[1], cmds[2]);
		return noError;
	}
	
	if (strcmp(cmds[0], "export") == 0)
	{
		Sexport(cmds[1], cmds[2]);
		return noError;
	}
	if (strcmp(cmds[0], "debug") == 0)
	{
		int i;
		char dir[LINE];
		
		for (i = 0; i < 12; i++)
		{
			sprintf(dir, "dir%ddir", i);
			Smkdir(dir);			
		}
		
		return noError;
	}
	
	printf("simfs: command not found: %s\n", command);
		
	
	return noError;
}

// initialize the file system
usageErrorType Init_fs()
{
	int fd, i;
	off_t rs;
	
	// assign memory for the pointer of global pointers
	g_pointer = (GLOBAL_Pointers*)malloc(sizeof(GLOBAL_Pointers));
	
	// assign memory for the superblock
	g_pointer->sb = (superBlock*)malloc(sizeof(superBlock));
	
	
	fd = open(IMG_NAME, O_RDWR | O_CREAT, S_IRWXU);
	
	/* write the super block */
	rs = read(fd, g_pointer->sb, sizeof(superBlock));
	if (rs <= 0)
	{	
		close(fd);		
		return unInitError;
	}

	// assign memory for these global pointers 
	g_pointer->bitMapPointer = (char*)malloc(g_pointer->sb->totalBlocks);
	g_pointer->freeInodePointer = (char*)malloc(g_pointer->sb->totalINode);
	g_pointer->firstiNode = (iNode*)malloc(g_pointer->sb->totalINode * sizeof(iNode));
	

	// read value from bitmap, iNodemap, and iNodes into memory
	rs = read(fd, g_pointer->bitMapPointer, g_pointer->sb->totalBlocks);
	rs = read(fd, g_pointer->freeInodePointer, g_pointer->sb->totalINode);
	rs = read(fd, g_pointer->firstiNode, g_pointer->sb->totalINode);

	//  initiate the current directory and the opened files
	g_pointer->currentDir = g_pointer->firstiNode->selfIndex;
	
	g_pointer->openedFiles = (OpenedFile*)malloc(g_pointer->sb->maxOpenFile * sizeof(OpenedFile));
	memset(g_pointer->openedFiles, 0, g_pointer->sb->maxOpenFile * sizeof(OpenedFile));
	
	g_pointer->openedFileCount = 0;

	close(fd);
	return noError;	
}
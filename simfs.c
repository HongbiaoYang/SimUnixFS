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
	
	cmds[args++] = strtok(command, " ");
	
	while (args < ARGS)
	{
		
		p = strtok(NULL, " ");
		if (p != NULL)
		{
			cmds[args++] = p;
			printf("cmds=%s #=%d\n", cmds[args-1], args);
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
	}
	
	if (strcmp(cmds[0], "open") == 0)
	{
		Sopen(cmds[1], cmds[2]);
	}
	
	if (strcmp(cmds[0], "read") == 0)
	{
		Sread(atoi(cmds[1]), atoi(cmds[2]));
	}
	
	if (strcmp(cmds[0], "write") == 0)
	{
		Swrite(atoi(cmds[1]), cmds[2]);
	}
	
	if (strcmp(cmds[0], "seek") == 0)
	{
		Sseek(atoi(cmds[1]), atoi(cmds[2]));
	}
	
	if (strcmp(cmds[0], "close") == 0)
	{
		Sclose(atoi(cmds[1]));
	}
	
	if (strcmp(cmds[0], "mkdir") == 0)
	{
		Smkdir(cmds[1]);
	}
	
	if (strcmp(cmds[0], "cd") == 0)
	{
		Scd(cmds[1]);
	}
	
	if (strcmp(cmds[0], "rmdir") == 0)
	{
		Srmdir(cmds[1]);
	}
	
	if (strcmp(cmds[0], "link") == 0)
	{
		Slink(cmds[1], cmds[2]);
	}
	
	if (strcmp(cmds[0], "unlink") == 0)
	{
		Sunlink(cmds[1]);
	}
	
	if (strcmp(cmds[0], "stat") == 0)
	{
		Sstat(cmds[1]);
	}
	
	if (strcmp(cmds[0], "ls") == 0)
	{
		Sls();
	}
	if (strcmp(cmds[0], "cat") == 0)
	{
		Scat(cmds[1]);
	}
	
	if (strcmp(cmds[0], "cp") == 0)
	{
		Scp(cmds[1], cmds[2]);
	}
	
	if (strcmp(cmds[0], "tree") == 0)
	{
		Stree();
	}
	
	if (strcmp(cmds[0], "import") == 0)
	{
		Simport(cmds[1], cmds[2]);
	}
	
	if (strcmp(cmds[0], "export") == 0)
	{
		Sexport(cmds[1], cmds[2]);
	}
		
	
	return noError;
}

// initialize the file system
usageErrorType Init_fs()
{
	int fd, i;
	off_t rs;
	
	g_pointer = (GLOBAL_Pointers*)malloc(sizeof(GLOBAL_Pointers));
	g_pointer->sb = (superBlock*)malloc(sizeof(superBlock));
	
	
	fd = open(IMG_NAME, O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
	
	/* write the super block */
	rs = read(fd, g_pointer->sb, sizeof(superBlock));
	if (rs <= 0)
	{	
		close(fd);		
		return unInitError;
	}

	g_pointer->bitMapPointer = (char*)malloc(g_pointer->sb->totalBlocks);
	g_pointer->freeInodePointer = (char*)malloc(g_pointer->sb->totalINode);
	g_pointer->firstiNode = (iNode*)malloc(g_pointer->sb->totalINode);

	rs = read(fd, g_pointer->bitMapPointer, g_pointer->sb->totalBlocks);
	rs = read(fd, g_pointer->freeInodePointer, g_pointer->sb->totalINode);
	rs = read(fd, g_pointer->firstiNode, g_pointer->sb->totalINode);

	g_pointer->currentDir = g_pointer->firstiNode->s_indirectPtr;

	close(fd);
	return noError;	
}
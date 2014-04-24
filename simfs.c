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
FILE* outputStream;
BOOL  outputFlag;

int main(int argc, char** argv) 
{
    strcpy(path, "/");
    char command[LINE];
    
    if (Init_fs() == unInitError)
    {   
    	PrintOutPut("Alert! Disk not formated; Use mkfs to format first!\n");
    }
  	else
  	{  		
  		PrintOutPut("Welcome to simulated file system!\nType HELP for help\n");
  	}

    PrintOutPut("$%s$>",path);        
    while (gets(command) != NULL)
    {
    	if (parseCommand(command) == exitError)
  		{
  			break;
  		}    	
    	
			// PrintOutPut("%s\n", command);
			PrintOutPut("$%s$>",path);        
        
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
	
	if (command[strlen(command) - 1] == '\n')
	{
		command[strlen(command) - 1] = '\0';
	}
	
	args = process_arguments(command, cmds);
	
	PrintOutPut("==============command called: %s =====================\n", cmds[0]);

	if (strcmp(cmds[0], "mkfs") != 0 &&
		  g_pointer->sb->totalBlocks == 0 &&
		  g_pointer->sb->totalINode == 0)
  {
  	PrintOutPut("Alert! Disk not formated; Use mkfs to format first!\n");
  	return unInitError;
  }
	
	
	if (strcmp(cmds[0], "exit") == 0)
	{
		PrintOutPut("Thanks, Bye!\n");
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
			PrintOutPut("open: missing operand\n");
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
			PrintOutPut("read: missing operand\n");
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
			PrintOutPut("write: missing operand\n");
		}
		else
		{
				Swrite(atoi(cmds[1]), cmds[2]);
				Swrite(atoi(cmds[1]), "\n");
		}						
		
		return noError;
	}
	
	if (strcmp(cmds[0], "seek") == 0)
	{
		if (args < 3)
		{
			PrintOutPut("seek: missing operand\n");
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
			PrintOutPut("close: missing operand\n");
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
			PrintOutPut("mkdir: missing operand\n");
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
			PrintOutPut("rmdir: missing operand\n");
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
			PrintOutPut("link: missing operand\n");
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
			PrintOutPut("unlink: missing operand\n");
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
			PrintOutPut("stat: missing operand\n");
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
			PrintOutPut("cat: missing operand\n");
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
			PrintOutPut("cp: missing operand\n");
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
		if (args < 3)
		{
			PrintOutPut("import: missing operand\n");
		}
		else
		{
			Simport(cmds[1], cmds[2]);
		}		

		return noError;
	}
	
	if (strcmp(cmds[0], "export") == 0)
	{
		if (args < 3)
		{
			PrintOutPut("export: missing operand\n");
		}
		else
		{
			Sexport(cmds[1], cmds[2]);
		}		

		return noError;
	}
	if (strcmp(cmds[0], "sh") == 0)
	{
		if (args < 2)
		{
			PrintOutPut("sh: missing operand\n");
		}
		else if (args == 2)
		{
			return batch_execute(cmds[1]);
		}
		else if (args == 3)
		{
			return batch_execute_output(cmds[1], cmds[2]);
		}
		
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

	
	PrintOutPut("simfs: command not found: %s\n", command);
		
	
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
	g_pointer->curDirNode = g_pointer->firstiNode + g_pointer->currentDir; 
	
	g_pointer->openedFiles = (OpenedFile*)malloc(g_pointer->sb->maxOpenFile * sizeof(OpenedFile));
	memset(g_pointer->openedFiles, 0, g_pointer->sb->maxOpenFile * sizeof(OpenedFile));
	
	g_pointer->openedFileCount = 0;

	close(fd);
	return noError;	
}

int process_arguments(char* command, char** paras)
{
	int args, quote;
	char *ptr1, *ptr2;
	char buf[LINE] = {0};
	
	args = 0;
	quote = 0;
	ptr1 = command;
	ptr2 = ptr1;
	
	while (*ptr2 != '\0')
	{
		
		if (*ptr2 == ' ' && quote == 0)
		{
			// strncpy(buf, ptr1, ptr2 - ptr1);
			// buf[ptr2 - ptr1] = 0;
			paras[args++] = ptr1;
			ptr1 = ptr2 + 1;		
			while (*ptr1 == ' ')
			{ 
				ptr1++;
			}			
			
		  *ptr2 = '\0';
			ptr2 = ptr1;
		}				

		if (*ptr2 == '"' && quote == 0)
		{
			quote = 1;
		}
		else if (*ptr2 == '"' && quote == 1)
		{
			quote = 0;
		}
		
		ptr2++;
		
	}
	
	if (*ptr1 != '\0')
	{ 
		paras[args++] = ptr1;
		*ptr2 = '\0';
	}
	
	return args;
}


usageErrorType batch_execute(char* script)
{
	FILE* fs;
	char buf[LINE] = {0};
	
	fs = fopen(script, "r");
	
	if (fs == NULL)
	{
		perror("Error open script!");
		return ioError;
	}
	
	while (fgets(buf, LINE, fs) != NULL)
	{
		if (parseCommand(buf) == exitError)
		{
			return exitError;
		}   
	}
	
	return noError;
	
}


usageErrorType batch_execute_output(char* script, char* output)
{
	outputStream = fopen(output, "a+");
	outputFlag = TRUE;
	
	usageErrorType ret;
	ret = batch_execute(script);
	
	fclose(outputStream);
	outputFlag = FALSE;
	
	return ret;	
}
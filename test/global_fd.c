#include "../commands.h"
#include "local.h"

FILE* gfd;
char* gChar;

FILE* Global_fd()
{
	gChar = (char*) malloc(100);
	strcpy(gChar, "hello this is from global fd.c");
	
	
	fseek(gfd, 5, SEEK_CUR);
	fwrite("hello world test", 16, 1, gfd);
	
	fflush(gfd);
	
	printf("gfd(in gfd)=%p\n", gfd);
	
	return gfd;
}

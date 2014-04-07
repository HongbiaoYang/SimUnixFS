#include "commands.h"

usageErrorType Smkfs()
{
	printf("mkfs called\n");
	
	int fd, i;
	off_t rs;
	
	fd = open(IMG_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
	
	
	
	// create the image with size 100MB	
	lseek(fd, 100 * ONE_MB - 1, SEEK_CUR);
	
	write(fd, "\n", 1);
	close(fd);	
	
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


usageErrorType Smkdir(char* dirname)
{
	printf("mkdir called\n");
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
	printf("ls called\n");
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






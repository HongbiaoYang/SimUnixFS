#include "../commands.h"
#include "local.h"

char* Global_var;

extern FILE*  gfd;
extern char* gChar;

void readFromFd()
{	
	int lfd;
	lfd = open("testFile.t", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
	
	printf("lfd=%d\n", lfd);
	
	char buf[LINE] = {'x','y','z'};
	
	read(3, buf, 3);
	printf("read from 3=%s\n", buf);
	
	close(3);
	
}

void testStrCat()
{
	char str[LINE] = {0};


	strcat(str, "hello,");
	strcat(str, "world,");
	strcat(str, "jackass,");
	strcat(str, str);
	
	printf("str=%s\n", str);
	
}


int main(int argc, char** argv)
{
	testStrCat(); 
	
	readFromFd();
	return 0;
	
	
	Global_var = (char*)malloc(100);
	memcpy(Global_var, "hello", 6);

	printf("size of pointer=%ld\n", sizeof(void*) );
	printf("size of iNode=%ld\n", sizeof(iNode) );
	printf("Gv=%s\n", Global_var);
	
	iNode inode;
	memset(&inode, 0, sizeof(iNode));
	printf("inode.type=%d\n", inode.type);


	char  buf[LINE]={'a'};
	int rs;
	FILE* lfd;
	gfd = NULL;

	gfd = fopen("testFile.t", "a+");

  lfd = gfd;
	fseek(gfd, 0, SEEK_CUR);
	fseek(lfd, 0, SEEK_CUR);

  printf("gfd = %p\n", gfd);


	rs = fread(buf, 4, 1, lfd);
	printf("read (%d)from lfd=%s\n",rs,  buf);

	//rs = fseek(gfd, 2, SEEK_CUR);
	//printf("fseek (%d)\n",rs);

	rs = fread(buf, 5, 1, gfd);
	printf("read (%d)from gfd=%s\n",rs,  buf);
	
	fclose(gfd);
	//fclose(lfd);
	
	
}


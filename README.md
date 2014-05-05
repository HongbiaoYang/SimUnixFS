SimUnixFS
=========

Simulated Unix like file system

Introduction
============
This simulated Unix-like file system program, is an interactive command line 
user interface, that emulates the functionality of the real Unix file system in 
user space. The program uses a fixed size file as the virtual disk, and 
"partition" this virtual disk into certain parts: the super block section, the
iNode bit map section, the iNode section, the data block map section, and the 
data block section. 

|super block|-bitMap-|-iNodeMap-|--iNode--|-------------DataBlock-------------|
|<------------------------Virtual Disk (100M)-------------------------------->|

System Design 
=============
1. Super block
The structure of super block is as below:
	typedef struct
	{
		int totalBlocks;
		int freeBlocks;
		int totalINode;
		int freeINode;
		
		int bitMapOffset;
		int iNodeMapOffset;
		int iNodeOffset;
		int dataOffset;
		
		int maxOpenFile;
	} 
	superBlock;
	
The first 4 members are statistic counts of the file system, which are used for 
initialization / formatting, and free iNode / block management. The next 4 
members are offsets for the sections mentioned in Introduction. To simplify the
system, all offsets are integer value that specify the number of bytes between
the beginning of the virtual disk and the beginning of each section. The last 
member is to just a user specified limit for maximum allowed number of files 
that can be opened at the same time.

2. BitMap
The bitmap is just a array of bits, which is used to indicate the free blocks
and occupied blocks. The size of this array is determined in the super block.
Each element of this array is a char (takes 1 byte), which set to 1 if the
corresponding data block is used, and set to 0 if it's empty.

3. iNodeMap
The iNodeMap is similary a the BitMap, but indicating the free iNodes and 
occupied iNodes. 

4. iNode
The structure of iNodes is as below:
	typedef struct
	{	
		int directPtr[D_PTR_CNT]; 
		int s_indirectPtr;
		int d_indirectPtr;
		int t_indirectPtr;
		char fileName[LINE];
		int selfIndex;
		int parentIndex;
		int link;
		int size;
		iNode_Type type; 
	} iNode;
	
Similar as actual Unix file system, the iNode here is to store the meta data
of files and directories. Each iNode is associated with a file or directory.
The member "type" is an enum that specifying whether the corresponding entry is
a file or a directory. The member "size" is the number of bytes taken by the 
file, or number of sub-entries for direcotry. The member "link" is the number 
of hard links to that file. The "selfIndex" and "parentIndex" is the index of
the current entry, and the index of the current entry's parent. This index here
is the offset of iNode's position in the iNode section, with this index, all 
iNode can be located by:
	iNode* entry = g_pointer->firstiNode + idx;
Here the pointer "entry" will point at the memory that stores the iNode whose 
index is "idx". The pointer g_pointer->firstiNode" is a global pointer that 
points the very first iNode. We will introduce the variable "g_pointer" in 
detail later. The member "fileName" is just a string buffer that stores the
readable name of the file / directory, here the maximum length of the file name
is limited by macro "LINE" (128 for current version). The rest of the members
are the pointers for the data block (for file) and sub-entries (for directory).
i) For file, the member "directPtr" is an integer array that store the index of 
the data block directly, which can store up to (BLOCK_SIZE * D_PTR_CNT = 4K*12 
= 48K) bytes of data.  If the file size exceed this size (48K), the single 
indirect pointer (s_indirectPtr) will be used. This pointer (integer index 
actually) will point to a data block that is used for store indexes to other 
data blocks. Since we use integer as index, the single indirect pointer is able
to store up to BLOCK_SIZE / sizeof(int) = 1K blocks of data. The double indirect
pointer is used to store the single indirect pointers, which is almos the same 
as the traditional Unix file system. Because the double indirect pointer can 
store big enough file, we are not using the triple indirect pointer for file.
ii) For directory, similarly, we use the direct pointer to store the index of
the sub entries, which can store up to 12 entries. When the nunber of entries 
exceeds 12, we will use indirect pointer to store more entries. In this system,
we only use one level of indirect pointer to store sub-entries, since we think
1024+12 = 1036 is a big enough number to store sub-entries (normally it is not
practical to put over a thousand files / sub directories in a directory). To 
avoid possible confusion, we use the t_indirectPtr to store the sub entries in 
directory, since this variable is not used in file iNode.

5. Data block
There is nothing to do with data block, unless we need to read or write data 
at a specific data block, there is no need to read the data block into memory
in advance. When a read / write request is initiated, we just open the whole
virtual disk file, seek to the specific location in data block area, and then 
perform the actual read / write using the system I/O call.

6. Global Pointers
Because many operations only need to read some data once, and write the data 
much less frequently, we don't need to do the system I/O call that frequently.
So we decide to cache all the data into memory except the data block. For some 
command (like "ls", "tree", "cd"), we only need to use the data in memory 
without doing the actual read at all; for those command (like "mkdir", "rmdir")
that will change the meta data we may need to write the change of the meta data
back to disk; for those command (like "open", "cp"), we need to change both meta
data and data, so we need to write changes of both into disk. Since the meta 
data is not very big (super block, bitMap, iNodeMap), we don't actually need to 
distinguish (indeed it's not hard to do that, though) which part is changed, and
just write everything from memory to disk. For disk data modification, we can't 
afford writing all of them back, so we just seek to specific location and do the
write / read that is necessary.
The Global Pointers are used to store such meta data for fast access. It has 
structure as below:
typedef struct
{
superBlock* sb;
char* bitMapPointer;
char* freeInodePointer;
iNode* firstiNode;
int currentDir;
iNode* curDirNode;
OpenedFile* openedFiles;
int openedFileCount;
}
GLOBAL_Pointers;

The first 4 are easy to understand, which are the pointer to the actual address
of the super block, bitMap, iNodeMap and iNode. Every time after the system is 
up, all those meta data will be kept in memory, so the addresses of those meta 
data in memory are stored in those global pointers until the system is down. The
5th and 6th member are used to store the current directory information, actually
these two are a little redundant, since the current directory iNode pointer can 
be found using the current directory index, but keeping this pointer in global
variable will increase the performance as this is a very frequently used. The 
next member is a struct that contains some information about the openning files,
this struct will be explained later in detail. The last member is the current
opened files of the system, which is used check if the opened file has exceed
the predefined limit.

7.  Opened File 
The OpenedFile struct is used to deal with the file access operations. The data
structure is shown as below:
typedef struct
{
	iNode* oNode;
	int offset;
	char* buf;
	int bufUsed;
	char flag;
}
OpenedFile;

From the GLOBAL_Pointer mentioned in the previous section, we see that there is 
a member "openedFiles", which is used globally, to manage the opened files. The 
1st member of this struct is "oNode", which is a iNode* typed pointer that is 
pointing to the iNode of the opened file. The 2nd member is offset, which is 
the current offset of the read / write position. The 3rd member is the buffer,
which is used to store the data to be written. This buffer is to assemble the
small writes together and reduce the number of times the system actually sync
the data into the disk. The member bufUsed is used to determine when the buffer
is reaching some critical threshold and thus a disk sync is required. The last
member flag is for the read/write privillege control.

How to run
==========
To run this system, just type the binary without arguments:
--------------------
./simfs
--------------------
Then you will see the string "$/$>", which means you have entered the console.
Then depending on whether it's the first time you run this system or not, a 
welcome message or alert message to remind you format the disk will appear in 
this console. Before you run "mkfs" (or run "sh" if you have "mkfs" command in 
your script), any other command will not be able to proceed. 
Since there are a lot of commands in this system, we are not going to cover all
of them in this document. To quickly test this system, you can run the commands
in batch by typing the following command in the console:
--------------------
sh sample.txt
--------------------
All commands in the file "sample.txt" (which is a file in the current directory)
will be executed line by line. If you want to see the output in a file instead
of dumping them in the console, just add a second argument in your command like:
--------------------
sh sample.txt output.txt
--------------------
Then all output will be stored in the file output.txt.
For more information, type "Help" in the console to see a manual of the command
list.
















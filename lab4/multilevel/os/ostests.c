#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"
#include "files.h"

#define num 256

void RunOSTests() {
	// STUDENT: run any os-level tests here



	char file0[12] = "file1.txt";
	char file1[12] = "file2.txt";
	int handle = -1;
	int handle1 = -1;
	int i;
	char data[50] = ". . . . . . . . . . . . . . . . . . . . . . . . . ";
	char data1[1000];
	char data2[1000];
	char readData[1000];


	printf("\n\n-----------------------------------Problem 4 Check-----------------------------------\n\n");
	handle = DfsInodeOpen(file0);
	if(handle != DFS_FAIL){
		printf("\n%s successfully opened with inode number:%d and size:%d\n",file0,handle,DfsInodeFilesize(handle));
		printf("0 size indicates new file 900 indicates file created from last time.\n");
	}

	if (DfsInodeFilesize(handle) == 900) {
		printf("\nReading old data:\n");
		DfsInodeReadBytes(handle,(void *)readData, 0, 900);
		printf("DATA__________________________________________________________________________\n");
		for(i=0;i<900;i++)
			printf("%c",readData[i]);
		printf("\nCheck if get old data\n\n");
	}


	for(i=0;i<1000;i++){
		data1[i]='0';
	}
	for(i=0;i<1000;i++){
		data2[i]='1';
	}

	DfsInodeWriteBytes(handle,(void *)data1, 0 ,100);
	DfsInodeWriteBytes(handle,(void *)data, 100, 50);
	DfsInodeWriteBytes(handle,(void *)data2, 150, 150);
	printf("%s: Wrote '0' 100 times, '. . . . . ' in the middle, and '1' 150 times.\n",file0);

	DfsInodeReadBytes(handle,(void *)readData, 0, 300);

	printf("DATA__________________________________________________________________________\n");
	for(i=0;i<300;i++)
		printf("%c",readData[i]);
	printf("\n%s: Check if get the same data after read...\n\n",file0);

	DfsInodeWriteBytes(handle,(void *)readData, 300, 300);
	DfsInodeWriteBytes(handle,(void *)readData, 600, 300);

	DfsInodeReadBytes(handle,(void *)readData, 0, 900);
	printf("DATA__________________________________________________________________________\n");
	for(i=0;i<900;i++)
		printf("%c",readData[i]);
	printf("\n%s: Check if the whole 300 bytes are replicated a total three times now...\n\n",file0);


	for(i=0;i<16;i++)
		DfsInodeWriteBytes(handle,(void *)data, i*50+5, 50);
	DfsInodeWriteBytes(handle,(void *)data, i*50+5, 50);
	DfsInodeWriteBytes(handle,(void *)data, (1+i)*50+5, 40);

	DfsInodeReadBytes(handle,(void *)readData, 0, 900);
	printf("DATA__________________________________________________________________________\n");
	for(i=0;i<900;i++)
		printf("%c",readData[i]);
	printf("\n%s: Check if get '00000. . . . . . . . . . .11111'\n\n",file0);


	printf("%s: Filesize: %d, should be 900 bytes.\n\n",file0,DfsInodeFilesize(handle));



	handle1 = DfsInodeOpen(file1);
	if(handle1 != DFS_FAIL){
		printf("%s successfully opened with inode number:%d and size:%d\n",file1,handle1,DfsInodeFilesize(handle1));
	}
	for(i=0;i<800;i++)
		DfsInodeWriteBytes(handle1,(void *)data1, i*512, 512);
	printf("%s: Filesize: %d, should be 400Kbytes. Contains all ascii '0'.\n\n",file1,DfsInodeFilesize(handle1));



/*


	printf("\n\n-----------------------------------Problem 6 Check-----------------------------------\n\n");

	fd = FileOpen(file1, "r");
	printf("\n%s successfully opened with fd number:%d\n",file1,fd);
	//FileWrite(fd, data, 50);
	FileRead(fd, readData, 50);
	for(i=0;i<50;i++)
		printf("%c",readData[i]);
	printf("\nRead first 50 bytes of %s\n",file1);


	fd1 = FileOpen("abc.txt", "w");
	printf("%s successfully opened with fd number:%d\n","abc.txt",fd1);
	//FileRead(fd1, readData, 0);

	FileClose(fd);
	printf("\n%s successfully closed with fd number:%d\n",file1,fd);
	fd = FileOpen("new.txt", "w");
	printf("%s successfully opened with fd number:%d\n","new.txt",fd);

	FileClose(fd1);
	printf("\n%s successfully closed with fd number:%d\n","abc.txt",fd1);

	FileDelete(file0);
	//FileDelete(file1);
	FileDelete("abc.txt");

	for (i=0; i<2*1024; i++) {
		FileWrite(fd, data2, 512);
		FileSeek(fd, 1, FILE_SEEK_END);
	}
	printf("Wrote 1 MB to file new.txt\n");

	FileSeek(fd, 1024, FILE_SEEK_END);
	//FileRead(fd, readData, 50);
	FileClose(fd);
	printf("\n%s successfully closed with fd number:%d\n","new.txt",fd);

	fd = FileOpen("new.txt", "r");
	printf("%s successfully opened with fd number:%d\n","new.txt",fd);
	FileSeek(fd, 1024, FILE_SEEK_END);
	for(i=0;i<50;i++)
		readData[i] = '-';
	FileRead(fd, readData, 50);
	for(i=0;i<50;i++)
		printf("%c",readData[i]);
	printf("\nRead 50 bytes of %s\n","new.txt");

	printf("\n\n-----------------------------------test file truncate 'w'-----------------------------------\n\n");



	fd = FileOpen("test", "w");
	FileWrite(fd, data2, 512);
	FileClose(fd);
	printf("Size is %d\n", DfsInodeFilesize(DfsInodeFilenameExists("test")));
	fd = FileOpen("test", "r");
	FileClose(fd);
	printf("Size is %d\n", DfsInodeFilesize(DfsInodeFilenameExists("test")));
	fd = FileOpen("test", "w");
	FileClose(fd);
	printf("Size is %d\n", DfsInodeFilesize(DfsInodeFilenameExists("test")));

*/

	printf("\n\n-----------------------------------Check END-----------------------------------\n\n");



}


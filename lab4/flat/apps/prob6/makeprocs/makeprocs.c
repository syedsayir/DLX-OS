#include "usertraps.h"
#include "misc.h"
#include "files_shared.h"


void main (int argc, char *argv[])
{
	
	int i, fd, fd1;
	char file1[12] = "file2.txt";
	char file0[12] = "file1.txt";
	char data2[1024];
	char readData[1024];

	for(i=0;i<1024;i++){
		data2[i]='@';
	}


	


	fd1 = file_open("abc111.txt", "w");
	//file_read(fd1, readData, 0);

//	file_close(fd);
//	Printf("\n%s successfully closed with fd number:%d\n",file1,fd);
//	fd = file_open("new.txt", "w");
//	Printf("%s successfully opened with fd number:%d\n","new.txt",fd);

//	file_close(fd1);
//	Printf("\n%s successfully closed with fd number:%d\n","abc.txt",fd1);

//	file_delete(file0);
	//file_delete(file1);
//	file_delete("abc.txt");

//	for (i=0; i<2*1024; i++) {
		file_write(fd1, data2, 1024);
//		file_seek(fd, 1, FILE_SEEK_END);
//	}
	Printf("Wrote 1024 B to file new.txt\n");

	file_close(fd1);
	//file_seek(fd, 1024, FILE_SEEK_END);
	//file_read(fd, readData, 50);
	fd1 = file_open("abc111.txt", "r");

fd=	file_read(fd1, readData, 1024);

//	fd = file_open("new.txt", "r");
	Printf("successfully opened with fd number:%d\n",fd);
//	file_seek(fd, 1024, FILE_SEEK_END);
	for(i=0;i<1024;i++)
	Printf("%c ,",readData[i]);
fd=	file_read(fd1, readData, 1024);
	Printf("successfully opened with fd number:%d\n",fd);

}

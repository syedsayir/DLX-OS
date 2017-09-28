#include <stdio.h>
#include <stdlib.h>


int resolvePath(char* path, char dirs[10][70]) {
	int i = 0; //index into path
	int j = 0; //index into dirs
	int k = 0; //index into dirs[j]
	char curr;
	if (path[0] == 0) {
		printf("Error: Empty path Passed.\n");
		return -1;
	}
	if (path[0] == 47) {
		dirs[0][0] = '/';
		dirs[0][1] = 0;
		i++;
		j++;
	}
	// asolute address start from root
	while(path[i]){
		curr = path[i];
		if (k == 0 && curr == 47) {
			printf("Invalid path Passed!\n");
			return -1;
		}
		if (curr == 47) {
			dirs[j][k] = 0;
			j++;
			k = 0;
		}
		else {
			dirs[j][k] = curr;
			k++;
		}
		i++;
	}
	dirs[j][k] = 0;
	return j+1;
}

void main(){
	int i = 0;
	char dirs[10][70];
	printf("i : %d\n",i);
	if ((i = resolvePath("dir1/abc/file.tx/abc", dirs)) != -1) {
	printf("%s\n",dirs[0]);
	printf("%s\n",dirs[1]);
	printf("%s\n",dirs[2]);
	printf("%s\n",dirs[3]);
	printf("%s\n",dirs[4]);
	}
	printf("i:%d\n",i);
}

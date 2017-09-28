#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
	int pid, i;
	i = 0;
	Printf("before fork : i is:%d\n",i);

	pid=getpid();
	pid = unixfork();
	if (pid==0) {
		Printf("child : My Pid is:%d\n",getpid());
		Printf("child : i is:%d\n",i);
		i = 1;
		Printf("child : changed i to 1 and it is:%d\n",i);
	}
	else {
		Printf("Parent : My Pid is:%d\n",getpid());
		Printf("Parent : i is:%d\n",i);
		i = 2;
		Printf("Parent : changed i to 2 and it is:%d\n",i);
	}
}

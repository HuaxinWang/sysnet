#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/*creat*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*errno*/
#include <errno.h>

typedef struct student
{
	int id;
	char name[32];
	float score;
}STU;

int main(int argc,char *argv[])
{
	int fd = open(argv[1],O_RDONLY|O_CREAT,0664);
	STU tmp;
	int ret = -1;
	while(1)
	{
		ret = read(fd,&tmp,sizeof(STU));
		if(-1 == ret || 0 == ret)
		{
			break;
		}
		printf("id:%d,name:%s,score:%.2f\n",tmp.id,tmp.name,tmp.score);
	}
	close(fd);
	return 0;
}

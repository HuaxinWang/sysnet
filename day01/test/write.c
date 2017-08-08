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
	STU stu[3] = {{1001,"zhangsan",89},{1002,"lisi",87},{1003,"wangwu",78}};
	int fd = open(argv[1],O_WRONLY|O_CREAT,0664);
	write(fd,stu,sizeof(stu));
	close(fd);
	return 0;
}

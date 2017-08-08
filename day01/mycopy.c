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

int main(int argc,char *argv[])
{
	int srcfd = -1;
	//O_CREAT:若文件不存在，则以指定的权限创建文件
	srcfd = open(argv[1],O_RDONLY);
	if(-1 == srcfd)
	{	
		perror("open");
		return -1;		
	}
	printf("打开文件成功\n");

	int destfd = -1;
	//O_CREAT:若文件不存在，则以指定的权限创建文件
	destfd = open(argv[2],O_WRONLY|O_CREAT,0664);
	if(-1 == srcfd)
	{	
		perror("open");
		return -1;		
	}
	printf("打开文件成功\n");

	char caBuf[4096] = {'\0'};
	int  ret = -1;
	while(1)
	{
		ret = read(srcfd,caBuf,4096);
		if(-1 == ret)
		{
			perror("read");
			break;
		}
		else if(0 == ret)
		{
			printf("拷贝完成\n");
			break;
		}
		write(destfd,caBuf,ret);
	}

	close(srcfd);//关闭文件
	close(destfd);//关闭文件
		
	return 0;		
}

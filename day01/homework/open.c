#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc,char *argv[])
{
	int fd = -1;
	//若以以下方式打开文件，若文件不存在不会自动创建文件
	//参数一：要打开的文件的路径
	//参数二：
	//		O_RDONLY表示以只读的方式打开文件
	//		O_WRONLY表示以只写的方式打开文件
	//		O_RDWR表示以读写的方式打开文件
	fd = open(argv[1],O_RDONLY);
	//fd = open(argv[1],O_WRONLY);
	//fd = open(argv[1],O_RDWR);
	if(fd == -1)
	{
		perror("open");
	}
	else
	{
		printf("打开文件,fd = [%d]\n",fd);
		close(fd);
	}
	

	return 0;
}

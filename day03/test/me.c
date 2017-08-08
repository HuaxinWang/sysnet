#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
/*creat*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <errno.h>  //errno

int main(int argc, char *argv[])
{
	int fd = open("myfifo", O_RDWR);
	int fd2 = open("myfifo2",O_RDWR);
	pid_t pid = fork();
	while(1)
	{
		if(pid > 0)
		{
			char caMsg[64] = {'\0'};
			memset(caMsg, '\0', sizeof(caMsg));
			printf("请输入数据:\n");
			read(STDIN_FILENO, caMsg, 64);
			write(fd, caMsg, strlen(caMsg));
		}
		else if (0 == pid)
		{
			char caMsg[64] = {'\0'};
			read(fd2, caMsg, 64);
			printf("获得数据:\n");
			write(STDOUT_FILENO, caMsg, strlen(caMsg));
		}
	}
	sleep(1);
	return 0;
}





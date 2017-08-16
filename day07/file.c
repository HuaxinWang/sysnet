#include "file.h"

int openFile(const char *pathname, int flags)
{
	int fd = -1;
	fd = open(pathname, flags|O_CREAT, 0664);
	if (-1 == fd)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}
	return fd;
}

void saveUsrListToFile(int fd, const UsrList *list)
{
	if (NULL == list)
	{
		return;
	}
	UsrInfo *node = list->pFirstNode;
	while (NULL != node)
	{
		write(fd, node, sizeof(UsrInfo));
		node = node->pNext;
	}
}

void getUsrInfoFromFile(int fd, UsrList *list)
{
	if (NULL == list)
	{
		return;
	}
	UsrInfo *node = NULL;
	int ret = -1;
	while (1)
	{
		node = makeUsrInfoNode();
		ret = read(fd, node, sizeof(UsrInfo));
		if (-1 == ret || 0 == ret)
		{
			free(node);
			break;
		}
		node->sockfd = -1;
		node->pNext = NULL;
		insertUsrList(list, node);
	}
}







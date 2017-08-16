#include "server.h"
#include "list.h"
#include "protocol.h"
#include "file.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  //htons

#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
List *g_pList = NULL;
uint g_uiBaseId = 1000;

static void handleRegistRequest(int sockfd, PDU *pdu)
{
	//产生节点用于保留客户端的注册信息并存入链表
	//eg: id 和 密码
	Node *node = makeNode();
	node->sockfd = -1;
	node->uiId = g_uiBaseId;
	strncpy(node->caPwd, pdu->caMsg, PWD_LEN);
	insertList(g_pList, node);
	
	//将链表中的数据写入文件
	saveUsrInfoToFile(USRINFO_PATH, g_pList);

	//给客户端产生一个注册回复
	PDU *respdu = makePDU(0);
	respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
	respdu->uiTo = g_uiBaseId;
	g_uiBaseId++;
	sendPDU(sockfd, respdu);
	free(respdu);
}

static void handleLoginRequest(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	Node *node = g_pList->pFirstNode;

	while (NULL != node)
	{
		if (node->uiId == pdu->uiFrom
		    && 0 == strncmp(node->caPwd, pdu->caMsg, PWD_LEN))
		{
			if (-1 == node->sockfd)
			{
				node->sockfd = sockfd;
			}
			else
			{
				node = NULL;
			}
			break;
		}
		node = node->pNext;
	}
	
	PDU *respdu = NULL;
	
	if (NULL != node)
	{
		respdu = makePDU(strlen(LOGIN_OK));
		strncpy(respdu->caMsg, LOGIN_OK
				, strlen(LOGIN_OK));
	}
	
	else
	{
		respdu = makePDU(strlen(LOGIN_FAILED));
		strncpy(respdu->caMsg, LOGIN_FAILED
			    , strlen(LOGIN_FAILED));
	}
	
	respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
	sendPDU(sockfd, respdu);
	free(respdu);
}

static void handlePrivateChatRequest(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	Node *node = g_pList->pFirstNode;
	while (NULL != node)
	{
		if (node->uiId == pdu->uiTo)
		{
			sendPDU(node->sockfd, pdu);
			break;
		}
		node = node->pNext;
	}
}

static void handleLsRequest(int sockfd)
{	
	DIR *dir = opendir("./receive");

	if(NULL == dir)
	{
		perror("没有目录!\n");
		return;
	}
	char msg[4096] = {'\0'};
	struct dirent *pdirent = NULL;
	int i = 0;
	int num = 0;
	while(1)
	{
		pdirent = readdir(dir);
			
		if(NULL != pdirent)
		{
			if(0 == strncmp(".",pdirent->d_name,1))
			{}
			else 
			{
				strcat(msg+i,pdirent->d_name);
				i += 64;
				num++;
			}
		}
		
		else break;
	}
	PDU *pdu = makePDU(sizeof(msg));
	for(i = 0 ; i < 4096 ; i++)
	{
		pdu->caMsg[i] = msg[i];
	}
	pdu->filesize = num;
	pdu->uiMsgType = ENUM_MSG_TYPE_LS_FILE_REQUEST;

	sendPDU(sockfd, pdu);
}

static void handleGroupChatRequest(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	Node *node = g_pList->pFirstNode;
	while (NULL != node)
	{
		if (node->sockfd > 0)
		{
			sendPDU(node->sockfd, pdu);
		}
		node = node->pNext;
	}
}

static void handleExitRequest(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	Node *node = findNodeById(pdu->uiFrom, g_pList);
	if (NULL != node)
	{
		node->sockfd = -1;
	}
}
static void handlesend(int sockfd,PDU *pdu)
{
	if(NULL == pdu)
	{
		return;
	}
	int fd = -1;
	char caname[64]= "./receive/";
	strcat(caname,pdu->caname);
	
	fd = open(caname,O_WRONLY|O_CREAT,0664);
	if(-1 == fd)
	{
		perror("open");
		return;
	}
	
	int writeed = 0;
	int left = pdu->filesize;
	int ret = -1;
	while(left)
	{
		if(left > 4096)
		{
			ret = write(fd,(char *)((pdu->caMsg)+writeed),4096);
		}
		
		else
		{
			ret = write(fd,(char *)((pdu->caMsg)+writeed),left);
		}
		
		if(-1 == ret || 0 == ret)
		{
			break;
		}
			
		writeed += ret;
		left -= ret;
	}
	
}
static void handledown(int sockfd,PDU *rvpdu)
{
	int fd = -1;
	char caname[64] = "./receive/";
	strcat(caname,rvpdu->caname);
	fd = open(caname,O_RDONLY,0664);
	if(-1 == fd)
	{
		printf("没有该文件\n");
		return;
	}
	unsigned int len = lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);
	char buf[4096] = {'\0'};	
	PDU *pdu = makePDU(len);
	int ret = -1;
	pdu->uiMsgType = ENUM_MSG_TYPE_DOWN_REQUEST;
	int readed = 0;
	int left = len; 
	while(left)
	{
		if(len > 4096)
		{
			ret = read(fd,(char*)buf+readed,4096);
		}
		else
		{
			ret = read(fd,(char*)buf+readed,left);
		}
		if(-1 == ret || 0 == ret)
		{
			break;
		}
	 	readed += ret;
		left -= ret;		
	}
	strcpy(pdu->caname,rvpdu->caname);
	pdu->filesize = len;
	strncpy(pdu->caMsg,buf,strlen(buf));
	sendPDU(sockfd,pdu);
	close(fd);
}
static void handleremv(int sockfd,PDU *pdu)
{	
	int fd = -1;
	char caname[64] = "./receive/";
	strcat(caname,pdu->caname);
	fd = open(caname,O_RDONLY,0664);
	if(-1 == fd)
	{
		printf("没有该文件\n");
		return;
	}
	remove(caname);
	close(fd);
}
static void handlerename(int sockfd,PDU *pdu)
{	
  	int fd = -1;
	char caname[64] = "./receive/";
	char newname[64] = "./receive/";
	strcat(caname,pdu->caname);
	strcat(newname,pdu->caMsg);
	
	fd = open(caname,O_RDONLY,0664);
	
	if(-1 == fd)
	{
		perror("open");
		return;
	}	
	rename(caname,newname);
	close(fd);

}
static void *handleClient(void *arg)
{
	int sockfd = (int)arg;
	PDU *pdu = NULL;
	while (1)
	{
		//接收客户端的数据
		pdu = recvPDU(sockfd);
		if (NULL == pdu)
		{
			pthread_exit(NULL);
		}
		//判断消息类型
		//根据消息类型的不同做出不同的处理
		switch (pdu->uiMsgType)
		{
		//处理客户端的注册请求
			case ENUM_MSG_TYPE_REGIST_REQUEST:
				handleRegistRequest(sockfd, pdu);
				break;	
		//处理客户端的登录请求
			case ENUM_MSG_TYPE_LOGIN_REQUEST:
				handleLoginRequest(sockfd, pdu);
				break;	
		//处理客户端的私聊请求
			case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
				handlePrivateChatRequest(sockfd, pdu);
				break;	
		//处理客户端的群聊请求
			case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
				handleGroupChatRequest(sockfd, pdu);
				break;	
		//处理客户端的退出请求
			case ENUM_MSG_TYPE_EXIT_REQUEST:
				handleExitRequest(sockfd, pdu);
				printf("线程退出\n");
				pthread_exit(NULL);
				break;	
			case ENUM_MSG_TYPE_LS_FILE_REQUEST:
				handleLsRequest(sockfd);
				break;	
			case ENUM_MSG_TYPE_DOWN_REQUEST:
				handledown(sockfd, pdu);		
				break;	
			case ENUM_MSG_TYPE_SEND_REQUEST:
				handlesend(sockfd, pdu);	
				break;	
			case ENUM_MSG_TYPE_REMV_REQUEST:
				handleremv(sockfd, pdu);	
				break;
			case ENUM_MSG_TYPE_RENAME_REQUEST:
				handlerename(sockfd, pdu);	
				break;
			default:
				break;
		}
		free(pdu);
	}
}

void setBaseId()
{
	Node *node = g_pList->pFirstNode;
	int sign = 0;
	while (NULL != node)
	{
		if (g_uiBaseId < node->uiId)
		{
			g_uiBaseId = node->uiId;
			sign = 1;
		}	
		node = node->pNext;
	}
	if (1 == sign)
	{
		g_uiBaseId++;
	}
}

int makeSocket()
{
	//产生socket用于监听客户端的连接
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

void makeBind(int sockfd)
{
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(8888);
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bzero(servAddr.sin_zero, 8);

	//将sockfd和特定的ip及端口绑定
	//表示通过该scokfd来监听从绑定的ip连接过来的
	//作用于指定端口的客户端
	int ret = -1;
	ret = bind(sockfd
			   , (struct sockaddr *)&servAddr
			   , sizeof(servAddr));
	if (-1 == ret)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}
}

void makeListen(int sockfd)
{
	//设置该sockfd每次能够处理的最大客户端数
	int ret = listen(sockfd, 10);
	if (-1 == ret)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
}

void acceptClient(int sockfd)
{
	struct sockaddr_in clientAddr;
	int clientSockfd = -1;
	int iLen = sizeof(clientAddr);
	pthread_t thread;
	while (1)
	{
		printf("等待客户端的连接...\n");
		
		//阻塞等待客户端的连接
		//若有客户端连接过来，
		//则会自动将客户端的相应信息存入clientAddr中
		//然后往下执行
		//若有客户端连接服务器成功
		//则产生一个新的socket
		//该新的socket用于服务器和客户端数据交换
		
		clientSockfd = accept(sockfd,(struct sockaddr *)&clientAddr,&iLen);
		
		if (-1 == clientSockfd)
		{
			perror("accept");
			break;
		}

		//inet_ntoa:将整形表示的ip
		//          转换成点分十进制表示的ip
		printf("ip为%s的客户端连接到服务器\n"
			   , inet_ntoa(clientAddr.sin_addr));
		
		printf("产生的新的用于数据交换的sockfd:%d\n"
			   , clientSockfd);
	    
		//每来一个客户端的连接
		//创建一个新的线程来专门处理该客户端
		pthread_create(&thread, NULL
					, handleClient
					, (void *)clientSockfd);
	}
}

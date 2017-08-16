#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
/*socket*/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  //htons
#include <netinet/in.h> //inet_addr
#include <pthread.h>

#include "list.h"
#include "file.h"
#include "protocol.h"


UsrList *g_pUsrList = NULL;
uint g_uiBaseId = 1000;


void handleClientRegistRequest(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	//将客户端的注册信息保存到链表中
	uint uiId = g_uiBaseId++;
	UsrInfo *node = makeUsrInfoNode();
	node->uiId = uiId;
	memcpy(node->caPwd, pdu->caMsg, PWD_LEN);
	insertUsrList(g_pUsrList, node);
	//将链表中的信息写入文件
	int fd = openFile(USR_INFO_FILE, O_RDWR);
	saveUsrListToFile(fd, g_pUsrList);
	close(fd);

	//生成回复信息给客户端
	PDU *respdu = makePDU(strlen(REGIST_OK));
	respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
	respdu->uiTo = uiId;
	memcpy(respdu->caMsg, REGIST_OK, strlen(REGIST_OK));
	writePDU(sockfd, respdu);
	free(respdu);
}

void handleClientLoginRequest(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	char caPwd[PWD_LEN] = {'\0'};
	strncpy(caPwd, pdu->caMsg, PWD_LEN);
	UsrInfo *node = NULL;
	node = findUsrLisByIdAndPasswd(g_pUsrList
					          , pdu->uiFrom, caPwd);
	PDU *respdu = NULL;
	if (NULL != node)
	{
		respdu = makePDU(strlen(LOGIN_OK));	
		memcpy(respdu->caMsg, LOGIN_OK
			   , strlen(LOGIN_OK));
	}
	else
	{
		respdu = makePDU(strlen(LOGIN_FAILED));	
		memcpy(respdu->caMsg, LOGIN_FAILED
			   , strlen(LOGIN_FAILED));
	}
	respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
	writePDU(sockfd, respdu);
	free(respdu);
}

void *handleClient(void *arg)
{
	int sockfd = (int)arg;
	PDU *pdu = NULL;
	while (1)
	{
		//获得客户端发送过来的信息
		pdu = readPDU(sockfd);
		if (NULL == pdu)
		{
			break;
		}
		//根据消息类型的不同做不同处理
		switch (pdu->uiMsgType)
		{
		case ENUM_MSG_TYPE_REGIST_REQUEST:
			handleClientRegistRequest(sockfd, pdu);
			break;
		case ENUM_MSG_TYPE_LOGIN_REQUEST:
			handleClientLoginRequest(sockfd, pdu);
			break;
		default:
			break;	
		}
		free(pdu);
		pdu = NULL;
	}
#if 0
	uint uiId = g_uiBaseId;
	g_uiBaseId++;
	//将新连接的客户端的信息生成节点保存到链表中
	UsrInfo *node = makeUsrInfoNode();
	node->sockfd = sockfd;
	node->uiId = uiId;
	insertUsrList(g_pUsrList, node);

	//将新分配的id发送给客户端
	char caId[128] = {'\0'};
	strcpy(caId, "服务器分配的Id:");
	sprintf(caId+strlen(caId), "%d", uiId);
	write(sockfd, caId, strlen(caId));

	//将新产生的客户端id告知其他客户端
	memset(caId, '\0', 128);
	strcpy(caId, "有新客户端产生,id为:");
	sprintf(caId+strlen(caId), "%d", uiId);
	node = g_pUsrList->pFirstNode;
	while (NULL != node)
	{
		write(node->sockfd, caId, strlen(caId));
		node = node->pNext;
	}

	char caMsg[64] = {'\0'};
	int ret = 0;
	int per = -1;
	char caRealMsg[64] = {'\0'};
	char *p = NULL;
	while (1)
	{
		memset(caMsg, '\0', 64);
		memset(caRealMsg, '\0', 64);
		ret = read(sockfd, caMsg, 64);
		if (-1 == ret || 0 == ret)
		{
			break;
		}
		// 1000-->"0 hello server"
		// 1000-->"1002 hello server"
		sscanf(caMsg, "%d", &per);

		// "4 says: hello server" --> 5
		p = strchr(caMsg, ' ');
		
		sprintf(caRealMsg, "%d says: %s", uiId, p);
		
		if (0 == per)
		{
			//群发
			node = g_pUsrList->pFirstNode;
			while (NULL != node)
			{
				write(node->sockfd
					  , caRealMsg
					  , strlen(caRealMsg));
				node = node->pNext;
			}
		}
		else
		{
			//发送给指定id的客户端
			node = findUsrLisById(g_pUsrList, per);
			if (NULL != node)
			{
				write(node->sockfd, caRealMsg
					  , strlen(caRealMsg));
			}
		}
	}
#endif
}

int main(void)
{
	g_pUsrList = makeUsrList();
	//将之前注册的用户信息从文件中读取出来
	//放入链表中
	int fd = openFile(USR_INFO_FILE, O_RDWR);
	getUsrInfoFromFile(fd, g_pUsrList);
	close(fd);

	int sockfd = -1;
	//AF_INET:使用ipv4协议来通信
	//SOCK_STREAM:可靠传输  --> tcp
	//SOCK_DGRAM:非可靠传输 --> udp
	//0:表示使用传输方式相对应的默认协议
	//成功返回socket文件描述符
	//可以像读写文件一样方便的发送和接收数据
	//失败返回-1
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	//设置要监听的服务器的ip地址
	//将点分十进制字符串表示的ip转换成整数表示
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//设置要监听的服务器的端口，用于识别应用
	//htons:将数据由本地字节序转换成网络字节序表示
	//字节序:大端字节序和小端字节序
	servAddr.sin_port = htons(8888);
	
	//将指定大小的空间数据置为0
	bzero(servAddr.sin_zero, 8);

	int ret = -1;
	//设置要监听的ip和端口
	ret = bind(sockfd
			   , (struct sockaddr *)&servAddr
			   , sizeof(servAddr));
	if (-1 == ret)
	{
		perror("ret");
		return -1;
	}
	//设置服务器每次能够处理最大的客户端数
	//并且使sockfd处于监听状态
	listen(sockfd, 10);

	struct sockaddr_in clientAddr;
	int iLen = sizeof(clientAddr);
	int clientSockfd = -1;
	pthread_t thr;
	while (1)
	{
		printf("接收客户端的连接...\n");
		//接收由sockfd监听到的客户端的连接
		//若没有客户端的连接则阻塞等待
		//并连接来的客户端的基本信息写入clientAddr中
		//连接成功产生一个新的socket，
		//该新产生的socket由于服务器和该客户端通信
		//失败返回-1
		clientSockfd = accept(sockfd
					, (struct sockaddr *)&clientAddr
					, &iLen);
		if (-1 == clientSockfd)
		{
			perror("accept");
			break;
		}
		//inet_ntoa:
		//将整数表示的ip转换成点分十进制字符串表示
		printf("ip为%s的客户端连接到服务器\n"
			   , inet_ntoa(clientAddr.sin_addr));
		printf("新产生的sockfd=%d\n", clientSockfd);
	
		//每来一个客户端的连接
		//专门创建一个线程处理该客户端	
		pthread_create(&thr, NULL, handleClient
					   , (void *)clientSockfd);	
	}

	return 0;
}

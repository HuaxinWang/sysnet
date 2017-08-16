#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  //htons

#include "client.h"
#include "protocol.h"
#include "file.h"
uint g_uiId = 0;

static void handleChat(PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	printf("%u says: \n", pdu->uiFrom);
	write(STDOUT_FILENO, pdu->caMsg, pdu->uiMsgLen);
}
static void handlefile(PDU *pdu)
{
	if(NULL == pdu)
	{
		return;
	}
	printf("文件:\n");
	
	int i = 0 , j = 0;
	
	for(i = 0 ; i < pdu->filesize ; i++)
	{
		printf("%s\t",pdu->caMsg+j);
		j += 64;
	}
	printf("\n");
}

static void handledown(PDU *pdu)
{
	if(NULL == pdu)
	{
		return;
	}
	int fd = -1;
	char caname[64]= "./download/";
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
// 循环接收服务器的信息并处理
void *handleServer(void *arg)
{
	int sockfd = (int)arg;
	PDU *pdu = NULL;
	while (1)
	{
		//   接收服务器的消息
	 	pdu = recvPDU(sockfd);
		if (NULL == pdu)
		{
			printf("和服务器已断开\n");
			exit(0);
		}
		//根据消息的类型做出不同的处理
		switch (pdu->uiMsgType)
		{
		//处理服务器返回的注册回复信息
			case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
			case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
				handleChat(pdu);
				break;
			case ENUM_MSG_TYPE_LS_FILE_REQUEST:
				handlefile(pdu);
				break;	
			case ENUM_MSG_TYPE_DOWN_REQUEST:
				handledown(pdu);
				printf("下载完成\n");
				break;
			default:
				break;
		}
		free(pdu);
	}
}

int makeSocket()
{
	//AF_INET:ipv4
	//SOCK_STREAM:使用可靠传输-->tcp
	//SOCK_DGRAM:非可靠传输-->udp
	//0: 使用传输默认的协议
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

void connectToServer(int sockfd)
{
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	//htons:表示将主机字节序转换为网络字节序
	//字节序：大端字节序，小端字节序
	//端口用来标识应用
	servAddr.sin_port = htons(8888);
	//设置要连接的服务器的ip地址
	//inet_addr:将点分十进制表示的ip转换成整数表示
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//将指定地址的开始往后的多少个字节置为0
	bzero(servAddr.sin_zero, 8);

	int ret = -1;
	//连接服务器
	ret = connect(sockfd
				  , (struct sockaddr *)&servAddr
	              , sizeof(servAddr));
	if (-1 == ret)
	{
		perror("connect");
		exit(EXIT_FAILURE);
	}
	printf("成功连接服务器\n");
}

int loginOrRegistFace()
{
	printf("  欢迎\n");
	printf("1,登录\n");
	printf("2,注册\n");
	printf("0,退出\n");
	printf("请输入选项:\n");
	int num = 0;
	scanf("%d", &num);
	return num;
}
 
static void regist(int sockfd)
{
	PDU *pdu = makePDU(PWD_LEN);
	pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
	printf("请输入注册需要的密码：\n");
	scanf("%s", pdu->caMsg);
	sendPDU(sockfd, pdu);
	free(pdu);

	pdu = recvPDU(sockfd);
    if (ENUM_MSG_TYPE_REGIST_RESPOND
         == pdu->uiMsgType)
	{
		g_uiId = pdu->uiTo;
		printf("获得注册的id: %u\n", g_uiId);
	}
	else
	{
		printf("注册失败\n");
	}
	free(pdu);
}

static int login(int sockfd)
{
	int id = 0;
	PDU *pdu = makePDU(PWD_LEN);
	printf("请输入id:\n");
	scanf("%u", &pdu->uiFrom);
	id = pdu->uiFrom;
	printf("请输入密码:\n");
	scanf("%s", pdu->caMsg);
	pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
	sendPDU(sockfd, pdu);
	free(pdu);

	pdu = recvPDU(sockfd);
	if (ENUM_MSG_TYPE_LOGIN_RESPOND 
	    && 0 == strncmp(LOGIN_OK, pdu->caMsg
				        , pdu->uiMsgLen))
	{
		printf("登录成功\n");
		g_uiId = id;
		return 1;
	}
	printf("登录失败\n");
	write(STDOUT_FILENO, pdu->caMsg, pdu->uiMsgLen);
	putchar('\n');

	return -1;
}

void exitPrograms(int sockfd)
{
	PDU *pdu = makePDU(0);
	pdu->uiFrom = g_uiId;
	pdu->uiMsgType = ENUM_MSG_TYPE_EXIT_REQUEST;
	sendPDU(sockfd, pdu);
	free(pdu);
	printf("发送退出请求\n");	
}

void loginOrRegist(int sockfd)
{
	int num = -1;
	int ret = -1;
	while (1)
	{
		num = loginOrRegistFace();
		switch (num)
		{
		case 1:
			ret = login(sockfd);
			break;
		case 2:
			regist(sockfd);
			break;
		case 0:
			exitPrograms(sockfd);
			exit(EXIT_SUCCESS);
		default:
			printf("输入有误!!!\n");
			break;
		}
		if (1 == ret)
		{
			break;
		}
	}
}

static int chatFace()
{
	printf("-------------\n");
	printf("1:私聊\n");
	printf("2:群聊\n");
	printf("3:群文件\n");
	printf("4:下载文件\n");
	printf("5:上传文件\n");
	printf("6:重命名文件\n");
	printf("7:删除文件\n");
	printf("0:返回\n");
	printf("请输入选项:\n");
	int num = 0;
	scanf("%d", &num);
	return num;
}
static void download(int sockfd)
{
	PDU *pdu = makePDU(sizeof(PDU));
	printf("请输入下载的文件名:\n");
	scanf("%s",pdu->caname);

	pdu->uiFrom = g_uiId;
	pdu->uiMsgType = ENUM_MSG_TYPE_DOWN_REQUEST;
	sendPDU(sockfd, pdu);
	free(pdu);
}
static void privateChat(int sockfd)
{
	printf("请输入聊天的对象:\n");
	uint perid = 0;
	scanf("%u", &perid);

	char caMsg[PER_MAX_IO_BYTES] = {'\0'};
	printf("请输入聊天信息:\n");
	read(STDIN_FILENO, caMsg
	     , PER_MAX_IO_BYTES);
	PDU *pdu = makePDU(strlen(caMsg));
	pdu->uiFrom = g_uiId;
	pdu->uiTo = perid;
	strncpy(pdu->caMsg, caMsg, strlen(caMsg));
	pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
	sendPDU(sockfd, pdu);
	free(pdu);
}

static void groupChat(int sockfd)
{
	char caMsg[PER_MAX_IO_BYTES] = {'\0'};
	printf("请输入聊天信息:\n");
	read(STDIN_FILENO, caMsg
	     , PER_MAX_IO_BYTES);
	PDU *pdu = makePDU(strlen(caMsg));
	pdu->uiFrom = g_uiId;
	strncpy(pdu->caMsg, caMsg, strlen(caMsg));
	pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
	sendPDU(sockfd, pdu);
	free(pdu);
}
void myFile(int sockfd)
{
	PDU *pdu = makePDU(1);
	pdu->uiFrom = g_uiId;
	pdu->uiMsgType = ENUM_MSG_TYPE_LS_FILE_REQUEST;
	sendPDU(sockfd, pdu);
	free(pdu);
}
void sendfile(int sockfd)
{
	char caname[64] = {'\0'};
	printf("请输入文件名\n");
	scanf("%s",caname);
	while(getchar() != '\n');
	
	
	int fd = -1;
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
	pdu->uiMsgType = ENUM_MSG_TYPE_SEND_REQUEST;
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
	strcpy(pdu->caname,caname);
	pdu->filesize = len;
	strncpy(pdu->caMsg,buf,strlen(buf));
	sendPDU(sockfd,pdu);
	close(fd);
}
void renafile(int sockfd)
{	
	char caname[64] = {'\0'};
	char newname[64] = {'\0'};
	printf("请输入文件名\n");
	scanf("%s",caname);
	while(getchar() != '\n');
	
	printf("请输入新名字\n");
	scanf("%s",newname);
	while(getchar() != '\n');
	
	
	PDU *pdu = makePDU(sizeof(newname));
	pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_REQUEST;
	strncpy(pdu->caname,caname,strlen(caname));		
	strncpy(pdu->caMsg,newname,strlen(newname));	
	sendPDU(sockfd,pdu);
}
void remvfile(int sockfd)
{
	char caname[64] = {'\0'};
	printf("请输入文件名\n");
	scanf("%s",caname);
	while(getchar() != '\n');
	
	PDU *pdu = makePDU(1);
	pdu->uiMsgType = ENUM_MSG_TYPE_REMV_REQUEST;
	strncpy(pdu->caname,caname,strlen(caname));	
	sendPDU(sockfd,pdu);
}
void chat(int sockfd)
{
	pthread_t thread;
	pthread_create(&thread, NULL, handleServer
				   , (void *)sockfd);

	int num = -1;
	while (1)
	{
		num = chatFace();
		switch (num)
		{
		case 1:
			privateChat(sockfd);
			break;
		case 2:
			groupChat(sockfd);
			break;
		case 3:
			myFile(sockfd);
			break;
		case 4:
			download(sockfd);
			break;
		case 5:
			sendfile(sockfd);
			break;
		case 6:
			renafile(sockfd);
			break;
		case 7:
			remvfile(sockfd);
			break;
		case 0:
			pthread_cancel(thread);
			return;
		default:
			break;
		}
	}
}




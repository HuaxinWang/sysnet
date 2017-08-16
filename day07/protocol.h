#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "public.h"

#define REGIST_OK       "regist ok"
#define REGIST_FAILED   "regist failed"

#define LOGIN_OK        "login ok"
#define LOGIN_FAILED    "login failed"

enum ENUM_MSG_TYPE
{
	ENUM_MSG_TYPE_MIN = 0,
	ENUM_MSG_TYPE_REGIST_REQUEST,       //注册请求
	ENUM_MSG_TYPE_REGIST_RESPOND,       //注册回复
	ENUM_MSG_TYPE_LOGIN_REQUEST,        //登录请求
	ENUM_MSG_TYPE_LOGIN_RESPOND,        //登录回复
	ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,
	ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,
	ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,
	ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,
	ENUM_MSG_TYPE_EXIT_REQUEST,
	ENUM_MSG_TYPE_EXIT_RESPOND,
	ENUM_MSG_TYPE_MAX = 0x00ffffff
};

typedef struct PDU       //PDU 通信协议数据单元
{
	uint uiPDULen;       //发送数据的总大小
	uint uiMsgType;      //消息类型
	uint uiFrom;         //谁发的
	uint uiTo;           //发给谁
	uint uiMsgLen;       //实际消息的大小
	char caMsg[4];       //实际消息
}PDU;

//申请存储空间用来存放发送或者接收的数据
PDU *makePDU(uint uiMsgLen);
//接收数据
PDU *readPDU(int sockfd);
//发送数据
void writePDU(int sockfd, PDU *pdu);


#endif

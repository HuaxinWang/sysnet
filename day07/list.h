#ifndef LIST_H
#define LIST_H

#include "public.h"

typedef struct UsrInfo
{
	int sockfd;
	uint uiId;
	char caPwd[PWD_LEN];
	struct UsrInfo *pNext;
}UsrInfo;

typedef struct UsrList
{
	int iLen;             //链表的长度
	UsrInfo *pFirstNode;  //指向第一个节点
}UsrList;
//创建节点
UsrInfo *makeUsrInfoNode();
//创建链表
UsrList *makeUsrList();
//将节点插入链表
void insertUsrList(UsrList *list, UsrInfo *node);
//显示节点
void showUsrInfoNode(const UsrInfo *node);
//显示链表
void showUsrList(const UsrList *list);
//根据id查找信息
UsrInfo *findUsrLisById(const UsrList *list, uint id);

//根据id和密码查找用户信息
UsrInfo *findUsrLisByIdAndPasswd(const UsrList *list
				    , uint id, char caPwd[PWD_LEN]);

#endif

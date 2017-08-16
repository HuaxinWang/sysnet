#include "protocol.h"

//申请存储空间用来存放发送或者接收的数据
PDU *makePDU(uint uiMsgLen)
{
	//获得要申请空间的总大小
	uint uiPDULen = sizeof(PDU)-4*sizeof(char)
			        +uiMsgLen;
	PDU *pdu = (PDU *)malloc(uiPDULen);
	if (NULL == pdu)
	{
		printf("malloc pdu failed\n");
		exit(EXIT_FAILURE);
	}
	memset(pdu, 0, uiPDULen);
	pdu->uiPDULen = uiPDULen;
	pdu->uiMsgLen = uiMsgLen;
	return pdu;
}

//接收数据
PDU *readPDU(int sockfd)
{
	int ret = -1;
	uint uiPDULen = 0;
	//先接收首部的四字节的数据用来获得数据的总大小
	ret = read(sockfd, &uiPDULen, sizeof(uint));
	if (-1 == ret || 0 == ret)
	{
		return NULL;
	}
	uint uiMsgLen = uiPDULen - (sizeof(PDU)-4*sizeof(char));
	PDU *pdu = makePDU(uiMsgLen);
	//未接收的数据大小
	uint uiLeft = uiPDULen - sizeof(uint);
	//已接收的数据大小
	uint uiRecved = sizeof(uint);
	while (uiLeft)
	{
		if (uiLeft > 4096)
		{
			ret = read(sockfd, (char*)pdu+uiRecved
					   , 4096);
		}
		else
		{
			ret = read(sockfd, (char*)pdu+uiRecved
					   , uiLeft);
		}
		if (-1 == ret || 0 == ret)
		{
			break;
		}
		uiLeft -= ret;
		uiRecved += ret;
	}

	return pdu;
}

//发送数据
void writePDU(int sockfd, PDU *pdu)
{
	if (NULL == pdu)
	{
		return;
	}
	uint uiLeft = pdu->uiPDULen;
	uint uiWrited = 0;
	int ret = -1;
	while (uiLeft)
	{
		if (uiLeft > 4096)
		{
			ret = write(sockfd, (char*)pdu+uiWrited
						, 4096);
		}
		else
		{
			ret = write(sockfd, (char*)pdu+uiWrited
					    , uiLeft);
		}
		if (-1 == ret)
		{
			perror("write pdu");
			break;
		}
		uiLeft -= ret;
		uiWrited += ret;
	}
}









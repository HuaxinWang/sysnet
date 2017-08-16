#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct Test
{
	int id;
	char caMsg[4];
}Test;

int main(void)
{
	Test test;
	test.id = 10012;
//	strcpy(test.caMsg, "234567890csaca");

	Test *p = (Test *)malloc(128);
#if 0
	test
	{
		int id; 
		char caMsg[124];
	}
#endif
	p->id = 10012;
	strcpy(p->caMsg, "hello world 大家好");
	printf("%s\n", p->caMsg);
	

	p = (Test *)malloc(1024);
#if 0
	test
	{
		int id;
		char caMsg[1020];
	}
#endif
	p->id = 909090;
	strcpy(p->caMsg, "AAAAAAAAAAAAAAAAAAAAAAAAAA");
	printf("%s\n", p->caMsg);

	return 0;
}

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

void error_quit(const char *msg)
{
	perror(msg);
	exit(-1);
}

void change_path(const char *path)
{
	printf("Leave %s Successed . . .\n",getcwd(NULL,0));

	if(chdir(path)==-1)
	error_quit("chdir");

	printf("Entry %s Successed . . .\n",getcwd(NULL,0));
}

void _copy_file(const char *old_path,const char *new_path)
{
	FILE *in,*out;
	size_t len;
	char buf[64];
	char *p=getcwd(NULL,0);

	if((in=fopen(old_path,"rb"))==NULL)
		error_quit(old_path);

	change_path(new_path);

	if((out=fopen(old_path,"wb"))==NULL)
		error_quit(old_path);

	while(!feof(in))
	{
		bzero(buf,sizeof(buf));

		len=fread(&buf,1,sizeof(buf)-1,in);
		fwrite(&buf,len,1,out);
	}

	fclose(in);
	fclose(out);

	change_path(p);
}

char *get_rel_path(const char *dir,const char *path)
{
	char *rel_path;
	unsigned long d_len,p_len;

	d_len=strlen(dir);
	p_len=strlen(path);
	if((rel_path=malloc(d_len+p_len+2))==NULL)
		error_quit("malloc");
	bzero(rel_path,d_len+p_len+2);

	strncpy(rel_path,path,p_len);
	strncat(rel_path,"/",sizeof(char));
	strncat(rel_path,dir,d_len);

	return rel_path;
}

void copy_dir(const char *old_path,const char *new_path)
{
	DIR *dir;
	struct stat buf;
	struct dirent *dirp;
	char *p=getcwd(NULL,0);

	if((dir=opendir(old_path))==NULL)
		error_quit(old_path);
	if(mkdir(new_path,0777)==-1)
		error_quit(new_path);

	change_path(old_path);

	while((dirp=readdir(dir)))
	{
		if(strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0)
		continue;
		if(stat(dirp->d_name,&buf)==-1)
			error_quit("stat");
		if(S_ISDIR(buf.st_mode))
		{
			copy_dir(dirp->d_name,get_rel_path(dirp->d_name,new_path));
			continue;
		}
		_copy_file(dirp->d_name,new_path);
	}

	closedir(dir);
	change_path(p);
}

int main(int argc,char **argv)
{
	copy_dir(argv[1],argv[2]);

	return 0;
}

#ifndef FILE_H
#define FILE_H

#include "public.h"
#include "list.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define USR_INFO_FILE "./usr.info"

int openFile(const char *pathname, int flags);
void saveUsrListToFile(int fd, const UsrList *list);
void getUsrInfoFromFile(int fd, UsrList *list);

#endif

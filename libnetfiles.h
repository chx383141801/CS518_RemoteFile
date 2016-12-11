#ifndef LIB_NET_FILES_H
#define LIB_NET_FILES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "request.h"

#define PORT_NUM 9000

/* Permission stuff */
#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR   3

/* Mode stuff */
#define UNRESTRICT  0
#define EXCLUSIVE   1
#define TRANSACTION 3

#define BUFFER_SIZE 2048

int netopen(char *pathname, int flags);
ssize_t netread(int fd, void *buf, size_t nbyte);
ssize_t netwrite(int fd, void *buf, size_t nbyte);
int netclose(int fd);
int netserverinit(char *hostname, int filemode);

#endif

#ifndef NET_FILES_SERVER_H
#define NET_FILES_SERVER_H

/* C library stuff */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <linux/limits.h>

/* C network stuff */
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

/* Utility stuff */
#include "request.h"
#include "my_queue.h"

/* Micro define */
#define PORT_NUM 9000
#define MAX_DATA_SIZE 2048
#define BUFFER_SIZE 2048
#define MAX_FD_NUM 2048
#define MAX_NAME_LEN 256

/* @Description: fd_list contains the info of 
 *  file descriptor. You can check the following
 *  name to understand the meaning. File server 
 *  get the fd from client and check the fd_list
 *  to get the info such as permission of that fd
 * */
typedef struct fd_info 
{
	char filename[256];	//the file name which this fd represents
	int permission;		//O_RDONLY or O_WRONLY or O_RDWR of this fd
	int isInuse;		//If this fd is in use (reduce the check time from queue)
	int mode;			//Unrestricted or Exclusive or Transaction
	//TODO: add more such as timestamp
}fd_info;
#define FD_INFO_SIZE sizeof(fd_info)

// container of file descriptors;
// in this assignment, we only have 2048 fd
fd_info fd_list[2048];

/* @Description: This is a linked list which 
 *  store the info of a file. The struct file_info
 *  record the current status of given file name.
 *  The next pointer points to the next file_info
 *  block.
 * */
typedef struct file_info
{
	struct file_info *next;
	char filename[MAX_NAME_LEN];
	int mode;					// open mode
	int cur_permission;			// current file open permission
	//TODO: add more members?
	int isOpen;					// 0 means not open; >0 means open and the number 
								// reqpresents the current number of users who is
								// opening this file
}file_info;

// head pointer of the linked list (only pointer, no other content)
file_info *file_info_front;

#endif

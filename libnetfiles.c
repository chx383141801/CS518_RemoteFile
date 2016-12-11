#include "libnetfiles.h"

/* Global variables */
int isInit = 0;					// mark the initialization status
int portnum = PORT_NUM;
int mode = 0;
char *servername = NULL;		

/********** Instrumental function part begin ***********/

/* @Description: create a connection
 * @return: sockedt fd on success; -1 on failed and errno is set
 * */
int getconnection()
{
	struct sockaddr_in server_addr;
	int sockfd;

	// Create a socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return -1;
	}

	// Initialize sockaddr_in
	memset((void*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnum);
	server_addr.sin_addr.s_addr = inet_addr(servername);

	// Get connection
	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
	{
		perror("connection");
		return -1;
	}
	return sockfd;
}

/********** Instrumental function part end ************/

/* @Description: check host name and file mode; then set server
 *  name and isInit
 * @return: 0 on success; -1 on failed and herror is set 
 * */
int netserverinit(char *hostname, int filemode)
{
	if (isInit == 1)
		return 0;

	struct in_addr **addr_list;
	struct hostent *he = gethostbyname(hostname);

	if (he == NULL)
	{
		herror("gethostbyname");
		return -1;
	}

	if (filemode < 0 || filemode > 3 || filemode == 2)
	{
		printf("File mode: INVALID_FILE_MODE\n");
		return -1;
	}

	addr_list = (struct in_addr**)he->h_addr_list;
	servername = inet_ntoa(*addr_list[0]);
	mode = filemode;

	isInit = 1;
	return 0;
}

/* @Description: send request packet to server to ask for opening a file
 * @return: fd is returned on success; -1 on failed and errno is set
 * */
int netopen(char* pathname, int flags)
{
	int sockfd;
	char send_buffer[BUFFER_SIZE];
	char recv_buffer[BUFFER_SIZE];

	//TODO: set errno
	if (isInit == 0 || (sockfd = getconnection()) == -1)
		return -1;

	req_t *req = (req_t*)malloc(REQ_SIZE);
	req->req_type = 1;
	req->fd = -1;
	req->mode = mode;
	memcpy(req->pathname, pathname, strlen(pathname)+1);

	// by default we set permission to O_RDWR
	if (flags < 1 || flags > 3)
		req->permission = 3;
	else
		req->permission = flags;

	// copy request packet to send buffer
	memcpy(send_buffer, req, REQ_SIZE);

	// send data through socket
	int byte_sent;
	if ((byte_sent = send(sockfd, send_buffer, REQ_SIZE, 0)) == -1)
	{
		perror("send");
		free(req);
		close(sockfd);
		return -1;
	}

	// receive the reply from server
	int byte_recv;
	if ((byte_recv = recv(sockfd, recv_buffer, REQ_SIZE, 0)) == -1)
	{
		perror("receive");
		free(req);
		close(sockfd);
		return -1;
	}

	memcpy(req, recv_buffer, REQ_SIZE);
	if (req->fd == -1)
	{
		errno = req->errnum;
		perror("open");
		free(req);
		close(sockfd);
		return req->fd;
	}
	else
	{
		free(req);
		close(sockfd);
		return req->fd;
	}

}

/* Solution: 
 * 1) send request message and wait for reply
 * 2) if we receive confirm msg from server then start to send data
 * 3) close socket
 * */
ssize_t netwrite(int fd, void *buf, size_t nbyte)
{
}

/* Solution:
 * 1) send request message and wait for reply
 * 2) if we confirm msg received, then continue to receive data
 * 3) close socket
 * */
ssize_t netread(int fd, void *buf, size_t nbyte)
{
}

/* @Description: send close file request
 * @return: 0 on success; -1 on error and errno is set
 * */
int netclose(int fd)
{
	int sockfd;
	char send_buffer[BUFFER_SIZE];
	char recv_buffer[BUFFER_SIZE];

	//TODO: set errno
	if (isInit == 0 || (sockfd = getconnection()) == -1)
		return -1;

	req_t *req = (req_t*)malloc(REQ_SIZE);
	req->req_type = 8;
	req->fd = fd;

	// copy request packet to send buffer
	memcpy(send_buffer, req, REQ_SIZE);

	// send data through socket
	int byte_sent;
	if ((byte_sent = send(sockfd, send_buffer, REQ_SIZE, 0)) == -1)
	{
		perror("send");
		free(req);
		close(sockfd);
		return -1;
	}

	// receive the reply from server
	int byte_recv;
	if ((byte_recv = recv(sockfd, recv_buffer, REQ_SIZE, 0)) == -1)
	{
		perror("receive");
		free(req);
		close(sockfd);
		return -1;
	}

	memcpy(req, recv_buffer, REQ_SIZE);
	if (req->errnum != 0)
	{
		errno = req->errnum;
		perror("close");
		free(req);
		close(sockfd);
		return -1;
	}
	else
	{
		free(req);
		close(sockfd);
		return 0;
	}

}






























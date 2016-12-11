#include "netfileserver.h"

////////////////////////////////////////////////////////////

/* Initialize fd info list */
void init_info_list()
{
	for (int i = 0 ; i < MAX_FD_NUM ; i++)
	{
		memset(fd_list[i].filename, 0, MAX_NAME_LEN);
		fd_list[i].permission = -1;
		fd_list[i].isInuse = 0;
		fd_list[i].mode = 0;
	}
}

////////////////////////////////////////////////////////////

/* process open deny for open function use */
void open_deny(req_t *rply)
{
	rply->fd = -1;
	rply->errnum = EACCES;
}

////////////////////////////////////////////////////////////

/* process open allow for open function use */
void open_allow(req_t *req, req_t *rply, file_info *ptr)
{
	int fd;
	switch (req->permission)
	{
		case 1: fd = open(req->pathname, O_RDONLY);
				break;
		case 2: fd = open(req->pathname, O_WRONLY);
				break;
		case 3: fd = open(req->pathname, O_RDWR);
				break;
	}
	if (fd != -1)
	{
		//TODO: not handled if queue_size <= 0
		if (queue_size() > 0)
			rply->fd = dequeue();
		fd_list[rply->fd].permission = req->permission;
		memcpy(fd_list[rply->fd].filename, req->pathname, MAX_NAME_LEN);
		fd_list[rply->fd].isInuse = 1;
		fd_list[rply->fd].mode = req->mode;
		//TODO: if need to add timestamp, please remember to add here

		// update file_info list
		ptr->isOpen++;
		ptr->cur_permission = ptr->cur_permission > req->permission ? ptr->cur_permission : req->permission;	
	}
	else
	{
		rply->fd = -1;
		rply->errnum = errno;
	}
	close(fd);
}

////////////////////////////////////////////////////////////

//TODO: check correctness 
void process_open(int sockfd, req_t *req)
{
	char send_buffer[BUFFER_SIZE];
	req_t *rply = (req_t*)malloc(REQ_SIZE);

	// Initialize errnum to -1
	rply->errnum = -1;
	// First check status
	int isExist = 0;
	file_info *ptr = file_info_front;
	while (ptr->next != NULL)
	{
		ptr = ptr->next;
		if (strcmp(req->pathname, ptr->filename) == 0)
		{
			isExist = 1;
			break;
		}
	}

	// File exist and it's open
	if (isExist == 1 && ptr->isOpen > 0)
	{

		if (ptr->mode == 3 || req->mode == 3)
			open_deny(rply);
		else if (ptr->mode == 2 && ptr->cur_permission != O_RDONLY)
			open_deny(rply);
		else if (ptr->mode == 2 && ptr->cur_permission == O_RDONLY)
			open_allow(req, rply, ptr);
		else if (ptr->mode == 0 && ptr->cur_permission != O_RDONLY)
		{
			if (req->mode == 2)
				open_deny(rply);
			else
				open_allow(req, rply, ptr);
		}
		else if (ptr->mode == 0 && ptr->cur_permission == O_RDONLY)
			open_allow(req, rply, ptr);
		else 
		{
			printf("ERROR: Unexpected case happens!\n");
			exit(-2);
		}
	}
	// File is not open or not exist
	else if ((isExist == 1 && ptr->isOpen == 0) || isExist == 0)
	{
		int fd;
		switch (req->permission)
		{
			case 1: fd = open(req->pathname, O_RDONLY);
					break;
			case 2: fd = open(req->pathname, O_WRONLY);
					break;
			case 3: fd = open(req->pathname, O_RDWR);
					break;
		}
		if (fd != -1)
		{
			//TODO: not handled if queue_size <= 0
			if (queue_size() > 0)
				rply->fd = dequeue();
			fd_list[rply->fd].permission = req->permission;
			memcpy(fd_list[rply->fd].filename, req->pathname, MAX_NAME_LEN);
			fd_list[rply->fd].isInuse = 1;
			fd_list[rply->fd].mode = req->mode;
			//TODO: if need to add timestamp, please remember to add here

			// Modify file_info list (Initialize the file_info list for the new file)
			if (isExist == 0)
			{
				file_info *newfptr = (file_info*)malloc(sizeof(file_info));
				memcpy(newfptr->filename, req->pathname, MAX_NAME_LEN);
				newfptr->isOpen = 1;
				newfptr->mode = req->mode;
				newfptr->cur_permission = req->permission;
				newfptr->next = ptr->next;
				ptr->next = newfptr;
			}
			else
			{
				ptr->isOpen++;
				ptr->mode = req->mode;
				ptr->cur_permission = req->permission;
				memcpy(ptr->filename, req->pathname, MAX_NAME_LEN);
			}
		}
		else
		{
			rply->fd = -1;
			rply->errnum = errno;
		}
		close(fd);
	}

	memcpy(send_buffer, rply, REQ_SIZE);
	free(rply);
	int send_byte;
	if((send_byte = send(sockfd, send_buffer, BUFFER_SIZE, 0)) == -1)
		perror("send");

}

////////////////////////////////////////////////////////////

//TODO
/* Extension B: multi-thread read */
void process_read()
{
}

////////////////////////////////////////////////////////////

//TODO
/* Extension B: multi-thread write */
void process_write()
{
}

////////////////////////////////////////////////////////////

/* @Description: close file; If no error, the reply message will have
 *  errnum 0; if has error, the reply message has correponding errno
 * */
void process_close(int sockfd, req_t *req)
{
	char send_buffer[BUFFER_SIZE];
	req_t *rply = (req_t*)malloc(REQ_SIZE);

	// Initialize errnum to 0
	rply->errnum = 0;
	// First, check if it is exist
	int isExist = 0;
	file_info *ptr = file_info_front;
	if (fd_list[req->fd].isInuse != 0)
	{
		while (ptr->next != NULL)
		{
			ptr = ptr->next;
			if (strcmp(fd_list[req->fd].filename, ptr->filename) == 0)
			{
				isExist = 1;
				break;
			}
		}
	}
	// check if file exist or the fd is correct
	if (isExist == 0 || fd_list[req->fd].isInuse == 0)
		rply->errnum = EBADF;
	else
	{
		// clear file info list
		ptr->isOpen--;

		//DEBUG
		if (ptr->isOpen < 0)
			printf("ERROR: isopen < 0 when close file!\n");

		ptr->mode = 0;
		ptr->cur_permission = 0;
		// update the mode and cur_permission (bad part)
		if (ptr->isOpen > 0)
		{
			for (int i = 0 ; i < MAX_FD_NUM ; i++)
			{
				if (strcmp(ptr->filename, fd_list[i].filename) == 0)
				{
					ptr->mode = ptr->mode > fd_list[i].mode ? 
						ptr->mode : fd_list[i].mode;
					ptr->cur_permission = 
						ptr->cur_permission > fd_list[i].permission ? 
						ptr->cur_permission : fd_list[i].permission;
				}
			}
		}
		// enqueue the fd
		enqueue(req->fd);
		// clear fd list
		memset(fd_list[req->fd].filename, 0, MAX_NAME_LEN);
		fd_list[req->fd].permission = -1;
		fd_list[req->fd].isInuse = 0;
	}
	
	memcpy(send_buffer, rply, REQ_SIZE);
	free(rply);
	int send_byte;
	if ((send_byte = send(sockfd, send_buffer, BUFFER_SIZE, 0)) == -1)
		perror("send");
}

////////////////////////////////////////////////////////////

/* @Description: this function is the management function, it
 *  decides which function to call to process data
 * */
void *recv_data(void *fd)
{
	int client_sockfd;
	int rcv_byte, send_byte;
	char recv_buffer[BUFFER_SIZE];
	req_t *req = (req_t*)malloc(REQ_SIZE);

	client_sockfd = *((int*)fd);
	
	// receive data
	if ((rcv_byte = recv(client_sockfd, recv_buffer, REQ_SIZE , 0)) == -1)
	{
		perror("receive");
		exit(errno);
	}

	memcpy(req, recv_buffer, REQ_SIZE);

	switch (req->req_type)
	{
		case 1: process_open(client_sockfd, req);
				break;

		//TODO: update the function read here
		case 2: process_read();
				break;

		//TODO: update the function write here
		case 4: process_write();
				break;
		case 8: process_close(client_sockfd, req);
	}

	free(req);
	free(fd);
	close(client_sockfd);
	pthread_exit(NULL);
}

/////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	struct sockaddr_in server_addr, client_addr;
	int portnum;
	int socketfd;
	socklen_t cliaddr_len;
	pid_t pid;
	char *client_ip_addr;

	cliaddr_len = sizeof(client_addr);
	//TODO: Left space for arguments check
	//TODO: Need to decide if we fix the potnum to 9000
	
	//Get port number from input
	portnum = atoi(argv[1]);
	if (portnum < 8000 || portnum > 65535)
	{
		printf("Please choose a port in the range: 8000-65535\n");
		return 1;
	}

	//Initialize socket
	if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Failed to initialize socket\n");
		exit(errno);
	}

	memset((void*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(portnum);

	//Bind socket 
	if (bind(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0)
	{
		perror("bind error");
		exit(errno);
	}
	
	//Listen for connections, queue size is 10 
	listen(socketfd, 10);

	// Initialize queue and file_info list
	init_queue();
	init_info_list();
	file_info_front = (file_info*)malloc(sizeof(file_info));
	file_info_front->next = NULL;

	while(1)
	{
		int *client_sockfd = (int *)malloc(sizeof(int));

		//accept the connection
		if ((*client_sockfd = accept(socketfd, (struct sockaddr*)&client_addr, &cliaddr_len)) == -1)
		{
			perror("Failed to accept connections on socket\n");
			exit(errno);
		}

		client_ip_addr = inet_ntoa(client_addr.sin_addr);
		printf("[+] Client conneted from IP address: %s\n", client_ip_addr);
	
		// Create multi-thread to accept multi-client connections
		pthread_t thread_id;

		if (pthread_create(&thread_id, NULL, recv_data, client_sockfd) != 0)
		{
			perror("Fail to create thread");
			close(*client_sockfd);
			free(client_sockfd);
			break;
		}
	}
	close(socketfd);
	return 0;

}

#ifndef REQUEST_H
#define REQUEST_H

typedef struct request_packet
{
	int req_type;		// request types: 1 open; 2 read; 4 write; 8 close
	int fd;				// file descriptor
	int permission;		// O_RDONLY, O_WRONLY, O_RDWR (1,2,3)
	int mode;			// <RW>: 0 unrestrict (00); 1 exclusive (01); 3 transaction (11)
	int index;			// record the index of the packet
	int is_last;		// mark if this is the last packet
	int errnum;			// to set errno
	char pathname[256];	// path/name of file
}req_t;
#define REQ_SIZE sizeof(req_t)

#endif

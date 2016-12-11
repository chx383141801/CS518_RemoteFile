#include "libnetfiles.h"

int main()
{
	netserverinit("localhost", UNRESTRICT);
	int fd;
	if ((fd = netopen("temp.txt", O_RDONLY)) == -1)
	{
		printf("Failed\n");
	}
	printf("fd: %d\n", fd);

	int ret;
	if((ret = netclose(fd)) == -1)
		printf("close failed\n");

	if(ret == 0)
		printf("close success\n");
	return 0;
}

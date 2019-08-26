#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>
#define MAGICALNUMBER 243
#define CHAT_CONFIG_STT _IOW(MAGICALNUMBER, 1,char *)
#define CHAT_CONFIG_USERNAME _IOW(MAGICALNUMBER, 2,char *)
#define CHAT_CONFIG_PID _IOW(MAGICALNUMBER, 3,char *)

int main(int argc, char *argv[])
{
	int fd;
	long value, number;
	char *input;
	printf("Opening driver\n");
	fd = open("/proc/Chat_Add_Pid", O_RDWR);
	if(fd<0)
	{
		printf("Cannot open device file...n");
		return 0;
	}
	char *option = argv[1];
	int sw = atoi(option);
	input = argv[2];
	switch(sw)
	{
		case 1:
			printf("Writing the new pid to driver \n");
			ioctl(fd,CHAT_CONFIG_PID, (char *) input);
			printf("Closing driver\n");
			break;
		case 2:
			printf("Writing the new username to driver\n");
			ioctl(fd,CHAT_CONFIG_USERNAME,(char*) input);
			printf("closing driver\n");
			break;
		case 3:
			printf("Writing the new status to driver \n");
			ioctl(fd,CHAT_CONFIG_STT, (char *) input);
			printf("Closing driver\n");
			break;
		default:
			break;
	}
	close(fd);
	return 0;
}

#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

struct sockaddr_in sockaddr1;
int sockfd;
char ipaddr[15];

#define port 3333

void clink()
{
	/*1. create socket*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	/*2. init addr*/
	memset(&sockaddr1, 0, sizeof(sockaddr1));
	sockaddr1.sin_family = AF_INET;
	sockaddr1.sin_addr.s_addr = inet_addr(ipaddr);
	sockaddr1.sin_port = htons(port);

	/*3. connect to server*/
	connect(sockfd, (struct sockaddr *)&sockaddr1, sizeof(sockaddr1));
}

void upload_file(char *filename)
{
	char cmd = 'U';
	int size = strlen(filename);
	struct stat fstat;
	int fd;
	char buf[1024];
	int count;

	fd = open(filename, O_RDONLY);

	//send operationtype
	write(sockfd, &cmd, 1);

	//send file name
	write(sockfd, &size, 4 );
	write(sockfd, filename, size);

	//send file length
	stat(filename, &fstat);
	write(sockfd, (void *)(fstat.st_size), 4);

	//send file content
	while((count = read(fd, (void *)buf, 1024)) > 0){
		write(sockfd, &buf, count);	
	}
	
	close(fd);
}

void download_file(char *filename)
{
	char cmd = 'U';
	int size = strlen(filename);
	int fd;
	char buf[1024];
	int count = 0;
	int filesize;
	int tmpsize;
	//1. send operationtype
	write(sockfd, (void *)&cmd, 1);

	//2. send file name
	write(sockfd, &size, 4);
	write(sockfd, filename, size);
	
	//2.1 create file
	open(filename, O_RDWR|O_CREAT, 0777);

	//3. receive file length
	read(sockfd,&filesize, 4);
	
	printf("filesize %d\r\n", filesize);
	//4. receive file content
	while ((count = read(sockfd, (void *)buf, 1024)) > 0){
		write(fd, &buf, count);
		tmpsize += count;

		printf("tmpsize %d\r\n", tmpsize);

		if (tmpsize == filesize){
			break;
		}

	close(fd);

		
	}	
}

void quit(){
	char cmd = 'Q';

	//1. send quit operationcode
	write(sockfd, &cmd, 1);
	//2. clear screen
	system("clear");

	//3. exit
	exit(0);
}
void menu(void)
{
	char command;
	char file_u[30];
	char file_d[30];
	char c;

	while (1)
	{
		printf("\n-------------------------1.Upload file-------------------------------");
		printf("\n-------------------------2.Download file-----------------------------");
		printf("\n-------------------------3.Exit--------------------------------------");
		printf("\n Please input the command:");

		command = getchar();

		switch (command){
			case '1':
				printf("please input file name:");
				while ((c = getchar()) != '\n' && c != EOF);
				fgets(file_u, 30, stdin);
				file_u[strlen(file_u)-1] = '\0';
				upload_file(file_u);
			//ipload
				break;
			case '2':
			//download
				printf("Please input file name:");
//				while((c = getchar()) != '\n' && c != EOF);
				fgets(file_d, 30, stdin);
				file_d[strlen(file_d) - 1] = '\0';
				download_file(file_d);
				break;
			case '3':
				//exit
				break;
			default:
				printf("Please input the right command\n");
				break;
		}
	}
}

int main(int argc, char **argv)
{
	if (argc != 2){
		printf("usage ./client xxx.xxx.xxx.xxx(xxx:0-255)");
		exit(0);
	}

	
	strcpy(ipaddr, argv[1]);
	/*1. Establish link*/
	clink();

	/*2.implement upload download and menu function*/
	menu();
	/*3.close link*/
	close(sockfd);

	return 0;
}

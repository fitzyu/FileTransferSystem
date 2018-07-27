#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h> 
#include <stdlib.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define port 3333
int  sockclient;
struct sockaddr_in sockaddr1;
char ipaddr[15];

SSL_CTX *ctx;
SSL *ssl;


int linkS() 
{	
	if((sockclient=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
	    perror("socket");	
	    exit(0);
	}
		
	memset(&sockaddr1,0,sizeof(sockaddr1));
	sockaddr1.sin_family = AF_INET;
	sockaddr1.sin_addr.s_addr = inet_addr(ipaddr);
	sockaddr1.sin_port = htons(port);
	
	if(connect(sockclient,(struct sockaddr* )&sockaddr1,sizeof(sockaddr1))==-1)
	{
	    perror("connect");
	    exit(0);
	}

	/*创建SSL*/
	/*申请一个SSL套接字*/
	ssl = SSL_new(ctx);
	/*绑定读写套接字*/
	SSL_set_fd(ssl, sockclient);
	/*完成握手*/
	SSL_connect(ssl);
	return 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~上传文件~~~~~~~~~~~~~~~~~~~~~~~~~
void upload_file(char *filename)
{	
	int fd;
	char buf[1024];
	int count=0;
	int size = strlen(filename);
	char cmd = 'U';

	struct stat fstat;
		
	if((fd=open(filename,O_RDONLY))==-1)
	{
		perror("open: ");
		return;
	}
	
	/*发送上传命令*/
	//write(sockclient,&cmd,1);
	SSL_write(ssl, &cmd, 1);
	/*发送文件名*/
	//write(sockclient,(void *)&size,4);
	//write(sockclient,filename,size);
	SSL_write(ssl,(void *)&size,4);
	SSL_write(ssl,filename,size);
	/*发送文件长度*/
	if(stat(filename,&fstat)==-1)
		return;
	
	//write(sockclient,(void *)&(fstat.st_size),4);
	SSL_write(ssl,(void *)&(fstat.st_size),4);
	/*发送文件内容*/
	while((count=read(fd,(void *)buf,1024))>0)
	{
		//write(sockclient,&buf,count);	
		SSL_write(ssl,&buf,count);	
	}		
	
	close(fd);

}
//~~~~~~~~~~~~~~~~~~~~~~~下载文件~~~~~~~~~~~~~~~~~~~~~~~~~

void download_file(char *filename)
{
	int fd;
	char buf[1024];
	int count=0;
	int filesize = 0;
	int tmpsize = 0;
	int namesize = 0;
	char cmd = 'D';
	
	int size = strlen(filename);
	
	/*发送下载命令*/
	//write(sockclient,(void *)&cmd,1);
	SSL_write(ssl,(void *)&cmd,1);
	
	/*发送文件名*/
	//write(sockclient,&size,4);
	//write(sockclient,filename,size);
	SSL_write(ssl,&size,4);
	SSL_write(ssl,filename,size);
	
	/*创建文件*/
	if((fd=open(filename,O_RDWR|O_CREAT,0777))<0)
	{
		perror("open error:\n");	
	}
	
	/*接收文件长度*/
	//read(sockclient,&filesize,4);	
	SSL_read(ssl,&filesize,4);
	
	while((count=SSL_read(ssl,(void *)buf,1024))>0)
	{
		write(fd,&buf,count);
		tmpsize += count;
		if(tmpsize==filesize)
			break;	

	}
	
	close(fd);	
}


void quit()
{
	char cmd = 'Q';
	
	//write(sockclient,(void *)&cmd,1);
	SSL_write(ssl,(void *)&cmd,1);
	system("clear");
	/*SSL退出*/
	/*关闭套接字*/
	SSL_shutdown(ssl);
	/*释放SSL套接字*/
    SSL_free(ssl);
    close(sockclient);
	/*释放SSL环境*/
    SSL_CTX_free(ctx);
				
	exit(0);	
}

void menu()
{
	char command;
	char file_u[30];
	char file_d[30];
	char tmp;
	char c;
	
	while(1)
	{
		printf("\n------------------------------  1.Upload Files  ------------------------------\n");
		printf("------------------------------  2.Download Files  ------------------------------\n");
		printf("------------------------------      3.Exit   ------------------------------------\n");
		printf("Please input the Client command:");	

		command=getchar();
		
		switch(command)
		{
			case '1':
			{
					printf("Upload File:");
					
					while ((c=getchar()) != '\n' && c != EOF);
					
					fgets(file_u,30,stdin);
					
					file_u[strlen(file_u)-1]='\0';

					upload_file(file_u);
		  	}
			break;
				
			case '2':
				{
					printf("Download Files:");
					
					while ((c=getchar()) != '\n' && c != EOF);
					
					fgets(file_d,sizeof(file_d),stdin);
					
					file_d[strlen(file_d)-1]='\0';
					
					download_file(file_d);
			  	}
				break;
				
			case '3':
				quit();
				
				break;
			
			default:
				printf("Please input right command\n");
				break;
		}
	}
}


int main(int argc,char *args[])
{
    if(argc!=2)
    {
	    printf("format error: you mast enter ipaddr like this : client 192.168.0.6\n");
	    exit(0);
    }
    
    strcpy(ipaddr,args[1]); 
    /*SSL库初始化*/
    SSL_library_init();
	/*加载加密算法函数和单向散列算法函数*/
    OpenSSL_add_all_algorithms();
	/*进行错误信息的初始化*/
    SSL_load_error_strings();
	/*SSL_CTX_new 申请一个会话环境 SSLv23_client_method 创建本次环境所使用的协议*/
    ctx = SSL_CTX_new(SSLv23_client_method());
	
    linkS();
    
    menu();
    
    //close(sockclient);
    
    return 0;
}

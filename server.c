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

char clientIP[15];									/*文件列表*/
int sockfd;                      
int new_fd;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
int sin_size,portnumber = 3333;

SSL_CTX *ctx;
SSL *ssl;

void handle(char cmd)
{
	char filename[100];

	int filesize = 0;
	int tmpsize = 0;
	int namesize = 0;
	int count=0;

	int fd;

	
	struct stat fstat;
	char buf[1024];

	switch(cmd)
	{	
		case 'U':
		{	
			
			/*接收文件名*/
			//read(new_fd,&namesize,4);
			//read(new_fd,(void *)filename,namesize);
			SSL_read(ssl,&namesize,4);
			SSL_read(ssl,(void *)filename,namesize);
			filename[namesize]='\0';
			
			/*创建文件*/
			if((fd=open(filename,O_RDWR|O_CREAT,0777))<0)
			{
				perror("open error:\n");	
			}
			
			/*接收文件大小*/
			//read(new_fd,&filesize,4);		
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
		break;
		
		case 'D':
		{	
			/* 接收文件名 */
			SSL_read(ssl, &namesize,4);
			SSL_read(ssl,filename,namesize);
			filename[namesize]='\0';
			
			if((fd=open(filename,O_RDONLY))==-1)
			{
				perror("open: ");
				return;
			}
			
			/*发送文件长度*/
			if(stat(filename,&fstat)==-1)
				return;
			
			SSL_write(ssl,&(fstat.st_size),4);
			
			/*发送文件内容*/
			while((count=read(fd,(void *)buf,1024))>0)
			{
				SSL_write(ssl,&buf,count);	
			}
			
			close(fd);

		}
		break;
	}	
}


/*主函数*/
void main()
{
	int i=0;
	char cmd;
	
	char pwd[100];
        char* temp;
	/*SSL初始化*/
	SSL_library_init();
	OpenSSL_add_all_algorithms();
  	SSL_load_error_strings();
  	ctx = SSL_CTX_new(SSLv23_server_method());
  	/* 载入数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
  	getcwd(pwd,100);
  	if(strlen(pwd)==1)
    		pwd[0]='\0';
  	if (SSL_CTX_use_certificate_file(ctx, temp=strcat(pwd,"/cacert.pem"), SSL_FILETYPE_PEM) <= 0)
  	{
    		ERR_print_errors_fp(stdout);
    		exit(1);
  	}
  	/* 载入用户私钥 */
  	getcwd(pwd,100);
  	if(strlen(pwd)==1)
    		pwd[0]='\0';
  	if (SSL_CTX_use_PrivateKey_file(ctx, temp=strcat(pwd,"/privkey.pem"), SSL_FILETYPE_PEM) <= 0)
  	{
    		ERR_print_errors_fp(stdout);
    		exit(1);
  	}
  	/* 检查用户私钥是否正确 */
  	if (!SSL_CTX_check_private_key(ctx))
  	{
    		ERR_print_errors_fp(stdout);
    		exit(1);
  	}
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket error");	
		exit(-1);
	}

	bzero(&server_addr,sizeof(struct sockaddr_in));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(portnumber);	
	
	if(bind(sockfd,(struct sockaddr*)(&server_addr),sizeof(struct sockaddr))<0)
	{
	    perror("Bind error");
	    exit(1);		
	}
	
	if(listen(sockfd, 5)==-1)
	{
	    perror("listen error");
	    exit(1);
	}	
	
	
	while(1)
	{
		if((new_fd = accept(sockfd,(struct sockaddr *)(&client_addr),&sin_size)) == -1)
		{
			perror("accept error!");
			exit(-1);
		}
			
		/*创建SSL*/	
       		ssl = SSL_new(ctx);
       		SSL_set_fd(ssl, new_fd);
       		if (SSL_accept(ssl) == -1)
       		{
          		perror("accept");
          		close(new_fd);
       		}				
		
		while(1)
		{
		        /*读取命令*/
			SSL_read(ssl,&cmd,1);

			if(cmd == 'Q')
			{
                        	close(new_fd);
				break;
			}
			else
			{
				handle(cmd);
			}
		}
		/*SSL退出*/
		SSL_shutdown(ssl);
    	SSL_free(ssl);
		close(new_fd);		
	}
	close(sockfd);
}



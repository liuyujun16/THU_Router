
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#define MAXBUF 8191
#define PORT_NUM 5000



char *substring(char *input, int i_begin, int i_end)
{
	int cnt = 0;
	int size = (i_end - i_begin) + 2;
	char *str = (char *)malloc(size);

	memset(str, 0, size);

	for (int i = i_begin; i <= i_end; i++)
	{
		str[cnt] = input[i];
		cnt++;
	}

	return str;
}


int PORT_analyze(char *buf_value, int *ip)
{
	int value;
	int c = 0;
	buf_value = substring(buf_value, 5, strlen(buf_value) - 1);

	char *token = strtok(buf_value, ",");
	while (token != NULL)
	{
		
		ip[c] = atoi(token);
		token = strtok(NULL, ",");
		c++;
	}

	return 1;
}

void read_list(int connfd){
	char sentence[8191];
		while (1) {
				memset(sentence, 0, sizeof(sentence));
					int n = read(connfd, sentence, 8192);
					if (n == 0) {
						memset(sentence, 0, sizeof(sentence));
						break;
					}
					printf("%s\r",sentence);			
				}
			
}
void RETR_data(int connfd,int sockfd,char *name){
	char sentence[8191];
	FILE *fp;
	char file_contents[8192];
	int n = read(sockfd, sentence, MAXBUF);
			sentence[n - 2] = '\0';
				printf("%s\r\n", sentence);
			
				fp = fopen(name, "wb");
			
				while (1) {
				
				n = read(connfd, file_contents, 8192);
				if (n == 0) {
				
						break;
					}
				
					fwrite(file_contents, 1, n, fp);
				}
			fclose(fp);

}

void STOR_data(int datafd,int mainfd,char *name){
		char sentence[8191];
		char file_contents[8192];
		FILE *fp;
		int n = read(mainfd, sentence, 8191);
				sentence[n - 2] = '\0';
				printf("%s\r\n", sentence);
			
				fp = fopen(name, "rb");
				while (!feof(fp)) {
				int len = fread(file_contents, 1, sizeof(file_contents), fp);
					n = write(datafd, file_contents, len);
				}
				fclose(fp);
}
int main(int argc, char **argv) {
	int m_sockfd, sockfd, listenfd, connfd;
	struct sockaddr_in addr;
	char sentence[8192],sentence2[8192];
	int len;
	int p;
	int PASV = 1;
	FILE *fp;
	int port = 5000;
	char m_sentence[8192];
	char file_contents[8192];
	
	char address[100] = "127.0.0.1";
	int ips[6];
	ips[4] = 0;
	ips[5] = 21;
	printf("%d argc \n", argc);
	if (argc == 1)
	{
		chdir("/tmp");
	}
	if (argc == 3)
	{
		if (!strcmp(argv[1], "-port"))
		{
			port = atoi(argv[2]);
		}
		if (!strcmp(argv[1], "-root"))
		{
			chdir(argv[2]);
		}
	}
	if (argc == 5)
	{
		if (!strcmp(argv[1], "-port"))
		{
			port = atoi(argv[2]);
			chdir(argv[4]);
		}
		if (!strcmp(argv[1], "-root"))
		{
			chdir(argv[2]);
			port = atoi(argv[4]);
		}
	}
	printf("%s %d addres \n", address,port);
	sockfd = connect_client(address,port);
	int first = read(sockfd, sentence, MAXBUF);
	sentence[first - 1] = '\0';
	sentence[first ] = '\r';
	printf("%s\n", sentence);

	while (1) {
		fgets(sentence, 4096, stdin);
		len = strlen(sentence);
		sentence[len -1] = '\r';
		sentence[len] = '\n';
		sentence[len + 1] = '\0';
		len = strlen(sentence);
		p = 0;
		while (p < len) {
			int n = write(sockfd, sentence, len);
			if (n < 0) {
				
				return 1;
			}
			else {
				p += n;
				break;
			}
		}
		sentence[p - 2] = '\0';
		sprintf(address, "%d.%d.%d.%d", ips[0], ips[1], ips[2], ips[3]);
		///aaa

			p = 0;
		while (1) {
			int n = read(sockfd, sentence2, 8191);
			if (n < 0) {
				printf("Error read(): %s(%d)\n", strerror(errno), errno);
				return 1;
			}
			else {
				p += n;
				break;
			}
		}
		sentence2[p - 2] = '\0';
		sentence2[p - 1] = '\r';
		sentence2[p ] = '\n';
		printf("readed :%s \n", sentence2);
		if (!strncmp(sentence2, "227", 3)) {

			PORT_analyze(substring(sentence2,22,strlen(sentence2)),ips);
			PASV = 1;
		
		}
		if (!strncmp(sentence2, "425", 3)) {
			strcpy(sentence,"none");
		}

			////aaa
		if (!strncmp(sentence, "PORT", 4)) {
			
			PORT_analyze(sentence,ips);
			port=ips[4]*256+ips[5];
			printf("port = %d\r\n",ips[4]*256+ips[5]);
			PASV=0;
		}
		else if (!strncmp(sentence, "STOR", 4)) {
			printf("STOR");
			if (!strcmp(sentence, "STOR ")) {
								strcpy(sentence, "550 File dose not exist");
						}
			if (PASV == 0) {
				listenfd = connect_server(port);
				if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
					printf("Error accept(): %s(%d)\n", strerror(errno), errno);
					continue;
				}
			}
			if(PASV ==1){
				connfd=connect_client(address,port);
			}	
				get_info(sentence, 5);
				STOR_data(connfd,sockfd,sentence);
			if(PASV ==0){
				close(listenfd);
			}
				close(connfd);
			
		}
		else if (!strncmp(sentence, "RETR", 4)) {
			printf("RETR");
			if (PASV == 0) {
				listenfd = connect_server(port);
				if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
					printf("Error accept(): %s(%d)\n", strerror(errno), errno);
					continue;
				}
			}
			if(PASV ==1){
				connfd=connect_client(address,port);
			}	
				get_info(sentence, 5);
				RETR_data(connfd,sockfd,sentence);
			if(PASV ==0){
				close(listenfd);
			}
				close(connfd);
			
		}
		else if(!strncmp(sentence,"LIST",4)){
			if (PASV == 0) {
				listenfd = connect_server(port);
				if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
					printf("Error accept(): %s(%d)\n", strerror(errno), errno);
					continue;
				}
			}
			if(PASV == 1) {
				connfd = connect_client(address,port);
			}
				read_list(connfd);
				if(PASV ==0){
				close(listenfd);
			}
				
				close(connfd);
			
		}
		

	
		else if (!strncmp(sentence,"221",3)) {
			close(sockfd);
			return 0;
		}
	}
}

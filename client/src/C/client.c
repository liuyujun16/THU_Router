#include "client.h"

#define MAXBUF 8191
#define PORT_NUM 5000


int main(int argc, char **argv) {
	int m_sockfd, sockfd, listenfd, connfd;
	struct sockaddr_in addr;
	char sentence[8192];
	int len;
	int p;
	PORT info;
	int PASV_mode = 1;
	FILE *fp;
	int port = 5000;
	char m_sentence[8192];
	char file_contents[8192];
	char ip_address[20] = "127.0.0.1";

	if (argc == 5)
	{
		port = atoi(argv[2]);
		strcpy(ip_address, argv[4]);
	}
	sockfd = connect_client(ip_address,PORT_NUM);
	p = read(sockfd, sentence, MAXBUF);
	sentence[p - 1] = '\0';
	printf("%s\r\n", sentence);

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
				("Error write(): %s(%d)\n", strerror(errno), errno);
				return 1;
			}
			else {
				p += n;
				break;
			}
		}
		sentence[p - 2] = '\0';
		if (!strncmp(sentence, "PORT", 4)) {
			int _port[10];
			divide_port(sentence,_port,5);
			PASV_mode = 0;
			info.port = _port[4] * 256 + _port[5];
			printf("port = %d\r\n",info.port);
		}
		else if (!strncmp(sentence, "STOR", 4)) {
			if (PASV_mode == 0) {
                print("stor stor ");
				listenfd = connect_server(info.port);
				if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
					printf("Error accept(): %s(%d)\n", strerror(errno), errno);
					continue;
				}
				int n = read(sockfd, m_sentence, 8191);
				m_sentence[n - 2] = '\0';
				printf("%s\r\n", m_sentence);
				get_info(sentence, 5);
				fp = fopen(sentence, "rb");
				while (!feof(fp)) {
					len = fread(file_contents, 1, sizeof(file_contents), fp);
					n = write(connfd, file_contents, len);
				}
				fclose(fp);
				close(listenfd);
				close(connfd);
			}
			else {
				m_sockfd = connect_client(info.ip_address,info.port);
				int n = read(sockfd, m_sentence, 8191);
				m_sentence[n - 2] = '\0';
				printf("%s\r\n", m_sentence);
				get_info(sentence, 5);
				fp = fopen(sentence, "rb");
				while (!feof(fp)) {
					len = fread(file_contents, 1, sizeof(file_contents), fp);
					n = write(m_sockfd, file_contents, len);
				}
				fclose(fp);
				close(m_sockfd);
			}
		}
		else if (!strncmp(sentence, "RETR", 4)) {
			if (PASV_mode == 0) {
				listenfd = connect_server(info.port);
				if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
					printf("Error accept(): %s(%d)\n", strerror(errno), errno);
					continue;
				}
				int n = read(sockfd, m_sentence, MAXBUF);
				m_sentence[n - 2] = '\0';
				printf("%s\r\n", m_sentence);
				get_info(sentence, 5);
				fp = fopen(sentence, "wb");
				while (1) {
					n = read(connfd, file_contents, 8192);
					if (n == 0) {
						break;
					}
					fwrite(file_contents, 1, n, fp);
				}
				fclose(fp);
				close(listenfd);
				close(connfd);
			}
			else {
				m_sockfd = connect_client(info.ip_address,info.port);
				int n = read(sockfd, m_sentence, 8191);
				m_sentence[n - 2] = '\0';
				printf("%s\r\n", m_sentence);
				get_info(sentence, 5);
				fp = fopen(sentence, "wb");
				while (1) {
					n = read(m_sockfd, file_contents, 8192);
					if (n == 0) {
						break;
					}
					fwrite(file_contents, 1, n, fp);
				}
				fclose(fp);
				close(m_sockfd);
			}
		}
		else if(!strncmp(sentence,"LIST",4)){
			if (PASV_mode == 0) {
				listenfd = connect_server(info.port);
				if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
					printf("Error accept(): %s(%d)\n", strerror(errno), errno);
					continue;
				}
				while (1) {
					memset(sentence, 0, sizeof(sentence));
					int n = read(connfd, sentence, 8192);
					if (n == 0) {
						memset(sentence, 0, sizeof(sentence));
						break;
					}
					printf("%s\r",sentence);			
				}
				close(listenfd);
				close(connfd);
			}
			else {
				m_sockfd = connect_client(info.ip_address,info.port);
			while (1) {
					memset(sentence, 0, sizeof(sentence));
					int n = read(m_sockfd, sentence, 8192);
					if (n == 0) {
						memset(sentence, 0, sizeof(sentence));
						break;
					}
					printf("%s\r",sentence);			
				}
				close(m_sockfd);
			}
		}
		

		p = 0;
		while (1) {
			int n = read(sockfd, sentence, 8191);
			if (n < 0) {
				printf("Error read(): %s(%d)\n", strerror(errno), errno);
				return 1;
			}
			else {
				p += n;
				break;
			}
		}
		sentence[p - 2] = '\0';
		printf("%s\r\n", sentence);
		if (!strncmp(sentence, "227", 3)) {
			memset(info.ip_address, 0, sizeof(info.ip_address));
			char ip[4];
			int _port[10];
			get_info(sentence, 26);
			divide_port(sentence,_port,1);
			for(int i = 0; i < 3; i++){
				sprintf(ip,"%d",_port[i]);
				strcat(info.ip_address,ip);
				strcat(info.ip_address,".");
				memset(ip,0,sizeof(ip));
			}
			sprintf(ip,"%d",_port[3]);
			strcpy(info.ip_address,"127.0.0.1");
			PASV_mode = 1;
			info.port = _port[4] * 256 + _port[5];
			printf("ip_address = %s \r\nport = %d\r\n",info.ip_address,info.port);
		}
		else if (!strncmp(sentence,"221",3)) {
			close(sockfd);
			return 0;
		}
	}
}

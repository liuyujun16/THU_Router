#include "server.h"

#define MAXCLIENT 30
#define PORT_NUM 5000

int main(int argc, char *argv[]) {
	PORT info;
	int transfer_bytes = 0;
	int transfer_files = 0;
	int port = 5000;
	int is_user_id = 0;
	int is_user_pwd = 0;
	int id_num;
	int PASV_mode = -1;
	int listenfd, connfd, sockfd;
	struct sockaddr_in addr;
	char sentence[8192];
	int p;
	int len;
	char file_contents[8192];
	int m_listenfd, m_connfd;
	char m_sentence[8192];
	FILE *fp;
	user user_table[MAXCLIENT];
	char directory[30] = "/tmp";
	char origin_name[100];


	if (argc == 5)
	{
		port = atoi(argv[2]);
		strcpy(directory, argv[4]);
	}
	chdir(directory);
	for (int i = 0; i < MAXCLIENT; i++) {
		strcpy(user_table[i].id,"anonymous");
		strcpy(user_table[i].pwd,"anonymous@");
		user_table[i].is_working = 0;
	}
	
	listenfd = connect_server(port);

	while (1) {
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			continue;
		}
		for (int i = 0; i < MAXCLIENT; i++) {
			if (waitpid(user_table[i].pid, NULL, WNOHANG)) {
				user_table[i].is_working = 0;
			}
		}
		for (int i = 0; i < MAXCLIENT; i++) {
			if (!user_table[i].is_working) {
				user_table[i].is_working = 1;
				id_num = i;
				break;
			}
			if (i == MAXCLIENT-1) {
				printf("Server is busy\n");
				id_num = -1;
			}
		}
		if (id_num == -1) {
			strcpy(sentence, "Server is busy");
			continue;
		}
		else {
			strcpy(sentence, "220 Anonymous FTP server ready\r\n");
		}
		len = strlen(sentence);
		int k = write(connfd, sentence, len);
		printf("USER %d connected\n", id_num);
		user_table[id_num].pid = fork();
		if (user_table[id_num].pid == 0) {
			while (1) {
				p = 0;
				while (1) {
					int n = read(connfd, sentence, 8191);
					if (n < 0) {
						printf("Error read(): %s(%d)\n", strerror(errno), errno);
						close(connfd);
						continue;
					}
					else {
						p += n;
						break;
					}
				}
				sentence[p-2] = '\0';
				printf("%s\n", sentence);
			
					if (is_user_id && is_user_pwd) {
						if (!strcmp("SYST", sentence)) {
							strcpy(sentence, "215 UNIX Type: L8");
						}
						else if (!strncmp("TYPE", sentence, 4)) {
							if (!strncmp("TYPE I", sentence, 6)) {
								strcpy(sentence, "200 Type set to I.");
							}
							else {
								strcpy(sentence, "504 Type dose not exist");
							}
						}
						else if (!strncmp("PORT ", sentence, 5)) {
							char ip[4];
							int _port[6];
                            printf("1");
                           
							divide_port(sentence,_port,5);
                            printf("3333");
                           
							for(int i = 0; i < 3; i++){
                               
								sprintf(ip,"%d",_port[i]);
                                printf("%d",_port[i]);
                              
								strcat(info.ip_address,ip);
								strcat(info.ip_address,".");
								memset(ip,0,sizeof(ip));
                               
							}
                        
							sprintf(ip,"%d",_port[3]);
                            
							strcat(info.ip_address,ip);
                            strcpy(info.ip_address,"127.0.0.1");
							printf("ip = %s\r\n",info.ip_address);
                            printf("3");
							info.port = _port[4] * 256 + _port[5];
							printf("PORT_port = %d\r\n",info.port);
							PASV_mode = 0;
                            printf("4");
							strcpy(sentence, "200 PORT command successful.");
						}
						else if (!strcmp("PASV", sentence)) {
							PASV_mode = 1;
							srand(time(NULL));
							int _port =  rand() % 45535 + 20000;
							int port1 = _port / 256;
							int port2 = (_port - (port1 * 256));
							strcpy(info.ip_address,"127.0.0.1");
							info.port = _port;
							sprintf(sentence, "227 Entering Passive Mode (127,0,0,1,%d,%d)\r\n",port1,port2);
							len = strlen(sentence);
							int n = write(connfd, sentence, len);

							m_listenfd = connect_server(info.port);
							if ((m_connfd = accept(m_listenfd, NULL, NULL)) == -1) {
								printf("Error accept(): %s(%d)\n", strerror(errno), errno);
								continue;
							}
	
							continue;
						}
						else if (!strncmp("RETR ", sentence, 5)) {
							get_info(sentence, 5);
							len = strlen(sentence);
                            printf("it is retr");
                            printf("%s\n",sentence);
                         
							fp = fopen(sentence, "rb");
							if (fp == NULL) {
								strcpy(sentence, "550 file dose not exist");
							}
							else {
								fseek(fp, 0, SEEK_END);
								int file_size = ftell(fp);
								fseek(fp, 0, SEEK_SET);
								
								if (PASV_mode == 0) {
                                  
									sprintf(m_sentence, "150 Opening BINARY mode data connection for %s( %d bytes).\r\n", sentence, file_size);					
									sockfd = connect_client(info.ip_address, info.port);
									len = strlen(m_sentence);
									int n = write(connfd, m_sentence, len);
									while (!feof(fp)) {
										len = fread(file_contents, 1, sizeof(file_contents), fp);
										n = write(sockfd, file_contents, len);
									}
                                    
									transfer_files++;
									transfer_bytes = transfer_bytes + file_size;
									fclose(fp);
                                   
									strcpy(sentence, "226 Transfer complete.");
                                    printf("done!no");
									close(sockfd);
                                   
								}
								else if (PASV_mode == 1){
									sprintf(m_sentence, "150 Opening BINARY mode data connection for %s( %d bytes).\r\n", sentence, file_size);
									len = strlen(m_sentence);
									int n = write(connfd, m_sentence, len);
                                  
									while (!feof(fp)) {
										len = fread(file_contents, 1, sizeof(file_contents), fp);
										n = write(m_connfd, file_contents, len);
									}
									transfer_files++;
									transfer_bytes = transfer_bytes + file_size;
									strcpy(sentence, "226 Transfer complete.");
									fclose(fp);
									close(m_listenfd);
									close(m_connfd);
								}
								else {
									strcpy(sentence,"425 Please input PASV or PORT first.");
								}
								sleep(20);
							}
						}
						else if (!strncmp("STOR ", sentence, 5)) {
							if (!strcmp(sentence, "STOR ")) {
								strcpy(sentence, "550 File dose not exist");
							}
							else{
									get_info(sentence, 5);
									if (PASV_mode == 0) {
										
										sockfd = connect_client(info.ip_address, info.port);
										sprintf(m_sentence, "150 Opening BINARY mode data connection for %s.\r\n", sentence);
										len = strlen(m_sentence);
										int n = write(connfd, m_sentence, len);
										fp = fopen(sentence, "wb");
										while (1) {
											int n = read(sockfd, file_contents, 8192);
											if (n == 0) {
												strcpy(sentence, "226 Transfer complete.");
												break;
											}
											fwrite(file_contents, 1, n, fp);
										}
										transfer_files++;
										fclose(fp);
										close(sockfd);
									}
									else if (PASV_mode == 1){
										sprintf(m_sentence, "150 Opening BINARY mode data connection for %s.\r\n", sentence);
										len = strlen(m_sentence);
										int n = write(connfd, m_sentence, len);
										fp = fopen(sentence, "wb");
										while (1) {
											int n = read(m_connfd, file_contents, 8192);
											if (n == 0) {
												strcpy(sentence, "226 Transfer complete.");
												break;
											}
											fwrite(file_contents, 1, n, fp);
										}
										transfer_files++;
										fclose(fp);
										close(m_listenfd);
										close(m_connfd);
									}
									else{
										strcpy(sentence,"425 Please input PASV or PORT first.");
									}
							}
						}
						else if(!strcmp(sentence, "QUIT") || !strcmp(sentence, "ABOR")) {
							sprintf(sentence, "221 You have transferred %d bytes in %d files\n221 Thank you for using the FTP server.\n221 Goodbye.\r\n",transfer_bytes,transfer_files);
							len = strlen(sentence);
							int n = write(connfd, sentence, len);
							close(connfd);
							return 0;
						}
						else if(!strncmp("MKD", sentence, 3)){
							get_info(sentence, 4);
							int k = mkdir(sentence, S_IRWXU);
						}
						else if(!strncmp("PWD",sentence,3)){
							char buf[100];
							memset(sentence, 0, sizeof(sentence));
							getcwd(buf,sizeof(buf));
							sprintf(sentence,"Now directory: %s",buf);
						}
						else if(!strncmp("CWD", sentence, 3)){
							get_info(sentence, 4);
							chdir(sentence);
						}
						else if(!strncmp("RNFR",sentence, 4)){//RNTO origin new
							get_info(sentence,5);
							memset(origin_name, 0, sizeof(origin_name));
							strcpy(origin_name,sentence);
							sprintf(sentence,"You have chosen %s to change name",origin_name);
						}
						else if(!strncmp("RNTO",sentence, 4)){//RNTO origin new
							get_info(sentence,5);

							char new_name[100];
							int flag = 0;
							int cnt_new = 0;
							int cnt_origin = 0;
							strcpy(new_name,sentence);

							if(strlen(origin_name) == 0)
								strcpy(sentence,"You should choose file first.");
							else{
								sprintf(sentence,"Change file name from %s to %s successfully.",origin_name,new_name);
								rename(origin_name,new_name);
								memset(origin_name, 0, sizeof(origin_name));
							}
						}
						else if (!strncmp("LIST", sentence, 4)){
							DIR *dir;
							struct dirent *dp;
							dir = opendir(".");
							if (PASV_mode == 0){
								sockfd = connect_client(info.ip_address, info.port);			
							}
							while ((dp = readdir(dir)) != NULL){
								memset(sentence, 0, sizeof(sentence));
								strcpy(sentence, dp->d_name);
								strcat(sentence, "\r\n");
								if (PASV_mode == 0){	
									len = strlen(sentence);	
									write(sockfd, sentence, len);
								}
								else if (PASV_mode == 1){
									len = strlen(sentence);	
									write(m_connfd, sentence, len);
								}
							}
							strcpy(sentence,"=======================\r\n");
							if (PASV_mode == 0) {			
								close(sockfd);
							 }
							else if (PASV_mode == 1){		
								close(m_listenfd);
								close(m_connfd);
							}
							(void)closedir(dir);
						}
						else if (!strncmp("RMD", sentence, 3)){
							char buf[100];
							memset(buf, 0 , sizeof(buf));
							get_info(sentence, 4);
							strcpy(buf,sentence);
							printf("DELETE = %s\r\n",buf);
							rmdir(buf);
							sprintf(sentence,"Delete %s from directory successfully!",buf);
						}
						
						else {
							strcpy(sentence, "500 Command dose not exist");
						}
					}
					else if (!is_user_id) {
						if (!strncmp(sentence, "USER ", 5)) {
							get_info(sentence, 5);
							if (!strcmp(sentence, user_table[id_num].id)) {
								strcpy(sentence, "331 Guest login ok, send your complete e-mail address as password.");
								is_user_id = 1;
							}
							else {
								strcpy(sentence, "504 User dose not exist");
							}
						}
						else {
							strcpy(sentence, "500 Command dose not exist");
						}
					}
					else {
						if (!strncmp(sentence, "PASS ",5)) {
							get_info(sentence, 5);
							if (!strcmp(sentence, user_table[id_num].pwd)) {
								strcpy(sentence, "230 Welcome to server.");
								is_user_pwd = 1;
							}
							else {
								strcpy(sentence, "504 Password dose not exist");
							}
						}
						else {
							strcpy(sentence, "500 Command dose not exist");
						}
					}
		
				p = 0;
				len = strlen(sentence);
				sentence[len] = '\r';
				sentence[len + 1] = '\n';
				sentence[len + 2] = '\0';
				len = strlen(sentence);
				printf("%s\n", sentence);
				while (p < len) {
					int n = write(connfd, sentence, len);
					if (n < 0) {
						printf("Error write(): %s(%d)\n", strerror(errno), errno);
						return 1;
					}
					else {
						p += n;
					}
				}
			}
		}
	}
	close(listenfd);
}


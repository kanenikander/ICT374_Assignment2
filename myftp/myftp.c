/*
 * File: myftp.c
 * Author: Kane Nikander and Ananth Kadekodi
 *
 * Description: This file is the main implementation program for the myftp client service. It also provides the frontend 
 * program functionalities relating to the myftp client service. 
 */

#include <stdio.h> 
#include <stdlib.h>
#include <netdb.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>

#include <dirent.h> 
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SERV_IP_ADDRESS		"134.115.4.185" //ceto ip address
#define SERV_TCP_PORT 		40048 //port number

void interface(int conn, char *hostName);

int main(int argc, char *argv[]) {
	
	struct in_addr inAddr;
	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SERV_TCP_PORT);
	int sock = socket(AF_INET,SOCK_STREAM,0);

	//CHECK ARGS
	if(argc == 1) { //default (no args)
		serAddr.sin_addr.s_addr = inet_addr(SERV_IP_ADDRESS);
	} else if(argc == 2) { //second arg included
		if(inet_aton(argv[1],&inAddr)) { // input is valid IP address
			serAddr.sin_addr.s_addr = inAddr.s_addr; 
		} else {
			printf("An error has occured. IP Address is not valid.\n");
			exit(1);
		}		
	} else if(argc > 2) { //too many args
		printf("Invalid input. Please try again.\n");
		exit(1);
	}

	//Establish Connection
	if(connect(sock, (struct sockaddr*)&serAddr, sizeof(serAddr)) >= 0) {
		interface(sock,inet_ntoa(serAddr.sin_addr));
	} else {
		printf("An error has occured. Could not connect.\n");
	}

	exit(0);
	
}

void interface(int conn, char *hostName) {
	
	char command[256];
	char *tempArg1;	//holds first arg
	char *tempArg2; //holds second arg
	int tempInt;
	char tempChar;
	char tempString[256];
	int terminate = 0; //bool to determine when user quits
	
	printf("\x1B[37m"); //color: white
	printf("Session started.\n");
	do {
		
		//Get current path (pwd method)
		tempChar = 'a';
		write(conn, &tempChar, sizeof(tempChar)); //send request
		read(conn, &tempChar, sizeof(tempChar)); //get acknowledgement
		if(tempChar == 'A') {
			read(conn, &tempChar, sizeof(tempChar)); 
			if(tempChar == '1') {
				read(conn, &tempInt, sizeof(tempInt));
				read(conn, tempString, (sizeof(char)*ntohs(tempInt)));
			}
		} else { //if connect error
			tempString[0] = 0;
			strcat(tempString,"PATH_ERROR");
		}
		
		//Print formatted pront with colours
		printf("\x1B[32m"); //color: green
		printf("%s",hostName);
		printf("\x1B[37m"); //color: white
		printf(":");
		printf("\x1B[36m"); //color: cyan
		printf("~%s",tempString);
		printf("\x1B[37m"); //color: white
		printf("$ ");

		//Get input
		fgets(command,256,stdin);
		if(strlen(command) == 1) { //nothing entered
			tempArg1 = ""; //set first arg to blank
		} else if((command[strlen(command) - 1]) == '\n') {
			command[strlen(command) - 1] = '\0';
			tempArg1 = strtok(command," \n"); //gets first arg
			tempArg2 = strtok(NULL," \n"); //gets second arg
		}
			
		//Command options
		if(!strcmp(tempArg1,"pwd")) { //command: pwd
		
			tempChar = 'a';
			write(conn, &tempChar, sizeof(tempChar)); //send request
			read(conn, &tempChar, sizeof(tempChar)); //get acknowledgement
			if(tempChar == 'A') {
				read(conn, &tempChar, sizeof(tempChar)); 
				if(tempChar == '1') {
					read(conn, &tempInt, sizeof(tempInt));
					read(conn, tempString, (sizeof(char)*ntohs(tempInt)));
					printf("%s\n", tempString);
				} else if(tempChar == '0') {
					printf("An error has occured.\n"); //no local directory
				} else {
					printf("An internal error has occured.\n"); //problem with protocol
				}
			} else {
				printf("An internal error has occured.\n"); //problem with protocol
			}
		
		} else if(!strcmp(tempArg1,"dir") || !strcmp(tempArg1,"ls")) { //command: dir / ls
		
			tempChar = 'b';
			write(conn, &tempChar, sizeof(tempChar)); //send request
			read(conn, &tempChar, sizeof(tempChar)); //get acknowledgement
			if(tempChar == 'B') {
				read(conn, &tempChar, sizeof(tempChar));
				if(tempChar == '1') {
					char cont;
					read(conn, &cont, sizeof(cont)); //get acknowledgement for loop
					while(cont == '1') {
						read(conn, &tempChar, sizeof(tempChar));
						
						read(conn, &tempInt, sizeof(tempInt));  //recieve: filename length
						tempInt = ntohs(tempInt);
						read(conn, tempString, (sizeof(char)*tempInt)); //recieve: filename
						
						//Output format in colours
						if(tempChar == 'f') {
							printf("\x1B[37m"); //color: white
						} else if(tempChar == 'd') {
							if(!strcmp(tempString,".") || !strcmp(tempString,"..")) {
								printf("\x1B[36m"); //color: cyan
							} else {
								printf("\x1B[32m"); //color: green
							}
						} else {
							printf("\x1B[31m"); //color: red
						}
						
						printf("%s  ", tempString);
						read(conn, &cont, sizeof(cont)); //get acknowledgement for loop
					}
					printf("\x1B[37m"); //color: white
					printf("\n");
					
				} else if(tempChar == '0') {
					printf("An error has occured.\n"); //no files
				} else {
					printf("An internal error has occured.\n"); //problem with protocol
				}
			} else {
				printf("An internal error has occured.\n"); //problem with protocol
			}	
		
		} else if(!strcmp(tempArg1,"cd")) { //command: cd
		
			tempChar = 'c';
			write(conn, &tempChar, sizeof(tempChar)); //send request
			read(conn, &tempChar, sizeof(tempChar)); //get acknowledgement
			if(tempChar == 'C') { 
			
				if(tempArg2 == NULL) {
					tempArg2 = "/";
				}
			
				tempInt = strlen(tempArg2) + 1; 
				tempInt = htons(tempInt); 
			
				write(conn, &tempInt, sizeof(tempInt));
				write(conn, tempArg2, (sizeof(char)*(ntohs(tempInt))));
				read(conn, &tempChar, sizeof(tempChar)); 
				if(tempChar == '0') {
					printf("An error has occured.\n"); //invalid arg2
				} else if(tempChar != '1') {
					printf("An internal error has occured.\n"); //problem with protocol
				}
			} else {
				printf("An internal error has occured.\n"); //problem with protocol
			}
			
		} else if(!strcmp(tempArg1,"get")) { //command: get
			
			if(tempArg2 != NULL) {
			
				tempChar = 'd';
				write(conn, &tempChar, sizeof(tempChar)); //send request
				read(conn, &tempChar, sizeof(tempChar)); //get acknowledgement
				if(tempChar == 'D') { 
					tempInt = strlen(tempArg2) + 1; 
					tempInt = htons(tempInt); 
				
					write(conn, &tempInt, sizeof(tempInt));
					write(conn, tempArg2, sizeof(char)*(ntohs(tempInt)));
					
					read(conn, &tempChar, sizeof(tempChar)); //get acknowledgement of file exists
					if(tempChar == '1') { 
					
						FILE *file;
						file = fopen(tempArg2,"w");
						char contents;
					
						read(conn, &tempChar, sizeof(tempChar)); //file reading
						while(tempChar == '1') {
							read(conn, &tempChar, sizeof(tempChar));
							fprintf(file,"%c",tempChar);
							read(conn, &tempChar, sizeof(tempChar)); //file stopped
						}
						
						fclose(file);
						printf("File transferred successfully \n"); 

					} else if (tempChar = '1'){
						printf("File does not exist.\n"); //no file
					}
				
				} else if (tempChar = '2'){
					printf("An internal error has occured.\n"); //problem with protocol
				}
			
			} else {
				printf("File name was not specified.\n"); //no file name stated
			}
			
		} else if(!strcmp(tempArg1,"put")) { //command: put
		
			if(tempArg2 != NULL) {
				
				tempChar = 'e';
				write(conn, &tempChar, sizeof(tempChar)); //send request
				read(conn, &tempChar, sizeof(tempChar)); //get acknowledgement
				if(tempChar == 'E') { 
					FILE *file;
					file = fopen(tempArg2,"r");
				
					if(file != NULL) {
						tempChar = '1';
						write(conn, &tempChar, sizeof(tempChar)); //send: command succeed
						
						tempInt = strlen(tempArg2) + 1; 
						tempInt = htons(tempInt); 
				
						write(conn,&tempInt,sizeof(tempInt));
						write(conn,tempArg2,(sizeof(char)*(ntohs(tempInt))));
						
						char contents;
						while(!feof(file)) {
							
							tempChar = '1';
							write(conn, &tempChar, sizeof(tempChar)); //continue sending data
							
							contents = fgetc(file);
							
							if(feof(file)) {
								contents = ' ';
							}
							write(conn, &contents, sizeof(contents));
							
						}
						tempChar = '0';
						write(conn, &tempChar, sizeof(tempChar)); //stop sending data
						
						fclose(file);
						printf("File transferred successfully \n"); 
					} else {
						tempChar = '0';
						write(conn, &tempChar, sizeof(tempChar)); //send: command falied
						printf("File does not exist.\n"); //no file
					}
				
				} else if (tempChar = '2') {
					printf("An internal error has occured.\n"); //problem with protocol
				}
				
			} else {
				printf("File name was not specified.\n"); //no file name stated
			}
		
		} else if(!strcmp(tempArg1,"lcd")) { //command: lcd
		
			if (chdir(tempArg2) == -1) {
				printf("An error has occured. Please try again.\n");
			}
			
		} else if(!strcmp(tempArg1,"ldir")) { //command: ldir
		
			DIR *d;
			struct dirent *dir;
			d = opendir(".");
		
			if(d) {
				while((dir = readdir(d)) != NULL) {
					if (dir->d_type == DT_REG) {
						printf("\x1B[37m"); //color: white
					} else if (dir->d_type == DT_DIR) {
						if(!strcmp(dir->d_name,".") || !strcmp(dir->d_name,"..")) {
							printf("\x1B[36m"); //color: cyan
						} else {
							printf("\x1B[32m"); //color: green
						}
					} else {
						printf("\x1B[31m"); //color: red
					}
					printf("%s  ",dir->d_name);
				}
				closedir(d);
				printf("\x1B[37m"); //color: white
				printf("\n");
			}
			
		} else if(!strcmp(tempArg1,"lpwd")) { //command: lpwd
		
			if(getcwd(tempString, sizeof(tempString)) != NULL) {
				printf("%s\n", tempString);
			} else {
				printf("An error has occured.\n");
			}
			
		} else if(!strcmp(tempArg1,"bye") || !strcmp(tempArg1,"close") || !strcmp(tempArg1,"disconnect") || !strcmp(tempArg1,"exit") || !strcmp(tempArg1,"quit")) { //command: bye / close / disconnect / exit / quit
		
			terminate = 1;
			
		} else if(!strcmp(tempArg1,"")) { //no command
		
			//do nothing
			
		} else { //unknown command
		
			printf("Invalid input. Please try again.\n");
			
		}
		
	} while(terminate == 0);
	
	printf("Session ended.\n");
	exit(1);
	
}

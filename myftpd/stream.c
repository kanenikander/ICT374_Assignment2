/*
* File name: stream.c
* Author: Kane Nikander and Ananth Kadekodi
*
* Description: This file contains program implementation that relates to providing service to the client that has connected	
* to the myftpd server. 
*/

#include "stream.h"

void serveAClient(int conn) {
	char command; 
	int tempInt;
	char tempChar;
	char tempString[256];
	int error = 0; 
	
	while(error == 0) {
		
		if (read(conn, &command, sizeof(char)) > 0) {
			
			//Select command
			if (command == 'a') { //command: pwd
			
				tempChar = 'A';
				write(conn, &tempChar, sizeof(tempChar)); //send: acknowledgement char
				
				if(getcwd(tempString, 256) != NULL) { //statement: succeeded
					tempChar = '1';
					write(conn, &tempChar, sizeof(tempChar)); //send: command succeeded
					
					tempInt = strlen(tempString) + 1;
					tempInt = htons(tempInt); //convert: host to network short
					
					write(conn, &tempInt, sizeof(tempInt)); //send: length of path
					write(conn, tempString, ntohs(tempInt)); //send: path
					writeLog("pwd");
				} else { //statement: failed
					tempChar = '0';
					write(conn, &tempChar, sizeof(tempChar));  //send: command failed
				}
			
			} else if (command == 'b') { //command: dir / ls
			
				tempChar = 'B';
				write(conn, &tempChar, sizeof(char)); //send: acknowledgement char
				
				DIR *d;
				struct dirent *dir;
				d = opendir(".");
				
				if(d) { //statement: succeeded
					tempChar = '1';
					write(conn, &tempChar, sizeof(tempChar)); //send: command succeeded
					
					while((dir = readdir(d)) != NULL) { //files remaining
						tempChar = '1';
						write(conn, &tempChar, sizeof(tempChar)); //send: command succeeded
						
						if (dir->d_type == DT_REG) { //files
							tempChar = 'f';
						} else if (dir->d_type == DT_DIR) { //directories
							tempChar = 'd';
						} else { //other
							tempChar = 'o';
						}
						write(conn, &tempChar, sizeof(tempChar)); 
						
						tempInt = strlen(dir->d_name) + 1; 
						tempInt = htons(tempInt); //convert: host to network short
						
						write(conn, &tempInt, sizeof(tempInt)); //send: length of filname
						write(conn, dir->d_name, ntohs(tempInt)); //send: filename
					}
					tempChar = '0';
					write(conn, &tempChar, sizeof(tempChar)); //send: finished
					closedir(d);
					writeLog("dir");
					
				} else { //statement: failed
					tempChar = '0';
					write(conn, &tempChar, sizeof(tempChar)); //send: command failed
				}
			
			} else if (command == 'c') { //command: cd
				
				tempChar = 'C';
				write(conn, &tempChar, sizeof(char));//send: acknowledgement char
				
				read(conn,&tempInt,sizeof(tempInt)); // read in directory name length
				tempInt = ntohs(tempInt); //convert: network to host short
				read(conn, tempString, (sizeof(char) * tempInt)); // read in directory name
				
				if(chdir(tempString) == 0) { //statement: succeeded
					tempChar = '1';
					write(conn, &tempChar, sizeof(tempChar)); //send: command succeeded
					writeLog("cd");
				} else { //statement: failed
					tempChar = '0';
					write(conn, &tempChar, sizeof(tempChar)); //send: command failed
				}
				
			} else if (command == 'd') { //command: get
			
				tempChar = 'D';
				write(conn, &tempChar, sizeof(char)); //send: acknowledgement char
				
				read(conn,&tempInt,sizeof(tempInt)); //read in size of arg2
				tempInt = ntohs(tempInt); //convert: network to host short
				read(conn, tempString, (sizeof(char) * tempInt)); //read in arg2
				
				FILE *file;
				file = fopen(tempString,"r+");
					
				if(file != NULL) {
					tempChar = '1';
					write(conn, &tempChar, sizeof(tempChar)); //send: command succeed
					
					char contents;
					while(!feof(file)) {
						
						tempChar = '1';
						write(conn, &tempChar, sizeof(tempChar)); //continue sending data
						
						contents = fgetc(file);
						
						if(feof(file)) {
							contents = ' '; //sets EOF char to blank
						}
						write(conn, &contents, sizeof(contents));
						
					}
					tempChar = '0';
					write(conn, &tempChar, sizeof(tempChar)); //stop sending data
					
					fclose(file);
					writeLog("get");
				} else {
					tempChar = '0';
					write(conn, &tempChar, sizeof(tempChar)); //send: command succeed
				}
				
			} else if (command == 'e') { //command: put
			
				tempChar = 'E';
				write(conn, &tempChar, sizeof(char));//send: acknowledgement char
			
				read(conn, &tempChar, sizeof(tempChar)); //get acknowledgement of file exists
				if(tempChar == '1') { 
				
					read(conn,&tempInt,sizeof(tempInt)); //get file name length
					tempInt = ntohs(tempInt); //convert: network to host short
					read(conn, tempString, (sizeof(char)*tempInt)); //get file name
				
					FILE *file;
					file = fopen(tempString,"w");
					char contents;
					
					read(conn, &tempChar, sizeof(tempChar)); //file reading
					while(tempChar == '1') {
						read(conn, &tempChar, sizeof(tempChar));
						fprintf(file,"%c",tempChar);
						read(conn, &tempChar, sizeof(tempChar)); //file stopped	
					}
				
					fclose(file);
					writeLog("put");
				} else {
					//no file to write from
				}
			
			}
		
		} else {
			
			error = 1;
			
		}
		
	}
	
}

void writeLog(char *message) {
	FILE *file;
	file = fopen("log.txt","a");
	if(file != NULL) {
		fprintf(file,"%s\n",message);
		fclose(file);
	}
}

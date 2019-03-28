/*
* File name: myftpd.c
* Author: Kane Nikander and Ananth Kadekodi
*
* Description:  This is the main file that provides program implementation for myftpd server. This myftpd
* server has been implemented to provide a simple file transfer protocol as stated in assignment 2 of ICT 374.
* Additionally, this file also contains the program implementation that involves setting up the myftpd server 
* as a daemon process. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#include "stream.h"

#define SERV_TCP_PORT 40048 //port number

void claimZombies();
void startDaemon(void);

int main(int argc, char *argv[]) {
	
	int sock;
	int conn;
	int cliAddrLength;
	struct sockaddr_in servAddr;
	struct sockaddr_in cliAddr;
	pid_t pid;
	
	printf("Hosting...\n");
	startDaemon();

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("An error has occured.\n");
		exit(1);
	}

	//build socket address
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(SERV_TCP_PORT);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 

	//bind server
	if (bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
		printf("Port %d is already in use.\n",SERV_TCP_PORT);
		exit(1);
	} else {
		printf("PORT: %d\n", SERV_TCP_PORT);
		printf("PID: %d\n", getpid());
		writeLog("starting up");
	}

	//become listening socket
	listen(sock, 5);
	
	// REF: The following code is the third party code used from page 63 of Lecture 8 (ICT 374)
	while (1) {
		cliAddrLength = sizeof(cliAddr);
		conn = accept(sock, (struct sockaddr *)&cliAddr, &cliAddrLength);
		
		if (conn < 0) {
			if (errno == EINTR) {
				continue;
			}
			printf("An error has occured with the connection.\n");
			exit(1);
		}
		
		pid = fork();
		if (pid < 0) {
			printf("An error has occured with the connection.\n");
			exit(1);
		} else if (pid > 0) {
			close(conn);
			continue;
		}

		close(sock);
		serveAClient(conn);

		exit(0);
	}
}

// REF: The following code is the third party code used from page 61 of Lecture 8 (ICT 374)
void claimZombies() {
	pid_t pid;

	while(pid > 0) {
		pid = waitpid(0, (int*)0, WNOHANG); //claim zombies
	}
}

// REF: The following code is the third party code used from page 61 of Lecture 8 (ICT 374)
void startDaemon(void) {
	pid_t pid;
	struct sigaction act;

	if ((pid = fork()) < 0) {
		exit(1);
	} else if (pid != 0) {
		exit(0);
	}

	setsid(); //become session leader
	umask(0); //clear file mode 
	
	act.sa_handler = claimZombies;
	sigemptyset(&(act.sa_mask));
	act.sa_flags = SA_NOCLDSTOP;
	
	if((sigaction(SIGCHLD, &act, (struct sigaction*)0)) != 0) {
		exit(1);
	}
}

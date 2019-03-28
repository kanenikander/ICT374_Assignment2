/*
* stream.h provides methods to run on the server that listen for client instructions, process those requests and send back data. 
*/

#ifndef _STREAM_H
#define _STREAM_H

#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
* serveAClient takes a command sent from the client and processes each request.
*/
void serveAClient(int);

/*
* writeLog writes a one line message to a log file.
*/
void writeLog(char *message);

#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <string.h>

#define buffSize 4096
#define defaultServerPort 4747
#define SA struct sockaddr
#define CONNECTIONS 1000
#define serverRootDirectory "../www/"
#define defaultPage "../www/index.html"
#define forbiddenDirectory "../www/C/"

int connectionSocketDescriptor[CONNECTIONS], n;
char *request,requestedFile[800];

char *createAbsolutePath(char *filename){
        char *absolutePathToFile = realpath(serverRootDirectory, NULL);
        strcat(absolutePathToFile, filename);


        return realpath(absolutePathToFile,NULL);
}

void printFileDetails(int socketDescriptor, int fileDescriptor, int statusCode, char *httpMethod){
        char currentTime[100], lastModificationTime[100], response[buffSize];
        time_t now = time(0);
        struct tm tm = *gmtime(&now);
        struct stat filestat;
        int fileSize;

        fstat(fileDescriptor,&filestat);//get file stata
        strftime(currentTime, sizeof(currentTime), "%a, %d %b %Y %H:%M:%S %Z", &tm);
        strftime(lastModificationTime, sizeof(lastModificationTime), "%a, %d %b %Y %H:%M:%S %Z", gmtime(&(filestat.st_mtime)));
        fileSize = filestat.st_size;//size of the file

        if (fileDescriptor < 0) {
                if (statusCode == 400) {
                        if(strncmp(httpMethod, "HEAD", 4) == 0)
                        {
                                strftime(currentTime, sizeof(currentTime), "%a, %d %b %Y %H:%M:%S %Z", &tm);
                                snprintf(response,sizeof(response),"HTTP/1.0 %d Bad Request\r\nDate: %s\r\nServer: UNIX\r\n\r\n",statusCode,currentTime);
                                write(socketDescriptor, response, strlen(response));
                                exit(0);
                        }
                        else if (strncmp(httpMethod, "GET", 3) == 0)
                        {
                                snprintf(response,sizeof(response),"HTTP/1.0 %d Bad Request\r\nDate: %s\r\nServer: UNIX\r\n\r\n<!DOCTYPE HTML>\n<html lang=en>\n<head>\n\t<meta charset=UTF-8>\n\t<title>%d Bad Request</title>\n</head>\n<body>\n\t<h1>%d Bad Request</h1>\n</body>\n</html>",statusCode,currentTime,statusCode, statusCode );
                                write(socketDescriptor, response, strlen(response));
                                exit(0);
                        }
                }
                else if (statusCode == 501) {
                        snprintf(response,sizeof(response),"HTTP/1.0 %d Not Implemented\r\nDate: %s\r\nServer: UNIX\r\n\r\n<!DOCTYPE HTML>\n<html lang=en>\n<head>\n\t<meta charset=UTF-8>\n\t<title>%d Not Implemented</title>\n</head>\n<body>\n\t<h1>%d Not Implemented</h1>\n</body>\n</html>\n",statusCode,currentTime, statusCode, statusCode);
                        write(socketDescriptor, response, strlen(response));
                        exit(0);
                }
                else if (statusCode == 404) {
                        if(strncmp(httpMethod, "HEAD", 4) == 0)
                        {
                                strftime(currentTime, sizeof(currentTime), "%a, %d %b %Y %H:%M:%S %Z", &tm);
                                snprintf(response,sizeof(response),"HTTP/1.0 %d Not Found\r\nDate: %s\r\nServer: UNIX\r\nContent-Type: text/html\r\n\r\n",statusCode,currentTime);
                                write(socketDescriptor, response, strlen(response));
                                exit(0);
                        }
                        else if (strncmp(httpMethod, "GET", 3) == 0)
                        {
                                snprintf(response,sizeof(response),"HTTP/1.0 %d Not Found\r\nDate: %s\r\nServer: UNIX\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\n<html lang=en>\n<head>\n\t<meta charset=UTF-8>\n\t<title>%d Not Found</title>\n</head>\n<body>\n\t<h1>%d Not Found</h1>\n</body>\n</html>\r\n\r\n",statusCode,currentTime,statusCode,statusCode );
                                write(socketDescriptor, response, strlen(response));
                                exit(0);
                        }
                }
        }
        else if(fileDescriptor > 0) {
                if (statusCode == 403) {
                        snprintf(response,sizeof(response),"HTTP/1.0 %d Forbidden\r\nDate: %s\r\nServer: UNIX\r\n\r\n<!DOCTYPE HTML>\n<html lang=en>\n<head>\n\t<meta charset=UTF-8>\n\t<title>%d Forbidden</title>\n</head>\n<body>\n\t<h1>%d Forbidden</h1>\n\t<h3>Permission denied.You cannot access /C/ directory.</h3>\n</body>\n</html>\n",statusCode,currentTime,statusCode,statusCode);
                        write(socketDescriptor, response, strlen(response));
                        exit(0);
                }
                else{
                  if (strncmp(httpMethod, "HEAD", 4) == 0) {
                    snprintf(response,sizeof(response),"HTTP/1.0 %d OK\r\nDate: %s\r\nServer: UNIX\r\nLast-Modified: %s\r\nContent-Length: %d\r\nContent-Type: text/html\r\n\r\n",statusCode,currentTime,lastModificationTime,fileSize);
                    write(socketDescriptor, response, strlen(response));
                  }
                  else if (strncmp(httpMethod, "GET", 3) == 0)
                  {
                    snprintf(response,sizeof(response),"HTTP/1.0 %d OK\r\nDate: %s\r\nServer: UNIX\r\nLast-Modified: %s\r\nContent-Length: %d\r\nContent-Type: text/html\r\n\r\n",statusCode,currentTime,lastModificationTime,fileSize);
                    write(socketDescriptor, response, strlen(response));
                          while ((n=read(fileDescriptor, response, 4096)) > 0) {
                                  write(socketDescriptor, response, n);
                                  exit(0);
                          }
                          exit(0);
                  }

                }
        }
}

int GetPort(char *port, int defaultport)
{
        char *p;

        errno = 0;
        long port_val = strtol(port, &p, 10);

        if (errno != 0 || *p != '\0' || port_val > INT_MAX || (port_val < 1025 && port_val != 80)) {
                printf("%s\n\n","It is not a valid port number...Default port number will be used");
                return defaultport;
        } else {
                return port_val;
        }
}

int createServer(int port)
{
        int serverSocketDescriptor, setSocketOptionValue = 1;
        struct sockaddr_in serverAddress;

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

        while (1)
        {
                if((serverSocketDescriptor = socket(serverAddress.sin_family, SOCK_STREAM, 0)) == -1)
                {
                        continue;
                }

                //make the socket reusable again
                setsockopt(serverSocketDescriptor, SOL_SOCKET, SO_REUSEADDR, &setSocketOptionValue, sizeof(setSocketOptionValue));
                //bind an address to a socket
                if((bind(serverSocketDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) == 0)
                {
                        break;
                }
                sleep(5);
        }

        if((listen(serverSocketDescriptor, 99999) != 0))
        {
                fprintf(stderr, "webserver: fatal error getting listening socket\n");
                exit(1);
        }
        return serverSocketDescriptor;
}

char* getRequest(int getSocketDescriptor)
{
        char *request;
        request = malloc(sizeof(char) * buffSize);

        int recv_size = recv(getSocketDescriptor, request, buffSize, 0);

        if(recv_size < 0)
        {
                perror("Internal server error");
                return NULL;
        }
        else if (recv_size == 0)
        {
                perror("Connection Terminated");
                return NULL;
        }
        else
        {
                printf("%s\n", request);
                return request;
        }
}

void sendResponse(int getSocketDescriptor, char *request, char *serverRootDirectoryParameter){
        int getFileDescriptor, n;
        char method[5];

        if(strncmp(request, "HEAD", 4) == 0 || strncmp(request, "head", 4) == 0)
                strncpy(method, "HEAD", 4);
        else if(strncmp(request, "GET", 3) == 0 || strncmp(request, "get", 3) == 0)
                strncpy(method, "GET", 3);
        else
                printFileDetails(getSocketDescriptor, -1, 501, NULL);

        int m = 0;
        for(uint16_t i = 0; i < sizeof(request) && request[i] != '/'; i++)
        {
                if(request[i + 1] == '/')
                {//copying request path to the PathToFile character by character until it face a space
                        for(int j = i + 1; request[j] != ' '; j++)
                        {
                                requestedFile[m] += request[j];
                                m++;
                        }
                }
        }

        char *absolutePathToRequestedFile = createAbsolutePath(requestedFile);
        char *absolutePathToForbiddenDirectory = realpath(forbiddenDirectory, NULL);
        getFileDescriptor = open(absolutePathToRequestedFile, O_RDONLY);

        if (getFileDescriptor < 0) {
                if (strlen(request) > 800)
                        printFileDetails(getSocketDescriptor, getFileDescriptor, 400, method);
                else
                        printFileDetails(getSocketDescriptor, getFileDescriptor, 404, method);
        }
        else if(getFileDescriptor > 0) {
                if (strncmp(absolutePathToRequestedFile, absolutePathToForbiddenDirectory, strlen(absolutePathToForbiddenDirectory)) == 0) {

                        printFileDetails(getSocketDescriptor, getFileDescriptor, 403, method);
                }
                else{
                        //here I want to show index.html page
                        if (strncmp(method, "GET", 3) == 0) {
                                if (strcmp(requestedFile,"/") == 0) {
                                        int openIndexFile = open(defaultPage,O_RDONLY);
                                        printFileDetails(getSocketDescriptor, openIndexFile, 200, method);
                                }
                                else{
                                        printFileDetails(getSocketDescriptor, getFileDescriptor, 200, method);
                                }
                        }
                        else if(strncmp(method, "HEAD", 4) == 0) {
                                if (strcmp(requestedFile,"/") == 0 ) {
                                        int openIndexFile = open(defaultPage,O_RDONLY);
                                        printFileDetails(getSocketDescriptor, openIndexFile, 200, method);
                                }
                                else{
                                        printFileDetails(getSocketDescriptor, getFileDescriptor, 200, method);
                                }
                        }
                }
        }

}

void EmptyconnectionSocketDescriptorF()
{
        for(int i = 0; i < CONNECTIONS; i++)
        {
                connectionSocketDescriptor[i] = -1;
        }
}


void helpF()
{
        printf("-h Show help\n");
        printf("-p Specify a port to the server\n");
        exit(2);
}

int main (int argc, char *argv[]) {
        int Connection = 0, port = defaultServerPort, option = 0;
        struct sockaddr_in connectionAddress;
        socklen_t connectionAddressLength = sizeof(connectionAddress);

        while((option = getopt(argc, argv, "p:h")) != -1)
        {
                switch(option)
                {
                case 'p':
                        port = GetPort(argv[2], defaultServerPort);
                        break;
                case 'h':
                        helpF();
                        break;
                default:
                        helpF();
                        break;
                }
        }
        EmptyconnectionSocketDescriptorF();
        int serverSocketDescriptor = createServer(port);

        while (1)
        {
                printf("Waiting for a connection on a port %d \n\n", port);
                connectionSocketDescriptor[Connection] = accept(serverSocketDescriptor, (SA *) &connectionAddress, &connectionAddressLength);
                if(connectionSocketDescriptor[Connection] < 0)
                {
                        perror("Connection in not established");
                }
                else
                {
                        request = malloc(sizeof(char) * buffSize);
                        request = getRequest(connectionSocketDescriptor[Connection]);
                        pid_t pid = fork();

                        if ( pid == 0 )//creating a new process
                        {
                                close(serverSocketDescriptor);
                                sendResponse(connectionSocketDescriptor[Connection], request, serverRootDirectory);
                                close(connectionSocketDescriptor[Connection]);
                                exit(0);
                        }
                        else
                        {
                                waitpid(pid, NULL, 0);
                                close(connectionSocketDescriptor[Connection]);
                        }
                }
                connectionSocketDescriptor[Connection]=-1;
                while (connectionSocketDescriptor[Connection]!=-1)
                        Connection = (Connection+1)%CONNECTIONS;
        }
        return 0;
}

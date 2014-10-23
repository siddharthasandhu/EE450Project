

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#define BACKLOG 10
#define SIDTPORT "43010"
#define SIDFPORT "24010"
#define SIDPORT "21010" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int phase3(){
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	int NumOfClients=2;
	char s[INET6_ADDRSTRLEN];
	int rv;
	char *clientName;
	char receivedMes[128];
	char *docName;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo("nunki.usc.edu", SIDTPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}
	struct hostent *lh = gethostbyname("nunki.usc.edu");
	printf("The File Server 3 has TCP port number %s",SIDTPORT);
	printf(" and IP address  %s .\n",inet_ntoa(*((struct in_addr *)lh->h_addr)));
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}
	freeaddrinfo(servinfo); // all done with this structure
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	while(1) { // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

			if (recv(new_fd, receivedMes, 128, 0)==-1){
				perror("receive");
			}
			char * pc;
			pc= strtok (receivedMes," ");
			clientName=pc;
			printf("File server 3 received request from %s, ",pc);
			pc = strtok (NULL, " ");
			printf(" for the file %s.\n ",pc);
			docName=pc;
			if (send(new_fd, docName, 20, 0) == -1)
				perror("send");
			printf("File server 1 has sent %s, ",docName);
			printf(" to %s.\n ",clientName);
			close(new_fd);
			exit(0);
		}
		close(new_fd);
		NumOfClients--;// parent doesn't need this
	}
	return 0;
}

int main()
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	char *mes="File_Server3 43010";

	struct addrinfo hints, *servinfo, *p,*servFile,*res;
	int rv,rs;
	int y=1;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	if ((rv = getaddrinfo("nunki.usc.edu", SIDPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	if ((rv = getaddrinfo("nunki.usc.edu", SIDFPORT, &hints, &servFile)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	struct hostent *lh = gethostbyname("nunki.usc.edu");
	printf("The File Server 3 has UDP port number %s",SIDFPORT);
	printf(" and IP address  %s .\n",inet_ntoa(*((struct in_addr *)lh->h_addr)));
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &y,sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);
		}
		res=servFile;
		if (bind(sockfd, res->ai_addr,res->ai_addrlen) == -1){
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	if ((numbytes = sendto(sockfd, mes, strlen(mes), 0,p->ai_addr, p->ai_addrlen)) == -1) {
		printf("hello\n");
		perror("talker: sendto");
		exit(1);
	}
	printf("The registration request from File Server 3 has been sent to the Directory Server.\n ");
	printf("End of Phase 2 for File Server 3.\n ");
	close(sockfd);
	freeaddrinfo(servinfo); // all done with this structure
	freeaddrinfo(servFile);
	phase3();
	return 0;
}


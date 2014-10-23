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
#define SIDFPORT "33010"
#define SIDPORT "31010" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int phase3(char buf1[]){
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	char *doc="Client2 doc2";
	struct hostent *host;
	int rv;
	char PORTNAME[5];
	//char str[];
	char * pch;
	char * fileServName;
	char s[INET6_ADDRSTRLEN];
	pch = strtok (buf1," ");
	fileServName=pch;
	printf("The request by Client 2 is present in %s ",pch);
	pch = strtok (NULL, " ");
	strcpy(PORTNAME,pch);
	PORTNAME[5]='\0';
	printf("and the File Server's TCP port number is %s. \n",PORTNAME);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	printf("End of Phase 2.\n");
	if ((rv = getaddrinfo("nunki.usc.edu", PORTNAME, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	// all done with this structure
	if((send(sockfd, doc, strlen(doc), 0))==-1){
		perror("send");exit(1);
	}
	printf("The file request has been sent to the %s . \n",fileServName);
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");exit(1);
	}
	buf[numbytes] = '\0';

	printf("Client 2 received %s", buf);
	printf(" from %s . \n",fileServName);
	printf("End of Phase 3 for Client 2.\n");

	freeaddrinfo(servinfo);
	close(sockfd);
	return 0;
}

int main()
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	char *mes="Client2 doc2";

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
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	struct hostent *lh = gethostbyname("nunki.usc.edu");
	printf("The Client 2 has UDP port number %s",SIDFPORT);
	printf(" and IP address  %s .\n",inet_ntoa(*((struct in_addr *)lh->h_addr)));
	if ((numbytes = sendto(sockfd, mes, strlen(mes), 0,p->ai_addr, p->ai_addrlen)) == -1) {
		printf("hello\n");
		perror("talker: sendto");
		exit(1);
	}
	printf("The file request from Client 2 has been sent to the Directory Server.\n");
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';
	close(sockfd);
	freeaddrinfo(servinfo); // all done with this structure
	phase3(buf);
	return 0;
}


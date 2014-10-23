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
#define SIDPORT "21010"
#define SIDFPORT "31010"
#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int checkInFile(char *str, char *fileName) {
	int fileCounter = 0;
	int lineCounter = 0;
	int marker = 0;
	int counterLoop = 1;
	int FileServ1 = 0;
	int FileServ2 = 0;
	int FileServ3 = 0;
	int file1 = 0;
	int file2 = 0;
	int file3 = 0;
	int fileCount = 0;
	int lineNum = 0;
	char *serverNames;
	int i = 0;
	static const char filename[] = "resource.txt";
	static const char filename1[] = "topology.txt";
	FILE *file12 = fopen(filename1, "r");
	FILE *file = fopen(filename, "r");

	if (file != NULL) {
		char line[128]; /* or other suitable maximum line size */
		while (fgets(line, sizeof line, file) != NULL) /* read a line */
		{
			//	fputs(line, stdout); /* write the line */
			char * pch;
			char *length;
			pch = strtok(line, " ");
			serverNames = pch;
			pch = strtok(NULL, " ");
			length = pch;
			int i = 0;
			while (i < atoi(length)) {
				//printf("%s\n", pch);
				pch = strtok(NULL, " ");
				if (strcmp(fileName, pch) == 0) {
					if (strcmp(serverNames, "File_Server1") == 0) {
						file1 = 1;
					} else if (strcmp(serverNames, "File_Server2") == 0) {
						file2 = 1;
					} else if (strcmp(serverNames, "File_Server3") == 0) {
						file3 = 1;
					}
					fileCounter++;
				}
				i++;
			}
			lineNum++;
		}
		fclose(file);
	} else {
		perror(filename); /* why didn't the file open? */
	}
	if (strcmp(str, "Client1") == 0) {
		marker = 1;
	}
	if (strcmp(str, "Client2") == 0) {
		marker = 2;
	}
	if (file12 != NULL) {
		char line[128]; /* or other suitable maximum line size */
		char * pch;
		while (fgets(line, sizeof line, file12) != NULL) /* read a line */
		{
			//fputs(line, stdout); /* write the line */
			if (counterLoop == marker) {
				pch = strtok(line, " ");
				FileServ1 = atoi(pch);
				pch = strtok(NULL, " ");
				FileServ2 = atoi(pch);
				pch = strtok(NULL, " ");
				FileServ3 = atoi(pch);
			}
			counterLoop++;
		}
		fclose(file);
	} else {
		perror(filename); /* why didn't the file open? */
	}
	if (FileServ1 < FileServ2 && FileServ1 < FileServ3) {
		return 0;
	}
	if (FileServ2 < FileServ1 && FileServ2 < FileServ3) {
		return 1;
	}
	if (FileServ3 < FileServ2 && FileServ3 < FileServ2) {
		return 2;
	}
}

char* checkAgain(char buf[]) {
	char *str = buf;
	char * pch;
	char *fileName;
	char *stri;
	char *stringInfo=(char*) malloc(128);
	char *temp;
	int fileCounter = 0;
	char line[128];
	pch = strtok(str, " ,.-");
	stri = pch;
	pch = strtok(NULL, " ,.-");
	fileName = pch;
	static const char filename[] = "directory.txt";
	FILE *file = fopen(filename, "r");
	int i = checkInFile(stri, fileName);
	if (file != NULL) {
		/* or other suitable maximum line size */
		while (fgets(line, sizeof line, file) != NULL) /* read a line */
		{

			//fputs(line, stdout); /* write the line */
			if (i == fileCounter) {
				temp = line;
				strcpy(stringInfo,temp);
				break;
			}
			if (i == fileCounter) {
				temp = line;
				strcpy(stringInfo,temp);
				break;
			}
			if (i == fileCounter) {
				temp = line;
				strcpy(stringInfo,temp);
				break;
			}
			fileCounter++;
		}
		fclose(file);
	} else {
		perror(filename); /* why didn't the file open? */
	}
	return stringInfo;
}
int Phase2(){

	int sockfd;
	int backlog;
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo, *res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_DGRAM;
	int numbytes;
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr;
	char *mes;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	int j = 1;

	int clientServ = 1;
	struct hostent *lh = gethostbyname("nunki.usc.edu");
	//Phase 1 of the assignment
	if ((status = getaddrinfo("nunki.usc.edu", SIDFPORT, &hints, &servinfo))
			> 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}
	printf("The Directory Server has UDP port number %s",SIDFPORT);
	printf(" and IP address  %s .\n",inet_ntoa(*((struct in_addr *)lh->h_addr)));
	for (res = servinfo; res != NULL; res = res->ai_next) {
		if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))
				== -1) {
			perror("some issue");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &j, sizeof(int))
				== -1) {
			perror("setsockopt");
			exit(1);
		}
		if ((bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (res == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	//phase 2
	while (1) {
		addr_len = sizeof their_addr;
		numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,	(struct sockaddr *) &their_addr, &addr_len);
		if (numbytes == -1) {
			perror("recvfrom");
			exit(1);
		}
		printf("The Directory Server has received request from File Server %i.\n", clientServ);
		buf[numbytes] = '\0';
		mes = checkAgain(buf);
		printf("File Server details has been sent to client %i . \n",clientServ);
		if ((numbytes = sendto(sockfd, mes, strlen(mes), 0,(struct sockaddr *) &their_addr, addr_len)) == -1) {
			printf("hello\n");
			perror("talker: sendto");
			exit(1);
		}

		clientServ++;
	}
	printf("End of Phase 1 for the directory server.");
	close(sockfd);
	freeaddrinfo(servinfo);
	return 0;
}
int main() {
	FILE *f;
	int sockfd;
	int backlog;
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo, *res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_DGRAM;
	int numbytes;
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr;
	char *mes;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	int j = 1;
	int fileServ = 1;
	struct hostent *lh = gethostbyname("nunki.usc.edu");
	//Phase 1 of the assignment
	if ((status = getaddrinfo("nunki.usc.edu", SIDPORT, &hints, &servinfo))
			> 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}
	printf("The Directory Server has UDP port number %s",SIDPORT);
	printf(" and IP address  %s .\n",inet_ntoa(*((struct in_addr *)lh->h_addr)));
	for (res = servinfo; res != NULL; res = res->ai_next) {
		if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))
				== -1) {
			perror("some issue");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &j, sizeof(int))
				== -1) {
			perror("setsockopt");
			exit(1);
		}
		if ((bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (res == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	int diCount=0;
	if (FILE * file = fopen("directory.txt", "r"))
	{
		remove("directory.txt");

	}
	while (fileServ<4) {
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,	(struct sockaddr *) &their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		printf("The Directory Server has received request from File Server %i.\n",fileServ);
		buf[numbytes] = '\0';

		f = fopen("directory.txt", "a");
		if (!f) {
			return 0;
		}

		fputs(buf, f);
		fprintf(f, "\n");
		fclose(f);
		fileServ++;
	}
	if(diCount==0){
		printf("The directory.txt has been created.\n");
		diCount++;
	}
	printf("End of Phase 1 for the directory server.\n");
	close(sockfd);
	freeaddrinfo(servinfo);
	Phase2();
	return 0;
}

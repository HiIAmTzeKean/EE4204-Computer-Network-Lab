/*******************************
udp_client.c: the source file of the client in udp
********************************/

#include "headsock.h"

void str_cli1(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len);
float str_cli(FILE *fp, int sockfd, long *num_of_bytes, struct sockaddr *addr, int addrlen);

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in ser_addr;
    char **pptr;
    struct hostent *sh;
    struct in_addr **addrs;
    char *DESTINATION_IP = argv[1];
    char *FILENAME = argv[2];
    FILE *fp;

    if (argc != 3) {
        // Require <IP> <FILENAME>
        printf("parameters not match.\n");
        exit(0);
    }

    // get host's information
    if ((sh = gethostbyname(DESTINATION_IP)) == NULL) {
        printf("error when gethostbyname");
        exit(0);
    }

    printf("canonical name: %s\n", sh->h_name);
    for (pptr = sh->h_aliases; *pptr != NULL; pptr++){
        printf("the aliases name is: %s\n", *pptr); // printf socket information
    }
    
    switch (sh->h_addrtype) {
    case AF_INET:
        printf("AF_INET\n");
        break;
    default:
        printf("unknown addrtype\n");
        break;
    }

    addrs = (struct in_addr **)sh->h_addr_list;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); // create socket
    if (sockfd < 0)
    {
        printf("ERROR: Failed to create socket on client\n");
        exit(1);
    }

    // configure server addr
    bzero(&ser_addr,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(MYUDP_PORT);
    memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
    //ser_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // need to estiablish connection with server for ACK message
    int ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));
	if (ret < 0) {
		printf ("ERROR: connection to server failed\n"); 
		close(sockfd); 
		exit(1);
	}

    // check if exist file
    if((fp = fopen (FILENAME,"r+t")) == NULL) {
		printf("File doesn't exit\n");
		exit(0);
	}
    printf("INFO: File: %s, found!\n", FILENAME);

    // str_cli1(stdin, sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in), &len); // receive and send
    long num_of_bytes = 0;
    float timeTaken, dataRate;
    timeTaken = str_cli(fp, sockfd, &num_of_bytes,
                        (struct sockaddr *)&ser_addr,
                        sizeof(struct sockaddr_in));

    dataRate = (num_of_bytes/(float) timeTaken);
    printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n",
            timeTaken, (int)num_of_bytes, dataRate);

    close(sockfd);
    fclose(fp);
    exit(0);
}

long getFileSize(FILE *fp) {
    long FILESIZE;
    fseek (fp , 0 , SEEK_END);
    FILESIZE = ftell (fp);
	rewind (fp);
    return FILESIZE;
}

long sendMessage(int fileSize, char *buf, int sockfd, struct sockaddr *addr, int addrlen) {
    long currentLine = 0;
    int sendLength;
    int n;
    struct ack_so ack;

    char frame[DATALEN];

    while(currentLine <= fileSize) {
		if ((fileSize+1-currentLine) <= DATALEN)
			sendLength = fileSize+1-currentLine;
		else 
			sendLength = DATALEN;
		memcpy(frame, (buf+currentLine), sendLength);
		// n = send(sockfd, &frame, sendLength, 0);
        n = sendto(sockfd,&frame,sendLength,0, addr, addrlen);

		if(n == -1) {
			printf("send error!"); //send the data
			exit(1);
		}
        printf("LOG: frame sent!\n");
		currentLine += sendLength;
	}
    
    //receive the ack
    printf("INFO: Waiting for ACK by server\n");
    n = recvfrom(sockfd,&ack,2, 0,addr,(socklen_t *)&addrlen);

	if (n == -1) {
		printf("error when receiving\n");
		exit(1);
	}
	if (ack.num != 1 || ack.len != 0)
		printf("error in transmission\n");
    return currentLine;
}

void tv_sub(struct  timeval *out, struct timeval *in) {
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

float str_cli(FILE *fp, int sockfd, long *num_of_bytes, struct sockaddr *addr, int addrlen) {
    long FILESIZE;
    char *buf;
    float time_inv = 0.0;
    struct timeval sendTime, recvTime;

    FILESIZE = getFileSize(fp);

    printf("INFO: The file length is %d bytes\n", (int)FILESIZE);
	printf("INFO: the packet length is %d bytes\n",DATALEN);

    // allocate memory to contain the whole file.
	buf = (char *) malloc (FILESIZE);
	if (buf == NULL) {
        exit(2);
    }

    // copy the file into the buffer.
	fread(buf,1,FILESIZE,fp);
    buf[FILESIZE] ='\0';

    // start timer
    gettimeofday(&sendTime, NULL);

    // send message
    *num_of_bytes = sendMessage(FILESIZE,buf,sockfd,addr,addrlen);

    // long currentLine = 0;
    // int sendLength;
    // int n;
    // struct ack_so ack;

    // char frame[DATALEN];

    // while(currentLine <= FILESIZE) {
	// 	if ((FILESIZE+1-currentLine) <= DATALEN)
	// 		sendLength = FILESIZE+1-currentLine;
	// 	else 
	// 		sendLength = DATALEN;
	// 	memcpy(frame, (buf+currentLine), sendLength);
	// 	// n = send(sockfd, &frame, sendLength, 0);
    //     n = sendto(sockfd,&frame,sendLength,0, addr, addrlen);

	// 	if(n == -1) {
	// 		printf("send error!"); //send the data
	// 		exit(1);
	// 	}
    //     printf("LOG: frame sent!\n");
	// 	currentLine += sendLength;
	// }
    
    // //receive the ack
    // printf("INFO: Waiting for ACK by server\n");
    // n = recvfrom(sockfd,&ack,2, 0,addr,(socklen_t *)&addrlen);

	// if (n == -1) {
	// 	printf("error when receiving\n");
	// 	exit(1);
	// }
	// if (ack.num != 1 || ack.len != 0)
	// 	printf("error in transmission\n");
    // *num_of_bytes=currentLine;



    // end timer
    gettimeofday(&recvTime, NULL);
    // calculate total duration
    tv_sub(&recvTime, &sendTime);                                                                 // get the whole trans time
	time_inv += (recvTime.tv_sec)*1000.0 + (recvTime.tv_usec)/1000.0;
	return (time_inv);
}

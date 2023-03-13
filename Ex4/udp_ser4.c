/**************************************
udp_ser.c: the source file of the server in udp transmission
**************************************/
#include "headsock.h"

void str_ser(int sockfd, struct sockaddr *addr, int addr_len);                                                          // transmitting and receiving function

int main(void)
{
	int sockfd;
	struct sockaddr_in my_addr;

    // create socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);          
	if (sockfd<0){
		printf("error in socket!");
		exit(1);
	}

    // Init server address
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8);

    // bind server to socket
    int ret = bind(sockfd, (struct sockaddr*) &my_addr, sizeof(struct sockaddr));
	if (ret == -1) {           //bind socket
		printf("ERROR: error in binding");
		exit(1);
	}

    // Wait and listen
	printf("waiting for data\n");
    str_ser(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)); 
	
	close(sockfd);
	exit(0);
}

void sendAck(int sockfd, struct sockaddr *addr, int addr_len) {
    struct ack_so ack;

    ack.num = 1;
    ack.len = 0;
    if (sendto(sockfd, &ack, 2, 0, addr, addr_len)==-1) {
        printf("ERROR send error!\n");								
        exit(1);
    }
    printf("INFO: sending ACK to client\n");	
}

void str_ser(int sockfd, struct sockaddr *addr, int addr_len)
{
    char buf[BUFSIZE];
	char recvString[DATALEN];
	int n = 0, end = 0;
    long currentLine = 0;
    FILE *fp;

    while(!end)
	{
        n = recvfrom(sockfd, &recvString,DATALEN, 0,
                    addr,(socklen_t *)&addr_len);
		if (n == -1) {
			printf("error when receiving\n");
			exit(1);
		}

        printf("INFO: receiving a frame!\n");
        // check if EOF
        // if EOF then remove the last char
		if (recvString[n-1] == '\0') {
			end = 1;
			n --;
		}
        
        // copy data over to buff
		memcpy((buf+currentLine), recvString, n);
		currentLine += n;

        // send ack 
        sendAck(sockfd,addr,addr_len);
	}

    // save to file
    if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL) {
		printf("File doesn't exit\n");
		exit(0);
	}
    printf("INFO: writing to file\n");	
	fwrite (buf,1,currentLine,fp);
	fclose(fp);
	printf("INFO: file of %d bytes has been saved\n", (int)currentLine);
}

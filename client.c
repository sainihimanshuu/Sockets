#include<stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 80
#define MAXDATASIZE 100

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family==AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[]){
    struct addrinfo hints, *res, *p;
    int sockfd;
    char dst[INET6_ADDRSTRLEN], buf[MAXDATASIZE];
    int numbytes;

    if(argc!=2){
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;

    if(getaddrinfo(argv[1], PORT, &hints, &res)!=0){
        perror("client: getaddrinfo");
        exit(1);
    }

    for(p=res;p!=NULL;p=p->ai_next){
        sockfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sockfd==-1){
            perror("client: socket");
            continue;
        }

        inet_ntop(p->ai_family, get_in_addr(p->ai_addr), dst, sizeof dst);
        printf("client: attempting connection to %s\n", dst);

        if(connect(sockfd, p->ai_addr, p->ai_addrlen)==-1){
            perror("client: connect");
            close(sockfd);
            continue;
        }
    }

    if(p==NULL){
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), dst, sizeof dst);
    printf("client: connected to %s\n", dst);

    freeaddrinfo(res);

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);

    close(sockfd);

    return 0;
}
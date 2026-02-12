#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

#define PORT 80
#define BACKLOG 10

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family==AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void sigchld_handler(int s){
    (void)s;
    
    int saved_errno=errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno=saved_errno;
}

int main(){
    struct addrinfo hints, *res, *p;
    struct sockaddr_storage their_addr; 
    int sockfd, new_fd;
    char dst[INET6_ADDRSTRLEN], s[INET6_ADDRSTRLEN];
    struct sigaction sa;

    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;

    if(getaddrinfo(NULL, PORT, &hints, &res)!=0){
        perror("getaddrinfo");
        exit(1);
    }

    for(p=res;p!=NULL;p=p->ai_next){
        sockfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sockfd==-1){
            perror("socket");
            continue;
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen)!=0){
            close(sockfd);
            perror("bind");
            exit(1);
        }

        inet_ntop(p->ai_family, get_in_addr(p->ai_addr), dst, sizeof dst);
        printf("Server will be listening at %s", dst);

        break;
    }

    freeaddrinfo(res);

    if(p==NULL){
        fprintf(stderr, "server failed to bind\n");
        exit(1);
    }

    if(listen(sockfd, BACKLOG)==-1){
        perror("listen");
        exit(1);
    }

    sa.sa_handler=sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL)==-1){
        perror("sigaction");
        exit(1);
    }

    printf("waiting for server connections\n");

    while(1){
        int sin_size=sizeof their_addr;
        new_fd=accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if(new_fd==-1){
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("accepted connection from %s", s);

        if(!fork()){
            close(sockfd);
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }

    return 0;
}
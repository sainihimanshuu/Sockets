#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "80"

int main(int argc, char *argv[]){
    struct addrinfo hints, *res, *p;
    struct sockaddr_in *ipv4;
    struct sockaddr_in6 *ipv6;
    char ipstr[INET6_ADDRSTRLEN];
    void *addr;

    //hints.ai_family=AF_INET;
    hints.ai_family=AF_UNSPEC; //can be any, ipv4 or ipv6
    hints.ai_socktype=SOCK_STREAM;
    //hints.ai_flags=AI_PASSIVE;

    if (argc != 2) {
        fprintf(stderr,"usage: showip hostname\n");
        return 1;
    }

    if(getaddrinfo(argv[1], PORT, &hints, &res)!=0){
        fprintf(stderr, "error while getting addr info");
        return -1;
    }

    printf("IP addresses for %s:\n", argv[1]);

    for(p=res;p!=NULL;p=p->ai_next){

        if(p->ai_family==AF_INET){
            ipv4=(struct sockaddr_in *)p->ai_addr;
            addr=&(ipv4->sin_addr);
        } else if(p->ai_family==AF_INET6){
            ipv6=(struct sockaddr_in6 *)p->ai_addr;
            addr=&(ipv6->sin6_addr);
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("%s\n", ipstr);
    }

    return 0;
}
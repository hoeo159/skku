#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>

#define MAXCLNT 1024
#define MAXSEAT 256

int isUserActive[MAXCLNT];
int listUserPassword[MAXCLNT];
int isSeatReserve[MAXSEAT];
fd_set readset, copyset;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

typedef struct _query{
    int user; // 0 ~ 1023
    int action; // 0: termination, 1 ~ 5: actions, -1: out of range
    int data;
}   query;

void *query_handler(void *arg){
    int client_socket = *((int *)arg);
    query q;
    int n;
    while((n = read(client_socket, &q, sizeof(q))) > 0){
        pthread_mutex_lock(&m);

        if (!(q.user | q.action | q.data)){
            for(int idx = 0; idx < MAXCLNT; idx++){
                if(isUserActive[idx] == client_socket){
                    isUserActive[idx] = -1;
                }
            }
            write(client_socket, isSeatReserve, sizeof(isSeatReserve));
        }
        else if(q.action == 1){
            int num = 0;
            if(isUserActive[q.user] > 0){
                num = -1;
            }
            else{
                if(listUserPassword[q.user] < 0){
                    listUserPassword[q.user] = q.data;
                    isUserActive[q.user] = client_socket;
                    num = 1;
                }
                else{
                    if(listUserPassword[q.user] == q.data){
                        isUserActive[q.user] = client_socket;
                        num = 1;
                    }
                    else{
                        num = -1;
                    }
                }
            }
            write(client_socket, &num, sizeof(num));
        }
        else if(q.action == 2){ // reserve seat
            int num = 0;
            if(q.data < 0 || q.data > 255){
                num = -1;
            }
            else if(isUserActive[q.user] != client_socket){
                num = -1;
            }
            else if(isSeatReserve[q.data] > 0){
                num = -1;
            }
            else{
                isSeatReserve[q.data] = q.user;
                num = q.data;
            }
            write(client_socket, &num, sizeof(num));
        }
        else if(q.action == 3){ // check reservation
            int num = 0;
            if(isUserActive[q.user] != client_socket){
                num = -1;
            }
            else if(isSeatReserve[q.data] != q.user){
                num = -1;
            }
            else{
                num = q.data;
            }
            write(client_socket, &num, sizeof(num));
        }
        else if(q.action == 4){
            int num = 0;
            if(isUserActive[q.user] != client_socket){
                num = -1;
            }
            else if(isSeatReserve[q.data] != q.user){
                num = -1;
            }
            else{
                isSeatReserve[q.data] = -1;
                num = q.data;
            }
            write(client_socket, &num, sizeof(num));
        }
        else if(q.action == 5){
            int num = 0;
            if(isUserActive[q.user] != client_socket){
                num = -1;
            }
            else{
                isUserActive[q.user] = -1;
                num = 1;
            }
            write(client_socket, &num, sizeof(num));
        }
        else{
            int num = -1;
            write(client_socket, &num, sizeof(num));
        }
    
        pthread_mutex_unlock(&m);
    }

    FD_CLR(client_socket, &readset);
    close(client_socket);
    pthread_exit(NULL);
}

int main (int argc, char *argv[]) {
    int listenfd, connfd, caddrlen;
    struct sockaddr_in saddr, caddr;
    pthread_t tid[MAXCLNT];
    int threadNum = 0;

    int port = atoi(argv[1]);

    memset(isSeatReserve, -1, sizeof(isSeatReserve));
    memset(listUserPassword, -1, sizeof(listUserPassword));
    memset(isUserActive, -1, sizeof(isUserActive));

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket() failed.\n");
        exit(1);
    }

    memset((char *)&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        printf("bind() failed.\n");
        exit(2);
    }

    if (listen(listenfd, 5) < 0) {
        printf("listen() failed.\n");
        exit(3);
    }

    FD_ZERO(&readset);
    FD_SET(listenfd, &readset);
    int fdmax = listenfd;
    int fdnum;

    while (1) {
        copyset = readset;

        if((fdnum = select(fdmax + 1, &copyset, NULL, NULL, NULL)) < 0){
            printf("select() failed!\n");
            exit(0);
        }

        for(int i = 0; i < fdmax + 1; i++){
            if(FD_ISSET(i, &copyset)){
                if(i == listenfd){
                    // 맨 처음에 입장할 때, connfd가 입장한 클라
                    caddrlen = sizeof(caddr);
                    if ((connfd = accept(listenfd, (struct sockaddr *)&caddr, (socklen_t *)&caddrlen)) < 0) {
                        printf ("accept() failed.\n");
                        continue;
                    }
                    
                    FD_SET(connfd, &readset);

                    if (fdmax < connfd){
                        fdmax = connfd;
                    }

                    pthread_create(&tid[threadNum], NULL, query_handler, (void *)&connfd);
                    pthread_detach(tid[threadNum]);
                    threadNum++;
                }
            }
        }
    }

    for (int i = 0; i < threadNum; i++) {
        pthread_join(tid[i], NULL);
    }

    close(listenfd);
    return 0;
}

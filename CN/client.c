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

#include <sys/msg.h>
#include <sys/ipc.h>

#define SHMSIZE 4096
#define MAXLINE 512

#define MSGKEY 0x12345

// 52.79.217.87 50000
// 54.180.150.212 50000

// 3.223.251.83 8080

struct msgbuf {
    long mtype;
    char mbuff[MAXLINE];
};


int main (int argc, char *argv[]) {
    int n, cfd;
    struct hostent *h;
    struct sockaddr_in saddr;
    char buf[MAXLINE];
    char *host = argv[1];
    int port = atoi(argv[2]);

    if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket() failed.\n"); 
        exit(1);
    }

    if ((h = gethostbyname(host)) == NULL) {
        printf("invalid hostname %s\n", host); 
        exit(2);
    }

    memset((char *)&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    memcpy((char *)&saddr.sin_addr.s_addr, (char *)h->h_addr, h->h_length);
    saddr.sin_port = htons(port); // host데이터를 network short? 으로 바꾼다

    if (connect(cfd,(struct sockaddr *)&saddr,sizeof(saddr)) < 0) { // 구조체 크기가 같아서 강제 형변환 가능!!)
        printf("connect() failed.\n");
        exit(3);
    }

    fd_set readset, copyset;
    FD_ZERO(&readset);
    FD_SET(0, &readset);
    FD_SET(cfd, &readset);
    int fdmax = cfd;
    int fdnum;

    int msgid;
    struct msgbuf msg;

    if((msgid = msgget(MSGKEY, IPC_CREAT | 0666)) < 0){
        perror("msgget() failed");
        exit(1);
    }
    msg.mtype = 1;


    while (1) {
        copyset = readset;

        if((fdnum = select(fdmax + 1, &copyset, NULL, NULL, NULL)) < 0){
            printf("select() failed.\n");
            exit(4);
        }

        // get a message from message queue and send it to server
        if((n = msgrcv(msgid, &msg, MAXLINE, 0, IPC_NOWAIT)) > 0){
            write(cfd, msg.mbuff, n);
            memset(msg.mbuff, 0, MAXLINE);
        }

        if(FD_ISSET(0, &copyset)){
            // cli > server
            if((n = read(0, buf, MAXLINE)) > 0){
                buf[strlen(buf) - 1] = '\0';
                if(strcmp(buf, "quit") == 0){
                    break;
                }
                else{
                    buf[strlen(buf)] = '\n';
                    write(cfd, buf, n);
                }
            }
        }

        if(FD_ISSET(cfd, &copyset)){
            // ser > cli
            if((n = read(cfd, buf, MAXLINE)) > 0){
                write(0, buf, n);
            }
        }

        memset(buf, 0, MAXLINE);
    }

    if(msgctl(msgid, IPC_RMID, NULL) < 0){
        perror("msgctl() failed");
        exit(1);
    }

    close(cfd);
}

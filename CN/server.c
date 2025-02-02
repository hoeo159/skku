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

#define MAXLINE 256
#define MAXINPUT 128

#define MSGKEY 0x12345

// 3.223.251.83 8080
// nc -vz 52.79.217.87 50000
// nc -vz 54.180.150.212 50000

struct msgbuf {
    long mtype;
    char mbuff[MAXLINE];
};

int main (int argc, char *argv[]) {
    int n, listenfd, connfd, caddrlen, fw, mode = 0;
    struct sockaddr_in saddr, caddr;
    char buf[MAXINPUT];
    char randnum[5][MAXLINE];
    char shmData[MAXLINE];
    int port = atoi(argv[1]);
    int id[MAXINPUT], userNum = 0, cntNum = 0, flag = 0, N = 5;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket() failed.\n");
        exit(1);
    }

    for(int i = 0; i < 5; i++){
        memset(randnum[i], 0, MAXLINE);
    }

    memset((char *)&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        printf("bind() failed.\n");
        exit(2);
    }

    // listen (sockfd, backlog)
    // backlog is the number of clients that can wait for the server to accept
    if (listen(listenfd, 3) < 0) {
        printf("listen() failed.\n");
        exit(3);
    }

    fd_set readset, copyset;
    FD_ZERO(&readset);
    FD_SET(listenfd, &readset);
    int fdmax = listenfd;
    int fdnum;

    int msgid;
    struct msgbuf msg;

    if((msgid = msgget(MSGKEY, IPC_CREAT | 0666)) < 0){
        perror("msgget() failed");
        exit(1);
    }

    while (1) {
        copyset = readset;

        if((fdnum = select(fdmax + 1, &copyset, NULL, NULL, NULL)) < 0){
            printf("select() failed!\n");
            exit(0);
        }

        for(int i = 0; i < fdmax + 1; i++){
            if(FD_ISSET(i, &copyset)){
                if(i == listenfd){
                    caddrlen = sizeof(caddr);
                    if ((connfd = accept(listenfd, (struct sockaddr *)&caddr, (socklen_t *)&caddrlen)) < 0) {
                        printf ("accept() failed.\n");
                        continue;
                    }

                    FD_SET(connfd, &readset);

                    if (fdmax < connfd){
                        fdmax = connfd;
                    }

                    // new user joined
                    char strbuf[MAXLINE];
                    id[connfd] = 1;
                    userNum++;
                    snprintf(strbuf, sizeof(strbuf), "new joined. %d current users, got %d tokens\n", userNum, cntNum);
                    printf("%s", strbuf);
                }
                else{
                    // read from client and show at server
                    if((n = read(i, buf, MAXINPUT)) > 0) {
                        snprintf(randnum[cntNum], n, "%s", buf);
                        write(0, randnum[cntNum], n);
                        write(0, "\n", 1);
                        cntNum++;
                        if(cntNum == N){
                            flag = 1;
                            break;
                        }
                    }
                    else {
                        // user leaved
                        FD_CLR(i, &readset);
                        id[i] = 0;
                        char strbuf[MAXLINE];
                        userNum--;
                        printf("someone out\n");
                    }
                }
            }
        }
        
        memset(buf, 0, MAXLINE);
        
        if(flag == 1){
            break;
        }
    }
    
    printf("Server closed\n");
    for(int i = 0; i < N; i++){
        printf("%s", randnum[i]);
        int len = strlen(randnum[i]);
        if (len > 0 && randnum[i][len - 1] == '\n') {
            randnum[i][len - 1] = '\0';
        }
        strcat(shmData, randnum[i]);

        if(i != N - 1){
            strcat(shmData, ",");
        }
    }

    // send the shmData to message queue
    msg.mtype = 1;
    strcpy(msg.mbuff, shmData);
    if(msgsnd(msgid, &msg, strlen(shmData), 0) < 0){
        perror("msgsnd() failed");
        exit(1);
    }

    close(listenfd);
    close(connfd);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h> 
#include <sys/ipc.h> 

#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <time.h>

#define MAX_LEN 200
#define MAX_IPC 3
#define BUF_SIZE 1024

pid_t pid, child_pid, npid;
int exitnum;

char* cmd_type1[] = {"ls", "man", "grep", "sort", "awk", "bc"};
char* cmd_type2[] = {"head", "tail", "cat"};
char* cmd_type3[] = {"mv", "rm", "cp", "cd"};
char* cmd_type4[] = {"pwd", "exit"};
char* ipc_format[] = {"|", "<", ">", ">>"};
char* cmd_func[] = {"head", "tail", "cat", "cp", "mv", "rm", "cd", "pwd", "exit"};
char* cmd_notbin[] = {"man", "sort", "awk", "bc", "head", "tail"};

int find_ipc_format(char* arg){
    for(int i = 0; i < strlen(arg); i++){
        if(arg[i] == '|'){
            return 0;
        }
    }

    for(int i = 0; i < 4; i++){
        if(strcmp(arg, ipc_format[i]) == 0){
            return i;
        }
    }
    return -1;
}

int find_notbin_cmd(char** arg, int argNum){
    for(int i = 0; i < 6; i++){
        for(int j = 0; j < argNum; j++){
            if(strcmp(arg[j], cmd_notbin[i]) == 0){
                return -1;
            }
        }
    }
    return 1;
}

void signal_sigint(int sig){
    if (sig == SIGINT) {
        printf("\n");
    }
}

void signal_sigtstp(int sig){
    printf("\n");
    if(npid == 0){
        kill(npid, SIGUSR1);
        exit(0);
    }
    else{
        int status;
        child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED);

        if(WIFSTOPPED(status)){
            kill(child_pid, SIGKILL);
        }
    }
}

void signal_sigusr1(int sig){
    exit(0);
}

void func_cat(int in_fd, int out_fd){
    char buffer[BUF_SIZE];
    ssize_t bytesRead;

    while((bytesRead = read(in_fd, buffer, BUF_SIZE)) > 0){
        if(write(out_fd, buffer, bytesRead) != bytesRead){
            perror("Error write\n");
            exit(1);
        }
    }

    if(bytesRead == -1){
        perror("Error read file\n");
        exit(1);
    }
}

void func_cp(char* src, char* dest){
    int src_fd = open(src, O_RDONLY);
    if(src_fd == -1){
        perror("Error open src file\n");
        exit(1);
    }

    int dest_fd = open(dest, O_CREAT | O_WRONLY | O_TRUNC, 0755);
    if(dest_fd == -1){
        perror("Error open dest file\n");
        exit(1);
    }

    char buffer[BUF_SIZE];
    ssize_t bytesRead, bytesWrite;

    while((bytesRead = read(src_fd, buffer, sizeof(buffer))) > 0){
        bytesWrite = write(dest_fd, buffer, bytesRead);
        if(bytesWrite != bytesRead){
            perror("Error write dest file\n");
            close(src_fd);
            close(dest_fd);
            exit(1);
        }
    }

    if(bytesRead == -1){
        perror("Error read src file");
        close(src_fd);
        close(dest_fd);
        exit(1);
    }

    close(src_fd);
    close(dest_fd);
}

void func_head(int in_fd, int out_fd, int line){
    char buffer[BUF_SIZE];
    ssize_t bytesRead;
    int lineCnt = 0;
    while(lineCnt < line && (bytesRead = read(in_fd, buffer, 1) > 0)){
        write(out_fd, buffer, 1);
        if(buffer[0] == '\n'){
            lineCnt++;
        }
    }

    if(bytesRead == -1){
        perror("Error read file\n");
        exit(1);
    }
}

void func_tail(int in_fd, int out_fd, int line){
    char buffer[BUF_SIZE];
    ssize_t bytesRead;
    int total_line = 0;

    while((bytesRead = read(in_fd, buffer, 1) > 0)){
        if(buffer[0] == '\n'){
            total_line++;
        }
    }

    lseek(in_fd, 0, SEEK_SET);

    int lineCnt = total_line - line;

    while(lineCnt > 0 && (bytesRead = read(in_fd, buffer, 1) > 0)){
        if(buffer[0] == '\n'){
            lineCnt--;
        }
    }
    while((bytesRead = read(in_fd, buffer, 1) > 0)){
        write(out_fd, buffer, 1);
    }

    if(bytesRead == -1){
        perror("Error read file\n");
        exit(1);
    }
}

void func_pwd(int out_fd){
    char pathname[BUF_SIZE] = {0, };
    if(getcwd(pathname, BUF_SIZE) == NULL){
        perror("Error pwd\n");
        exit(1);
    }
    if(write(out_fd, pathname, strlen(pathname)) == -1){
        perror("Error write\n");
        exit(1);
    }
    if(write(out_fd, "\n", 1) == -1){
        perror("Error write\n");
        exit(1);
    }
}

void head(int* in_fd, int* out_fd, char** arg, int index){
    char filename[MAX_LEN];
    int line = 0;
    int flag = 0;
    if(index > 2){
        if(strcmp(arg[1], "-n") == 0){
            if(*in_fd == 0 && arg[3] != NULL){
                strcpy(filename, arg[3]);
                flag = 1;
            }
            line = atoi(arg[2]);
        }
    }
    else{
        if(*in_fd == 0 && arg[1] != NULL){
            strcpy(filename, arg[1]);
            flag = 1;
        }
        line = 10;
    }

    if(flag == 1){
        *in_fd = open(filename, O_RDONLY);
        if(*in_fd == -1){
            perror("Error read file\n");
            exit(1);
        }
    }
    func_head(*in_fd, *out_fd, line);
}

void tail(int* in_fd, int* out_fd, char** arg, int index){
    char filename[MAX_LEN];
    int line = 0;
    int flag = 0;
    if(index > 1){
        if(strcmp(arg[1], "-n") == 0){
            if(*in_fd == 0 && arg[3] != NULL){
                strcpy(filename, arg[3]);
                flag = 1;
            }
            line = atoi(arg[2]);
        }
    }
    else{
        if(*in_fd == 0 && arg[1] != NULL){
            strcpy(filename, arg[1]);
            flag = 1;
        }
        line = 10;
    }

    if(flag == 1){
        *in_fd = open(filename, O_RDONLY);
        if(*in_fd == -1){
            perror("Error read file\n");
            exit(1);
        }
    }
    func_tail(*in_fd, *out_fd, line);
}

int main(){
    int child_status;
    int type = 0;
    size_t size;
    int fd[2] = {0, };

    npid = getpid();

    // signal(SIGINT, signal_sigint);
    // signal(SIGTSTP, signal_sigtstp);
    // signal(SIGUSR1, signal_sigusr1);

    while(1){
        char* cmd = NULL;
        char cmdcpy[MAX_LEN];
        int in_fd = 0;
        int out_fd = 1;

        getline(&cmd, &size, stdin);
        type = 0;

        if(strcmp(cmd, "\n") == 0)
            continue;
        cmd[strlen(cmd) - 1] = '\0';
        
        strcpy(cmdcpy, cmd);

        char *arg[MAX_LEN], *leftarg[MAX_LEN], *rightarg[MAX_LEN];
        char cpyarg[MAX_LEN][MAX_LEN];
        char *pipe_arg[MAX_LEN], ipcCnt = 0;
        char cmd_arg[4][MAX_LEN] = {0, };
        int argNum = 0, ipcNum = 0;
        char path[MAX_LEN];
        int ipcIndex[MAX_IPC];
        char ipc_arg[MAX_IPC][3] = {0, };
        int cmdidx =  0;

        for(int i = 0; i < strlen(cmd); i++){
            if(cmd[i] == ' '){
                if(cmdidx > 0){
                    cpyarg[argNum][cmdidx] = '\0';
                    arg[argNum] = strdup(cpyarg[argNum]);
                    argNum++;
                    cmdidx = 0;
                }
            }
            else if(cmd[i] == '|' || cmd[i] == '<' || cmd[i] == '>'){
                if(cmdidx > 0){
                    cpyarg[argNum][cmdidx] = '\0';
                    arg[argNum] = strdup(cpyarg[argNum]);
                    argNum++;
                    cmdidx = 0;
                    cpyarg[argNum][cmdidx++] = cmd[i];
                    if(cmd[i] == '>' && cmd[i+1] == '>'){
                        cpyarg[argNum][cmdidx++] = cmd[i + 1];
                        i++;
                    }
                    cpyarg[argNum][cmdidx] = '\0';
                    arg[argNum] = strdup(cpyarg[argNum]);
                    argNum++;
                    cmdidx = 0;
                }
                else{
                    cpyarg[argNum][cmdidx++] = cmd[i];
                    if(cmd[i] == '>' && cmd[i+1] == '>'){
                        cpyarg[argNum][cmdidx++] = cmd[i + 1];
                        i++;
                    }
                    cpyarg[argNum][cmdidx] = '\0';
                    arg[argNum] = strdup(cpyarg[argNum]);
                    argNum++;
                    cmdidx = 0;
                }
            }
            else{
                cpyarg[argNum][cmdidx] = cmd[i];
                cmdidx++;
            }
        }

        if(cmdidx > 0){
            cpyarg[argNum][cmdidx] = '\0';
            arg[argNum] = strdup(cpyarg[argNum]);
            argNum++;
            cmdidx = 0;
        }
        arg[argNum] = '\0';

        for(int i = 0; i < argNum; i++){
            if(find_ipc_format(arg[i]) >= 0){
                cmd_arg[ipcNum][strlen(cmd_arg[ipcNum]) - 1] = '\0';
                ipcIndex[ipcNum] = i;
                strcpy(ipc_arg[ipcNum], arg[i]);
                ipcNum++;
            }
            else{
                strcat(cmd_arg[ipcNum], arg[i]);
                strcat(cmd_arg[ipcNum], " ");
            }
        }

        cmd_arg[ipcNum][strlen(cmd_arg[ipcNum])] = '\0';

        if(strcmp(arg[0], "quit") == 0){
            free(cmd);
            exit(0);
        }
        if(strcmp(arg[0], "exit") == 0){
            free(cmd);
            if(argNum > 1){
                exitnum = atoi(arg[1]);
                fprintf(stderr, "exit\n");
                exit(exitnum);
            }
            else{
                fprintf(stderr, "exit\n");
                exit(0);
            }
        }

        if(ipcNum == 0){
            pid = fork();
            if(pid == 0){
                if(strcmp(arg[0], "cat") == 0){
                    char filename[MAX_LEN];
                    strcpy(filename, arg[1]);
                    in_fd = open(filename, O_RDONLY);
                    if(in_fd == -1){
                        perror("Error read file\n");
                        exit(1);
                    }
                    func_cat(in_fd, STDOUT_FILENO);
                    exit(0);
                }
                else if(strcmp(arg[0], "head") == 0){
                    head(&in_fd, &out_fd, arg, argNum);
                    exit(0);
                }
                else if(strcmp(arg[0], "tail") == 0){
                    tail(&in_fd, &out_fd, arg, argNum);
                    exit(0);
                }
                else if(strcmp(arg[0], "cd") == 0){
                    if(chdir(arg[1]) == -1){
                        printf("cd: %s\n", strerror(errno));
                    }
                    exit(0);
                }
                else if(strcmp(arg[0], "cp") == 0){
                    func_cp(arg[1], arg[2]);
                    exit(0);
                }
                else if(strcmp(arg[0], "rm") == 0){
                    if(unlink(arg[1]) == -1){
                        printf("rm : %s\n", strerror(errno));
                    }
                    exit(0);
                }
                else if(strcmp(arg[0], "mv") == 0){
                    if(rename(arg[1], arg[2]) == -1){
                        printf("mv : %s\n", strerror(errno));
                    }
                    exit(0);
                }
                else if(strcmp(arg[0], "pwd") == 0){
                    func_pwd(out_fd);
                    exit(0);
                }
                else if(arg[0][0] == '.' && arg[0][1] == '/'){
                    sprintf(path, "%s", &arg[0][2]);
                }
                else if(find_notbin_cmd(arg, argNum) > 0){
                    sprintf(path, "/bin/%s", arg[0]);
                }
                else{
                    sprintf(path, "/usr/bin/%s", arg[0]);
                }

                if(execv(path, arg) == -1){
                    printf("swsh: Command not found\n");
                    exit(1);
                }
            }
            else{
                wait(NULL);
                if(strcmp(arg[0], "exit") == 0){
                    break;
                }
            }
            
        }
        else if(ipcNum > 0){
            if(strchr(cmdcpy, '|') != NULL){
                int pipeNum = 0;
                int startflag = 0, endflag = 0;

                for(int i = 0; i < ipcNum; i++){
                    if(ipc_arg[i][0] == '|'){
                        pipeNum++;
                    }
                }

                for(int i = 0; i <= ipcNum; i++){
                    int leftidx = 0, rightidx = 0;
                    int tmpidx;
                    type = 0;

                    if(ipc_arg[ipcNum - 1][0] == '>'){
                        if(i == ipcNum){
                            break;
                        }
                    }

                    if(startflag == 1){
                        tmpidx = i - 1;
                        startflag = 0;
                    }
                    else{
                        tmpidx = i;
                    }

                    char *copy = strdup(cmd_arg[tmpidx]);
                    char *ptr = strtok(copy, " ");
                    while (ptr != NULL) {
                        leftarg[leftidx] = ptr;
                        ptr = strtok(NULL, " ");
                        leftidx++;
                    }
                    leftarg[leftidx] = NULL;

                    char *_copy = strdup(cmd_arg[(i + 1) % (ipcNum + 1)]);
                    char *_ptr = strtok(_copy, " ");
                    while (_ptr != NULL) {
                        rightarg[rightidx] = _ptr;
                        _ptr = strtok(NULL, " ");
                        rightidx++;
                    }
                    rightarg[rightidx] = NULL;

                    if(i == 0 && ipc_arg[0][0] == '<'){
                        in_fd = open(rightarg[0], O_RDONLY);
                        if(in_fd == -1){
                            perror("Error read file\n");
                            exit(1);
                        }
                        startflag = 1;
                        continue;
                    }

                    pipe(fd);
                    pid = fork();

                    if(pid == 0){
                        if(i != ipcNum){
                            dup2(fd[1], 1);
                        }
                        if(in_fd != 0){
                            dup2(in_fd, 0);
                        }

                        if(i == ipcNum - 1 && ipc_arg[ipcNum - 1][0] == '>'){
                            // >> 처리하기
                            if(strcmp(ipc_arg[ipcNum - 1], ">>") == 0){
                                out_fd = open(rightarg[0], O_CREAT | O_WRONLY | O_APPEND, 0755);
                                dup2(out_fd, 1);           

                                if(leftarg[0][0] == '.' && leftarg[0][1] == '/'){
                                    sprintf(path, "%s", &leftarg[0][2]);
                                }
                                else if(strcmp(leftarg[0], "cat") == 0){
                                    if(in_fd == 0 && leftarg[1] != NULL){
                                        in_fd = open(leftarg[1], O_RDONLY);
                                        if(in_fd == -1){
                                            perror("Error open file\n");
                                            exit(1);
                                        }
                                    }
                                    func_cat(in_fd, out_fd);
                                    exit(0);                               
                                }
                                else if(strcmp(leftarg[0], "head") == 0){
                                    head(&in_fd, &out_fd, leftarg, leftidx);
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "tail") == 0){
                                    tail(&in_fd, &out_fd, leftarg, leftidx);
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "cd") == 0){
                                    if(chdir(leftarg[1]) == -1){
                                        printf("cd: %s\n", strerror(errno));
                                    }
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "cp") == 0){
                                    func_cp(leftarg[1], leftarg[2]);
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "rm") == 0){
                                    if(unlink(leftarg[1]) == -1){
                                        printf("rm : %s\n", strerror(errno));
                                    }
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "mv") == 0){
                                    if(rename(leftarg[1], leftarg[2]) == -1){
                                        printf("mv : %s\n", strerror(errno));
                                    }
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "pwd") == 0){
                                    func_pwd(out_fd);
                                    exit(0);
                                }
                                else if(find_notbin_cmd(leftarg, leftidx) > 0){
                                    sprintf(path, "/bin/%s", leftarg[0]);
                                }
                                else{
                                    sprintf(path, "/usr/bin/%s", leftarg[0]);
                                }

                                if(execv(path, leftarg) == -1){
                                    printf("swsh: Command not found\n");
                                    exit(1);
                                }
                                endflag = 1;
                            }
                            else{
                                out_fd = open(rightarg[0], O_CREAT | O_WRONLY | O_TRUNC, 0755);
                                dup2(out_fd, 1);
                                
                                if(leftarg[0][0] == '.' && leftarg[0][1] == '/'){
                                    sprintf(path, "%s", &leftarg[0][2]);
                                }
                                else if(strcmp(leftarg[0], "cat") == 0){
                                    if(in_fd == 0 && leftarg[1] != NULL){
                                        in_fd = open(leftarg[1], O_RDONLY);
                                        if(in_fd == -1){
                                            perror("Error open file\n");
                                            exit(1);
                                        }
                                    }
                                    func_cat(in_fd, out_fd);
                                    exit(0);                               
                                }
                                else if(strcmp(leftarg[0], "head") == 0){
                                    head(&in_fd, &out_fd, leftarg, leftidx);
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "tail") == 0){
                                    tail(&in_fd, &out_fd, leftarg, leftidx);
                                    exit(0);
                                }
                                else if(find_notbin_cmd(leftarg, leftidx) > 0){
                                    sprintf(path, "/bin/%s", leftarg[0]);
                                }
                                else if(strcmp(leftarg[0], "cd") == 0){
                                    if(chdir(leftarg[1]) == -1){
                                        printf("cd: %s\n", strerror(errno));
                                    }
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "cp") == 0){
                                    func_cp(leftarg[1], leftarg[2]);
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "rm") == 0){
                                    if(unlink(leftarg[1]) == -1){
                                        printf("rm : %s\n", strerror(errno));
                                    }
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "mv") == 0){
                                    if(rename(leftarg[1], leftarg[2]) == -1){
                                        printf("mv : %s\n", strerror(errno));
                                    }
                                    exit(0);
                                }
                                else if(strcmp(leftarg[0], "pwd") == 0){
                                    func_pwd(out_fd);
                                    exit(0);
                                }
                                else if(find_notbin_cmd(leftarg, leftidx) > 0){
                                    sprintf(path, "/bin/%s", leftarg[0]);
                                }
                                else{
                                    sprintf(path, "/usr/bin/%s", leftarg[0]);
                                }

                                if(execv(path, leftarg) == -1){
                                    printf("swsh: Command not found\n");
                                    exit(1);
                                }
                                endflag = 1;
                            }
                        }
                        else{
                            close(fd[0]);
                        }
                        if(endflag != 1){
                            if(leftarg[0][0] == '.' && leftarg[0][1] == '/'){
                                sprintf(path, "%s", &leftarg[0][2]);
                            }
                            else if(strcmp(leftarg[0], "cat") == 0){
                                if(in_fd == 0 && leftarg[1] != NULL){
                                    in_fd = open(leftarg[1], O_RDONLY);
                                    if(in_fd == -1){
                                        perror("Error open file\n");
                                        exit(1);
                                    }
                                }
                                func_cat(in_fd, out_fd);
                                exit(0);                            
                            }
                            else if(strcmp(leftarg[0], "head") == 0){
                                head(&in_fd, &out_fd, leftarg, leftidx);
                                exit(0);
                            }
                            else if(strcmp(leftarg[0], "tail") == 0){
                                tail(&in_fd, &out_fd, leftarg, leftidx);
                                exit(0);
                            }
                            else if(strcmp(leftarg[0], "cd") == 0){
                                if(chdir(leftarg[1]) == -1){
                                    printf("cd: %s\n", strerror(errno));
                                }
                                exit(0);
                            }
                            else if(strcmp(leftarg[0], "cp") == 0){
                                func_cp(leftarg[1], leftarg[2]);
                                exit(0);
                            }
                            else if(strcmp(leftarg[0], "rm") == 0){
                                if(unlink(leftarg[1]) == -1){
                                    printf("rm : %s\n", strerror(errno));
                                }
                                exit(0);
                            }
                            else if(strcmp(leftarg[0], "mv") == 0){
                                if(rename(leftarg[1], leftarg[2]) == -1){
                                    printf("mv : %s\n", strerror(errno));
                                }
                                exit(0);
                            }
                            else if(strcmp(leftarg[0], "pwd") == 0){
                                func_pwd(out_fd);
                                exit(0);
                            }
                            else if(find_notbin_cmd(leftarg, leftidx) > 0){
                                sprintf(path, "/bin/%s", leftarg[0]);
                            }
                            else{
                                sprintf(path, "/usr/bin/%s", leftarg[0]);
                            }
                            if(execv(path, leftarg) == -1){
                                printf("swsh: Command not found\n");
                                exit(1);
                            }
                        }
                    }
                    else{
                        wait(NULL);
                        if(i != 0){
                            close(in_fd);
                        }
                        if(i != ipcNum){
                            close(fd[1]);
                        }
                        in_fd = fd[0];
                    }

                    free(copy);
                    free(_copy);
                }
            }
            else if(strchr(cmdcpy, '<') != NULL){
                int leftidx = 0;

                char *copy = strdup(cmd_arg[0]);
                char *ptr = strtok(copy, " ");
                while (ptr != NULL) {
                    leftarg[leftidx] = ptr;
                    ptr = strtok(NULL, " ");
                    leftidx++;
                }
                leftarg[leftidx] = NULL;

                pid = fork();

                if(pid == 0){
                    if(cmd_arg[1][strlen(cmd_arg[1]) - 1] == ' '){
                        cmd_arg[1][strlen(cmd_arg[1]) - 1] = '\0';
                    }
                    in_fd = open(cmd_arg[1], O_RDONLY, 0755);
                    out_fd = STDOUT_FILENO;
                    
                    if(in_fd == -1) {
                        printf("swsh: No such file\n");
                        exit(1);
                    }
                    
                    dup2(in_fd, 0);

                    if(ipc_arg[ipcNum - 1][0] == '>'){
                        if(cmd_arg[2][strlen(cmd_arg[2]) - 1] == ' '){
                            cmd_arg[2][strlen(cmd_arg[2]) - 1] = '\0';
                        }
                        // >> 처리하기
                        if(strcmp(ipc_arg[ipcNum - 1], ">>") == 0){
                            out_fd = open(cmd_arg[2], O_CREAT | O_WRONLY | O_APPEND, 0755);
                            dup2(out_fd, 1);           
                        }
                        else{
                            out_fd = open(cmd_arg[2], O_CREAT | O_WRONLY | O_TRUNC, 0755);
                            dup2(out_fd, 1);
                        }
                    }

                    if(leftarg[0][0] == '.' && leftarg[0][1] == '/'){
                        sprintf(path, "%s", &leftarg[0][2]);
                    }
                    else if(strcmp(leftarg[0], "cat") == 0){
                        if(in_fd == 0 && leftarg[1] != NULL){
                            in_fd = open(leftarg[1], O_RDONLY);
                            if(in_fd == -1){
                                perror("Error open file\n");
                                exit(1);
                            }
                        }
                        func_cat(in_fd, out_fd);
                        exit(0);                          
                    }
                    else if(strcmp(leftarg[0], "head") == 0){                        
                        head(&in_fd, &out_fd, leftarg, leftidx);
                        exit(0);
                    }
                    else if(strcmp(leftarg[0], "tail") == 0){                        
                        tail(&in_fd, &out_fd, leftarg, leftidx);
                        exit(0);
                    }
                    else if(strcmp(leftarg[0], "cd") == 0){
                        if(chdir(leftarg[1]) == -1){
                            printf("cd: %s\n", strerror(errno));
                        }
                        exit(0);
                    }
                    else if(strcmp(leftarg[0], "cp") == 0){
                        func_cp(leftarg[1], leftarg[2]);
                        exit(0);
                    }
                    else if(strcmp(leftarg[0], "rm") == 0){
                        if(unlink(leftarg[1]) == -1){
                            printf("rm : %s\n", strerror(errno));
                        }
                        exit(0);
                    }
                    else if(strcmp(leftarg[0], "mv") == 0){
                        if(rename(leftarg[1], leftarg[2]) == -1){
                            printf("mv : %s\n", strerror(errno));
                        }
                        exit(0);
                    }
                    else if(strcmp(leftarg[0], "pwd") == 0){
                        func_pwd(out_fd);
                        exit(0);
                    }
                    else if(find_notbin_cmd(leftarg, leftidx) > 0){
                        sprintf(path, "/bin/%s", leftarg[0]);
                    }
                    else{
                        sprintf(path, "/usr/bin/%s", leftarg[0]);
                    }

                    close(in_fd);

                    if(execv(path, leftarg) == -1){
                        printf("swsh: Command not found\n");
                        exit(1);
                    }
                }
                else{
                    wait(NULL);
                }
                free(copy);
            }
            else if(strchr(cmdcpy, '>') != NULL){
                int leftidx = 0, rightidx = 0;
                char *copy = strdup(cmd_arg[0]);
                char *ptr = strtok(copy, " ");
                while (ptr != NULL) {
                    leftarg[leftidx] = ptr;
                    ptr = strtok(NULL, " ");
                    leftidx++;
                }
                leftarg[leftidx] = NULL;

                char *_copy = strdup(cmd_arg[1]);
                char *_ptr = strtok(_copy, " ");
                while (_ptr != NULL) {
                    rightarg[rightidx] = _ptr;
                    _ptr = strtok(NULL, " ");
                    rightidx++;
                }
                rightarg[rightidx] = NULL;

                if(strcmp(ipc_arg[ipcNum - 1], ">>") == 0){
                    pid = fork();
                    if(pid == 0){
                        out_fd = open(rightarg[0], O_CREAT | O_WRONLY | O_APPEND, 0755);
                        dup2(out_fd, 1);           

                        if(leftarg[0][0] == '.' && leftarg[0][1] == '/'){
                            sprintf(path, "%s", &leftarg[0][2]);
                        }
                        else if(strcmp(leftarg[0], "cat") == 0){
                            if(in_fd == 0 && leftarg[1] != NULL){
                                in_fd = open(leftarg[1], O_RDONLY);
                                if(in_fd == -1){
                                    perror("Error open file\n");
                                    exit(1);
                                }
                            }
                            func_cat(in_fd, out_fd);
                            exit(0);                            
                        }
                        else if(strcmp(leftarg[0], "head") == 0){
                            head(&in_fd, &out_fd, leftarg, leftidx);
                            exit(0);
                        } 
                        else if(strcmp(leftarg[0], "tail") == 0){
                            tail(&in_fd, &out_fd, leftarg, leftidx);
                            exit(0);
                        }  
                        else if(strcmp(leftarg[0], "cd") == 0){
                            if(chdir(leftarg[1]) == -1){
                                printf("cd: %s\n", strerror(errno));
                            }
                            exit(0);
                        }
                        else if(strcmp(leftarg[0], "cp") == 0){
                            func_cp(leftarg[1], leftarg[2]);
                            exit(0);
                        }
                        else if(strcmp(leftarg[0], "rm") == 0){
                            if(unlink(leftarg[1]) == -1){
                                printf("rm : %s\n", strerror(errno));
                            }
                            exit(0);
                        }
                        else if(strcmp(leftarg[0], "mv") == 0){
                            if(rename(leftarg[1], leftarg[2]) == -1){
                                printf("mv : %s\n", strerror(errno));
                            }
                            exit(0);
                        }
                        else if(strcmp(leftarg[0], "pwd") == 0){
                            func_pwd(out_fd);
                            exit(0);
                        }
                        else if(find_notbin_cmd(leftarg, leftidx) > 0){
                            sprintf(path, "/bin/%s", leftarg[0]);
                        }
                        else{
                            sprintf(path, "/usr/bin/%s", leftarg[0]);
                        }
                        if(execv(path, leftarg) == -1){
                            printf("swsh: Command not found\n");
                            exit(1);
                        }
                    }
                    else{
                        wait(NULL);
                    }
                }
                else{
                    pid = fork();
                    if(pid == 0){
                        out_fd = open(rightarg[0], O_CREAT | O_WRONLY | O_TRUNC, 0755);
                        dup2(out_fd, 1);           

                        if(leftarg[0][0] == '.' && leftarg[0][1] == '/'){
                            sprintf(path, "%s", &leftarg[0][2]);
                        }
                        else if(strcmp(leftarg[0], "cat") == 0){
                            if(in_fd == 0 && leftarg[1] != NULL){
                                in_fd = open(leftarg[1], O_RDONLY);
                                if(in_fd == -1){
                                    perror("Error open file\n");
                                    exit(1);
                                }
                            }
                            func_cat(in_fd, out_fd);
                            exit(0);                              
                        }
                        else if(strcmp(leftarg[0], "head") == 0){
                            head(&in_fd, &out_fd, leftarg, leftidx);
                            exit(0);
                        }
                        else if(strcmp(leftarg[0], "tail") == 0){
                            tail(&in_fd, &out_fd, leftarg, leftidx);
                            exit(0);
                        }
                        else if(strcmp(leftarg[0], "cd") == 0){
                            if(chdir(leftarg[1]) == -1){
                                printf("cd: %s\n", strerror(errno));
                            }
                            exit(0);
                        }
                        else if(strcmp(leftarg[0], "cp") == 0){
                            func_cp(leftarg[1], leftarg[2]);
                            exit(0);
                        }
                        else if(strcmp(leftarg[0], "rm") == 0){
                            if(unlink(leftarg[1]) == -1){
                                printf("rm : %s\n", strerror(errno));
                            }
                            exit(0);
                        }
                        else if(strcmp(leftarg[0], "mv") == 0){
                            if(rename(leftarg[1], leftarg[2]) == -1){
                                printf("mv : %s\n", strerror(errno));
                            }
                            exit(0);
                        }
                        else if(strcmp(leftarg[0], "pwd") == 0){
                            func_pwd(out_fd);
                            exit(0);
                        }
                        else if(find_notbin_cmd(leftarg, leftidx) > 0){
                            sprintf(path, "/bin/%s", leftarg[0]);
                        }
                        else{
                            sprintf(path, "/usr/bin/%s", leftarg[0]);
                        }
                        if(execv(path, leftarg) == -1){
                            printf("swsh: Command not found\n");
                            exit(1);
                        }
                    }
                    else{
                        wait(NULL);
                    }
                }
                free(_copy);
                free(copy);
            }
        }

        free(cmd);
    }
    return 0;
}
#include "pa1.h"

#define flag 1

int main(int argc, char **argv){
    char* filename = argv[1];
    char userInput[512];
    int fd, retClose, byteRead, typeInput = 0;
    if((fd = open(filename, O_RDONLY)) < 0){
        perror("open");
        exit(1);
    }
    while(1){
        if((byteRead = read(STDIN_FILENO, userInput, 512)) > 0){
            userInput[byteRead - 1] = '\0';
        }

        typeInput = checkType(userInput, byteRead);

        #if flag
        if(typeInput == 1){
            searchType1(fd, userInput);
        }
        else if(typeInput == 2){
            searchType2(fd, userInput, byteRead);
        }
        else if(typeInput == 3){
            searchType3(fd, userInput, byteRead);
        }
        else if(typeInput == 4){
            searchType4(fd, userInput, byteRead);
        }
        else{
            printString("Invalid Input", 13);
        }
        #endif
        if(printString("\n", 1) < 0){
            perror("write");
        }
    }

    if((retClose = close(fd)) < 0){
        perror("close");
        exit(1);
    }
    return 0;
}
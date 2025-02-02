#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

int printString(char* str, int len){
    return write(STDOUT_FILENO, str, len);
}

int printInt(int num){
    int digit = 1;
    int tmp = num;
    while(tmp / 10){
        tmp /= 10;
        digit++;
    }

    char* numString = (char*)malloc(sizeof(char) * digit + 1);
    for(int i = digit - 1; i >= 0; i--){
        numString[i] = '0' + num % 10;
        num /= 10;
    } 
    numString[digit] = '\0';
    printString(numString, digit + 1);
    free(numString);
}

char** seperateWord(char* str, int len, int* cnt){
    int cntSpace = 1;
    for(int i = 0; i < len; i++){
        if(str[i] == ' '){
            cntSpace++;
        }
    }

    char** ret = (char**)malloc((cntSpace) * sizeof(char*));
    for(int i = 0; i < cntSpace; i++){
        ret[i] = (char*)malloc(512 * sizeof(char));
    }

    int idx = 0;
    for(int i = 0; i < len; i++){
        if(str[i] == ' ' || str[i] == '\n'){
            ret[*cnt][idx] = '\0';
            idx = 0;
            *cnt = (*cnt) + 1;
        }
        else{
            ret[*cnt][idx] = str[i];
            idx++;
            if(i == len - 1){
                ret[*cnt][idx] = '\0';
                *cnt = (*cnt) + 1;
            }
        }
    }
    
    return ret;
}

void freeWord(char** words, int cnt) {
    for (int i = 0; i < cnt; i++) {
        free(words[i]);
    }
    free(words);
}

int checkType(char* input, int size){
    int flagSpace = 0;
    int cnt = 0;
    if(input[0] == '"' && input[size - 2] == '"'){
        return 3;
    }

    char** arrInput = seperateWord(input, size, &cnt);

    if(cnt == 3){
        if(arrInput[1][0] == '*'){
            freeWord(arrInput, cnt);
            return 4;
        }
    }

    for(int i = 0; i < size; i++){
        if(input[i] == ' '){
            return 2;
        }
    }
    return 1;
}

char upperWord(char str){
    if(str >= 'a' && str <= 'z'){
        return str - 32;
    }
    else
        return str;
}

int _strcmp(const char* str1, const char* str2){
    int i = 0;
    while(str1[i] != '\0' && str2[i] != '\0'){
        if(upperWord(str1[i]) != upperWord(str2[i])){
            return 1;
        }
        i++;
    }
    if(str1[i] != '\0' || str2[i] != '\0'){
        return 1;
    }

    return 0;
}

void readFileType2(int fd, char** arrInput, int cnt){
    char buffer[16];
    char word[512];
    int* visit = (int*)malloc(cnt * sizeof(int));
    off_t pos = 0;
    int cntFind = 0;
    long int len = 0, lineCount = 1;

    for(int i = 0; i < cnt; i++){
        visit[i] = 0;
    }

    while(read(fd, buffer, 1) > 0){
        pos = lseek(fd, 0, SEEK_CUR);

        if(buffer[0] != ' ' && buffer[0] != '\n'){
            word[len] = buffer[0];
            len++;
        }

        else{
            word[len] = '\0';

            for(int i = 0; i < cnt; i++){
                if(_strcmp(word, arrInput[i]) == 0 && visit[i] == 0){
                    cntFind++;
                    visit[i] = 1;
                }
            }

            if(cntFind == cnt){
                printInt(lineCount);
                printString(" ", 1);
                cntFind++;
            }

            if(buffer[0] == '\n'){
                lineCount++;
                cntFind = 0;
                for(int i = 0; i < cnt; i++){
                    visit[i] = 0;
                }
            }

            len = 0;
        }
    }

    pos = lseek(fd, 0, SEEK_SET);
}

int readLineType4(char* line, int len_line, char* leftWord, char* rightWord){
    int cnt = 0;
    char** arrWord = seperateWord(line, len_line, &cnt);
    int findFlag = 0; // 1 : find left, 2 : find Other

    for(int i = 0; i < cnt; i++){
        if((findFlag == 0 || findFlag == 1) && (_strcmp(leftWord, arrWord[i]) == 0)){
            findFlag = 1;
        }
        else if(findFlag == 1){
            if((_strcmp(leftWord, arrWord[i]) != 0)
             && (_strcmp(rightWord, arrWord[i]) != 0)){
                findFlag = 2;
            }
            else{
                findFlag = 0;
            }
        }
        else if(findFlag == 2 && (_strcmp(rightWord, arrWord[i]) == 0)){
            return 1;
        }
    }
    return 0;
}

void searchType1(int fd, char* userInput){
    char buffer[16];
    char word[512];
    off_t pos = 0;
    long int len = 0, lineCount = 1, wordCount = 0, wordpos = 0;
    int find_flag = 0;
    
    while(read(fd, buffer, 1) > 0){
        pos = lseek(fd, 0, SEEK_CUR);
        // write(STDOUT_FILENO, buffer, 1);
        if(buffer[0] != ' ' && buffer[0] != '\n'){
            if(upperWord(buffer[0]) == upperWord(userInput[0]) && len == 0){
                wordpos = wordCount;
            }
            word[len] = buffer[0];
            len++;
            wordCount++;
        }
        else{
            word[len] = '\0';

            if(_strcmp(userInput, word) == 0){
                printInt(lineCount);
                printString(":", 1);
                printInt(wordpos);
                printString(" ", 1);
            }

            if(buffer[0] == '\n'){
                lineCount++;
                wordCount = 0;
            }
            else{
                wordCount++;
            }

            len = 0;
        }
    }

    pos = lseek(fd, 0, SEEK_SET);
}

void searchType2(int fd, char* userInput, int size){
    int cnt = 0;
    char** arrInput = seperateWord(userInput, size, &cnt);

    readFileType2(fd, arrInput, cnt);

    freeWord(arrInput, cnt);
}

void searchType3(int fd, char* userInput, int size){
    char* _userInput = (char*)malloc(sizeof(char) * (size - 2));
    for(int i = 1; i < size - 2; i++){
        _userInput[i-1] = userInput[i];
    }
    _userInput[size - 1] = '\0';

    char buffer[16];
    off_t pos = 0;
    long int len = 0, lineCount = 1, wordCount = 0, wordpos = 0;
    int findflag = 0; // 0 : 찾는 중일때 1 : 첫 글자 찾아서 탐색 시작 2 : 공백까지 pass

    while(read(fd, buffer, 1) > 0){
        pos = lseek(fd, 0, SEEK_CUR);
        
        if(upperWord(buffer[0]) != upperWord(_userInput[0]) && findflag == 0){
            findflag = 2;
        }
        else{
            if(findflag == 0){
                findflag = 1;
                wordpos = wordCount; 
            }
        }

        if(len == size - 3){
            if(buffer[0] == ' ' || buffer[0] == '\n'){
                printInt(lineCount);
                printString(":", 1);
                printInt(wordpos);
                printString(" ", 1);
                findflag = 0;
                len = 0;
            }
            else{
                findflag = 2;
                len = 0;
            }
        }

        if(findflag == 2){
            if(buffer[0] == ' ' || buffer[0] == '\n'){
                findflag = 0;
                len = 0;
            }
            else{

            }
        }
        else if(findflag == 1){
            if(upperWord(buffer[0]) == upperWord(_userInput[len])){
                len++;
            }
            else{
                len = 0;
                findflag = 2;
            }
        }

        wordCount++;

        if(buffer[0] == '\n'){
            lineCount++;
            len = 0;
            wordCount = 0;
        }
    }
    pos = lseek(fd, 0, SEEK_SET);
}

void searchType4(int fd, char* userInput, int size){
    int cnt = 0;
    char** arrInput = seperateWord(userInput, size, &cnt);
    char* leftWord = arrInput[0];
    char* rightWord = arrInput[2];

    char buffer[16];
    char word[512];
    off_t pos = 0;
    long int len = 0, lineCount = 1;

    while(read(fd, buffer, 1) > 0){
        pos = lseek(fd, 0, SEEK_CUR);
        word[len] = buffer[0];
        len++;
        if(buffer[0] == '\n'){
            if(readLineType4(word, len, leftWord, rightWord)){
                printInt(lineCount);
                printString(" ", 1);
            }
            len = 0;
            lineCount++;
        }
    }

    freeWord(arrInput, cnt);
    pos = lseek(fd, 0, SEEK_SET);
}
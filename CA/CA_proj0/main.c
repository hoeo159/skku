#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 255

int main(int argc, char* argv[]){

    FILE *fp;

    fp = fopen(argv[1], "rb");

    if (fp == NULL) {
        printf("Error: failed to open file.");
        return -1;
    }
    
    unsigned char *line = (unsigned char*) malloc(sizeof(unsigned char) * 16); // 한 줄의 최대 크기는 16바이트로 가정
    while (fread(line, sizeof(unsigned char), 16, fp) == 16) { // 한 줄씩 읽기 위해 16바이트씩 읽어들임
        // 읽은 한 줄에 대한 처리
        for (int i = 0; i < 16; i++) {
            printf("%02x ", line[i]); // 각 바이트를 16진수로 출력
        }
        printf("\n");
    }
    fclose(fp);
    free(line);
    return 0;
}
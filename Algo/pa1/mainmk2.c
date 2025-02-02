#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int MAX_LENGTH = 10;

typedef struct node {
    char item[1000];
    struct node* next;
} Node;

typedef struct list{
    Node* tail;
    int len;
} List;

void InsertInitItem(List* plist, char* item)
{
    Node* newNode = (Node *)malloc(sizeof(Node));
    strcpy(newNode->item, item);
    newNode->next = newNode;

    plist->tail = newNode;
    plist->len++;
    return;
}

void InsertLast(List* plist, char* item)
{
    if (plist->len == 0)
        InsertInitItem(plist, item);
    else {
        Node* newNode = (Node *)malloc(sizeof(Node));
        strcpy(newNode->item, item);
        newNode->next = plist->tail->next;
        plist->tail->next = newNode;

        plist->tail = newNode;
        plist->len++;
    }
}

char* GetItem(List* plist, int idx){
    Node* cur = plist->tail->next;
    for(int i = 0; i < idx; i++){
        cur = cur->next;
    }
    return cur->item;
}

void PrintList(FILE *fp, List* plist, int attrib_N, int flag){
    Node* cur = plist->tail->next;

    int islast = 0;

    if(flag){
        islast = 1;
    }

    for(int j = 0; j <= attrib_N; j++){
        if(j == attrib_N){
            fprintf(fp, "%s", cur->item);
        }
        else    fprintf(fp, "%s:", cur->item);

        for(int i = 0; i <= strlen(cur->item); i++){
            if(cur->item[i] == '\n'){
                flag = 0;
                break;
            }
        }
        cur = cur->next;
    }

    if(flag && islast){
        fprintf(fp, "\n");
    }
    
    return;
}

void CountSort(List* user, int n, int key, int digit){
    int cnt_ascii[128];
    List* tmp = (List*)malloc(n * sizeof(List));
    memset(cnt_ascii, 0, sizeof(cnt_ascii));
    
    for(int i = 0; i < n; i++){
        cnt_ascii[(int)GetItem(&user[i], key)[digit]]++;
    }

    for(int i = 1; i < 128; i++){
        cnt_ascii[i] += cnt_ascii[i-1];
    }

    for(int i = n - 1; i >= 0; i--){
        int j = cnt_ascii[(int)GetItem(&user[i], key)[digit]] - 1;
        tmp[j] = user[i];
        cnt_ascii[(int)GetItem(&user[i], key)[digit]]--;
    }

    for(int i = 0; i < n; i++){
        user[i] = tmp[i];
    }

    free(tmp);
    return;
}

void RadixSort(List* user, int n, int key){
    for(int digit = MAX_LENGTH - 1; digit >= 0; digit--){
        CountSort(user, n, key, digit);
    }
    return;
}


int main(){
    FILE* fp;
    char* buff;
    int cnt, idx = 0, key_idx, attrib_N = 0, user_N = 0;
    List user[10001];

    fp = fopen("hw1_input.txt", "r");

    if(fp == NULL){
        printf("Error!\n");
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    cnt = ftell(fp);
    rewind(fp);

    buff = (char*)malloc(cnt);

    while(fgets(buff, sizeof(char) * cnt, fp) != NULL) {
        if(buff[0] == '$'){
            idx++;
            continue;
        }

        if(idx == 1){
            for(int i = 0; i < strlen(buff); i++){
                if(buff[i] == ':'){
                    attrib_N++;
                }
                if(buff[i] == '*'){
                    key_idx = attrib_N;
                }
            }
        }

        if(idx == 2){
            
            for(int i = 0; i <= strlen(buff); i++){
                if(buff[i] == '\n'){
                    buff[i] = '\0';
                    // buff[strlen(buff)-1] = 0;
                }
            }

            char *ptr = strtok(buff, ":");
            while (ptr != NULL){
                InsertLast(&user[user_N], ptr);
                ptr = strtok(NULL, ":");
                if(user[user_N].len == attrib_N + 1){
                    user_N++;
                }
            }
        }
    }

    fclose(fp);
    free(buff);

    RadixSort(user, user_N, key_idx);

    FILE *pfile = NULL;
    pfile = fopen("hw1_output.txt", "w");

    for(int i = 0; i < user_N; i++){
        if(i == user_N - 1){
            PrintList(pfile, &user[i], attrib_N, 0);
        }
        else    PrintList(pfile, &user[i], attrib_N, 1);
    }

    fclose(pfile);

    return 0;
}
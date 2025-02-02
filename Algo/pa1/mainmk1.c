#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char item[20];
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

    for(int j = 0; j <= attrib_N; j++){
        if(j == attrib_N){
            fprintf(fp, "%s", cur->item);
        }
        else    fprintf(fp, "%s:", cur->item);

        for(int i = 0; i < strlen(cur->item); i++){
            if(cur->item[i] == '\n'){
                flag = 0;
                break;
            }
        }
        cur = cur->next;
    }

    if(flag){
        fprintf(fp, "\n");
    }
    
    return;
}

void merge(List* user, int left, int mid, int right, int key){
    List* tmp = (List*)malloc((right - left + 1) * sizeof(List));

    int i = left, j = mid + 1, idx = 0;
    while (i <= mid && j <= right) // i~mid mid+1 ~ right 병합
	{
		if (strcmp(GetItem(&user[i], key), GetItem(&user[j], key)) <= 0){
            tmp[idx++] = user[i++];
        }
		else{
            tmp[idx++] = user[j++];
        }
	}
    if(i > mid){
        while(j <= right){
            tmp[idx++] = user[j++];
        }
    }
    else{
        while(i <= mid){
            tmp[idx++] = user[i++];
        }
    }

    int k = left;

    for (i = 0; i < idx; i++){
        // 같은 key 값 stable user에서 빠르게 만난 요소가 들어가도록
        while (k <= right && strcmp(GetItem(&user[k], key), GetItem(&tmp[i], key)) != 0){
            k++;
        }
        if (k <= right){
            user[k++] = tmp[i];
        }
    }

    free(tmp);
    return;
}

void mergesort(List* user, int left, int right, int key){
    if(left < right){
        int mid = (left + right) / 2;
        mergesort(user, left, mid, key);
        mergesort(user, mid + 1, right, key);
        merge(user, left, mid, right, key);
    }
}

int main(){
    FILE* fp;
    char* buff;
    int cnt, idx = 0, key_idx, attrib_N = 0, user_N = 0;
    List user[10001];

    fp = fopen("input.txt", "r");

    if(fp == NULL){
        printf("Error!");
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
            for(int i = 0; i < strlen(buff); i++){
                if(buff[i] == '\n'){
                    buff[i] = '\0';
                }
            }
            char *ptr = strtok(buff, ":");
            while (ptr != NULL)
            {
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

    mergesort(user, 0, user_N - 1, key_idx);

    FILE *pfile = NULL;
    pfile = fopen("output.txt", "w");

    for(int i = 0; i < user_N; i++){
        PrintList(pfile, &user[i], attrib_N, 1);
    }

    fclose(pfile);

    return 0;
}
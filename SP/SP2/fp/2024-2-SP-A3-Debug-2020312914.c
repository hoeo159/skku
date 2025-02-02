/*
     A program that analyzes the n-gram statistics of a text document,
    where n-gram means a sequence of n words occurring in a document.

    Program Components:
        Conversion of each word to lowercase
        Hashing to s buckets
        Scanning each hash bucket (linked list)
        Sorting all elements according to the frequencies
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_NGRAM 10
#define MAX_WORD_LEN 1000
#define HASH_TABLE_SIZE 363041
#define uint unsigned int

int N = 2;
int filesize = 0;
int word_count = 0;

#define SWAP(a, b) { Ngram* temp = a; a = b; b = temp; }

typedef struct NGRAM{
    char word[MAX_NGRAM][MAX_WORD_LEN];
    int count;
    struct NGRAM* next;
} Ngram;

typedef struct HASH_TABLE{
    Ngram* bucket[HASH_TABLE_SIZE];
    uint count;  // 전체 개수
} Hash_Table;

uint count_word(FILE* fp){
    uint count = 0;
    char buffer[MAX_WORD_LEN];
    while(fscanf(fp, "%s", buffer) != EOF){
        count++;
    }

    rewind(fp);
    return count;
}

uint get_string_length(char* str){
    uint len = 0;
    while(str[len] != '\0'){
        len++;
    }
    return len;
}

uint hash(char words[MAX_NGRAM][MAX_WORD_LEN]) {
    uint hash = 0;
    for(int i = 0; i < N; i++) {
        const char* str = words[i];
        while(*str){
            hash = (hash << 5) - hash + *str++;
        }
    }
    return hash % HASH_TABLE_SIZE;
}

Hash_Table* init_hash_table(){
    Hash_Table* ht = (Hash_Table*)malloc(sizeof(Hash_Table));
    for(int i = 0; i < HASH_TABLE_SIZE; i++){
        ht->bucket[i] = NULL;
    }
    return ht;
}

uint get_file_size(FILE* fp){
    uint ret = 0;
    while(fgetc(fp) != EOF){
        ret++;
    }
    rewind(fp);
    return ret;
}

void to_lower(char* str){
    uint i = 0;
    while (str[i] != '\0' && str[i + 1] != '\0') {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] - 'A' + 'a';
        }
        if (str[i + 1] >= 'A' && str[i + 1] <= 'Z') {
            str[i + 1] = str[i + 1] - 'A' + 'a';
        }
        i += 2;
    }

    while (str[i] != '\0'){
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] - 'A' + 'a';
        }
        i++;
    }
}


int mystrcmp(char* str1, char* str2){
    int result = 0;
    int i = 0;
    int j = 0;
    for(; i < get_string_length(str1) || j < get_string_length(str2); i++, j++){
        if(str1[i] != str2[j]){
            result = str1[i] - str2[j];
            break;
        }
    }
    return result;
}

void insert_N_gram(Hash_Table* table, char swnd[MAX_NGRAM][MAX_WORD_LEN]){
    uint idx = hash(swnd);
    Ngram* cur = table->bucket[idx];
    Ngram* prev = NULL;

    while(cur != NULL){
        int i;
        for(i = 0; i < N; i++){
            if(strcmp(cur->word[i], swnd[i]) != 0){
                break;
            }
        }
        if(i == N){
            cur->count++;
            return;
        }
        prev = cur;
        cur = cur->next;
    }

    Ngram* new = (Ngram*)malloc(sizeof(Ngram));
    for(int i = 0; i < N; i++){
        strcpy(new->word[i], swnd[i]);
    }
    new->count = 1;
    new->next = NULL;

    if(prev == NULL){
        table->bucket[idx] = new;
    }
    else{
        prev->next = new;
    }
    table->count++;
}

void get_N_gram(FILE* fp, Hash_Table* table){
    char buffer[MAX_WORD_LEN];
    char swnd[MAX_NGRAM][MAX_WORD_LEN] = {0, };
    int count = 0;

    while(fscanf(fp, "%s", buffer) != EOF){
        to_lower(buffer);
        strcpy(swnd[count++ % N], buffer);
        (count >= N) ? insert_N_gram(table, swnd) : (void)0;
    }
}


int compare(const void* a, const void* b){
    Ngram* ngramA = *(Ngram**)a;
    Ngram* ngramB = *(Ngram**)b;
    return ngramB->count - ngramA->count;
}

int partition(Ngram** arr, int low, int high){
    Ngram* pivot = arr[high];
    int i = low - 1;

    for(int j = low; j < high; j++){
        if(compare(&arr[j], &pivot) < 0){
            i++;
            SWAP(arr[i], arr[j]);
        }
    }
    SWAP(arr[i + 1], arr[high]);
    return i + 1;
}

void quicksort(Ngram** arr, int low, int high){
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

// loop unrolling
void bubble_sort(Ngram** arr, int count){
    for(int i = 0; i < count - 1; i++) {
        for(int j = 0; j < count - i - 1; j += 4){
            if(arr[j]->count < arr[j + 1]->count){
                SWAP(arr[j], arr[j + 1]);
            }
            if(j + 1 < count - i - 1 && arr[j + 1]->count < arr[j + 2]->count){
                SWAP(arr[j + 1], arr[j + 2]);
            }
            if(j + 2 < count - i - 1 && arr[j + 2]->count < arr[j + 3]->count){
                SWAP(arr[j + 2], arr[j + 3]);
            }
            if(j + 3 < count - i - 1 && arr[j + 3]->count < arr[j + 4]->count){
                SWAP(arr[j + 3], arr[j + 4]);
            }
        }
    }
}

void sort_N_gram(Hash_Table* table){
    Ngram* arr[HASH_TABLE_SIZE];
    int idx = 0;
    for(int i = 0; i < HASH_TABLE_SIZE; i++) {
        Ngram* cur = table->bucket[i];
        while(cur != NULL){
            arr[idx++] = cur;
            cur = cur->next;
        }
    }

    printf("start sorting\n");
    // quicksort(arr, 0, idx - 1);
    bubble_sort(arr, idx);

    for(int i = 0; i < 5; i++){
        for(int j = 0; j < N; j++){
            printf("%s ", arr[i]->word[j]);
        }
        printf("%d\n", arr[i]->count);
    }
}

uint get_bucket_count(Hash_Table* table) {
    return table->count;
}

void write_to_file(const char* filename, Hash_Table* table){
    FILE* fp = fopen(filename, "w");
    if(fp == NULL){
        perror("Error opening file");
        return;
    }

    for(int i = 0; i < HASH_TABLE_SIZE; i++){
        Ngram* cur = table->bucket[i];
        while (cur != NULL) {
            for(int j = 0; j < N; j++){
                fprintf(fp, "%s ", cur->word[j]);
            }
            fprintf(fp, "%d\n", cur->count);
            cur = cur->next;
        }
    }
    fclose(fp);
}
void print_N_gram(Hash_Table *table){
    printf("====================================\n");
    printf("file size: %d\n", filesize);
    printf("word count: %d\n", word_count);
    printf("N : %d\n", N);
    printf("bucket count: %d\n", get_bucket_count(table));
}

void free_N_gram(Hash_Table* table){
    for(int i = 0; i < HASH_TABLE_SIZE; i++){
        Ngram* cur = table->bucket[i];
        while(cur != NULL){
            Ngram* temp = cur;
            cur = cur->next;
            free(temp);
        }
    }
    free(table);
}

double get_time(){
    int fd = open("/proc/uptime", O_RDONLY);
    if(fd == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }

    char buffer[128];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if(bytes_read == -1){
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_read] = '\0';
    close(fd);

    double uptime;
    sscanf(buffer, "%lf", &uptime);
    return uptime;
}

int main(int argc, char* argv[]){
    const char* output_file = "output.txt";

    if(argc != 2 && argc != 3){
        printf("input error: %s <filename> <N(default : 2)\n", argv[0]);
        return 1;
    }
    FILE* fp = fopen(argv[1], "r");
    N = (argc == 3) ? atoi(argv[2]) : 2;
    if(fp == NULL){
        printf("file open error: %s\n", argv[1]);
        return 1;
    }

    printf("start\n");
    double start = get_time();

    Hash_Table* table = init_hash_table();
    filesize = get_file_size(fp);
    word_count = count_word(fp);
    double end1 = get_time();
    printf("init_hash_table time: %lf seconds\n", end1 - start);
    
    double start2 = get_time();
    get_N_gram(fp, table);
    double end2 = get_time();
    printf("get_N_gram time: %lf seconds\n", end2 - start2);

    double start3 = get_time();
    sort_N_gram(table);
    double end3 = get_time();
    printf("sort_N_gram time: %lf seconds\n", end3 - start3);

    print_N_gram(table);
    double start4 = get_time();
    write_to_file(output_file, table);
    double end4 = get_time();
    printf("write_to_file time: %lf seconds\n", end4 - start4);

    free_N_gram(table);
    fclose(fp);

    double end = get_time();

    printf("time: %lf seconds\n", end - start);
    return 0;
}
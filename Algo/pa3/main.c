#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

double graph[3000][3000]; 

typedef struct pair{
    char* key;
    int value;
} Pair;

int node_n, file_n, e_connected;

void countNode(FILE* file){
    rewind(file);

    node_n = 0;
    char ch;
    while((ch = fgetc(file)) != ' '){
        if (ch == '\n') {
            node_n++;
        }
    }
    rewind(file);
    return;
}

void countFileLine(FILE* file){
    rewind(file);

    file_n = 0;
    char ch;
    while((ch = fgetc(file)) != EOF){
        if (ch == '\n') {
            file_n++;
        }
    }
    rewind(file);
    return;
}

int getValue(char* str, Pair* pairs){
    for(int i = 0; i < node_n; i++){
        if(strcmp(pairs[i].key, str) == 0){
            return pairs[i].value;
        }
    }
    return -1;
}

char* getKey(int num, Pair* pairs){
    for(int i = 0; i < node_n; i++){
        if(num == pairs[i].value){
            return pairs[i].key;
        }
    }
    return "\0";
}

int is_weakly_connected(){
    int visited[3000] = { 0, };
    int queue[3000];
    int front = 0;
    int rear = 0;

    visited[0] = 1;
    queue[rear++] = 0;

    while(front != rear){
        int current = queue[front++];

        for(int i = 0; i < node_n; i++){
            if(graph[current][i] != 0 && !visited[i]){
                visited[i] = 1;
                queue[rear++] = i;
            }
        }
    }

    for(int i = 0; i < node_n; i++){
        if(!visited[i]){
            return 0;
        }
    }

    return 1;
}

int is_strongly_connected(){
    int visited[3000] = { 0, };
    int stack[3000];
    int top = -1;

    visited[0] = 1;
    stack[++top] = 0;

    while(top != -1){
        int current = stack[top--];

        for(int i = 0; i < node_n; i++){
            if(graph[current][i] != 0 && !visited[i]){
                visited[i] = 1;
                stack[++top] = i;
            }
        }
    }

    for(int i = 0; i < node_n; i++){
        if(!visited[i]){
            return 0;
        }
    }

    return 1;
}

int is_k_edge_connected(){
    if(e_connected < node_n - 1)
        return 0;

    double** temp_graph = (double**)malloc(node_n * sizeof(double*));
    for(int i = 0; i < node_n; i++){
        temp_graph[i] = (double*)malloc(node_n * sizeof(double));
        memcpy(temp_graph[i], graph[i], node_n * sizeof(double));
    }

    for(int i = 0; i < node_n; i++){
        for(int j = 0; j < node_n; j++){
            if(temp_graph[i][j] != 0){
                temp_graph[i][j] = 0;

                if(!is_weakly_connected()){
                    for(int k = 0; k < node_n; k++){
                        free(temp_graph[k]);
                    }
                    free(temp_graph);
                    return 0;
                }

                temp_graph[i][j] = graph[i][j];
            }
        }
    }

    for(int i = 0; i < node_n; i++){
        free(temp_graph[i]);
    }
    free(temp_graph);

    return 1;
}

int find_negative_cycle(){
    double distance[3000];
    for(int i = 0; i < node_n; i++){
        distance[i] = 0;
    }

    for(int k = 0; k < node_n; k++){
        for(int i = 0; i < node_n; i++){
            for(int j = 0; j < node_n; j++){
                if(graph[i][j] != 0 && distance[j] > distance[i] + graph[i][j]){
                    distance[j] = distance[i] + graph[i][j];
                    if(k == node_n - 1){
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
}

double get_diameter(){
    double** distance = (double**)malloc(node_n * sizeof(double*));
    for(int i = 0; i < node_n; i++){
        distance[i] = (double*)malloc(node_n * sizeof(double));
        for(int j = 0; j < node_n; j++){
            if(i == j){
                distance[i][j] = 0;
            }
            else if (graph[i][j] != 0){
                distance[i][j] = graph[i][j];
            }
            else{
                distance[i][j] = DBL_MAX;
            }
        }
    }

    for(int k = 0; k < node_n; k++){
        for(int i = 0; i < node_n; i++){
            for(int j = 0; j < node_n; j++){
                if(distance[i][k] + distance[k][j] < distance[i][j]){
                    distance[i][j] = distance[i][k] + distance[k][j];
                }
            }
        }
    }

    double max_distance = 0;
    for(int i = 0; i < node_n; i++){
        for(int j = 0; j < node_n; j++){
            if (distance[i][j] > max_distance && distance[i][j] != DBL_MAX){
                max_distance = distance[i][j];
            }
        }
    }

    if(max_distance == DBL_MAX){
        return -1;
    }

    return max_distance;
}

int* degree_centrality(){
    int* degreeArray = (int*)malloc(node_n * sizeof(int));
    int maxDegree = 0;
    int count = 0;

    for(int i = 0; i < node_n; i++){
        int inDegree = 0;
        int outDegree = 0;
        for(int j = 0; j < node_n; j++){
            if (graph[i][j] != 0)
                outDegree++;
            if (graph[j][i] != 0)
                inDegree++;
        }
        int degree = inDegree + outDegree;
        if(degree > maxDegree)
            maxDegree = degree;
    }
    for(int i = 0; i < node_n; i++){
        int inDegree = 0;
        int outDegree = 0;
        for(int j = 0; j < node_n; j++){
            if (graph[i][j] != 0)
                outDegree++;
            if (graph[j][i] != 0)
                inDegree++;
        }
        int degree = inDegree + outDegree;
        if(degree == maxDegree)
            degreeArray[count++] = i;
    }

    degreeArray = (int*)realloc(degreeArray, count * sizeof(int));
    return degreeArray;
}

double shortest_path_distance(int source, int destination) {
    double distance[3000];
    for(int i = 0; i < node_n; i++){
        distance[i] = DBL_MAX;
    }
    distance[source] = 0;

    for(int k = 0; k < node_n - 1; k++){
        for(int i = 0; i < node_n; i++) {
            for(int j = 0; j < node_n; j++) {
                if(graph[i][j] != 0 && distance[i] != DBL_MAX && distance[i] + graph[i][j] < distance[j]){
                    distance[j] = distance[i] + graph[i][j];
                }
            }
        }
    }

    return distance[destination];
}

double find_closeness_centrality(int node) {
    double sum_distance = 0.0;
    int reachable_nodes = 0;
    for(int i = 0; i < node_n; i++){
        if(i != node){
            double distance = shortest_path_distance(i, node) * (-1.0);
            // printf("%d <- %d  : %lf\n", node, i, distance);
            if(distance != DBL_MAX){
                sum_distance += distance;
                reachable_nodes++;
            }
        }
    }

    if(reachable_nodes == 0)
        return 0.0;
    // printf("n : %d sum : %lf\n", reachable_nodes, sum_distance);
    return (double)(reachable_nodes) / sum_distance;
}

int* closeness_centrality(){
    double* closenessArray = (double*)malloc(node_n * sizeof(double));
    double maxCloseness = 0.0;
    int count = 0;

    for(int i = 0; i < node_n; i++){
        closenessArray[i] = find_closeness_centrality(i);
        if (closenessArray[i] > maxCloseness)
            maxCloseness = closenessArray[i];
    }
    for(int i = 0; i < node_n; i++){
        if (closenessArray[i] == maxCloseness)
            count++;
    }

    int* maxClosenessArray = (int*)malloc(count * sizeof(int));
    count = 0;

    for(int i = 0; i < node_n; i++){
        if (closenessArray[i] == maxCloseness)
            maxClosenessArray[count++] = i;
    }

    free(closenessArray);
    return maxClosenessArray;
}

int main(int argc, char* argv[]){
    FILE* fp = fopen(argv[1], "r");

    if(fp == NULL) {
        printf("File Error.\n");
        return -1;
    }
    countNode(fp);
    countFileLine(fp);
    Pair* pairs = (Pair*)malloc(node_n * sizeof(Pair));

    char line[101];

    // 파일 입력 받기
    for(int i = 0; i < node_n; i++){
        fgets(line, sizeof(line), fp);
        int length = strlen(line);
        if (line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        pairs[i].key = strdup(line);
        pairs[i].value = i;
    }

    for(int i = 0; i < file_n - node_n; i++){
        char node1[100];
        char node2[100];
        double weight;

        if (fscanf(fp, "%s %s %lf", node1, node2, &weight) == 3) {
        int x = getValue(node1, pairs);
        int y = getValue(node2, pairs);
        
            if (x != -1 && y != -1) {
                graph[x][y] = weight;
            }
        }
    }
    int tmp;
    fscanf(fp, "%d", &tmp);
    e_connected = tmp;
    // 파일 입력 받기 끝

    if(is_weakly_connected()){
        printf("T\n");
    }
    else{
        printf("F\n");
    }

    if(is_strongly_connected()){
        printf("T\n");
    }
    else{
        printf("F\n");
    }

    if(is_k_edge_connected()){
        printf("T\n");
    }
    else{
        printf("F\n");
    }

    if(find_negative_cycle()){
        printf("T\n");
    }
    else{
        printf("F\n");
    }

    if(!find_negative_cycle()){
        if(get_diameter() == -1.0)
            printf("NULL\n");
        else{
            printf("%.1lf\n", get_diameter());
        }
    }
    else{
        printf("NULL\n");
    }

    int* maxDegreeArray = degree_centrality();
    int degree_len = sizeof(maxDegreeArray) / sizeof(int);
    for(int i = 0; i < degree_len; i++){
        printf("%s ", getKey(maxDegreeArray[i], pairs));
    }
    free(maxDegreeArray);
    printf("\n");

    if(!find_negative_cycle()){
        int* maxCloseArray = closeness_centrality();
        int close_len = sizeof(maxCloseArray) / sizeof(int);
        for(int i = 0; i < close_len - 1; i++){
            printf("%s ", getKey(maxCloseArray[i], pairs));
        }
        free(maxCloseArray);
        printf("\n");
    }
    else{
        printf("NULL\n");
    }

    fclose(fp);
    return 0;
}
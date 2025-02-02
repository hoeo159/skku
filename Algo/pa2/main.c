#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH 100000

int cur_idx = 0, func_cnt = 0, var_cnt = 0, global_cnt = 0;
char oper_array[] = {'!', '+', '-', '*', '/', '%', '^', '&', '?', '~', '=', '>', '<', '|', ':'};
char* type_keyword[] = {"const", "typedef", "static", "unsigned", "signed", "return", "continue", "break", "NULL", "nullptr"
"short", "int", "unsigned", "long", "char", "float", "double", "struct", "union", "enum", "void", "case", "default" };
char* func_array[100000];
char* var_array[100000];
char* global_var_array[100000];
int flag = 0, typeflag = 0, enum_flag;   

int isNum(char* str){
    for(int i = 0; i < cur_idx; i++){
        if((str[i] < '0' || str[i] > '9') && str[i] != '.'){
            return 0;
        }
    }
    return 1;
}

int isVar(char* str){
    for(int i = 0; i < var_cnt; i++){
        if(strcmp(str, var_array[i]) == 0){
            return 1;
        }
    }
    for(int i = 0; i < global_cnt; i++){
        if(strcmp(str, global_var_array[i]) == 0){
            return 1;
        }
    }
    return 0;
}

int isFunc(char* str){
    for(int i = 0; i < func_cnt; i++){
        if(strcmp(str, func_array[i]) == 0){
            return 1;
        }
    }
    return 0;
}

int isOper(char tmp){

    for(int i = 0; i < sizeof(oper_array); i++){
        int num_idx = i;
        if(tmp == oper_array[i]){
            return 1;
        }
    }
    return 0;
}

int isTypeKeyword(char* str){
    for(int i = 0; i < sizeof(type_keyword) / sizeof(type_keyword[0]); i++){
        if(strcmp(str, type_keyword[i]) == 0){
            return 1;
        }
    }
    return 0;
}

void LCS_length(char* str1[], int str1_len, char* str2[], int str2_len, int** len) {
    for (int i = 1; i <= str1_len; i++) {
        for (int j = 1; j <= str2_len; j++) {
            if (strcmp(str1[i-1], str2[j-1]) == 0) {
                len[i][j] = len[i-1][j-1] + 1;
            }
            else {
                len[i][j] = ((len[i-1][j] > len[i][j-1]) ? len[i-1][j] : len[i][j-1]);
            }
        }
    }
}

int LCS(char* str1[], int str1_len, char* str2[], int str2_len, char* result[]) {
    int** len = (int**)malloc((str1_len+1) * sizeof(int*));
        for(int i=0; i<=str1_len; i++) {
            len[i] = (int*)malloc((str2_len+1) * sizeof(int));
            len[i][0] = 0; // 0열은 0으로 초기화
        }
        for(int j=0; j<=str2_len; j++) {
            len[0][j] = 0; // 0행은 0으로 초기화
        }

    LCS_length(str1, str1_len, str2, str2_len, len);

    int i = str1_len;
    int j = str2_len;
    int k = 0;

    while (i > 0 && j > 0) {
        if (strcmp(str1[i-1], str2[j-1]) == 0) {
            result[k] = strdup(str1[i-1]);
            i--;
            j--;
            k++;
        }
        else if (len[i-1][j] >= len[i][j-1]) {
            i--;
        }
        else {
            j--;
        }
    }
    // 뒤집기
    for (int m = 0; m < k/2; m++) {
        char* tmp = result[m];
        result[m] = result[k-m-1];
        result[k-m-1] = tmp;
    }

    return k;
}


int main(int argc, char* argv[]){

    FILE* orifp = fopen(argv[1], "r");
    FILE* cpyfp = fopen(argv[2], "r");
    FILE* outputfp = fopen("hw2_output.txt", "w");

    char* str = malloc(sizeof(char) * MAX_LENGTH);
    char* buff;
    int cnt;
    int ori_cnt = 0, cpy_cnt = 0;
    char* originCode[100000];
    char* copiedCode[100000];

    if(orifp == NULL || cpyfp == NULL){
        "File open Error\n";
        return 1;
    }

    fseek(orifp, 0, SEEK_END);
    cnt = ftell(orifp); // for save how many bytes of input file
    rewind(orifp);

    buff = (char*)malloc(cnt);
    str[0] = '\0';

    while(fgets(buff, sizeof(char) * cnt, orifp) != NULL){
        for(int i = 0; i < strlen(buff); i++){
            // 주석 (//) 처리하기
            if(buff[i] == '/' && buff[i+1] == '/'){
                break;
            }

            // flag 1 : /* */ 주석 내부
            else if(buff[i] == '/' && buff[i+1] == '*'){
                flag = 1;
            }
            else if(buff[i] == '*' && buff[i+1] == '/'){
                i++;
                flag = 0;
            }
            else if(flag == 1){
                continue;
            }

            // flag 2 : string 내부
            else if((buff[i] == '\"' || buff[i] == '\'') && buff[i-1] != '\\'){
                if(flag == 0){
                    flag = 2;
                }
                else if(flag == 2){
                    char* tmp = "STR_LITERAL";
                    originCode[ori_cnt++] = tmp;
                    flag = 0;
                }
            }
            else if(flag == 2){
                continue;
            }

            //flag 3 : 전처리기 내부
            else if(buff[i] == '#'){
                flag = 3;
                str[cur_idx++] = buff[i];
            }
            else if(buff[i] == '<'){
                str[cur_idx++] = buff[i];
                flag = 3;
            }
            else if(buff[i] == ' ' && flag == 3){
                if(cur_idx != 0){
                    if(strstr(buff, "define") == NULL || strstr(str, "define") != NULL){
                        str[cur_idx++] = '\0';
                        originCode[ori_cnt++] = strdup(str);
                        memset(str, 0, cur_idx * sizeof(char));
                        cur_idx = 0;
                        flag = 0;
                    }
                }
            }
            else if(buff[i] == '>' && flag == 3){
                str[cur_idx++] = buff[i];
                flag = 0;
                if(cur_idx != 0){
                    str[cur_idx++] = '\0';
                    originCode[ori_cnt++] = strdup(str);
                    // printf("%s\n", str);
                    memset(str, 0, cur_idx * sizeof(char));
                    cur_idx = 0;
                }
            }
            else if(strstr(buff, "define") != NULL && buff[i] == '\n'){
                if(cur_idx != 0){
                    str[cur_idx] = '\0';
                    if(isNum(str)){
                        char* tmp = "NUM_LITERAL";
                        originCode[ori_cnt++] = tmp;
                        // printf("%s\n", tmp);
                    }
                    else{
                        if(isVar(str)){
                            char* tmp = "VAR";
                            originCode[ori_cnt++] = tmp;
                            
                        }
                        else
                            originCode[ori_cnt++] = strdup(str);
                        // printf("%s\n", str);
                    }
                    memset(str, 0, cur_idx * sizeof(char));
                    cur_idx = 0;
                }
            }
            else if(flag == 3){
                str[cur_idx++] = buff[i];
            }

            // operation 처리하기
            else if(isOper(buff[i]) || (buff[i] == '.' && isVar(str))){
                if(cur_idx != 0){
                    str[cur_idx] = '\0';
                    if(isNum(str)){
                        char* tmp = "NUM_LITERAL";
                        originCode[ori_cnt++] = tmp;
                        // printf("%s\n", tmp);
                    }
                    else if(isFunc(str)){
                        char* tmp = "FUNC";
                        originCode[ori_cnt++] = tmp;
                    }
                    else if(isVar(str)){
                        char* tmp = "VAR";
                        originCode[ori_cnt++] = tmp;
                    }
                    else{
                        if(typeflag == 1){
                            char* tmp = "VAR";
                            originCode[ori_cnt++] = tmp;                           
                        }
                        else{
                            originCode[ori_cnt++] = strdup(str);
                        }
                    }
                    memset(str, 0, cur_idx * sizeof(char));
                    cur_idx = 0;
                }
                char tmp[2];
                tmp[0] = buff[i];
                tmp[1] = '\0';
                originCode[ori_cnt++] = strdup(tmp);
                // printf("%s\n", tmp);
            }

                        // string 토큰 판별하기
            else if(buff[i] == ',' || buff[i] == ';' || buff[i] == ' ' || buff[i] == '\n' || buff[i] == '('
            || buff[i] == ')' || buff[i] == '}' || buff[i] == '[' || buff[i] == ']' || buff[i] == '{' || buff[i] == '='){
                
                if(cur_idx != 0){
                    str[cur_idx] = '\0';
                    if(isNum(str)){
                        char* tmp = "NUM_LITERAL";
                        originCode[ori_cnt++] = tmp;
                        // printf("%s\n", tmp);
                    }
                    else if(isVar(str)){
                        char* tmp = "VAR";
                        originCode[ori_cnt++] = tmp;
                        // printf("%s\n", tmp);
                    }
                    else if(isFunc(str)){
                        char* tmp = "FUNC";
                        originCode[ori_cnt++] = tmp;
                        // printf("%s\n", tmp);
                    }
                    // 첫 선언, VAR or Func or ETC...
                    else{
                        int findBracket = i;
                        while(buff[findBracket] == ' '){
                            findBracket++;
                        }
                        
                        if(isTypeKeyword(str)){
                            if(strstr(str, "enum") != NULL){
                                enum_flag = 1;
                            }
                            originCode[ori_cnt++] = strdup(str);
                            typeflag = 1;
                        }
                        // 함수 첫 선언부
                        else if((buff[findBracket] == '(') && (typeflag == 1)){
                            char* tmp = "FUNC";
                            originCode[ori_cnt++] = tmp;
                            // printf("%s\n", tmp);
                            func_array[func_cnt++] = strdup(str);
                            typeflag = 0;
                        }
                        // 변수 첫 선언부
                        else if(typeflag == 1 || enum_flag == 1){
                            char* tmp = "VAR";
                            originCode[ori_cnt++] = tmp;
                            // printf("%s\n", tmp);
                            var_array[var_cnt++] = strdup(str);
                            if(strstr(buff, ",") != NULL){
                                typeflag = 1;
                            }
                            else{
                                typeflag = 0;
                            }
                        }
                        else{
                            originCode[ori_cnt++] = strdup(str);
                            typeflag = 1;
                        }
                    }
                    memset(str, 0, cur_idx * sizeof(char));
                    cur_idx = 0;
                }

                if(buff[i] == ';' || buff[i] == '}' || buff[i] == '\n'){
                    typeflag = 0;
                    if(buff[i] == ';' && enum_flag == 1){
                        enum_flag = 0;
                    }
                }
            }

            // 이 외 구간에는 str 연결하기
            else if(buff[i] != '\n' || buff[i] != '(' || buff[i] != ')'
            || buff[i] != '{' || buff[i] != '}' || buff[i] != '[' || buff[i] != ']'){
                str[cur_idx++] = buff[i];
            }
        }
    }

    for(int i = 0; i < ori_cnt; i++){
        char* tmp = originCode[i];
        fprintf(outputfp, "%s\n", tmp);
    }

    fprintf(outputfp, "*****\n");

    // 모든 변수 초기화
    cur_idx = 0, func_cnt = 0, var_cnt = 0, global_cnt = 0, flag = 0, typeflag = 0, enum_flag = 0;
    memset(func_array, 0, sizeof(func_array));
    memset(var_array, 0, sizeof(var_array));
    memset(global_var_array, 0, sizeof(global_var_array));

    fseek(cpyfp, 0, SEEK_END);
    cnt = ftell(cpyfp); // for save how many bytes of input file
    rewind(cpyfp);

    buff = (char*)realloc(buff, cnt);
    str[0] = '\0';

    while(fgets(buff, sizeof(char) * cnt, cpyfp) != NULL){
        for(int i = 0; i < strlen(buff); i++){
            // 주석 (//) 처리하기
            if(buff[i] == '/' && buff[i+1] == '/'){
                break;
            }

            // flag 1 : /* */ 주석 내부
            else if(buff[i] == '/' && buff[i+1] == '*'){
                flag = 1;
            }
            else if(buff[i] == '*' && buff[i+1] == '/'){
                i++;
                flag = 0;
            }
            else if(flag == 1){
                continue;
            }

            // flag 2 : string 내부
            else if((buff[i] == '\"' || buff[i] == '\'') && buff[i-1] != '\\'){
                if(flag == 0){
                    flag = 2;
                }
                else if(flag == 2){
                    char* tmp = "STR_LITERAL";
                    copiedCode[cpy_cnt++] = tmp;
                    flag = 0;
                }
            }
            else if(flag == 2){
                continue;
            }

            //flag 3 : 전처리기 내부
            else if(buff[i] == '#'){
                flag = 3;
                str[cur_idx++] = buff[i];
            }
            else if(buff[i] == '<'){
                str[cur_idx++] = buff[i];
                flag = 3;
            }
            else if(buff[i] == ' ' && flag == 3){
                if(cur_idx != 0){
                    if(strstr(buff, "define") == NULL || strstr(str, "define") != NULL){
                        str[cur_idx++] = '\0';
                        copiedCode[cpy_cnt++] = strdup(str);
                        memset(str, 0, cur_idx * sizeof(char));
                        cur_idx = 0;
                        flag = 0;
                    }
                }
            }
            else if(buff[i] == '>' && flag == 3){
                str[cur_idx++] = buff[i];
                flag = 0;
                if(cur_idx != 0){
                    str[cur_idx++] = '\0';
                    copiedCode[cpy_cnt++] = strdup(str);
                    // printf("%s\n", str);
                    memset(str, 0, cur_idx * sizeof(char));
                    cur_idx = 0;
                }
            }
            else if(strstr(buff, "define") != NULL && buff[i] == '\n'){
                if(cur_idx != 0){
                    str[cur_idx] = '\0';
                    if(isNum(str)){
                        char* tmp = "NUM_LITERAL";
                        copiedCode[cpy_cnt++] = tmp;
                        // printf("%s\n", tmp);
                    }
                    else{
                        if(isVar(str)){
                            char* tmp = "VAR";
                            copiedCode[cpy_cnt++] = tmp;
                            
                        }
                        else
                            copiedCode[cpy_cnt++] = strdup(str);
                        // printf("%s\n", str);
                    }
                    memset(str, 0, cur_idx * sizeof(char));
                    cur_idx = 0;
                }
            }
            else if(flag == 3){
                str[cur_idx++] = buff[i];
            }

            // operation 처리하기
            else if(isOper(buff[i]) || (buff[i] == '.' && isVar(str))){
                if(cur_idx != 0){
                    str[cur_idx] = '\0';
                    if(isNum(str)){
                        char* tmp = "NUM_LITERAL";
                        copiedCode[cpy_cnt++] = tmp;
                        // printf("%s\n", tmp);
                    }
                    else if(isFunc(str)){
                        char* tmp = "FUNC";
                        copiedCode[cpy_cnt++] = tmp;
                    }
                    else if(isVar(str)){
                        char* tmp = "VAR";
                        copiedCode[cpy_cnt++] = tmp;
                    }
                    else{
                        if(typeflag == 1){
                            char* tmp = "VAR";
                            copiedCode[cpy_cnt++] = tmp;                           
                        }
                        else{
                            copiedCode[cpy_cnt++] = strdup(str);
                        }
                    }
                    memset(str, 0, cur_idx * sizeof(char));
                    cur_idx = 0;
                }
                char tmp[2];
                tmp[0] = buff[i];
                tmp[1] = '\0';
                copiedCode[cpy_cnt++] = strdup(tmp);
                // printf("%s\n", tmp);
            }

                        // string 토큰 판별하기
            else if(buff[i] == ',' || buff[i] == ';' || buff[i] == ' ' || buff[i] == '\n' || buff[i] == '('
            || buff[i] == ')' || buff[i] == '}' || buff[i] == '[' || buff[i] == ']' || buff[i] == '{' || buff[i] == '='){
                
                if(cur_idx != 0){
                    str[cur_idx] = '\0';
                    if(isNum(str)){
                        char* tmp = "NUM_LITERAL";
                        copiedCode[cpy_cnt++] = tmp;
                        // printf("%s\n", tmp);
                    }
                    else if(isVar(str)){
                        char* tmp = "VAR";
                        copiedCode[cpy_cnt++] = tmp;
                        // printf("%s\n", tmp);
                    }
                    else if(isFunc(str)){
                        char* tmp = "FUNC";
                        copiedCode[cpy_cnt++] = tmp;
                        // printf("%s\n", tmp);
                    }
                    // 첫 선언, VAR or Func or ETC...
                    else{
                        int findBracket = i;
                        while(buff[findBracket] == ' '){
                            findBracket++;
                        }
                        
                        if(isTypeKeyword(str)){
                            if(strstr(str, "enum") != NULL){
                                enum_flag = 1;
                            }
                            copiedCode[cpy_cnt++] = strdup(str);
                            typeflag = 1;
                        }
                        // 함수 첫 선언부
                        else if((buff[findBracket] == '(') && (typeflag == 1)){
                            char* tmp = "FUNC";
                            copiedCode[cpy_cnt++] = tmp;
                            // printf("%s\n", tmp);
                            func_array[func_cnt++] = strdup(str);
                            typeflag = 0;
                        }
                        // 변수 첫 선언부
                        else if(typeflag == 1 || enum_flag == 1){
                            char* tmp = "VAR";
                            copiedCode[cpy_cnt++] = tmp;
                            // printf("%s\n", tmp);
                            var_array[var_cnt++] = strdup(str);
                            if(strstr(buff, ",") != NULL){
                                typeflag = 1;
                            }
                            else{
                                typeflag = 0;
                            }
                        }
                        else{
                            copiedCode[cpy_cnt++] = strdup(str);
                            typeflag = 1;
                        }
                    }
                    memset(str, 0, cur_idx * sizeof(char));
                    cur_idx = 0;
                }

                if(buff[i] == ';' || buff[i] == '}' || buff[i] == '\n'){
                    typeflag = 0;
                    if(buff[i] == ';' && enum_flag == 1){
                        enum_flag = 0;
                    }
                }
            }

            // 이 외 구간에는 str 연결하기
            else if(buff[i] != '\n' || buff[i] != '(' || buff[i] != ')'
            || buff[i] != '{' || buff[i] != '}' || buff[i] != '[' || buff[i] != ']'){
                str[cur_idx++] = buff[i];
            }
        }
    } 

    for(int i = 0; i < cpy_cnt; i++){
        char* tmp = copiedCode[i];
        fprintf(outputfp, "%s\n", tmp);
    }
    fprintf(outputfp, "*****\n");

    char* result[100000];

    int res_cnt = LCS(originCode, ori_cnt, copiedCode, cpy_cnt, result);

    for(int i = 0; i < res_cnt; i++){
        fprintf(outputfp, "%s\n", result[i]);
    }

    fprintf(outputfp, "*****\n");

    float ratio = ((float)res_cnt / ((ori_cnt > cpy_cnt) ? cpy_cnt : ori_cnt)) * 100;

    fprintf(outputfp, "%.2f", ratio);

    free(buff);
    fclose(orifp);
    fclose(cpyfp);
    return 0;
}
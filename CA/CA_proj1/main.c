#include <stdio.h>
#include <stdlib.h>

void print_reg_R(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    printf("x%d, x%d, x%d", rd, rs1, rs2);
}

void print_reg_shamt(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    printf("x%d, x%d, %d", rd, rs1, rs2);   
}

void print_reg_I(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int imm = (n >> 20);
    printf("x%d, x%d, %d", rd, rs1, imm);
}

void print_reg_S(int n){
    // 20까지 땡기고 하위 5비트 0으로 만들기
    int imm = ((n >> 20) & 4064) + ((n >> 7) & 31);
    // 음수로 바꿔주기
    if(imm & 2048){
        imm |= 4294963200;
    }

    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    printf("x%d, %d(x%d)", rs2, imm, rs1);
    return;
}

void print_reg_I_s(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int imm = (n >> 20);
    printf("x%d, %d(x%d)", rd, imm, rs1);
}

void print_reg_B(int n){

    // 비트 자르기
    int imm = ((n >> 31) << 12) +
                (((n >> 25) & 63) << 5) +
                (((n >> 7) & 1) << 11) +
                (((n >> 8) & 15) << 1);

    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    printf("x%d, x%d, %d", rs1, rs2, imm);
}

void print_reg_J(int n){
    int imm = ((n >> 31) << 20) +
                (((n >> 12) & 255) << 12) +
                (((n >> 20) & 1) << 11) +
                (((n >> 21) & 1023) << 1);
    int rd = (n >> 7) & 31;
    printf("x%d, %d", rd, imm);
}

void print_reg_U(int n){
    long imm = (n >> 12) << 12;
    int rd = (n >> 7) & 31;
    printf("x%d, %ld", rd, imm);
}

void check_risc_V_51(int n){
    if(((n >> 12) & 7) == 7){
        printf(" and ");
    }
    else if(((n >> 12) & 6) == 6){
        printf(" or ");
    }
    else if(((n >> 12) & 5) == 5){
        if(((n >> 30) & 1) == 1)
            printf(" sra ");
        else    printf(" srl ");
    }
    else if(((n >> 12) & 4) == 4){
        printf(" xor ");
    }
    else if(((n >> 12) & 3) == 3){
        printf(" sltu ");
    }
    else if(((n >> 12) & 2) == 2){
        printf(" slt ");
    }
    else if(((n >> 12) & 1) == 1){
        printf(" sll ");
    }
    else if(((n >> 12) & 0) == 0){
        if(((n >> 30) & 1) == 1)
            printf(" sub ");
        else    printf(" add ");
    }
    print_reg_R(n);
    return;
}

void check_risc_V_19(int n){
    if(((n >> 12) & 7) == 7){
        printf(" andi ");
        print_reg_I(n);
    }
    else if(((n >> 12) & 6) == 6){
        printf(" ori ");
        print_reg_I(n);
    }
    else if(((n >> 12) & 5) == 5){
       if(((n >> 30) & 1) == 1)
            printf(" srai ");
        else    printf(" srli ");
        print_reg_shamt(n);
    }
    else if(((n >> 12) & 4) == 4){
        printf(" xori ");
        print_reg_I(n);
    }
    else if(((n >> 12) & 3) == 3){
        printf(" sltiu ");
        print_reg_I(n);
    }
    else if(((n >> 12) & 2) == 2){
        printf(" slti ");
        print_reg_I(n);
        
    }
    else if(((n >> 12) & 1) == 1){
        printf(" slli ");
        print_reg_shamt(n);
    }
    else if(((n >> 12) & 0) == 0){
        printf(" addi ");
        print_reg_I(n);
    }
    return;
}

void check_risc_V_35(int n){
    if(((n >> 12) & 2) == 2){
        printf(" sw ");
    }
    else if(((n >> 12) & 1) == 1){
        printf(" sh ");
    }
    else if(((n >> 12) & 0) == 0){
        printf(" sb ");
    }
    print_reg_S(n);
    return;
}

void check_risc_V_3(int n){
    if(((n >> 12) & 5) == 5){
        printf(" lhu ");
    }
    else if(((n >> 12) & 4) == 4){
        printf(" lbu ");
    }
    else if(((n >> 12) & 2) == 2){
        printf(" lw ");
    }
    else if(((n >> 12) & 1) == 1){
        printf(" lh ");
    }
    else if(((n >> 12) & 0) == 0){
        printf(" lb ");
    }
    print_reg_I_s(n);
    return;
}

void check_risc_V_99(int n){
    if(((n >> 12) & 7) == 7){
        printf(" bgeu ");
    }
    else if(((n >> 12) & 6) == 6){
        printf(" bltu ");
    }
    else if(((n >> 12) & 5) == 5){
        printf(" bge ");
    }
    else if(((n >> 12) & 4) == 4){
        printf(" blt ");
    }
    else if(((n >> 12) & 1) == 1){
        printf(" bne ");
    }
    else if(((n >> 12) & 0) == 0){
        printf(" beq ");
    }
    print_reg_B(n);
}

void check_risc_V_103(int n){
    printf(" jalr ");
    print_reg_I_s(n);
}

void check_risc_V_111(int n){
    printf(" jal ");
    print_reg_J(n);
}

void check_risc_V_23(int n){
    printf(" auipc ");
    print_reg_U(n);
}

void check_risc_V_55(int n){
    printf(" lui ");
    print_reg_U(n);
}

int main(int argc, char* argv[]){

    FILE *fp;
    unsigned int* buff;
    int cnt;

    fp = fopen(argv[1], "rb");

    if(fp == NULL){
        printf("Error!\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    cnt = ftell(fp);
    rewind(fp);

    buff = (unsigned int*)malloc(cnt);
    fread(buff, sizeof(unsigned int), cnt, fp);

    for(int i = 0; i < cnt/4; i ++) {
        printf("inst %d: ", i);
        printf("%08x", buff[i]);

        if((buff[i] & 127) == 111){
            check_risc_V_111(buff[i]);
        }

        else if((buff[i] & 127) == 103){
            check_risc_V_103(buff[i]);
        }

        else if((buff[i] & 127) == 99){
            check_risc_V_99(buff[i]);
        }

        else if((buff[i] & 127) == 55){
            check_risc_V_55(buff[i]);
        }

        else if((buff[i] & 127) == 51){
            check_risc_V_51(buff[i]);
        }

        else if((buff[i] & 127) == 35){
            check_risc_V_35(buff[i]);
        }

        else if((buff[i] & 127) == 23){
            check_risc_V_23(buff[i]);
        }

        else if((buff[i] & 127) == 19){
            check_risc_V_19(buff[i]);
        }

        else if((buff[i] & 127) == 3){
            check_risc_V_3(buff[i]);
        }

        else{
            printf(" unknown instruction");
        }

        printf("\n");
    }

    fclose(fp);
    free(buff);
    return 0;
}
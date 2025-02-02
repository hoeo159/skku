#include <stdio.h>
#include <stdlib.h>

int reg[32];
int variable[65540];

void add_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    reg[rd] = reg[rs1] + reg[rs2];
}

void sub_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    reg[rd] = reg[rs1] - reg[rs2];
}

void and_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    reg[rd] = (reg[rs1] & reg[rs2]);
}

void or_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    reg[rd] = (reg[rs1] | reg[rs2]);
}

void xor_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    reg[rd] = (reg[rs1] ^ reg[rs2]);
}

void sra_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    reg[rd] = reg[rs1] >> (reg[rs2] % 32);
    if (reg[rs1] < 0) {
        reg[rd] |= ((1 << reg[rs2]) - 1) << (32 - reg[rs2]);
    }
}

void srl_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    unsigned int rs2 = (n >> 20) & 31;
    reg[rd] = (unsigned int)reg[rs1] >> (reg[rs2] % 32);
}

void slt_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    reg[rd] = (reg[rs1] < reg[rs2]) ? 1 : 0;
}

void sll_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    reg[rd] = reg[rs1] << (reg[rs2] % 32);
}

void addi_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int imm = (n >> 20);
    reg[rd] = reg[rs1] +imm;
}

void slti_oper(int n) {
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int imm = (n >> 20);
    reg[rd] = (reg[rs1] < imm) ? 1 : 0;
}

void xori_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int imm = (n >> 20);
    reg[rd] = (reg[rs1] ^ imm);
}

void ori_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int imm = (n >> 20);
    reg[rd] = (reg[rs1] | imm);
}

void andi_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int imm = (n >> 20);
    reg[rd] = (reg[rs1] & imm);
}

void srai_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int imm = (n >> 20) & 31;
    reg[rd] = reg[rs1] >> imm;
    if (reg[rs1] < 0) {
        reg[rd] |= ((1 << imm) - 1) << (32 - imm);
    }
}

void srli_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    unsigned int imm = (n >> 20) & 31;
    reg[rd] = ((unsigned int)reg[rs1]) >> imm;
}

void slli_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int imm = (n >> 20) & 31;
    reg[rd] = reg[rs1] << imm;
}

void sw_oper(int n){
//  rs1[imm / 4] = rs2;

    int rs2 = (n >> 20) & 31;
    int rs1 = (n >> 15) & 31;
    
    int imm = ((n >> 20) & 4064) + ((n >> 7) & 31);
    // 음수로 바꿔주기
    if(imm & 2048){
        imm |= 4294963200;
    }
    if((reg[rs1] & 0x20000000) == 0x20000000){
        char value_8bit = reg[rs2];
        printf("%c",value_8bit);
        return;
    }
    int offset = reg[rs1] % 0x10000000;

    int tmp = reg[rs2];

    for(int i = 0; i < 4; i++){
        variable[offset + imm + i] = tmp & 0xFF;
        tmp = (unsigned int)tmp >> 8;
    }
}


void lw_oper(int n){
    int rd = (n >> 7) & 31;
    int rs1 = ((n >> 15) & 31);
    int imm = (n >> 20);

    int tmp = reg[0];
    int offset = reg[rs1] % 0x10000000;

    if((reg[rs1] & 0x20000000) == 0x20000000){
        int tmp;
        scanf("%d", &tmp);
        reg[rd] = tmp;
        return;
    }

    for(int i = 0; i < 4; i++){
        tmp += (variable[offset + imm + i] << (8 * i));
    }

    reg[rd] = tmp;
    // printf("x%d, %d(x%d)", rd, imm, rs1);
}

int bge_oper(int n){
    // 비트 자르기
    int imm = ((n >> 31) << 12) +
                (((n >> 25) & 63) << 5) +
                (((n >> 7) & 1) << 11) +
                (((n >> 8) & 15) << 1);

    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    
    if(reg[rs1] >= reg[rs2]){
        return imm;
    }
    return -1;
}

int blt_oper(int n){
    // 비트 자르기
    int imm = ((n >> 31) << 12) +
                (((n >> 25) & 63) << 5) +
                (((n >> 7) & 1) << 11) +
                (((n >> 8) & 15) << 1);

    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    
    if(reg[rs1] < reg[rs2]){
        return imm;
    }
    return -1;
}

int bne_oper(int n){
    // 비트 자르기
    int imm = ((n >> 31) << 12) +
                (((n >> 25) & 63) << 5) +
                (((n >> 7) & 1) << 11) +
                (((n >> 8) & 15) << 1);

    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    
    if(reg[rs1] != reg[rs2]){
        return imm;
    }
    return -1;
}

int beq_oper(int n){
    // 비트 자르기
    int imm = ((n >> 31) << 12) +
                (((n >> 25) & 63) << 5) +
                (((n >> 7) & 1) << 11) +
                (((n >> 8) & 15) << 1);

    int rs1 = (n >> 15) & 31;
    int rs2 = (n >> 20) & 31;
    
    if(reg[rs1] == reg[rs2]){
        return imm;
    }
    return -1;
}

void print_reg_I_s(int n){
    // int rd = (n >> 7) & 31;
    // int rs1 = (n >> 15) & 31;
    // int imm = (n >> 20);
    // printf("x%d, %d(x%d)", rd, imm, rs1);
}

void check_risc_V_51(int n){
    if(((n >> 12) & 7) == 7){
        and_oper(n);
    }
    else if(((n >> 12) & 6) == 6){
        or_oper(n);
    }
    else if(((n >> 12) & 5) == 5){
        if(((n >> 30) & 1) == 1)
            sra_oper(n);
        else    srl_oper(n);
    }
    else if(((n >> 12) & 4) == 4){
        xor_oper(n);
    }
    else if(((n >> 12) & 2) == 2){
        slt_oper(n);
    }
    else if(((n >> 12) & 1) == 1){
        sll_oper(n);
    }
    else if(((n >> 12) & 0) == 0){
        if(((n >> 30) & 1) == 1)
            sub_oper(n);
        else    add_oper(n);
    }
    return;
}

void check_risc_V_19(int n){
    if(((n >> 12) & 7) == 7)
        andi_oper(n);
    else if(((n >> 12) & 6) == 6)
        ori_oper(n);
    
    else if(((n >> 12) & 5) == 5){
        if(((n >> 30) & 1) == 1)
            srai_oper(n);
        else
            srli_oper(n);
    }
    else if(((n >> 12) & 4) == 4)
        xori_oper(n);
    else if(((n >> 12) & 2) == 2)
        slti_oper(n);
    else if(((n >> 12) & 1) == 1)
        slli_oper(n);
    else if(((n >> 12) & 0) == 0)
        addi_oper(n);
    return;
}

void check_risc_V_35(int n){
    if(((n >> 12) & 2) == 2)
        sw_oper(n);
    return;
}

void check_risc_V_3(int n){
    if(((n >> 12) & 2) == 2){
        lw_oper(n);
    }
    return;
}

int check_risc_V_99(int n){
    if(((n >> 12) & 5) == 5)
        return bge_oper(n);
    else if(((n >> 12) & 4) == 4)
        return blt_oper(n);
    else if(((n >> 12) & 1) == 1)
        return bne_oper(n);
    else if(((n >> 12) & 0) == 0)
        return beq_oper(n);
}

int jalr_oper(int n, int pc){
    //jalr
    int rd = (n >> 7) & 31;
    int rs1 = (n >> 15) & 31;
    int imm = (n >> 20);
    int jump = reg[rs1];
    reg[rd] = (pc + 1) * 4;
    reg[0] = 0;
    return imm + jump;
}

int jal_oper(int n, int pc){
    int imm = ((n >> 31) << 20) +
                (((n >> 12) & 255) << 12) +
                (((n >> 20) & 1) << 11) +
                (((n >> 21) & 1023) << 1);
    int rd = (n >> 7) & 31;
    reg[rd] = (pc + 1) * 4;
    reg[0] = 0;
    return imm;
}

void auipc_oper(int n, int pc){
    long imm = (n >> 12) << 12;
    int rd = (n >> 7) & 31;
    reg[rd] = pc * 4 + imm; // *4 수정
    reg[0] = 0;
}

void check_risc_V_55(int n){
    // lui
    long imm = (n >> 12) << 12;
    int rd = (n >> 7) & 31;
    reg[rd] = (imm & 0xFFFFF000);
}

int main(int argc, char* argv[]){
    
    FILE *fp, *var_fp;
    unsigned int* buff;
    unsigned int* var_buff;
    int N = 0, var_size = 0, var_idx = 0;
    int inst_count = 0;
    int num = 100;

    if(argc == 3){
        N = atoi(argv[2]);
    }
    else if(argc == 4){
        N = atoi(argv[3]);
    }
    inst_count = N; // 17inst

    fp = fopen(argv[1], "rb");
    
    if(argc == 4){
        var_fp = fopen(argv[2], "rb");

        if(var_fp == NULL){
            printf("Error!\n");
            return -1;
        }
    }

    if(fp == NULL){
        printf("Error!\n");
        return -1;
    }


    fseek(fp, 0, SEEK_END);
    int size = ftell(fp) / 4;
    rewind(fp);
    N = (size < N) ? size : N;

    buff = (unsigned int*)malloc(size * sizeof(unsigned int)); // buff는 항상 전체 파일 inst 읽기

    if(argc == 4){
        fseek(var_fp, 0, SEEK_END);
        int var_size = ftell(var_fp) / 4;
        rewind(var_fp);
        var_buff = (unsigned int*)malloc(var_size * sizeof(unsigned int));

        for(int idx = 0; idx < var_size; idx++){
            fread(&var_buff[idx], sizeof(unsigned int), 1, var_fp);
            for(int j = 0; j < 4; j++){
                variable[var_idx] = var_buff[idx] & 0xFF;
                var_buff[idx] = ((unsigned int)var_buff[idx] >> 8);
                var_idx++;
            }
        }
    }

    int i = 0;

    unsigned int* inst = (unsigned int*) malloc(size * sizeof(unsigned int));

    for(int i = 0; i < size; i++){
        fread(&buff[i], sizeof(unsigned int), 1, fp);
        inst[i] = buff[i];
    } 
    // 수정 : i <= N
    while(1){
        inst_count--;
        if(inst_count < 0){
            break;
        }
        // printf("inst : %d > %08x\n", i, inst[i]);

        if((inst[i] & 127) == 111){
            int offset = jal_oper(inst[i], i);
            if(offset != -1){
                i += (offset / 4);
                continue;
            }
        }

        else if((inst[i] & 127) == 103){
            int offset = jalr_oper(inst[i], i);
            if(offset != -1){
                i = (offset / 4);
                continue;
            }
        }

        else if((inst[i] & 127) == 99){
            int offset = check_risc_V_99(inst[i]);
            if(offset != -1){
                i += (offset / 4);
                continue;
            }
            else{

            }
        }

        else if((inst[i] & 127) == 55){
            check_risc_V_55(inst[i]);
        }

        else if((inst[i] & 127) == 51){
            check_risc_V_51(inst[i]);
        }

        else if((inst[i] & 127) == 35){
            check_risc_V_35(inst[i]);
        }

        else if((inst[i] & 127) == 23){
            auipc_oper(inst[i], i);
        }

        else if((inst[i] & 127) == 19){
            check_risc_V_19(inst[i]);
        }

        else if((inst[i] & 127) == 3){
            check_risc_V_3(inst[i]);
        }

        else{
            break;
        }

        reg[0] = 0;
        i++;
    }

    for(i = 0; i < 32; i++){
        printf("x%d: 0x%08x\n", i, reg[i]);
    }

    fclose(fp);
    free(buff);
    return 0;
}
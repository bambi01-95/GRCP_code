/* --------------------------------------------
bambi01-95 m058　
2023/07/05
1つのSTACKで全てのinstとpc＊を管理する
--------------------------------------------*/ 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

enum { PRINT, 
       DUP, INT, ADD, SUB, MUL, DIV, MOD, LESS, 
       JUMP, JUMPF, CALL, RET, 
       SWAP, PICK, DROP, HALT};

int inst[64];
int size = (sizeof(inst)/sizeof(*inst));
const int MAX = (sizeof(inst)/sizeof(*inst));

void push(int n){assert(size>0);inst[--size] = n;}
int pop(){assert(size<MAX);return inst[size++];}
int top(){return inst[size];}

int *inst_pc[64];
int size_pc = MAX;

void push_pc(int *n){
    assert(size_pc>0);
    inst_pc[--size_pc] = n;
}

int *pop_pc(){
    assert(size_pc<MAX);
    return inst_pc[size_pc++];
}



int run(int *pc){
    for(;;){
        int inst = *pc++;
        switch(inst){
            case PRINT:{printf("%d\n",pop());continue;}

            case DUP:{push(top());continue;};
            case INT:{int value = *pc++;push(value);continue;}
            case ADD:{int r = pop(); int l = pop();push(r+l);continue;}
            case SUB:{int r = pop(); int l = pop();push(l-r);continue;}
            case MUL:{int r = pop(); int l = pop();push(r*l);continue;}
            case DIV:{int r = pop(); int l = pop();push(l/r);continue;}
            case MOD:{int r = pop(); int l = pop();push(l%r);continue;}
            case LESS:{int r = pop();int l = pop();push(r>l);continue;}

            case JUMP:{int value = *pc++;pc += value;continue;}
            case JUMPF:{int value = *pc++;if(!pop()){pc += value;}continue;}

            case CALL:{int value = *pc++;push_pc(pc);pc +=value;continue;}
            case RET:{pc = pop_pc();continue;}

            case SWAP:{int a = pop();int b = pop();push(a);push(b);continue;}
            case PICK:{int value = pop();continue;}
            case DROP:{pop();continue;}

            case HALT:
                return 0;
            default:
                fprintf(stderr,"ARE YOU KIDDING\n");
                exit(1);
        }
    }
    return 0;
}
int program[] = {
    HALT,
};

int main(){
    run(program);
    return 0;
}



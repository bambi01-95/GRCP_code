#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

enum { PRINT,DUP, INT, ADD,SUB,MUL,DIV,MOD,LESS,JUMP,JUMPF,  HALT};

int inst[32];
int size = (sizeof(inst)/sizeof(*inst));
void push(int n){
    inst[--size] = n;
}
int pop(){
    return inst[size++];
}
int top(){
    return inst[size];
}

int program[] = {
    INT,    0,      // N = 0
    DUP,
    INT,   10,      // while (N < 10)
    LESS,
    JUMPF,  7,      // --> B
    DUP,
    PRINT,          //   print N
    INT,    1,      //   N += 1
    ADD,
    JUMP, -13,      // --> A
    HALT,
};

int vm(int *pc){
    for(;;){
        int inst = *pc++;
        switch(inst){
            case PRINT:{printf("%d\n",top());continue;}
            case DUP:{push(top());continue;};
            case INT:{int value = *pc++;push(value);continue;}
            case ADD:{int r = pop();int l = pop();push(r+l);continue;}
            case SUB:{int r = pop();int l = pop();push(l-r);continue;}
            case MUL:{int r = pop();int l = pop();push(r*l);continue;}
            case DIV:{int r = pop();int l = pop();push(l/r);continue;}
            case MOD:{int r = pop();int l = pop();push(l%r);continue;}
            case LESS:{int r = pop();int l = pop();push(r>l);continue;}
            case JUMP:{int value = *pc++;pc += value;continue;}
            case JUMPF:{int value = *pc++;if(!pop()){pc += value;}continue;}
            case HALT:
                return 0;
            default:
                fprintf(stderr,"nononono\n");
                exit(1);
        }
    }

    return 0;
}

int main(){
    vm(program);
    return 0;
}
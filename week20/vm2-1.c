/* --------------------------------------------
bambi01-95 m058　
2023/07/05
vm で関数を呼び出す

追加
SWAP, PICK, DROP, RET,REL,CALL
内容
DOUBLE()とnfib()を作成
--------------------------------------------*/ 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

enum { PRINT, DUP, INT, ADD, SUB, MUL, DIV, MOD, LESS,  JUMP, JUMPF, CALL, HALT, 
        SWAP, PICK, DROP, RET,};

int inst[64];
int size = (sizeof(inst)/sizeof(*inst));
const int MAX = (sizeof(inst)/sizeof(*inst));

void push(int n){assert(size>0);inst[--size] = n;}
int pop(){assert(size<MAX);return inst[size++];}
int top(){return inst[size];}
void pick(int n){push(inst[size + n]);}

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



int test_double[]  = {
    INT, 66,
    CALL, 7,
    PRINT,

    INT, 6,
    CALL, 2,
    PRINT,

    HALT,

    DUP,
    ADD,
    RET,
};


int fib[] = {
    INT, 15,

    CALL, 4,
    DUP,
    PRINT,
    DROP,
    HALT,

    DUP,
    INT, 2,
    LESS,
    JUMPF,4,//if(n<2)
    DROP,
    INT, 1,
    RET,    //return 1;

    DUP,    // n n
    INT, 1, // n n 1
    SUB,    // n n-1
    CALL, -16, // n fib(n-1)
    
    SWAP,   // fib(n-1) n 
    INT, 2, // fib(n-1) n 2
    SUB,    // fib(n-1) n-2
    CALL, -22,// fib(n-1) fib(n-2)
    ADD,    // fib(n-1)+fib(n-2)

    INT, 1, // fib(n-1)+fib(n-2) 1
    ADD,    // fib(n-1)+fib(n-2)+1
    RET,    
};


int fib15[] = {
    INT, 15,

    CALL, 4,
    DUP,
    PRINT,
    DROP,
    HALT,

    DUP,
    INT, 2,
    LESS,
    JUMPF,4,//if(n<2)
    DROP,
    INT, 1,
    RET,    //return 1;

    DUP,    // n n
    INT, 1, // n n 1
    SUB,    // n n-1
    CALL, -16, // n fib(n-1)
    
    PICK, 1,   // fib(n-1) n 
    INT, 2, // fib(n-1) n 2
    SUB,    // fib(n-1) n-2
    CALL, -23,// fib(n-1) fib(n-2)
    ADD,    // fib(n-1)+fib(n-2)

    INT, 1, // fib(n-1)+fib(n-2) 1
    ADD,    // fib(n-1)+fib(n-2)+1
    SWAP,
    DROP,
    RET,    
};


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
            case PICK:{int value = *pc++;pick(value);continue;}
            case DROP:{pop();continue;}
            case HALT:
                return 0;
            default:
                fprintf(stderr,"nononono\n");
                exit(1);
        }
    }
    return 0;
}

int nfib(int n){
    if(n<2)return 1;
    return nfib(n-1) + nfib(n-2) + 1;
}

int main(){
    printf("vm2-1\n");
    run(fib15);
    printf("stock in %d elements\n",MAX - size);
    return 0;
}
// address to address だdファdsファdファdsファdsf
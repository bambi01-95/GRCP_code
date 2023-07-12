/* --------------------------------------------
    bambi01-95 m058　
    2023/07/05
    1つのSTACKで全てのstackとpc＊を管理する
--------------------------------------------*/ 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

enum { 
    /*data*/INT, DUP ,DROP, SWAP, PICK,
    /*ope */ADD, SUB, MUL, DIV, MOD, LESS,
    /*func*/CALL,RET,
    /*cond*/JUMP,JUMPF,
    /*func*/PRINT, 
    /*end */HALT};

int *stack[64];
int size = (sizeof(stack)/sizeof(*stack));
const int MAX = (sizeof(stack)/sizeof(*stack));

void push(int *n){assert(size>0);stack[--size] = n;}
void ipush(int i){push((int*)(intptr_t)i);}

int *pop(){assert(size<MAX);return stack[size++];}
int ipop(){return(intptr_t)pop();}

int *top(){return stack[size];}
void pick(int n){push(stack[size+n]);}


int run(int *pc){
    for(;;){
        int stack = *pc++;
        switch(stack){
            case INT:{int value = *pc++;ipush(value);continue;}
            case DUP:{push(top());continue;};
            case DROP:{pop();continue;}
            case SWAP:{int *a = pop();int *b = pop();push(a);push(b);continue;}
            case PICK:{int value = *pc++;pick(value);continue;}

            case ADD:{int r = ipop(); int l = ipop();ipush(r+l);continue;}
            case SUB:{int r = ipop(); int l = ipop();ipush(l-r);continue;}
            case MUL:{int r = ipop(); int l = ipop();ipush(r*l);continue;}
            case DIV:{int r = ipop(); int l = ipop();ipush(l/r);continue;}
            case MOD:{int r = ipop(); int l = ipop();ipush(l%r);continue;}
            case LESS:{int r = ipop();int l = ipop();ipush(r>l);continue;}

            case CALL:{int value = *pc++;int* p = pop();push(pc);push(p);pc+=value;continue;}
            case RET:{int *p = pop();pc = pop();push(p);continue;}

            case JUMP:{int value = *pc++;pc += value;continue;}
            case JUMPF:{int value = *pc++;if(!pop()){pc += value;}continue;}

            case PRINT:{printf("%d\n",ipop());continue;}
            case HALT:{return 0;}
            default:
                fprintf(stderr,"ARE YOU KIDDING\n");
                exit(1);
        }
    }
    return 0;
}


int program[] = {
    INT, 15,

    CALL, 4,
    DUP,
    PRINT,
    DROP,
    HALT,

    DUP,
    INT, 2,
    LESS,
    JUMPF,4,
    DROP,
    INT, 1,
    RET,    

    DUP,    
    INT, 1, 
    SUB,    
    CALL, -16, 
    
    PICK, 1,  
    INT, 2, 
    SUB,    
    CALL, -23,
    ADD,    
    INT, 1, 
    ADD,   
    SWAP,
    DROP, 
    RET,  
};

int main(){
    printf("vm2-2.c\n");
    run(program);
    printf("stock in %d elements\n",MAX - size);
    return 0;
}
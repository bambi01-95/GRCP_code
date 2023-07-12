/* --------------------------------------------
    bambi01-95 m058　
    2023/07/11
    / TITLE / ()
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

int *inst[64];
int size = (sizeof(inst)/sizeof(*inst));
const int MAX = (sizeof(inst)/sizeof(*inst));


void push(int *n){assert(size>0);inst[--size] = n;}
void ipush(int i){push((int*)(intptr_t)i);}

int *pop(){assert(size<MAX);return inst[size++];}
int ipop(){return(intptr_t)pop();}

int *top(){return inst[size];}
int itop(){return(intptr_t)top();}
void pick(int n){push(inst[size + n]);}


int run(int *pc){
    for(;;){
        int inst = *pc++;
        switch(inst){
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

            case CALL:{int value = *pc++;push(pc);pc+=value;continue;}
            //pop result, pop PC, pop 1 argument, push result, continue
            case RET:{
                int value = *pc++;
                int *result = pop();
                pc = pop();
                while(value--){
                    pop();
                }
                push(result);  
                continue;
            }
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
    INT,    15,// n
    CALL,   2, // n p
    PRINT,
    HALT,
    PICK,   1,	//n p n
    INT,    2,  //n p n 2
    LESS,       //n p
    JUMPF,  4,  //n p
    INT,    1,      
    RET,    1,     
    PICK,   1,  
    INT,    1,  
    SUB,        
    CALL,   -18,
    PICK,   2,   
    INT,    2,   
    SUB,         
    CALL,   -25,
    ADD,        
    INT,    1,   
    ADD,        
    RET,    1,  // pop result, pop PC, pop 1 argument, push result, continue        
};


int main(){
    printf("vm2-2\n");
    run(program);
    printf("stock in %d elements\n",MAX - size);
    return 0;
}
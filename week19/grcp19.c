#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

enum { PRINT, DUP, INT, ADD, SUB, MUL, DIV, MOD, LESS, IF, JUMP, JUMPF, CALL, HALT};

int inst[32];
int size = (sizeof(inst)/sizeof(*inst));
const int MAX = (sizeof(inst)/sizeof(*inst));

void push(int n){
    assert(size>0);
    inst[--size] = n;
}

int pop(){
    assert(size<MAX);
    return inst[size++];
}

int top(){
    return inst[size];
}


int loop_program[] = {
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


int if_program[] = {
    INT,    0,      // N = 0
    DUP,            
    INT,   10,      // while (N1 < 10)
    LESS,           
    JUMPF,  15,      // --> B

    DUP,          
    INT, 2,
    MOD,            // if(N % 2)
    IF,
    JUMP, 1,        // else -->
    PRINT,

    DUP,
    PRINT,          //   print N
    INT,    1,      //   N += 1
    ADD,
    JUMP, -21,      // --> A
    HALT,
};


int test_if_program[] = {
    INT, 3,
    INT, 2,
    LESS,
    IF,             
    JUMP,3,
    INT, 32,
    PRINT,
    INT, 23,
    PRINT,
    HALT,
};

// ???? / define from the users?
int *functions[] = { test_if_program,if_program,loop_program };

int test_call[] = {
    CALL,0,
    CALL,1,
    CALL,2,
    HALT,
};

int vm(int *pc){
    for(;;){
        int inst = *pc++;
        switch(inst){
            case PRINT:{printf("%d\n",top());continue;}

            case DUP:{push(top());continue;};
            case INT:{int value = *pc++;push(value);continue;}
            case ADD:{int r = pop(); int l = pop();push(r+l);continue;}
            case SUB:{int r = pop(); int l = pop();push(l-r);continue;}
            case MUL:{int r = pop(); int l = pop();push(r*l);continue;}
            case DIV:{int r = pop(); int l = pop();push(l/r);continue;}
            case MOD:{int r = pop(); int l = pop();push(l%r);continue;}
            case LESS:{int r = pop();int l = pop();push(r>l);continue;}

            case IF:{if(pop()){pc+=2;}continue;}
            case CALL:{int value = *pc++;vm(functions[value]);continue;}

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
    vm(test_call);
    // vm(loop_program);
    // putchar('\n');
    // vm(test_if_program);
    // putchar('\n');
    // vm(if_program);
    // putchar('\n');
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef enum { ILLEGAL, Integer, Float} type_t;
union Object;
typedef union Object Object;
typedef Object* oop;

/*
    int -> addr
    12
    |00000000001010|
    |00000000010101|

    addr -> int
    |00000000010101|
    |00000000001010|
*/

struct Integer{
    type_t type;
    int   value;
};
struct Float{
    type_t type;
    float value;
};
union Object{
    type_t type;
    struct Integer Integer;
    struct Float Float;
};
oop newObject(type_t t){
    oop obj = malloc(sizeof(Object));
    obj->type = t;
    return obj;
}
type_t Object_type(oop obj){
    return((intptr_t)obj&1)?Integer:obj->type;
}

// int -> ptr
oop newInteger(int value){
    return (oop)(long)((value<<1)|1);
}

// ptr -> int
int Integer_value(oop obj){
    return ((intptr_t)obj)>>1;
}

oop newFloat(float value);
float Float_value(oop obj);
void println(oop obj);

oop prim_add(oop a,oop b){
    assert(Object_type(a)==Integer);
    assert(Object_type(b)==Integer);
    return newInteger(Integer_value(a) + Integer_value(b));
}

int main(){

    println(newInteger(4));
    println(newFloat(3.33));
    println(prim_add(newInteger(4),newInteger(59)));

    for(int i=1;i<=16;i++){
        printf("%2d: %p\n",i,newInteger(i));
        printf("           ->%d\n",Integer_value(newInteger(i)));
    }
    Object_type(newInteger(4));
    return 0;
    int a = 10;
    int* b;
    b = a;

}
/*
001
011 ->3

010
101 ->5

011
111 ->7

0100
1001->9
*/

oop newFloat(float value){
    oop obj = newObject(Float);
    obj->Float.value = value;
    return obj;
}

float Float_value(oop obj){
    assert(Object_type(obj)==Float);
    return obj->Float.value;
}
void println(oop obj){
    switch(Object_type(obj)){
        case Integer: printf("%d\n",Integer_value(obj));return;
        case Float:   printf("%f\n",Float_value(obj));return;
        default:assert(0);
    }
    return;
}
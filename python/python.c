#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

union Object;

typedef union Object Object;
typedef Object *oop;

typedef enum { ILLEGAL, Undefined, Integer, Symbol, Cell } type_t;

struct Integer { type_t type;  int   value; };
struct Symbol  { type_t type;  char *name;  };
struct Cell    { type_t type;  oop   a, d;  };

union Object
{
    type_t type;
    struct Integer Integer;
    struct Symbol  Symbol;
    struct Cell    Cell;
};

oop nil     = 0;
oop t       = 0;
oop globals = 0;

oop newObject(type_t type)
{
    oop obj = calloc(1, sizeof(Object));
    obj->type = type;
    return obj;
}

type_t Object_type(oop obj)
{
    assert(obj); // not a NULL pointer
    return obj->type;
}

oop newInteger(int value)
{
    oop obj = newObject(Integer);
    obj->Integer.value = value;
    return obj;
}

int Integer_value(oop obj)
{
    assert(Object_type(obj) == Integer);
    return obj->Integer.value;
}

char *Symbol_name(oop obj)
{
    assert(Object_type(obj) == Symbol);
    return obj->Symbol.name;
}

int  symbolCount = 0;
oop *symbolTable = 0;

oop newSymbol(char *name)
{
    int lo = 0, hi = symbolCount - 1;
    while (lo <= hi) {
	int mi = (lo + hi) / 2;
	oop sym = symbolTable[mi];
	int cmp = strcmp(name, Symbol_name(sym));
	if      (cmp < 0) hi = mi - 1;
	else if (cmp > 0) lo = mi + 1;
	else              return sym;
    }
    oop obj = newObject(Symbol);
    obj->Symbol.name = strdup(name);
    symbolTable = realloc(symbolTable, sizeof(oop) * (symbolCount + 1));
    for (int i = symbolCount;  i > lo;  --i)
	symbolTable[i] = symbolTable[i-1];
    ++symbolCount;
    symbolTable[lo] = obj;
    return obj;
}

oop newCell(oop a, oop d)
{
    oop obj = newObject(Cell);
    obj->Cell.a = a;
    obj->Cell.d = d;
    return obj;
}

oop Cell_a(oop obj)	{ assert(Object_type(obj) == Cell);  return obj->Cell.a; }
oop Cell_d(oop obj)	{ assert(Object_type(obj) == Cell);  return obj->Cell.d; }

void Cell_setA(oop obj, oop a)	{ assert(Object_type(obj) == Cell);  obj->Cell.a = a; }
void Cell_setD(oop obj, oop d)	{ assert(Object_type(obj) == Cell);  obj->Cell.d = d; }

int isident(int c)
{
    return isalpha(c) || strchr("!\"#$%&'*+,-/:;<=>?@\\^_`|~", c);
}

int nextchar(void)
{
    int c = getchar();
    while (isspace(c)) c = getchar();
    return c;
}

oop revlist(oop list, oop tail)
{
    while (Object_type(list) == Cell) {
	oop cell = list;
	list = list->Cell.d;  
	cell->Cell.d = tail;  
	tail = cell;        
    }
    return tail;
}

oop read(void)
{
    int c;
    c = nextchar();
    if (isdigit(c)) { // number
	int value = 0;
	do {
	    value = value * 10 + c - '0'; // pushes new digit onto the right end
	    c = getchar();
	} while (isdigit(c));
	    ungetc(c, stdin);
	    return newInteger(value);
    }
    if (isident(c)) { // symbol
        char string[64];
        int length = 0;
        do {
            string[length++] = c;
            c = getchar();
        } while((isident(c) || isdigit(c)) && length < sizeof(string) - 1);
        ungetc(c, stdin);
        string[length] = '\0';
        if (!strcmp(string, "nil")) return nil;
        return newSymbol(string);
    }
    if (c == '(') {
        oop list = nil;
        c = nextchar();
        while (c != ')' && c != '.') {
            ungetc(c, stdin);
            oop obj = read();
            if (!obj) {
            fprintf(stderr, "EOF while reading list\n");
            exit(1);
            }
            list = newCell(obj, list); // push element onto front of list
            c = nextchar();
        }
        oop last = nil;
        if (c == '.') {
            last = read();
            if (!last) {
            fprintf(stderr, "EOF while reading list\n");
            exit(1);
            }
            c = nextchar();
        }
        if (c != ')') {
            fprintf(stderr, "')' expected at end of list\n");
            exit(1);
        }
        return revlist(list, last);
    }
    if (c == EOF) return 0;
    fprintf(stderr, "illegal character: 0x%02x\n", c);
    exit(1);
    return 0;
}

void print(oop obj)
{
    switch (Object_type(obj)) {
	case ILLEGAL: {
	    fprintf(stderr, "ILLEGAL type encountered\n");
	    abort();
	}
	case Undefined:	printf("nil");				return;
	case Integer:	printf("%d", Integer_value(obj));	return;
	case Symbol:	printf("%s", Symbol_name(obj));		return;
	case Cell: { 
	    putchar('(');
	    for (;;) {
		print(obj->Cell.a);
		obj = obj->Cell.d;
		if (Object_type(obj) != Cell) break;
		putchar(' ');
	    }
	    if (nil != obj) {
		printf(" . ");
		print(obj);
	    }
	    putchar(')');
	    return;
	}
    }
    assert(0); // this can never be reached
}

void println(oop obj)
{
    print(obj);
    putchar('\n');
}



int main()
{
    for (;;) { // read-print loop
        oop obj = read();
        if (!obj) break;
        println(obj);
    }
    return 0;
}
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
    while (Object_type(list) == Cell) { // list not empty
	oop cell = list;      // the first cell in list
	list = list->Cell.d;  // remove cell from list
	cell->Cell.d = tail;  // push cell onto the
	tail = cell;          //   front of tail
    }
    return tail;
}

oop read(void) // read stdin and return an object, or 0 if EOF
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
    if (c == '(') { //  ()  (x)  (x . y)  (x y z)  (x y . z)
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
	case Cell: { // (1 . (2 . (3 . nil))) -> (1 2 3)
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


oop car(oop obj){return (Object_type(obj)==Cell)?obj->Cell.a:nil;}
oop cdr(oop obj){return (Object_type(obj)==Cell)?obj->Cell.d:nil;}
oop cadr(oop obj){return car(cdr(obj));}
oop caddr(oop obj){return car(cdr(cdr(obj)));}

oop assoc(oop obj,oop alist){
	while(Object_type(alist)==Cell){
		oop list = car(alist);
		if(car(list)==obj)
			return list;
		alist = cdr(alist);
	}
	return nil;
}


oop sym_define = 0;
oop sym_lambda = 0;//
// + - * /
oop sym_plus  = 0;
oop sym_minus = 0;
oop sym_multi = 0;
oop sym_divid = 0;
// == < <= > >=
oop sym_equal      = 0;//
oop sym_less       = 0;
oop sym_less_eq    = 0;//
oop sym_greater    = 0;
oop sym_greater_eq = 0;//
// if while for 
oop sym_if 	  = 0;
oop sym_while = 0;//
oop sym_for   = 0;//



oop eval(oop exp);
oop evlist(oop list){
	if(Object_type(list)!=Cell){
		return eval(list);
	}
	return newCell(eval(list->Cell.a),evlist(list->Cell.d));
}

oop apply(oop func,oop args){
	args = evlist(args);
	oop arg = car(args);
	if(Object_type(arg)!=Integer){
		fprintf(stderr,"Integer");
		exit(1);
	}
// <
	if(func==sym_less){

		while(Object_type(cdr(args))==Cell){
			args = cdr(args);
			if(Integer_value(arg)>=Integer_value(car(args)))
				return nil;
			arg = car(args);
		}
		return t;
	}
// >
	if(func==sym_greater){
		while(Object_type(cdr(args))==Cell){
			args = cdr(args);
			if(Integer_value(arg)>=Integer_value(car(args)))
				return nil;
			arg = car(args);
		}
		return t;
	}

	int value = 0;
	if(Object_type(arg)==Integer)
		value = Integer_value(car(args));
	args = cdr(args);
// +
	if(func==sym_plus){
		while(Object_type(args)==Cell){
			arg = car(args);
			if(Object_type(arg)!=Integer){
				fprintf(stderr,"Integer:+");
				exit(1);
			}
			value += Integer_value(arg);
			args = cdr(args);
		}
		return newInteger(value);
	}
// -
	if(func==sym_minus){
		while(Object_type(args)==Cell){
			arg = car(args);
			if(Object_type(arg)!=Integer){
				fprintf(stderr,"Integer:-");
				exit(1);
			}
			value -= Integer_value(arg);
			args = cdr(args);
		}
		return newInteger(value);
	}
// *
	if(func==sym_multi){
		while(Object_type(args)==Cell){
			arg = car(args);
			if(Object_type(arg)!=Integer){
				fprintf(stderr,"Integer:*");
				exit(1);
			}
			value *= Integer_value(arg);
			args = cdr(args);
		}
		return newInteger(value);
	}
// /
	if(func==sym_divid){
		while(Object_type(args)==Cell){
			arg = car(args);
			if(Object_type(arg)!=Integer){
				fprintf(stderr,"Integer:/");
				exit(1);
			}
			if(Integer_value(arg)==0){
				fprintf(stderr,"divided by 0");
				exit(1);
			}
			value /=Integer_value(arg);
			args  = cdr(args);
		}
		return newInteger(value);
	}
	

	fprintf(stderr,"ERROR: Cell apply()\n");
	exit(1);
	return 0;
}

oop eval(oop exp){
	switch(Object_type(exp)){
		case ILLEGAL:
			exit(1);
		case Undefined:
		case Integer:
			return exp;
		case Symbol:{
			oop association = assoc(exp,globals);
			if(Object_type(cdr(association))!=Integer){
				fprintf(stderr,"error: undefined variable:\n");
				exit(1);
			}
			return cdr(association);
		}
		case Cell:{
			oop func = car(exp);
			oop args = cdr(exp);
			if(func==sym_define){
				oop name = car(args);
				if(Object_type(name)!=Symbol){
					fprintf(stderr,"Cell\n");
					exit(1);
				}
				oop value = cadr(args);
				value = eval(value);
				globals = newCell(newCell(name,value),globals);
				return value;
			}

			if(func==sym_if){
				if(eval(car(args))==nil){
					printf("false\n");
					return eval(caddr(args));
				}else{
					printf("True\n");
					return eval(cadr(args));
				}
			}

			// if(func==sym_while){
			// 	while(car(args)!=nil){
			// 		eval(cadr(args));
			// 	}
			// 	return eval(caddr(args));
			// }

			// if(func==sym_for){

			// }

			return apply(func,args);
		}

	}
	fprintf(stderr,"ERROR: Cell eval()\n");
	exit(1);
	return 0;
}



int main()
{
    nil        	= newObject(Undefined);
	t          	= newSymbol("t");

	globals = nil;

	sym_define 	= newSymbol("define");
	sym_if     	= newSymbol("if");

	sym_plus   	= newSymbol("+");
	sym_minus  	= newSymbol("-");
	sym_multi  	= newSymbol("*");
	sym_divid  	= newSymbol("/");

	sym_less   	= newSymbol("<");
	sym_greater = newSymbol(">");



    for (;;) { // read-print loop
	oop obj = read();
	if (!obj) break;
	println(eval(obj));
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

union Object;

typedef union Object Object;
typedef Object *oop;
typedef oop (*prim_t)(oop,oop);

typedef enum { ILLEGAL, Undefined, Integer, Symbol,Primitive,Special, Cell } type_t;

struct Integer { type_t type;  int   value; };
struct Symbol  { type_t type;  char *name;  oop value;};
struct Primitive{type_t type;  prim_t function;};
struct Special  {type_t type;  prim_t function;};
struct Cell    { type_t type;  oop   a, d;  };


union Object
{
    type_t type;
    struct Integer Integer;
    struct Symbol  Symbol;
    struct Primitive Primitive;
    struct Special Special;
    struct Cell    Cell;
};

// global symbol
oop nil     = 0;
oop t       = 0;
oop globals = 0;

// function 
oop sym_define = 0;
oop sym_lambda = 0;
oop sym_quote  = 0;

// + - * /
oop sym_plus  = 0;
oop sym_minus = 0;
oop sym_multi = 0;
oop sym_divid = 0;

// == < <= > >=
oop sym_less       = 0;

// if while for 
oop sym_if 	  = 0;
oop sym_while = 0;
oop sym_tab = 0;


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

prim_t Primitive_func(oop obj){
    assert(Object_type(obj)==Primitive);
    return obj->Primitive.function;
}
prim_t Special_func(oop obj){
    assert(Object_type(obj)==Special);
    return obj->Special.function;
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

oop newPrimitive(prim_t function)
{
    oop obj = newObject(Primitive);
    obj->Primitive.function = function;
    return obj;
}
oop newSpecial(prim_t function)
{
    oop obj = newObject(Special);
    obj->Special.function = function;
    return obj;
}



/*////////////////////////////////////////////////////////

    NORMAL FUNC
    
////////////////////////////////////////////////////////*/

oop Cell_a(oop obj)	{ assert(Object_type(obj) == Cell);  return obj->Cell.a; }
oop Cell_d(oop obj)	{ assert(Object_type(obj) == Cell);  return obj->Cell.d; }

void Cell_setA(oop obj, oop a)	{ assert(Object_type(obj) == Cell);  obj->Cell.a = a; }
void Cell_setD(oop obj, oop d)	{ assert(Object_type(obj) == Cell);  obj->Cell.d = d; }


int isident(int c)
{
    return isalpha(c) || strchr("!#$%&*+-/:;<=>?@\\^_|~", c);
}

int nextchar(void)
{
    int c = getchar();
    while (isspace(c)||c==';'){
        if(c==';'){
            while(c!='\n'){
                c = getchar();
            }
        }
        else
            c = getchar();
    }
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

oop car(oop obj);       
oop cdr(oop obj);                                                                          // ! no
oop eval(oop exp,oop env);
oop cadr(oop obj);
oop assoc(oop obj,oop alist);
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
// 'a -> (quote a)
    if (c == '\''){
        oop list = read();
        return newCell(sym_quote,newCell(list,nil));
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
        oop newlist = revlist(list,last);
        return newlist;
 
    }
    if (c == EOF) return 0;
    fprintf(stderr, "illegal character: [%c] 0x%02x\n", c,c);
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
    case Primitive: printf("%p",Primitive_func(obj));   return;
    case Special : printf("%p",Special_func(obj));
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
oop cddr(oop obj){return cdr(cdr(obj));}

oop assoc(oop obj,oop alist){
	while(Object_type(alist)==Cell){
		oop list = car(alist);
		if(car(list)==obj)
			return list;
		alist = cdr(alist);
	}
    return nil;
}


oop pairlist(oop names,oop values,oop tail){
    if(Object_type(names)!=Cell)return tail;
    oop name = names->Cell.a;
    oop value = car(values);
    return newCell(newCell(name,value),pairlist(names->Cell.d,cdr(values),tail));
}

oop evlist(oop list,oop env){
	if(Object_type(list)!=Cell){
		return eval(list,env);
	}
	return newCell(eval(list->Cell.a,env),evlist(list->Cell.d,env));
}



/*////////////////////////////////////////////////////////

    PRIM SPEC FUNCION
    
////////////////////////////////////////////////////////*/


oop prim_less(oop args,oop env)
{
    oop arg = car(args);
    if(Object_type(arg)!=Integer){
        fprintf(stderr,"Integer arg");
        exit(1);
    }
    while(Object_type(cdr(args))==Cell){
        args = cdr(args);
        if(Integer_value(arg)>=Integer_value(car(args)))
            return nil;
        arg = car(args);
    }
    return t;
}
oop prim_plus(oop args,oop env)
{
    oop arg = car(args);
    if(Object_type(arg)!=Integer){
        fprintf(stderr,"Integer arg");
        exit(1);
    }
    int value = 0;
    if(Object_type(arg)==Integer)
        value = Integer_value(car(args));
    args = cdr(args);

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

oop prim_minus(oop args,oop env)
{
    oop arg = car(args);
    if(Object_type(arg)!=Integer){
        fprintf(stderr,"Integer arg");
        exit(1);
    }
    int value = 0;
    if(Object_type(arg)==Integer)
        value = Integer_value(car(args));
    args = cdr(args);
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
            
            
oop prim_multi(oop args,oop env)
{
    oop arg = car(args);
    if(Object_type(arg)!=Integer){
        fprintf(stderr,"Integer arg");
        exit(1);
    }
    int value = 0;
    if(Object_type(arg)==Integer)
        value = Integer_value(car(args));
    args = cdr(args);

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
oop prim_divid(oop args,oop env)
{
    oop arg = car(args);
    if(Object_type(arg)!=Integer){
        fprintf(stderr,"Integer arg");
        exit(1);
    }
    int value = 0;
    if(Object_type(arg)==Integer)
        value = Integer_value(car(args));
    args = cdr(args);
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
oop spec_define(oop args,oop env)
{
    oop name = car(args);
    if(Object_type(name)==Cell){
        fprintf(stderr,"Cell[eval->sym_define:\n");
        println(name);
        exit(1);
    }
    oop value = cadr(args);
    value = eval(value,env);
    name->Symbol.value = value;
    // globals = newCell(newCell(name,value),globals);
    return value;
}
oop spec_if(oop args,oop env)
{
    if(eval(car(args),env)==nil){
        return eval(caddr(args),env);
    }else{
        return eval(cadr(args),env);
    }
}
oop spec_while(oop args,oop env)
{
    while(eval(car(args),env)!=nil){
        oop arg = cdr(args);
        while(arg!=nil){
            eval(car(arg),env);
            arg = cdr(arg);
        }
    }
    return nil;
}
oop spec_quote(oop args,oop env)
{
    return car(args);
}


/*////////////////////////////////////////////////////////

    APPLY EVAL

////////////////////////////////////////////////////////*/



oop apply(oop func,oop args,oop env){
    switch(Object_type(func)){
        case Integer:
        case ILLEGAL:
        case Undefined:{
            printf("apply() Integer ILLEGAL Undefined :");
            println(func);
            exit(1);
        }
        case Primitive:{
            return func->Primitive.function(args,env);
        }
        case Special:{
            return func->Special.function(args,env);
        }
        case Symbol:{
            return apply(eval(func,env),args,env);
        }
        case Cell:{
            if(car(func)==sym_lambda){
                oop newenv = pairlist(cadr(func),args,env);
                oop newfunc = cddr(func);
                while(cdr(newfunc)!=nil){
                    eval(car(newfunc),newenv);
                    newfunc = cdr(newfunc);
                }
                return eval(car(newfunc),newenv);
            }
            return apply(eval(func,env),args,env); //args shoud be change to env!
        }
    }
	
	fprintf(stderr,"ERROR:end apply()\n");
	exit(1);
	return 0;
}



//                                                                      eval
oop eval(oop exp,oop env){
	switch(Object_type(exp)){
		case ILLEGAL:
			exit(1);
		case Undefined:
        case Primitive:
        case Special:
		case Integer:
			return exp;
		case Symbol:{
			oop list = assoc(exp,env);
			if(list==nil){
                oop value = exp->Symbol.value;//listには、何も入っていないのでexpを見る。
                if(value==0){
                    fprintf(stderr,"not defined value:[in eval symbol]");
                    exit(1);
                }
                return value;
			}
			return cdr(list);
		}
		case Cell:{
			oop func = car(exp);
			oop args = cdr(exp);
            if(func==sym_lambda){
                return newCell(func,args);
            }
            if(Object_type(func)!=Special)
                args = evlist(args,env);
			return apply(func,args,env);
		}
	}
    return exp;
	fprintf(stderr,"ERROR: Cell eval()\n");
	exit(1);
	return 0;
}
// define(newSymbol("+"), newPrimitive(prim_add));
oop define(oop name, oop value){
    oop association = newCell(name,value);
    globals = newCell(association,globals);
    return value;
}
/*////////////////////////////////////////////////////////

    MAIN
    
////////////////////////////////////////////////////////*/


int main()
{
    nil        	= newObject(Undefined);
	t          	= newSymbol("t");
    sym_tab   = newSymbol("     ");

    sym_quote   = newSymbol("quote");
    sym_lambda  = newSymbol("lambda");
	globals     = nil;

	sym_define 	= newSymbol("define");
	sym_if     	= newSymbol("if");
    sym_while   = newSymbol("while");

	sym_plus   	= newSymbol("+");
	sym_minus  	= newSymbol("-");
	sym_multi  	= newSymbol("*");
	sym_divid  	= newSymbol("/");
	sym_less   	= newSymbol("<");
    define(newSymbol("+"), newPrimitive(prim_plus));
    define(newSymbol("-"), newPrimitive(prim_plus));
    define(newSymbol("*"), newPrimitive(prim_plus));
    define(newSymbol("/"), newPrimitive(prim_plus));
    define(newSymbol("<"), newPrimitive(prim_plus));

    define(newSymbol("define"), newSpecial(spec_define));
    define(newSymbol("if"), newSpecial(spec_if));
    define(newSymbol("while"), newSpecial(spec_while));
    define(newSymbol("quote"), newSpecial(spec_quote));


    for (;;) { // read-print loop
        oop obj = read();
        if (!obj) break;
        println(eval(obj,globals));
    }

    return 0;
}
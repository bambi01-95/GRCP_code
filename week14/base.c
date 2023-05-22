// make prim_spec
// ./prim_spec < factorial.txt
// grcp13.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

union Object;

typedef union Object Object;
typedef Object *oop;
typedef oop (*prim_t)(oop,oop);

typedef enum { ILLEGAL, Undefined, Integer, Symbol, Primitive, Special, Closure, Cell } type_t;

struct Integer  { type_t type; int   value; };
struct Symbol   { type_t type; char *name;  oop value;};
struct Primitive{ type_t type; oop name; prim_t function;};
struct Special  { type_t type; oop name; prim_t function;};
struct Closure  { type_t type; oop name; oop parameter, expression, environment;};
struct Cell     { type_t type; oop a, d;  };

union Object
{
    type_t type;
    struct Integer Integer;
    struct Symbol  Symbol;
    struct Primitive Primitive;
    struct Special Special;
    struct Closure  Closure;
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


// appendix GRCP-10
oop sym_print = 0;
oop sym_println = 0;
oop sym_let     = 0;
oop sym_setq    = 0;

oop sym_env_define = 0;
oop sym_eqn = 0;

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

oop Closure_parameters(oop obj){
    assert(Object_type(obj)==Closure);
    return obj->Closure.parameter;
}

oop Closure_expressions(oop obj){
    assert(Object_type(obj)==Closure);
    return obj->Closure.expression;
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
    obj->Symbol.value = nil;
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
    obj->Primitive.name = nil;
    return obj;
}
oop newSpecial(prim_t function)
{
    oop obj = newObject(Special);
    obj->Special.function = function;
    obj->Special.name = nil;
    return obj;
}

oop newClosure(oop parameters, oop expressions,oop environment){
    oop obj = newObject(Closure);
    obj->Closure.parameter   = parameters;
    obj->Closure.expression  = expressions;
    obj->Closure.environment = environment;
    obj->Closure.name = nil;
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
    while (isspace(c)){
        // if(c==';'){
        //     while(c!='\n'){
        //         c = getchar();
        //     }
        // }
        // else
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
void println(oop);
oop read(void) // read stdin and return an object, or 0 if EOF
{
    int c;
    c = nextchar();
    if (c == ';'){
        while(c!='\n')c=getchar();
        return read();
    }
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
    if (c == '(') { // ( a b (a b))
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
	case Undefined: printf("nil"); return;
	case Integer:	printf("%d", Integer_value(obj));return;
	case Symbol:	printf("%s", Symbol_name(obj));  return;
    case Primitive: print(obj->Primitive.name);return;//printf("%p",Primitive_func(obj));return;
    case Special :  print(obj->Special.name);return;//printf("%p", Special_func(obj)); return;
    case Closure:{
        printf("<Closure.");
        print(obj->Closure.name);
        printf("> <exp.");
        print(Closure_expressions(obj));
        printf("> <para.");
        print(Closure_parameters(obj));
        printf("> <env.");
        print(obj->Closure.environment);
        putchar('>');
        return;
    }
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
    if(Object_type(names)!=Cell){
        return (names==nil)
            ?tail
            :newCell(newCell(names,values),tail);
    }
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
oop define(oop name, oop value);
oop apply(oop func,oop args,oop env);

// PRIM FUNCTION

oop prim_less(oop args,oop env)
{
    oop arg = car(args);
    if(Object_type(arg)!=Integer){
        fprintf(stderr,"Integer arg [prim_less]");
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

// (+ car(args) cadr(args))
oop prim_plus(oop args,oop env)
{
    int value = 0;
    oop arg = car(args);
    if(arg==nil){
        return newInteger(value);
    }
    if(Object_type(arg)!=Integer){
        fprintf(stderr,"ERROR not Integer value:[prim_plus]");
        exit(1);
    }
    value += Integer_value(arg);
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
    int value = 0;
    oop arg = car(args);
    if(arg==nil){
        return newInteger(value);
    }
    if(Object_type(arg)!=Integer){
        fprintf(stderr,"Integer arg");
        exit(1);
    }
    if(cadr(args)==nil)
        value -= Integer_value(arg);
    else
        value += Integer_value(arg);

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
    int value = 1;
    oop arg = car(args);
    if(arg==nil){
        return newInteger(value);
    }
    if(Object_type(arg)!=Integer){
        fprintf(stderr,"Integer arg");
        exit(1);
    }
    value *= Integer_value(arg);
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
    int value = 1;
    oop arg = car(args);
    if(arg==nil){
        return newInteger(value);
    }
    if(Object_type(arg)!=Integer){
        fprintf(stderr,"Integer arg");
        exit(1);
    }
    if(cadr(args)==nil)
        value /= Integer_value(arg);
    else
        value = Integer_value(arg);
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


// SPEC FUNCTION
oop spec_define(oop args,oop env)
{
    oop name = car(args);
    if(Object_type(name)==Cell){
        fprintf(stderr,"Cell[eval->sym_define:\n");
        exit(1);
    }
    oop value = cadr(args);
    value = eval(value,env);
    name->Symbol.value = value;
    define(name,value);
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

oop spec_lambda(oop args,oop env){
    return newClosure(car(args),cdr(args),env);
}


// .... FUNCTION
oop spec_print(oop args,oop env){
    print(args);
    while(args!=nil){
        print(eval(car(args),env));
        putchar(' ');
        args = cdr(args);
    }
    return nil;
}

oop spec_println(oop args,oop env){
    while(args!=nil){
        println(eval(car(args),env));
        args = cdr(args);
    }
    return nil;
}
// too long?
oop spec_let(oop args,oop env){
    //  (let ((n1 v1) (n2 v2) ...) e1 e2 ...)
    oop a_args = car(args); //((n1 v1) (n2 v2) ...) nx:para vx:valu
    oop values  = nil;
    oop parames = nil;
    while(a_args!=nil){
        oop paravalu = car(a_args);
        values = newCell(eval(cadr(paravalu),env),values);
        parames= newCell(car(paravalu),parames);
        a_args = cdr(a_args);
    }
    /*-using apply(), because inside spec function. but closure is not spec func...*/
    return apply(newClosure(parames,cdr(args),env),revlist(values,nil),env);
}

oop spec_setq(oop args,oop env){
    oop association = assoc(car(args),env);
    oop value = eval(cadr(args),env);
    if(association==nil){
        oop name = car(args);
        if(name->Symbol.value==nil){
            fprintf(stderr,"undefined value:[in spec_setq]");
            exit(1);
        }
        define(name,value);
        return value;
    }
    Cell_setD(association,value);
    return value;
}


/*////////////////////////////////////////////////////////

    APPLY EVAL

////////////////////////////////////////////////////////*/

oop apply(oop func,oop args,oop env){
    switch(Object_type(func)){
        case ILLEGAL:
        case Undefined:
        case Integer:{
            printf("apply() Integer / ILLEGAL / Undefined / Cell :");
            println(func);
            exit(1);
        }
        case Primitive:
            return func->Primitive.function(args,env);
        case Special:
            return func->Special.function(args,env);
        case Closure:{
            oop newenv  = pairlist(Closure_parameters(func),args,func->Closure.environment);
            oop newfunc = func->Closure.expression;
            // printf("env: ");println(newenv);
            while(cdr(newfunc)!=nil){
                eval(car(newfunc),newenv);
                newfunc = cdr(newfunc);
            }
            return eval(car(newfunc),newenv);
        }
        case Symbol:{
            return apply(eval(func,env),args,env);
        }
        case Cell:
            return func;
    }

	fprintf(stderr,"ERROR:end apply()\n");
	exit(1);
	return 0;
}


oop eval(oop exp,oop env){
	switch(Object_type(exp)){
		case ILLEGAL:
		case Undefined:
        case Integer:
        case Primitive:
        case Special:
        case Closure:
			return exp;
		case Symbol:{
			oop list = assoc(exp,env);
			if(list==nil){
                oop value = exp->Symbol.value;
                if(value==nil){
                    fprintf(stderr,"is not defined value:[in eval symbol]");
                    print(exp);
                    exit(1);
                }
                return value;
			}
			return cdr(list);
		}
		case Cell:{
			oop func = car(exp);
			oop args = cdr(exp);
            if(Object_type(func->Symbol.value)!=Special)
                args = evlist(args,env);
			return apply(func,args,env);
		}
	}
    printf("EVAL ERROR:");
    return nil;
}

oop define(oop name, oop value){
    if(Object_type(name)!=Symbol){
        fprintf(stderr,"ERROR: name should be Symbol type[define]");
    }
    switch(Object_type(value)){
        case Undefined:
        case ILLEGAL:
        case Symbol:
        case Integer:
        case Cell:
            break;

        case Primitive:{
            value->Primitive.name = name;
            break;
        }
        case Special:{
            value->Special.name = name;
            break;
        }
        case Closure:{
            value->Closure.name = name;
            break;
        }
    }
    name->Symbol.value = value;
    return value;
}

oop spec_env_define(oop args,oop env){
    oop pair = assoc(car(args),env);
    if(pair==nil){
        oop pair = newCell(car(args),eval(cadr(args),env));
        oop a = car(env);
        oop d = cdr(env);
        env->Cell.a = pair;
        env->Cell.d = newCell(a,d);     
    }
    else{
        Cell_setD(pair,eval(cadr(args),env));
    }
    return cadr(args);
}
oop prim_equal(oop args,oop env){
    print(car(args));printf("==");println(cadr(args));
    if(car(args)==cadr(args))
        return t;
    return nil;
}
/*////////////////////////////////////////////////////////

    MAIN
    
////////////////////////////////////////////////////////*/

int main()
{
    nil        	= newObject(Undefined);
	t          	= newSymbol("t");

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

    sym_print   = newSymbol("print");
    sym_println = newSymbol("println");
    sym_let     = newSymbol("let");
    sym_setq    = newSymbol("setq");

    sym_env_define = newSymbol("def");
    sym_eqn = newSymbol("==");
    
    define(newSymbol("+"), newPrimitive(prim_plus));
    define(newSymbol("-"), newPrimitive(prim_minus));
    define(newSymbol("*"), newPrimitive(prim_multi));
    define(newSymbol("/"), newPrimitive(prim_divid));
    define(newSymbol("<"), newPrimitive(prim_less));

    define(newSymbol("define"),newSpecial(spec_define));
    define(newSymbol("if"),    newSpecial(spec_if));
    define(newSymbol("while"), newSpecial(spec_while));
    define(newSymbol("quote"), newSpecial(spec_quote));
    define(newSymbol("lambda"),newSpecial(spec_lambda));

    define(newSymbol("print"),  newSpecial(spec_print));
    define(newSymbol("println"),newSpecial(spec_println));
    define(newSymbol("let"),    newSpecial(spec_let));
    define(newSymbol("setq"),   newSpecial(spec_setq));

    define(newSymbol("def"),newSpecial(spec_env_define));
    define(newSymbol("=="),newPrimitive(prim_equal));

    for (;;) { // read-print loop
        oop obj = read();
        if (!obj) break;
        println(eval(obj,globals));
    }
    return 0;
}

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
oop sym_define = 0;
oop sym_print = 0;
oop sym_println = 0;
oop sym_setq = 0;
oop func_list = 0;

// function in this
oop sym_atom = 0;
oop sym_integer = 0;
oop sym_symbol = 0;
oop sym_pair = 0;
oop sym_cons = 0;
oop sym_list = 0;
oop sym_car = 0;
oop sym_cdr = 0;
oop sym_set_car = 0;
oop sym_set_cdr= 0;
oop sym_eq     = 0;

oop sym_eval = 0;
oop sym_apply  = 0;
oop sym_evlist = 0;
oop sym_pairlist   = 0;
oop sym_assoc  = 0;

oop sym_lambda = 0;//
oop sym_quote = 0;
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




// function
oop eval(oop exp,oop env);
oop car(oop obj);




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

int  funcCount = 0;
oop *funcTable = 0;

oop addFunc(char *name)
{
    int lo = 0, hi = funcCount - 1;
    while (lo <= hi) {
	int mi = (lo + hi) / 2;
	oop sym = funcTable[mi];
	int cmp = strcmp(name, Symbol_name(sym));
	if      (cmp < 0) hi = mi - 1;
	else if (cmp > 0) lo = mi + 1;
	else              return sym;
    }
    oop obj = newObject(Symbol);
    obj->Symbol.name = strdup(name);
    funcTable = realloc(funcTable, sizeof(oop) * (funcCount + 1));
    for (int i = funcCount;  i > lo;  --i)
	funcTable[i] = funcTable[i-1];
    ++funcCount;
    funcTable[lo] = obj;
    return obj;
}

int searchFunc(oop list)
{
    if(Object_type(car(list))==Cell){
        return searchFunc(car(list));
    }
    if(Object_type(list)!=Symbol){
        return 0;
    }

    char* name = Symbol_name(list);
    int lo = 0, hi = funcCount - 1;
    while (lo <= hi) {
	int mi = (lo + hi) / 2;
	oop sym = funcTable[mi];
	int cmp = strcmp(name, Symbol_name(sym));
	if      (cmp < 0) hi = mi - 1;
	else if (cmp > 0) lo = mi + 1;
	else              return 1;
    }
    return 0;
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



/*
 * Read function
 */


int isident(int c)
{
    return isalpha(c) || strchr("!#$%&*+,-/:<=>?@\\^_`|~", c);// ' ; ""
}

// int nextchar(void)
// {
//     int c = getchar();
//     while (isspace(c)) c = getchar();
//     return c;
// }

//(2) appendix
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
        return revlist(list,last);
        oop newlist = revlist(list,last);
        if(searchFunc(newlist))
            return eval(newlist,globals);
        else
            return newlist;

        
    }
    if (c == EOF) return 0;
    fprintf(stderr, "illegal character: 0x%02x\n", c);
    exit(1);
    return 0;
}


/*
 * Print function
 */

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






/*
 * Eval function
 */


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

oop apply(oop func,oop args,oop env){
    switch(Object_type(func)){
        case Integer:
        case ILLEGAL:
        case Undefined:{
            printf("apply() Integer ILLEGAL Undefined :");
            println(func);
            exit(1);
        }
        case Symbol:{
            if(func==sym_lambda){
                return newCell(func,args);
            }
            args = evlist(args,env);
        // d
            if(func==sym_less){
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
        // +
            if(func==sym_plus){
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
        // -
            if(func==sym_minus){
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
        // *
            if(func==sym_multi){
                oop arg = car(args);
                if(Object_type(arg)!=Integer){
                    fprintf(stderr,"Integer arg");
                    exit(1);
                }
                int value = 1;
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
        // /
            if(func==sym_divid){
                oop arg = car(args);
                if(Object_type(arg)!=Integer){
                    fprintf(stderr,"Integer arg");
                    exit(1);
                }
                int value = 1;
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
            return apply(eval(func,env),args,env);
        }

        case Cell:{
            if(car(func)==sym_lambda){
                oop newenv = pairlist(cadr(func),args,env);
                func = cddr(func);
                while(Object_type(cdr(func))==Cell){//(define f (lambda (x) (+ x x) (if (< x 10) 1 0)))
                    eval(car(func),newenv);
                    func = cdr(func);
                }
                return eval(car(func),newenv);//caddr
            }
            return apply(eval(func,env),args,env); //args shoud be change to env!
            fprintf(stderr,"ERROR:end apply()\n");
	        exit(1);
        }
    }
	

	fprintf(stderr,"ERROR:end apply()\n");
	exit(1);
	return 0;
}

oop func_print(oop args){
    if(args==nil){
        return nil;
    }
    if(Object_type(car(args))==Cell){
        func_print(car(args));
    }
    else{
        print(car(args));
    }
    return func_print(cdr(args));
}

//                                                                      eval
oop eval(oop exp,oop env){
	switch(Object_type(exp)){
		case ILLEGAL:
			exit(1);
		case Undefined:
		case Integer:
			return exp;
		case Symbol:{
			oop association = assoc(exp,env);
            if(association==nil){
                fprintf(stderr,"ERROR: undefined variable(eval->symbol)\n");
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
				value = eval(value,env);
			    globals = newCell(newCell(name,value),globals);//env == globals
				return value;
			}

			if(func==sym_if){
				if(eval(car(args),env)==nil){
					return eval(caddr(args),env);
				}else{
					return eval(cadr(args),env);
				}
			}
            if(func==sym_while){
                while(eval(car(args),env)!=nil){
                    oop arg = cdr(args);
                    while(arg!=nil){
                        eval(car(arg),env);
                        arg = cdr(arg);
                    }
                }
                return nil;
            }
            if(func==sym_quote){
                return car(args);
            }



            // appendix (3)
            if(func==sym_print){
                while(args!=nil){
                    print(eval(car(args),env));
                    putchar(' ');
                    args = cdr(args);
                }
                return nil;// special
            }

            if(func==sym_println){
                while(args!=nil){
                    println(eval(car(args),env));
                    args = cdr(args);
                }
                return nil;// special
            }

            if(func==sym_setq){
                oop association = assoc(car(args),env);
                if(association==nil){
                    fprintf(stderr,"ERROR undefined variable(eval->cell->setq)\n");
                    exit(1);
                }
                oop value = eval(cadr(args),env);
                if(Object_type(value)!=Integer){
                    fprintf(stderr,"args is not Integer\n");
                    exit(1);
                }
                association->Cell.d = value;
                return cdr(association);
            }

            if(func==sym_atom){
                if(Object_type(car(args))==Cell)
                    return nil;
                return t;
            }

            if(func==sym_integer){
                if(Object_type(car(args))==Integer)
                    return t;
                return nil;
            }

            if(func==sym_symbol){
                if(Object_type(car(args))==Symbol)
                    return t;
                return nil;
            }
            if(func==sym_pair){
                if(Object_type(car(args))==Cell)
                    return t;
                return nil;
            }
            if(func==sym_cons){
                oop first = car(args);
                oop second = cadr(args);
                return newCell(first,second);
            }
            if(func==sym_list){
                return args;
            }
            if(func==sym_car){
                return car(args);
            }
            if(func==sym_cdr){
                return cdr(args);
            }
            if(func==sym_set_car){
                oop a = car(args);
                a = cadr(args);
            }
            if(func==sym_set_cdr){
                oop d = car(args);
                d = cadr(args);
            }
            if(func==sym_eq){
                if(Object_type(car(args))==Object_type(cadr(args)))
                    return t;
                return nil;
            }

            if(func==sym_eval){
                return eval(car(args),env);
            }
            if(func==sym_apply){
                apply(car(args),cadr(args),env);
            }
			return apply(func,args,env);
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
    sym_quote   = newSymbol("quote");
    sym_lambda  = newSymbol("lambda");
    sym_print   = newSymbol("print");
    sym_println = newSymbol("println");
	globals = nil;

	sym_define 	= newSymbol("define");
	sym_if     	= newSymbol("if");
    sym_while   = newSymbol("while");

	sym_plus   	= newSymbol("+");
	sym_minus  	= newSymbol("-");
	sym_multi  	= newSymbol("*");
	sym_divid  	= newSymbol("/");

	sym_less   	= newSymbol("<");
	sym_greater = newSymbol(">");
    sym_setq    = newSymbol("setq");

    sym_atom    = newSymbol("atom?");
    sym_integer = newSymbol("integer?");
    sym_symbol  = newSymbol("symbol?");
    sym_pair    = newSymbol("pair");
    sym_cons    = newSymbol("cons");
    sym_list    = newSymbol("list");
    sym_car     = newSymbol("car");
    sym_cdr     = newSymbol("cdr");
    sym_set_car = newSymbol("set-car");
    sym_set_cdr = newSymbol("set-cdr");
    sym_eq      = newSymbol("eq");

    sym_eval    = newSymbol("eval");

    addFunc("eval");
    addFunc("apply");

    for (;;) { // read-print loop
        oop obj = read();
        if (!obj) break;
        // println(eval(obj,globals));
        eval(obj,globals);
    }

    return 0;
}

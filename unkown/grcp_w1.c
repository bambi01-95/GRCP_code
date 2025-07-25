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
struct Symbol  { type_t type;  char *name;  oop value;};
struct Cell    { type_t type;  oop   a, d;  };
// if symbol
//     if in syntax
//         func 

union Object
{
    type_t type;
    struct Integer Integer;
    struct Symbol  Symbol;
    struct Cell    Cell;
};

// global symbol
oop nil     = 0;
oop t       = 0;
oop globals = 0;
oop syntaxTable = 0;

// function 
oop sym_define = 0;
oop sym_lambda = 0;
oop sym_quote  = 0;
oop sym_quasiquote = 0;
oop sym_unquote    = 0;
oop sym_unquote_splicing = 0;

// + - * /
oop sym_plus  = 0;
oop sym_minus = 0;
oop sym_multi = 0;
oop sym_divid = 0;

// == < <= > >=
oop sym_equal      = 0;
oop sym_less       = 0;
oop sym_less_eq    = 0;
oop sym_greater    = 0;
oop sym_greater_eq = 0;

// if while for 
oop sym_if 	  = 0;
oop sym_while = 0;
oop sym_for   = 0;
oop sym_cond  = 0;
oop sym_let   = 0;

oop sym_print   = 0;
oop sym_println = 0;
oop sym_setq    = 0;

oop sym_eval  = 0;
oop sym_apply = 0;

oop sym_tab = 0;


// function that defined in this code
oop sym_atom    = 0;
oop sym_integer = 0;
oop sym_symbol  = 0;
oop sym_cell    = 0;
oop sym_cons    = 0;
oop sym_list    = 0;
oop sym_car     = 0;
oop sym_cdr     = 0;
oop sym_set_car = 0;
oop sym_set_cdr = 0;
oop sym_eq      = 0;

oop sym_assoc   = 0;




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

                                                                        //no



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
    if(c=='`'){
        oop list = read();
        return newCell(sym_quasiquote,newCell(list,nil));
    }
    if(c==','){
        c = nextchar();
        if(c=='@'){
            oop list = read();
            return newCell(sym_unquote_splicing,newCell(list,nil));
        }
        ungetc(c,stdin);
        oop list = read();
        return newCell(sym_unquote,newCell(list,nil));
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
        oop newlist = revlist(list,last);

        // if(Object_type(car(newlist))==Symbol){
        //     if(assoc(car(newlist),syntaxTable)){
        //         oop symbol = car(newlist);
        //         return newCell(symbol->Symbol.value,cdr(newlist));
        //     }
        // }

        return newlist;
 
    }
    if (c == EOF) return 0;
    fprintf(stderr, "illegal character: %c 0x%02x\n", c,c);
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

        // =
            if(func==sym_equal){
                if(Integer_value(car(args))==Integer_value(cadr(args))){
                    return t;
                }
                return nil;
            }
        // <
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

            if(func==sym_less_eq){
                oop arg = car(args);
                if(Object_type(arg)!=Integer){
                    fprintf(stderr,"Integer arg");
                    exit(1);
                }
                while(Object_type(cdr(args))==Cell){
                    args = cdr(args);
                    if(Integer_value(arg)>Integer_value(car(args)))
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
        // /
            if(func==sym_divid){
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

/////////////////////////////////////////////////////////////////////////////
            // function in this code

            // printf("this is");
            // printf("%d\n",Object_type(car(args)));

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
            if(func==sym_cell){
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
                return car(car(args));
            }
            if(func==sym_cdr){
                return cdr(car(args));
            }
            if(func==sym_set_car){
                oop a = car(car(args));
                a = cadr(args);
                return car(args);
            }
            if(func==sym_set_cdr){
                oop d = cdr(car(args));
                d = cadr(args);
                return car(args);
            }
            if(func==sym_eq){
                if(car(args) == cadr(args))
                    return t;
                return nil;
            }

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
		case Integer:
			return exp;
		case Symbol:{
			oop association = assoc(exp,env);
			if(association==nil){
				fprintf(stderr,"error: undefined variable:[in eval]\nassoc->");
                println(association);
				exit(1);
			}
			return cdr(association);
		}
		case Cell:{
			oop func = car(exp);
			oop args = cdr(exp);

			if(func==sym_define){
				oop name = car(args);
				if(Object_type(name)==Cell){
                  name = eval(name,env);// not good 
					// fprintf(stderr,"Cell[eval->sym_define:\n");
                    // println(name);
					// exit(1);
				}
				oop value = cadr(args);
				value = eval(value,env);
                // printf("::");
                // println(name);
                // println(value);
                //name->Symbol.value = value;
			    globals = newCell(newCell(name,value),globals);
				return value;
			}

			if(func==sym_if){
                // println(car(args));
                // println(cadr(args));
                // println(caddr(args));
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

            if(func==sym_cond){
                while(args!=nil){
                    oop arg = car(args);
                    if(eval(car(arg),env)==t)
                        return eval(cadr(arg),nil);
                    args = cdr(args);
                }
                return nil;
            }

            if (func==sym_let){
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
                oop inside = newCell(revlist(parames,nil),cdr(args));// ((n1 n2..) e1 e2..) cdr(args); //e1 e2)
                return eval(newCell(newCell(sym_lambda,inside),revlist(values,nil)),env);
            }

            if(func==sym_quote){
                return car(args);
            }

            if(func==sym_quasiquote){
                if(Object_type(car(args))!=Cell)
                    return car(args);

                oop list = nil;
                args = car(args);
                while(args!=nil){
                    oop arg = car(args);

                    if(Object_type(arg)==Cell){

                        if(car(arg)==sym_unquote){
                            arg = eval(cadr(arg),env);
                        }

                        if(car(arg)==sym_unquote_splicing){
                            oop alist = cadr(arg);
                            // if(car(alist)!=sym_list){
                            //     fprintf(stderr,"ERROR:unquote-splicing include list type");
                            //     exit(1);
                            // }
                            alist = cdr(alist);
                            while(cdr(alist)!=nil){
                                list = newCell(car(alist),list);
                                alist = cdr(alist);
                            }
                            arg = car(alist);
                        }
                    }
                    list = newCell(arg,list);
                    args = cdr(args);
                }
                return revlist(list,nil);
            }
            // if(func==sym_eval){
            //     return eval(car(args),env);
            // }

            // if(func==sym_apply){
            //     return apply(args,cadr(args),env);
            // }
            if(func==sym_print){
                while(args!=nil){
                    print(eval(car(args),env));
                    putchar(' ');
                    args = cdr(args);
                }
                return sym_tab;
            }
            if(func==sym_println){
                while(args!=nil){
                    println(eval(car(args),env));
                    args = cdr(args);
                }
                return sym_tab;
            }
            if(func==sym_setq){
                oop association = assoc(car(args),env);
                if(association==nil){
                    fprintf(stderr,"ERROR: undefined variable\n(eval sym_setq)");
                    exit(1);
                }
                oop value = eval(cadr(args),env);
                Cell_setD(association,value);
				return value;
            }
            if(func==sym_assoc){
                printf("this is ");
                println(cdr(assoc(eval(car(args),env),globals)));
                return cdr(assoc(eval(car(args),env),globals));
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
    sym_tab   = newSymbol("     ");

    sym_quote   = newSymbol("quote");
    sym_quasiquote = newSymbol("quasiquote");
    sym_unquote    = newSymbol("unquote");
    sym_unquote_splicing = newSymbol("unquote-splicing");
    sym_lambda  = newSymbol("lambda");
	globals     = nil;
    syntaxTable = nil;

	sym_define 	= newSymbol("define");
	sym_if     	= newSymbol("if");
    sym_while   = newSymbol("while");
    sym_print   = newSymbol("print");
    sym_println = newSymbol("println");
    sym_setq    = newSymbol("setq");
    sym_cond    = newSymbol("cond");
    sym_let     = newSymbol("let");

    sym_eval    = newSymbol("eval");
    sym_apply   = newSymbol("apply");

    sym_equal   = newSymbol("=");
	sym_plus   	= newSymbol("+");
	sym_minus  	= newSymbol("-");
	sym_multi  	= newSymbol("*");
	sym_divid  	= newSymbol("/");
	sym_less   	= newSymbol("<");
    sym_less_eq = newSymbol("<=");
	sym_greater = newSymbol(">");


    sym_atom    = newSymbol("atom?");
    sym_integer = newSymbol("integer?");
    sym_symbol  = newSymbol("symbol?");
    sym_cell    = newSymbol("cell?");
    sym_cons    = newSymbol("cons");
    sym_list    = newSymbol("list");
    sym_car     = newSymbol("car");
    sym_cdr     = newSymbol("cdr");
    sym_set_car = newSymbol("set-car");
    sym_set_cdr = newSymbol("set-cdr");
    sym_eq      = newSymbol("eq?");

    sym_assoc   = newSymbol("assoc");
    for (;;) { // read-print loop
        oop obj = read();
        if (!obj) break;
        println(eval(obj,globals));
    }

    return 0;
}

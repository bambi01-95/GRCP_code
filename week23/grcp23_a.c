#ifndef USE_GC
# define USE_GC 1	// set to 0 (or comment out this line) to disable GC
#endif

#if USE_GC
# include <gc.h>
# define GC_calloc(COUNT, SIZE)	GC_malloc((COUNT) * (SIZE))
#else
# define GC_calloc(COUNT, SIZE)	calloc((COUNT), (SIZE))
# define GC_init()
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <setjmp.h>

union Object;

typedef union Object Object;
typedef Object *oop;

typedef enum { ILLEGAL, Undefined, Integer, Float, String, Symbol, Cell, Primitive, Special, Closure, Array, } type_t;
typedef enum { RET_RESULT, RET_RETURN, RET_BREAK, RET_CONTINUE,} return_t;

int alloc[Closure+1];

struct Result{
	oop value;
	return_t intent;
};
typedef struct Result Result;
typedef Result (*prim_t)(oop args, oop env);
int allocation[Closure + 1];
// struct Integer 	 { type_t type;  int    value; };
struct Float 	 { type_t type;  float  value; };
struct String  	 { type_t type;  char  *value; };
struct Symbol  	 { type_t type;  char  *name;  oop value;  oop syntax; };
struct Array     { type_t type;  oop *elements; int size; int capacity;};
struct Cell    	 { type_t type;  oop    a, d; };
struct Primitive { type_t type;  prim_t function;  oop name; };
struct Special   { type_t type;  prim_t function;  oop name; };
struct Closure   { type_t type;  oop    parameters, expressions, name, environment;  };

union Object
{
    type_t type;
    // struct Integer   Integer;
    struct Float     Float;
    struct String    String;
    struct Symbol    Symbol;
    struct Cell      Cell;
    struct Array     Array;
    struct Primitive Primitive;
    struct Special   Special;
    struct Closure   Closure;
};

oop nil                 = 0;

oop sym_t		        = 0;
oop sym_quote			= 0;
oop sym_unquote			= 0;
oop sym_unquote_splicing= 0;
oop sym_quasiquote		= 0;
oop sym_define          = 0;
oop sym_setq            = 0;
oop sym_while           = 0;
oop sym_if              = 0;

oop newObject(type_t type)
{
	alloc[type]++;
    oop obj = GC_calloc(1, sizeof(Object));
    obj->type = type;
    return obj;
}

type_t Object_type(oop obj)
{
    return(((intptr_t)obj&1)==1)?Integer:obj->type;
    // use memory
    assert(obj); // not a NULL pointer
    return obj->type;
}

oop newInteger(int value)
{
    return (oop)(intptr_t)((value<<1)|1);
    // use memory
    // oop obj = newObject(Integer);
    // obj->Integer.value = value;
    // return obj;
}

int Integer_value(oop obj)
{
    assert(((intptr_t)obj&1)==1);
    return(intptr_t)obj>>1;
    // use memory
    // assert(Object_type(obj) == Integer);
    // return obj->Integer.value;
}

oop newFloat(float value)
{
    oop obj = newObject(Float);
    obj->Float.value = value;
    return obj;
}

float Float_value(oop obj)
{
    assert(Object_type(obj) == Float);
    return obj->Float.value;
}

oop newString(char *value)
{
    oop obj = newObject(String);
    obj->String.value = value;
    return obj;
}

char *String_value(oop obj)
{
    assert(Object_type(obj) == String);
    return obj->String.value;
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
    assert(obj->Symbol.value == 0); // not defined
    symbolTable = realloc(symbolTable, sizeof(oop) * (symbolCount + 1));
    for (int i = symbolCount;  i > lo;  --i)
	symbolTable[i] = symbolTable[i-1];
    ++symbolCount;
    symbolTable[lo] = obj;
    return obj;
}

oop newArray(int capacity){
    oop obj = newObject(Array);
    obj->Array.size = 0;
    obj->Array.capacity = capacity;
    obj->Array.elements = calloc(capacity, sizeof(oop));
    return obj;
}

int Array_size(oop obj){
    assert(Object_type(obj)==Array);
    return obj->Array.size;
}

oop Array_put(oop obj,unsigned int index,oop element)
{
    assert(Object_type(obj)==Array);
    if(index >obj->Array.capacity){
        obj->Array.elements= realloc(obj->Array.elements,(index+1)*sizeof(oop));
        obj->Array.capacity=index + 1;
    }
    while(obj->Array.size<(index+1)){
        obj->Array.elements[obj->Array.size++]=nil;
    }
    return obj->Array.elements[index]=element;
}

oop Array_get(oop obj, unsigned int index){
    assert(Object_type(obj)==Array);
    if(index < obj->Array.size)
		return obj->Array.elements[index];
	return nil;
};

oop Array_push(oop obj,oop element){
    assert(Object_type(obj)==Array);
    if(obj->Array.size >= obj->Array.capacity){
		obj->Array.elements = realloc(obj->Array.elements,(obj->Array.size + 1) * sizeof(oop));
		obj->Array.capacity += 1;
    }
    obj->Array.elements[obj->Array.size++] = element;
    return obj;
};

oop Array_pop(oop obj){
    assert(Object_type(obj)==Array);
	if(obj->Array.size <= 0){
		fprintf(stderr,"Array is empty [array_pop]\n");
		exit(1);
	}
    oop element = obj->Array.elements[--obj->Array.size];
    obj->Array.elements[obj->Array.size] = nil;
    return element;
};


oop newPrimitive(prim_t function)
{
    oop obj = newObject(Primitive);
    obj->Primitive.function = function;
    obj->Primitive.name     = nil;
    return obj;
}

prim_t Primitive_function(oop obj)
{
    assert(Object_type(obj) == Primitive);
    return obj->Primitive.function;
}

oop Primitive_name(oop obj)
{
    assert(Object_type(obj) == Primitive);
    return obj->Primitive.name;
}

oop newSpecial(prim_t function)
{
    oop obj = newObject(Special);
    obj->Special.function = function;
    obj->Special.name     = nil;
    return obj;
}

prim_t Special_function(oop obj)
{
    assert(Object_type(obj) == Special);
    return obj->Special.function;
}

oop Special_name(oop obj)
{
    assert(Object_type(obj) == Special);
    return obj->Special.name;
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

oop newClosure(oop parameters, oop expressions, oop environment)
{
    oop obj = newObject(Closure);
    obj->Closure.parameters  = parameters;
    obj->Closure.expressions = expressions;
    obj->Closure.name        = nil;
    obj->Closure.environment = environment;
    return obj;
}

oop Closure_parameters(oop obj)
{
    assert(Object_type(obj) == Closure);
    return obj->Closure.parameters;
}

oop Closure_expressions(oop obj)
{
    assert(Object_type(obj) == Closure);
    return obj->Closure.expressions;
}

oop Closure_name(oop obj)
{
    assert(Object_type(obj) == Closure);
    return obj->Closure.name;
}

oop Closure_environment(oop obj)
{
    assert(Object_type(obj) == Closure);
    return obj->Closure.name;
}

void print(oop obj)
{
    switch (Object_type(obj)) {
	case ILLEGAL: {
	    fprintf(stderr, "ILLEGAL type encountered\n");
	    abort();
	}
	case Undefined:	printf("nil");						return;
	case Integer:	printf("%d", Integer_value(obj));			return;
	case Float:	printf("%f", Float_value(obj));				return;
	case String:	printf("%s", String_value(obj));			return;
	case Symbol:	printf("%s", Symbol_name(obj));				return;
    case Array:     printf("%p, size %d, capa%d",obj,obj->Array.size,obj->Array.capacity); return;
	case Primitive:	{
	    printf("<primitive.");
	    if (nil != Primitive_name(obj)) print(Primitive_name(obj));
	    else printf("%p", Primitive_function(obj));
	    printf(">");
	    return;
	}
	case Special: {
	    printf("<special.");
	    if (nil != Special_name(obj)) print(Special_name(obj));
	    else printf("%p", Special_function(obj));
	    printf(">");
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
	case Closure: {
	    printf("<Closure ");
	    if (nil != Closure_name(obj)) {
			print(Closure_name(obj));
			printf(" ");
	    }
	    if (nil == obj->Closure.parameters)
			printf("()");
	    else
			print(obj->Closure.parameters);
	    printf(">");
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

oop car(oop obj)	{ return Object_type(obj) == Cell ? obj->Cell.a : nil; }
oop cdr(oop obj)	{ return Object_type(obj) == Cell ? obj->Cell.d : nil; }
oop caar(oop obj)	{ return car(car(obj)); }
oop cadr(oop obj)	{ return car(cdr(obj)); }
oop cdar(oop obj)	{ return cdr(car(obj)); }
oop cddr(oop obj)	{ return cdr(cdr(obj)); }
oop caddr(oop obj)	{ return car(cdr(cdr(obj))); }


#define RETURN(value) ({return (struct Result){ value, RET_RESULT };})

#define EVAL(EXP, ENV) ({                                \
    Result _result = eval(EXP, ENV);                     \
	if (_result.intent != RET_RESULT) return _result;    \
	_result.value;                                       \
    })

#define APPLY(OBJ, ARGS,ENV) ({                          \
    Result _result = apply(OBJ,ARGS, ENV);               \
	if (_result.intent != RET_RESULT) return _result;    \
	_result.value;                                       \
    })

#define EVLIST(EXP, ENV) ({                              \
    Result _result = evlist(EXP, ENV);                   \
	if (_result.intent != RET_RESULT) return _result;    \
	_result.value;                                       \
    })

#define EXPAND(EXP, ENV) ({                              \
    Result _result = expand(EXP, ENV);                   \
	if (_result.intent != RET_RESULT) return _result;    \
	_result.value;                                       \
    })

	
oop assoc(oop key, oop alist)
{
    while (Object_type(alist) == Cell) {
	oop head = alist->Cell.a;
	if (car(head) == key) return head;
	alist = alist->Cell.d;
    }
    return nil;
}

Result eval(oop exp, oop env);

Result evlist(oop list, oop env)
{
    if (Object_type(list) != Cell) return eval(list, env);
    RETURN(newCell(EVAL(list->Cell.a, env), EVLIST(list->Cell.d, env)));
}

oop pairlist(oop keys, oop values, oop tail)
{
    if (Object_type(keys) != Cell) {
	if (Object_type(keys) == Symbol)
	    return newCell(newCell(keys, values), tail);
	return tail;
    }
    oop ass = newCell(keys->Cell.a, car(values));
    return newCell(ass, pairlist(keys->Cell.d, cdr(values), tail));
}

oop define(oop name, oop value)
{
    assert(Object_type(name) == Symbol);
    switch (Object_type(value)) {
	case Primitive: if (nil == Primitive_name(value)) value->Primitive.name = name;  break;
	case Special:   if (nil == Special_name(value)) value->Special.name = name;      break;
	case Closure:   if (nil == Closure_name(value)) value->Closure.name = name;      break;
	default:                                                                         break;
    }
    return name->Symbol.value = value;
}

Result apply(oop func, oop args, oop env)
{
    switch (Object_type(func)) {
	case Primitive:	return func->Primitive.function(args, env);
	case Special:	return func->Special.function(args, env);
	case Closure: {
	    oop params = func->Closure.parameters;
	    oop body   = func->Closure.expressions;
	    oop cenv   = func->Closure.environment;
	    oop value  = nil;
	    env = pairlist(params, args, cenv);
	    while (Object_type(body) == Cell) {
			//value = EVAL(body->Cell.a, env);
			Result _result = eval(body->Cell.a, env);
			switch(_result.intent){
				case RET_RETURN:{
					_result.intent = RET_RESULT;
					return _result;
				}
				default:
					value = _result.value;
			}
			body = body->Cell.d;
	    }
	    RETURN(value);
	}
	default:
	    break;
    }
    printf("cannot apply: ");
    println(func);
    exit(1);
    RETURN(nil);
}

Result prim_add(oop args, oop env)
{
    int isum = 0;
    for (;;) {
		if (Object_type(args) != Cell) RETURN(newInteger(isum));
		oop arg = args->Cell.a;
		if (Object_type(arg) != Integer) break;
		isum += Integer_value(arg);
		args = args->Cell.d;
    }
    float fsum = isum;
    for (;;) {
		if (Object_type(args) != Cell) RETURN(newFloat(fsum));
		oop arg = args->Cell.a;
		if      (Object_type(arg) == Integer) fsum += Integer_value(arg);
		else if (Object_type(arg) == Float)   fsum += Float_value(arg);
		else                                  break;
		args = args->Cell.d;
    }
    fprintf(stderr, "non-numeric argument in +\n");
    exit(1);
    RETURN(nil);
}

Result prim_subtract(oop args,oop env){
    if (Object_type(args) != Cell) {
		printf("not enough arguments to -\n");
		exit(1);
    }
    int diff = Integer_value(args->Cell.a);
    if (Object_type(args = args->Cell.d) != Cell) RETURN(newInteger(-diff));
    do {
		oop arg = args->Cell.a;
		diff = diff - Integer_value(arg);
    } while (Object_type(args = args->Cell.d) == Cell);
    RETURN(newInteger(diff));
}

Result prim_multiply(oop args, oop env)
{
    int product = 1;
    while (Object_type(args) == Cell) {
	oop arg = args->Cell.a;
	product = product * Integer_value(arg);
	args = args->Cell.d;
    }
    RETURN(newInteger(product));
}

Result prim_less(oop args, oop env)
{
    if (Object_type(args) != Cell || Object_type(args->Cell.d) != Cell) {
	printf("not enough arguments to <\n");
	exit(1);
    }
    oop lhs = args->Cell.a;
    oop rhs = args->Cell.d->Cell.a;
    if (Object_type(lhs) != Integer || Object_type(rhs) != Integer) {
	printf("non-integer argument to <\n");
	exit(1);
    }
    RETURN(((intptr_t)lhs < (intptr_t)rhs) ? sym_t : nil);
}

Result prim_greater(oop args, oop env)
{
    if (Object_type(args) != Cell || Object_type(args->Cell.d) != Cell) {
        printf("not enough arguments to <\n");
        exit(1);
    }
    oop lhs = args->Cell.a;
    oop rhs = args->Cell.d->Cell.a;
    if (Object_type(lhs) != Integer || Object_type(rhs) != Integer) {
        printf("non-integer argument to <\n");
        exit(1);
    }
    RETURN(((intptr_t)lhs > (intptr_t)rhs)? sym_t : nil);
}

int equal(oop a, oop b)
{
    if (a == b) return 1;
    int t = Object_type(a);
    if (t != Object_type(b)) return 0;
    switch (t) {
		case Undefined: return 1;
		case Integer:   return Integer_value(a) == Integer_value(b);
		case Primitive: return Primitive_function(a) == Primitive_function(b);
		case Special:   return Special_function(a) == Special_function(b);
		default:        break;
    }
    return 0;
}

Result prim_equal(oop args, oop env)
{
    oop value = car(args);
    args = cdr(args);
    while (Object_type(args) == Cell) {
	oop arg = car(args);
	if (!equal(value, arg)) RETURN(nil);
	args = cdr(args);
    }
    RETURN(sym_t);
}

Result prim_print(oop args, oop env)
{
    oop value = nil;
    while (Object_type(args) == Cell) {
	value = args->Cell.a;
	print(value);
	args = args->Cell.d;
	if (nil != args) printf(" ");
    }
    RETURN(value);
}

Result prim_println(oop args, oop env)
{
    oop value = nil;
    while (Object_type(args) == Cell) {
        value = args->Cell.a;
        println(value);
        args = args->Cell.d;
    }
    RETURN(value);
}


Result prim_putc(oop args, oop env)
{
    oop value = nil;
    while (Object_type(args) == Cell) {
	oop value = args->Cell.a;
		putchar(Integer_value(value));
		args = args->Cell.d;
    }
    RETURN(value);
}

Result prim_cons(oop args, oop env)
{
    RETURN(newCell(car(args), cadr(args)));
}

Result prim_car(oop args, oop env)
{
    RETURN(caar(args));
}

Result prim_cdr(oop args, oop env)
{
    RETURN(cdar(args));
}
Result prim_listP(oop args, oop env)
{
    RETURN(Object_type(car(args)) == Cell ? sym_t : nil);
}

oop read(FILE *fp);

Result prim_read(oop args, oop env)
{
    RETURN(read(stdin));
}

Result prim_eval(oop args, oop env)
{
    return eval(car(args), env);
}

Result prim_apply(oop args, oop env)
{
    return apply(car(args), cadr(args), env);
}

Result prim_length(oop args, oop env)
{
    oop arg = car(args);
    switch (Object_type(arg)) {
	case Undefined: RETURN( newInteger(0));
	case String:	RETURN( newInteger(strlen(arg->String.value)));
	case Symbol:	RETURN( newInteger(strlen(arg->Symbol.name)));
	case Cell: {
	    int len = 0;
	    while (Object_type(arg) == Cell) ++len, arg = arg->Cell.d;
	    RETURN( newInteger(len));
	}
	default:
	    fprintf(stderr, "non-lengthy thing in length\n");
	    exit(1);
    }
    RETURN(nil);
}

oop concat(oop a, oop b)
{
    return (Object_type(a) == Cell)
	? newCell(a->Cell.a, concat(a->Cell.d, b))
	: b;
}

Result prim_concat(oop args, oop env)
{
    oop a = car(args);
    oop b = cadr(args);
    switch (Object_type(a)) {
	case Undefined: RETURN(b);
	case String: {
	    if (Object_type(b) != String) {
		fprintf(stderr, "cannot concat string and non-string\n");
		exit(1);
	    }
	    int la = strlen(a->String.value), lb = strlen(b->String.value);
	    char *buf = malloc(la + lb + 1);
	    memcpy(buf,    a->String.value, la);
	    memcpy(buf+la, b->String.value, lb);
	    buf[la+lb] = 0;
	    oop s = newString(buf);
	    RETURN(s);
	}
	case Symbol: {
	    if (Object_type(b) != Symbol) {
		fprintf(stderr, "cannot concat symbol and non-symbol\n");
		exit(1);
	    }
	    int la = strlen(a->Symbol.name), lb = strlen(b->Symbol.name);
	    char *buf = malloc(la + lb + 1);
	    memcpy(buf,    a->Symbol.name, la);
	    memcpy(buf+la, b->Symbol.name, lb);
	    buf[la+lb] = 0;
	    oop s = newSymbol(buf);
	    free(buf);
	    RETURN(s);
	}
	case Cell:	RETURN(concat(a, b));
	default:
	    fprintf(stderr, "non-lengthy thing in length\n");
	    exit(1);
    }
    RETURN(nil);
}

Result prim_return(oop args,oop env){
	return (struct Result){ car(args), RET_RETURN};
}

Result prim_break(oop args,oop env){
	return (struct Result){ nil, RET_BREAK};
}

Result prim_continue(oop args,oop env){
	return (struct Result){ nil, RET_CONTINUE};
}

Result spec_lambda(oop args, oop env)
{
    RETURN(newClosure(car(args), cdr(args), env));
}

Result spec_define(oop args, oop env)
{
    oop name  = car(args);
    oop value = car(cdr(args));
    if (Object_type(name) != Symbol) {
        fprintf(stderr, "define: name is not a symbol\n");
        exit(1);
    }
    value = EVAL(value, env);
    RETURN(define(name, value));
}

Result spec_define_syntax(oop args, oop env)
{
    oop name  = car(args);
    oop value = car(cdr(args));
    if (Object_type(name) != Symbol) {
		fprintf(stderr, "define-syntax: name is not a symbol\n");
		exit(1);
    }
	value = EVAL(value,env);
    if (Object_type(value) != Closure) {
		println(value);
		fprintf(stderr, "define-syntax: value is not a closure\n");
		exit(1);
    }
    RETURN(name->Symbol.syntax = value);
}

Result spec_setq(oop args, oop env)
{
    oop name  = car(args);
    oop value = car(cdr(args));
    if (Object_type(name) != Symbol) {
	fprintf(stderr, "setq: name is not a symbol\n");
	exit(1);
    }
    value = EVAL(value, env);
    oop ass = assoc(name, env);
    if (Object_type(ass) == Cell)
	ass->Cell.d = value;
    else
	name->Symbol.value = value;
    RETURN(value);
}

Result spec_quote(oop args, oop env)
{
    RETURN(car(args));
}

Result spec_if(oop args, oop env)
{
    return EVAL(car  (args), env) == nil
	?  eval(caddr(args), env)
	:  eval(cadr (args), env);
}

Result spec_while(oop args, oop env) // (while test expressions...)
{
    oop test   = car(args);
    oop result = nil;
    args = cdr(args);
Label:
    while (nil != EVAL(test, env)) {
		oop exprs = args;
		while (Cell == Object_type(exprs)) {
			Result _result = eval(exprs->Cell.a, env);
			switch(_result.intent){
				case RET_BREAK:{
					_result.intent = RET_RESULT;
					return _result;
				}
				case RET_CONTINUE:{
					_result.intent = RET_RESULT;
					goto Label;
				}
				case RET_RETURN:{
					return _result;//if not work
				}
					
				default:{
					result = _result.value;
				}
			}
			exprs = exprs->Cell.d;
		}
    }
    RETURN(result);
}

Result spec_and(oop args, oop env) // (and exp exp exp ...)
{
    oop result = sym_t;
    while (Object_type(args) == Cell) {
	result = EVAL(args->Cell.a, env); // head of the list
	if (nil == result) RETURN(nil);
	args = args->Cell.d;         // tail of the list
    }
    RETURN(result);
}

Result spec_or(oop args, oop env) // (or exp exp exp ...)
{
    oop result = nil;
    while (Object_type(args) == Cell) {
	result = EVAL(args->Cell.a, env); // head of the list
	if (nil != result) RETURN(result);
	args = args->Cell.d;         // tail of the list
    }
    RETURN(result);
}

Result spec_not(oop args, oop env) // (or exp exp exp ...)
{
    RETURN(car(args) == nil ? sym_t : nil);
}

Result spec_let(oop args, oop env) // (let ((v1 e1) (v2 e2) ...) exprs...)
{
    oop env2 = env;
    oop bindings = car(args);
    oop exprs = cdr(args);
    while (Object_type(bindings) == Cell) {
		oop bind  = bindings->Cell.a;
		oop name  = car(bind);
		oop value = cadr(bind);
		env2 = newCell(newCell(name, EVAL(value, env)), env2); // NOTE: evaluate args in ORIGINAL environment
		bindings = bindings->Cell.d;
    }
    oop result = nil;
    while (Object_type(exprs) == Cell) {
		Result _result = eval(exprs->Cell.a, env2);
		switch(_result.intent){
			case RET_RETURN:{
				_result.intent = RET_RESULT;
				return _result;
			}
			default:
				result = _result.value;
		}
		exprs = exprs->Cell.d;
    }
    RETURN(result);
}


Result eval(oop exp, oop env)
{
    switch (Object_type(exp)) {
	case Undefined:	
	case Integer:	
	case Float:	
	case String:    RETURN(exp);
	case Symbol: {
	    oop ass = assoc(exp, env);
	    if (ass != nil) {
			assert(Object_type(ass) == Cell);
			RETURN(ass->Cell.d);
	    }
	    if (exp->Symbol.value)RETURN(exp->Symbol.value);
	    fprintf(stderr, "undefined variable: %s\n", exp->Symbol.name);
	    exit(1);
	    break;
	}
	case Cell: { // (function ...arguments...)
	    oop func = exp->Cell.a;
	    oop args = exp->Cell.d;
	    func = EVAL(func, env);
	    if (Object_type(func) != Special) args = EVLIST(args, env);
	    return apply(func, args, env);
	}
	default:	break;
    }
    assert(0);
    RETURN(nil);
}

int isident(int c)
{
    return isalpha(c) || strchr("!#$%&*+-/:;<=>?@\\^_|~", c);
}

int nextchar(FILE *fp)
{
    int c = getc(fp);
    for (;;) {
	while (isspace(c)) c = getc(fp);
	if (c != ';') break;
	for (;;) {
	    c = getc(fp);
	    if (EOF == c) return c;
	    if (c == '\n') break;
	}
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

oop read(FILE *fp) // read stdin and return an object, or 0 if EOF
{
    int c;
    c = nextchar(fp);
    if (isdigit(c)) { // number, float is: digits+ . digits* [e [-+] digits+]?
	char buf[32];
	int i = 0, dotted = 0;
	while ((i < sizeof(buf) - 1 && isdigit(c))) {
	    buf[i++] = c;
	    c = getc(fp);
	}
	if ((i < sizeof(buf) - 1 && '.'== c)) {
	    buf[i++] = c;
	    c = getc(fp);
	    ++dotted;
	    while ((i < sizeof(buf) - 1 && isdigit(c))) {
		buf[i++] = c;
		c = getc(fp);
	    }
	}
	ungetc(c, fp);
	buf[i] = 0;
	if (dotted && i > 1) return newFloat(atof(buf));
	return newInteger(atoi(buf));
    }
    if (isident(c)) { // symbol
	char string[64];
	int length = 0;
	do {
	    string[length++] = c;
	    c = getc(fp);
	} while((isident(c) || isdigit(c)) && length < sizeof(string) - 1);
	ungetc(c, fp);
	string[length] = '\0';
	if (!strcmp(string, "nil")) return nil;
	return newSymbol(string);
    }
    if (c == '(') { //  ()  (x)  (x . y)  (x y z)  (x y . z)
	oop list = nil;
	c = nextchar(fp);
	while (c != ')' && c != '.') {
	    ungetc(c, fp);
	    oop obj = read(fp);
	    if (!obj) {
		fprintf(stderr, "EOF while reading list\n");
		exit(1);
	    }
	    list = newCell(obj, list); // push element onto front of list
	    c = nextchar(fp);
	}
	oop last = nil;
	if (c == '.') {
	    last = read(fp);
	    if (!last) {
		fprintf(stderr, "EOF while reading list\n");
		exit(1);
	    }
	    c = nextchar(fp);
	}
	if (c != ')') {
	    fprintf(stderr, "')' expected at end of list\n");
	    exit(1);
	}
	list = revlist(list, last);
	return list;
    }
    if (c == '\'') {
		oop obj = read(fp);
		if (!obj) {
			fprintf(stderr, "EOF while reading quoted data\n");
			exit(1);
		}
		return newCell(sym_quote, newCell(obj, nil));
    }
    if (c == '`') {
		oop obj = read(fp);
		if (!obj) {
			fprintf(stderr, "EOF while reading quoted data\n");
			exit(1);
		}
		return newCell(sym_quasiquote, newCell(obj, nil));
    }
    if (c == ',') {
		oop unquote = sym_unquote;
		int d = getc(fp);
		if (d == '@') unquote = sym_unquote_splicing;
		else          ungetc(d, fp);
		oop obj = read(fp);
		if (!obj) {
			fprintf(stderr, "EOF while reading quoted data\n");
			exit(1);
		}
		return newCell(unquote, newCell(obj, nil));
    }
    if ('"' == c) { // string
	char string[64];
	int length = 0;
	while(('"' != (c = getc(fp))) && length < sizeof(string) - 1)
	    string[length++] = c;
	string[length] = '\0';
	return newString(strdup(string));
    }
    if (c == EOF) return 0;
    fprintf(stderr, "illegal character: 0x%02x\n", c);
    exit(1);
    return 0;
}

Result expand(oop obj, oop env)
{
    if (Object_type(obj) != Cell) RETURN(obj); // cannot be a macro
    oop head = obj->Cell.a;
    if (Object_type(head) == Symbol && head->Symbol.syntax)
	// make sure it is not quote HERE !
	return expand(APPLY(head->Symbol.syntax, obj, env), env);  // is a (macro ...)
    oop tail = obj->Cell.d;
    RETURN(newCell(EXPAND(head, env), EXPAND(tail, env))); // expand head and tail recursively
}


int opt_v = 0;

void replFile(FILE *fp)
{
    // discard #!interpreter line
    int c = getc(fp);
    if (c != '#') ungetc(c, fp);
    else while (c >= ' ') c = getc(fp);

    for (;;) { // read-print loop
	oop obj = read(fp);
	if (!obj) break; /* EOF */	if (opt_v > 1) { println(obj); }
		Result _result1 = expand(obj,nil);
		obj = _result1.value;
		if (opt_v > 2) { printf("  -> ");  println(obj); }
		Result _result2 = eval(obj,nil);
		switch(_result2.intent){
			case RET_RESULT:break;
			case RET_RETURN:{
				fprintf(stderr,"return outside function\n");
				exit(1);
			}
			case RET_BREAK:
			{
				fprintf(stderr,"break outside loop\n");
				exit(1);
			}
			case RET_CONTINUE:
			{
				fprintf(stderr,"continue outside loop\n");
				exit(1);
			}
			default:{
				fprintf(stderr,"not defined case in replFile\n");
				exit(1);
			}
		}
		obj = _result2.value;
		if (opt_v > 0) { if (opt_v > 1) printf("  => ");  println(obj); }
    }
}


void replPath(char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
	perror(path);
	exit(1);
    }
    replFile(fp);
    fclose(fp);
}

/*
-------------------------2023/07/18------------------------
//†
    2023/07/18
    reading array
    define 
*/
    /* 0    1           2       3       4      5    6       7   8*/
enum {HALT, LITERAL, VARIABLE, CALL, DEFINE, SETQ, JUMP, JUMPF,DROP,};

oop execute(oop program,oop env){
    int pc = 0;
    oop stack = newArray(10);
    for(;;){
        oop inst = Array_get(program,pc++);
        switch(Integer_value(inst)){
            case LITERAL:{
                oop value = Array_get(program,pc++); 
                Array_push(stack,value);
                continue;
            }
            case VARIABLE:{
                oop name = Array_get(program,pc++);
                oop ass = assoc(name,env);
                if (ass != nil) {
                    assert(Object_type(ass) == Cell);
                    Array_push(stack,(ass->Cell.d));
                    continue;
                }
                if (name->Symbol.value){
                    Array_push(stack,name->Symbol.value);
                    continue;
                }
                fprintf(stderr, "undefined variable: %s\n", name->Symbol.name);
                exit(1);
                continue;
            }
            case CALL:{
                oop value = Array_get(program,pc++);
                oop func = Array_pop(stack);
                int n = Integer_value(value);
                oop args = nil;
                while(n--){
                    args = newCell(Array_pop(stack),args);
                }
                args = revlist(args,nil);
                oop result = nil;
                switch(Object_type(func)){
                    case Primitive:{
                        result = func->Primitive.function(args, env).value;
                        break;
                    }
                    default:{
                        fprintf(stderr,"cannot call\n");
                        exit(1);
                    }
                }
                Array_push(stack,result);
                continue;
            }
            case DEFINE:{
                oop name  = Array_get(program,pc++); 
                oop value = Array_pop(stack);
                Array_push(stack,value);//->Array_top();
                name->Symbol.value = value;
                continue;
            }
            case SETQ:{
                oop name  = Array_get(program,pc++); 
                oop value = Array_pop(stack);
                Array_push(stack,value);//->Array_top();
                oop ass   = assoc(name,env);
                if (Object_type(ass) == Cell){
                    ass->Cell.d = value;
                }else{
                    name->Symbol.value = value; 
                }
                continue;
            }
            case DROP:{
                Array_pop(stack);//-> POP()
                continue;
            }
            case JUMPF:{
                int offset = Integer_value(Array_get(program,pc++)); 
                oop cond = Array_pop(stack);
                if(cond == nil)pc += offset;
                continue;
            }
            case JUMP:{
                int offset = Integer_value(Array_get(program,pc++)); 
                pc += offset;
                continue;
            }
            case HALT:{
                if(Array_size(stack)==1){
                    return Array_pop(stack);
                }
                printf("HALT with %d items on stack\n",Array_size(stack));
                return nil;
            }
        }
    }
}

// omit writing
#define emit(X) Array_push(program,X)
#define emitI(A) emit(newInteger(A))
#define emitS(A) emit(newSymbol(A))
#define emitII(OP,A) emitI(OP); emitI(A)
#define emitIS(OP,A) emitI(OP); emitS(A)
#define emitIO(OP,A) emitI(OP); emit(A)

void compile(oop program,oop exp, oop env);

// (+ 1 2) -> (1 2) -> (2 nil) -> nil
// 2 -> 1 -> +
int compileArgs(oop program,oop exp,oop env){
    if (Object_type(exp) != Cell) return 0;
    int n = compileArgs(program, exp->Cell.d, env);
    compile(program,exp->Cell.a,env);
    return n + 1;
}

int compileBody(oop program,oop exp, oop env){
    int count = 0;
    if(exp == nil)return 0;
    for(;;){
        if(Object_type(exp)==Cell){
            count += 1;
            emitI(DROP);
            compile(program, exp->Cell.a, env);
            exp = exp->Cell.d;
        }
        else{
            return (count == 1)?0:1;
        }
    }
}



void compile(oop program,oop exp, oop env){
    switch(Object_type(exp)){
        case Float:
        case String:
        case Integer:{
            emitIO(LITERAL,exp);
            break;
        }
        case Symbol:{
            emitIO(VARIABLE,exp);
            break;
        }
        case Cell:{
            oop head = exp->Cell.a;
            if (head == sym_define){// (define symbol value)
                oop name = cadr(exp);
                oop valu = caddr(exp);
                if (Object_type(name) != Symbol){
                    fprintf(stderr,"define [complie]\n");
                    exit(1);
                }
                compile(program,valu,env);
                emitIO(DEFINE, name);
                break;
            }
            if (head == sym_setq){// (setq symbol value)
                oop name = cadr(exp);
                oop valu = caddr(exp);
                if (Object_type(name) != Symbol){
                    fprintf(stderr,"setq [complie]\n");
                    exit(1);
                }
                compile(program,valu,env);
                emitIO(SETQ, name);
                break;
            }
            if (head == sym_while){// (while (cond) body...)
                oop cond = cadr(exp);
                oop body = cddr(exp);

                emitIO(LITERAL,nil);int p_cond = program->Array.size;// 
                compile(program,cond,env);
                emitII(JUMPF,0);int p_jumpf = program->Array.size;
                /* whlie */
                int i = compileBody(program,body,env);
                emitII(JUMP, 0);int p_jump = program->Array.size;
                /* done*/
                int p_done = p_jump;//should be remove
                /* jumpf */
                
                Array_put(program,p_jumpf-1,newInteger(p_done-p_jumpf-1+i));
                /* jump  */
                Array_put(program,p_jump -1,newInteger(p_cond-p_jump));
                break;
            }
            if (head == sym_if){//(if (cond) (cons) (altr))
                oop cond = cadr(exp);
                oop cons = caddr(exp);
                oop altr = caddr(cdr(exp));


                compile(program,cond,env);
                emitII(JUMPF,0);//->else
                int jump1 = program->Array.size;
                /*  if  */
                compile(program,cons,env);
                emitII(JUMP,0);//->done
                int jump2 = program->Array.size;
                /* else */
                int p_else    = program->Array.size;
                compile(program,altr,env);
                /* done */
                int p_done = program->Array.size;
                /* jumpf */
                Array_put(program, jump1-1, newInteger(p_else - jump1));
                /* jump  */
                Array_put(program, jump2-1, newInteger(p_done - jump2));
                break;
            }
            int n = compileArgs(program,exp,env);
            emitII(CALL, n-1);
            break;
        }
        default:{
            fprintf(stderr,"cannot compile\n");
            exit(1);
        }
    }
}

oop compileProgram(oop exp, oop env){
    oop program = newArray(10);
    compile(program,exp,env);
    emitI(HALT);
    return program;
}

void println_Array(oop obj){
	println(obj);
    assert(Object_type(obj)==Array);
    int Size = obj->Array.size;
    if(Size==0){printf("empty\n");return;}
    while(Size){
        print(obj->Array.elements[--Size]);
        putchar(' ');
    }
    putchar('\n');
    return;
}

int main(int argc, char **argv)
{
    GC_init();

	for(int i; i<Closure+1; i++){
		alloc[i] = 0;
	}

    nil        	     = newObject(Undefined);
    sym_t      		 = newSymbol("t");

    sym_quote 		 = newSymbol("quote");
    sym_unquote 	 = newSymbol("unquote");
    sym_unquote_splicing = newSymbol("unquote-splicing");
    sym_quasiquote 	 = newSymbol("quasiquote");

    sym_define       = newSymbol("define");
    sym_setq         = newSymbol("setq");
    sym_while        = newSymbol("while");
    sym_if           = newSymbol("if");

    define(newSymbol("+"),     	newPrimitive(prim_add     ));
    define(newSymbol("-"),     	newPrimitive(prim_subtract));
    define(newSymbol("*"),     	newPrimitive(prim_multiply));
    define(newSymbol("<"),     	newPrimitive(prim_less    ));
	define(newSymbol(">"),      newPrimitive(prim_greater ));
    define(newSymbol("="),     	newPrimitive(prim_equal   ));
    define(newSymbol("print"), 	newPrimitive(prim_print   ));
    define(newSymbol("println"),newPrimitive(prim_println ));
    define(newSymbol("putc"),  	newPrimitive(prim_putc    ));
    define(newSymbol("cons"),  	newPrimitive(prim_cons    ));
    define(newSymbol("car"),   	newPrimitive(prim_car     ));
    define(newSymbol("cdr"),   	newPrimitive(prim_cdr     ));
	define(newSymbol("list?"), 	newPrimitive(prim_listP   ));
	define(newSymbol("read"),  	newPrimitive(prim_read    ));
    define(newSymbol("eval"),  	newPrimitive(prim_eval    ));
    define(newSymbol("apply"), 	newPrimitive(prim_apply   ));
    define(newSymbol("length"), newPrimitive(prim_length  ));
    define(newSymbol("concat"), newPrimitive(prim_concat  ));
	define(newSymbol("return"), newPrimitive(prim_return  ));
	define(newSymbol("break"),  newPrimitive(prim_break   ));
	define(newSymbol("continue"),newPrimitive(prim_continue));

    define(newSymbol("lambda"),        newSpecial(spec_lambda));
    define(newSymbol("define"),        newSpecial(spec_define));
    define(newSymbol("define-syntax"), newSpecial(spec_define_syntax));
    define(newSymbol("setq"),          newSpecial(spec_setq  ));
    define(newSymbol("quote"),         newSpecial(spec_quote ));
    define(newSymbol("if"),            newSpecial(spec_if    ));
    define(newSymbol("while"),         newSpecial(spec_while ));
    define(newSymbol("and"),           newSpecial(spec_and   ));
    define(newSymbol("or"),            newSpecial(spec_or    ));
    define(newSymbol("not"),           newSpecial(spec_not   ));
    define(newSymbol("let"),           newSpecial(spec_let   ));

    for(;;){
        oop program = read(stdin);
        program = compileProgram(program, nil);
        println_Array(program);
        println(execute(program, nil));
    }
    return 0;
    
    int argn = 1, booted = 0, repled = 0;
    while (argn < argc) {
		char *arg = argv[argn++];
		if      (!strcmp(arg, "-v")) ++opt_v;
		else if (!strcmp(arg, "+v")) --opt_v;
		else if (!strcmp(arg, "-b")) ++booted;
		else if (!strcmp(arg, "-" )) {
			if (!booted++) replPath("boot.grl");
			replFile(stdin);
			++repled;
		}
		else {
			if (!booted++) replPath("boot.grl");
			replPath(arg);
			++repled;
		}
    }
    if (!repled) {
		replPath("boot.grl");
		replFile(stdin);
    }
	for(int i; i<Closure+1; i++){
		printf("%d: %d\n",i,alloc[i]);
	}
    return 0;
}
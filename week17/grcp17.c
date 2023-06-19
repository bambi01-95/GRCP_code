//memo
// copy of prof Piumarta sensei code
// grcp

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
#include <setjmp.h>


union Object;

typedef union Object Object;
typedef Object *oop;

typedef enum { ILLEGAL, Undefined, Integer, Float, String, Symbol, Primitive, Special, Cell, Closure } type_t;

typedef oop (*prim_t)(oop args, oop env);

struct Integer 	 { type_t type;  int    value; };
struct Float 	 { type_t type;  float  value; };
struct String  	 { type_t type;  char  *value; };
struct Symbol  	 { type_t type;  char  *name;  oop value;  oop syntax; };
struct Primitive { type_t type;  prim_t function;  oop name; };
struct Special   { type_t type;  prim_t function;  oop name; };
struct Cell    	 { type_t type;  oop    a, d; };
struct Closure   { type_t type;  oop    parameters, expressions, name, environment;  };

union Object
{
    type_t type;
    struct Integer   Integer;
    struct Float     Float;
    struct String    String;
    struct Symbol    Symbol;
    struct Primitive Primitive;
    struct Special   Special;
    struct Cell      Cell;
    struct Closure   Closure;
};

oop nil     = 0;

oop sym_t			= 0;
oop sym_quote			= 0;
oop sym_unquote			= 0;
oop sym_unquote_splicing  	= 0;
oop sym_quasiquote		= 0;

oop newObject(type_t type)
{
    oop obj = GC_calloc(1, sizeof(Object));
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

oop assoc(oop key, oop alist)
{
    while (Object_type(alist) == Cell) {
	oop head = alist->Cell.a;
	if (car(head) == key) return head;
	alist = alist->Cell.d;
    }
    return nil;
}

oop eval(oop exp, oop env);

oop evlist(oop list, oop env)
{
    if (Object_type(list) != Cell) return eval(list, env);
    return newCell(eval(list->Cell.a, env), evlist(list->Cell.d, env));
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

oop apply(oop func, oop args, oop env)
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
			value = eval(body->Cell.a, env);
			body = body->Cell.d;
	    }
	    return value;
	}
	default:
	    break;
    }
    printf("cannot apply: ");
    println(func);
    exit(1);
    return 0;
}

oop prim_add(oop args, oop env)
{
    int isum = 0;
    for (;;) {
		if (Object_type(args) != Cell) return newInteger(isum);
		oop arg = args->Cell.a;
		if (Object_type(arg) != Integer) break;
		isum += Integer_value(arg);
		args = args->Cell.d;
    }
    float fsum = isum;
    for (;;) {
		if (Object_type(args) != Cell) return newFloat(fsum);
		oop arg = args->Cell.a;
		if      (Object_type(arg) == Integer) fsum += Integer_value(arg);
		else if (Object_type(arg) == Float)   fsum += Float_value(arg);
		else                                  break;
		args = args->Cell.d;
    }
    fprintf(stderr, "non-numeric argument in +\n");
    exit(1);
    return 0;
}

oop prim_subtract(oop args, oop env)
{
    if (Object_type(args) != Cell) {
		printf("not enough arguments to -\n");
		exit(1);
    }
    int diff = Integer_value(args->Cell.a);
    if (Object_type(args = args->Cell.d) != Cell) return newInteger(-diff);
    do {
		oop arg = args->Cell.a;
		diff = diff - Integer_value(arg);
    } while (Object_type(args = args->Cell.d) == Cell);
    return newInteger(diff);
}

oop prim_multiply(oop args, oop env)
{
    int product = 1;
    while (Object_type(args) == Cell) {
	oop arg = args->Cell.a;
	product = product * Integer_value(arg);
	args = args->Cell.d;
    }
    return newInteger(product);
}

oop prim_less(oop args, oop env)
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
    return lhs->Integer.value < rhs->Integer.value ? sym_t : nil;
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

oop prim_equal(oop args, oop env)
{
    oop value = car(args);
    args = cdr(args);
    while (Object_type(args) == Cell) {
	oop arg = car(args);
	if (!equal(value, arg)) return nil;
	args = cdr(args);
    }
    return sym_t;
}

oop prim_print(oop args, oop env)
{
    oop value = nil;
    while (Object_type(args) == Cell) {
	value = args->Cell.a;
	print(value);
	args = args->Cell.d;
	if (nil != args) printf(" ");
    }
    return value;
}

oop prim_putc(oop args, oop env)
{
    oop value = nil;
    while (Object_type(args) == Cell) {
	oop value = args->Cell.a;
	putchar(Integer_value(value));
	args = args->Cell.d;
    }
    return value;
}

oop prim_cons(oop args, oop env)
{
    return newCell(car(args), cadr(args));
}

oop prim_car(oop args, oop env)
{
    return caar(args);
}

oop prim_cdr(oop args, oop env)
{
    return cdar(args);
}

oop prim_listP(oop args, oop env)
{
    return Object_type(car(args)) == Cell ? sym_t : nil;
}

oop read(FILE *fp);

oop prim_read(oop args, oop env)
{
    return read(stdin);
}

oop prim_eval(oop args, oop env)
{
    return eval(car(args), env);
}

oop prim_apply(oop args, oop env)
{
    return apply(car(args), cadr(args), env);
}

oop prim_length(oop args, oop env)
{
    oop arg = car(args);
    switch (Object_type(arg)) {
	case Undefined: return newInteger(0);
	case String:	return newInteger(strlen(arg->String.value));
	case Symbol:	return newInteger(strlen(arg->Symbol.name));
	case Cell: {
	    int len = 0;
	    while (Object_type(arg) == Cell) ++len, arg = arg->Cell.d;
	    return newInteger(len);
	}
	default:
	    fprintf(stderr, "non-lengthy thing in length\n");
	    exit(1);
    }
    return 0;
}

oop concat(oop a, oop b)
{
    return (Object_type(a) == Cell)
	? newCell(a->Cell.a, concat(a->Cell.d, b))
	: b;
}

oop prim_concat(oop args, oop env)
{
    oop a = car(args);
    oop b = cadr(args);
    switch (Object_type(a)) {
	case Undefined: return b;
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
	    return s;
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
	    return s;
	}
	case Cell:	return concat(a, b);
	default:
	    fprintf(stderr, "non-lengthy thing in length\n");
	    exit(1);
    }
    return 0;
}

oop spec_lambda(oop args, oop env)
{
    return newClosure(car(args), cdr(args), env);
}

oop spec_define(oop args, oop env)
{
    oop name  = car(args);
    oop value = car(cdr(args));
    if (Object_type(name) != Symbol) {
	fprintf(stderr, "define: name is not a symbol\n");
	exit(1);
    }
    value = eval(value, env);
    return define(name, value);
}

oop spec_define_syntax(oop args, oop env)
{
    oop name  = car(args);
    oop value = car(cdr(args));
    if (Object_type(name) != Symbol) {
	fprintf(stderr, "define-syntax: name is not a symbol\n");
	exit(1);
    }
    value = eval(value, env);
    if (Object_type(value) != Closure) {
	fprintf(stderr, "define-syntax: value is not a closure\n");
	exit(1);
    }
    return name->Symbol.syntax = value;
}

oop spec_setq(oop args, oop env)
{
    oop name  = car(args);
    oop value = car(cdr(args));
    if (Object_type(name) != Symbol) {
	fprintf(stderr, "setq: name is not a symbol\n");
	exit(1);
    }
    value = eval(value, env);
    oop ass = assoc(name, env);
    if (Object_type(ass) == Cell)
	ass->Cell.d = value;
    else
	name->Symbol.value = value;
    return value;
}

oop spec_quote(oop args, oop env)
{
    return car(args);
}

oop spec_if(oop args, oop env)
{
    return eval(car  (args), env) == nil
	?  eval(caddr(args), env)
	:  eval(cadr (args), env);
}

oop spec_while(oop args, oop env) // (while test expressions...)
{
    oop test   = car(args);
    oop result = nil;
    args = cdr(args);
    while (nil != eval(test, env)) {
	    oop exprs = args;
        while (Cell == Object_type(exprs)) {
            result = eval(exprs->Cell.a, env);
            exprs = exprs->Cell.d;
        }
    }
    return result;
}

oop spec_and(oop args, oop env) // (and exp exp exp ...)
{
    oop result = sym_t;
    while (Object_type(args) == Cell) {
	result = eval(args->Cell.a, env); // head of the list
	if (nil == result) return nil;
	args = args->Cell.d;         // tail of the list
    }
    return result;
}

oop spec_or(oop args, oop env) // (or exp exp exp ...)
{
    oop result = nil;
    while (Object_type(args) == Cell) {
	result = eval(args->Cell.a, env); // head of the list
	if (nil != result) return result;
	args = args->Cell.d;         // tail of the list
    }
    return result;
}

oop spec_not(oop args, oop env) // (or exp exp exp ...)
{
    return car(args) == nil ? sym_t : nil;
}



struct Buff{
  struct Buff *next;
  jmp_buf *buf;
};

struct Buff *buffArray;
int buffCount = 0;
oop returnValue;

void push(jmp_buf *buf){
    struct Buff *new = GC_malloc(sizeof(struct Buff));
    new->buf = buf;
    new->next = buffArray;
    buffArray = new;
    buffCount++;
    return;
}

jmp_buf *pop(void){
    if(buffCount<1){
        printf(stderr,"ERROR: pop\n");
        exit(1);
    }
    jmp_buf * buf = buffArray->buf;
    buffArray = buffArray->next;
    buffCount--;
    return buf;
}

oop prim_return(oop args,oop env){
  returnValue = eval(car(args),env);
  if(buffCount>0){
    jmp_buf *buf = pop();
    longjmp(*buf,1);
  }
  fprintf(stderr,"no argument in the buffArray\n");
  exit(1);
  return nil;
}


oop spec_let(oop args, oop env) // (let ((v1 e1) (v2 e2) ...) exprs...)
{
    oop env2 = env;
    oop bindings = car(args);
    oop exprs = cdr(args);
    while (Object_type(bindings) == Cell) {
		oop bind  = bindings->Cell.a;
		oop name  = car(bind);
		oop value = cadr(bind);
		env2 = newCell(newCell(name, eval(value, env)), env2); // NOTE: evaluate args in ORIGINAL environment
		bindings = bindings->Cell.d;
    }
    oop result = nil;
    jmp_buf buf;
    push(&buf);
    if(setjmp(buf)){
        result = returnValue;
    }else{
        while (Object_type(exprs) == Cell) {
            result = eval(exprs->Cell.a, env2);
            exprs = exprs->Cell.d;
        }
        pop();
    }
    return result;
}

oop eval(oop exp, oop env)
{
    switch (Object_type(exp)) {
	case Undefined:	return exp;
	case Integer:	return exp;
	case Float:	return exp;
	case String:    return exp;
	case Symbol: {
	    oop ass = assoc(exp, env);
	    if (ass != nil) {
		assert(Object_type(ass) == Cell);
		return ass->Cell.d;
	    }
	    if (exp->Symbol.value)
		return exp->Symbol.value;
	    fprintf(stderr, "undefined variable: %s\n", exp->Symbol.name);
	    exit(1);
	    break;
	}
	case Cell: { // (function ...arguments...)
	    oop func = exp->Cell.a;
	    oop args = exp->Cell.d;
	    func = eval(func, env);
	    if (Object_type(func) != Special) args = evlist(args, env);
	    return apply(func, args, env);
	}
	default:	break;
    }
    assert(0);
    return 0;
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

oop expand(oop obj, oop env)
{
    if (Object_type(obj) != Cell) return obj; // cannot be a macro
    oop head = obj->Cell.a;
    if (Object_type(head) == Symbol && head->Symbol.syntax)
	// make sure it is not quote HERE !
	return expand(apply(head->Symbol.syntax, obj, env), env);  // is a (macro ...)
    oop tail = obj->Cell.d;
    return newCell(expand(head, env), expand(tail, env)); // expand head and tail recursively
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
	obj = expand(obj, nil);		if (opt_v > 2) { printf("  -> ");  println(obj); }
	obj = eval(obj, nil);		if (opt_v > 0) { if (opt_v > 1) printf("  => ");  println(obj); }
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


int main(int argc, char **argv)
{
    GC_init();

    nil        	         = newObject(Undefined);

    sym_t      		 = newSymbol("t");
    sym_quote 		 = newSymbol("quote");
    sym_unquote 	 = newSymbol("unquote");
    sym_unquote_splicing = newSymbol("unquote-splicing");
    sym_quasiquote 	 = newSymbol("quasiquote");

    define(newSymbol("+"),     	newPrimitive(prim_add     ));
    define(newSymbol("-"),     	newPrimitive(prim_subtract));
    define(newSymbol("*"),     	newPrimitive(prim_multiply));
    define(newSymbol("<"),     	newPrimitive(prim_less    ));
    define(newSymbol("="),     	newPrimitive(prim_equal   ));
    define(newSymbol("print"), 	newPrimitive(prim_print   ));
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
    define(newSymbol("return"), newPrimitive(prim_return  )); //grcp17

    define(newSymbol("lambda"),        newSpecial(spec_lambda));
    define(newSymbol("define"),        newSpecial(spec_define));
    define(newSymbol("define-syntax"), newSpecial(spec_define_syntax));
    define(newSymbol("setq"),          newSpecial(spec_setq));
    define(newSymbol("quote"),         newSpecial(spec_quote));
    define(newSymbol("if"),            newSpecial(spec_if));
    define(newSymbol("while"),         newSpecial(spec_while));
    define(newSymbol("and"),           newSpecial(spec_and));
    define(newSymbol("or"),            newSpecial(spec_or));
    define(newSymbol("not"),           newSpecial(spec_not));
    define(newSymbol("let"),           newSpecial(spec_let));

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

    return 0;
}
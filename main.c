#include <stdio.h>
#include <ctype.h>
#include <stdlib.h> /* To provide a declaration of ‘exit’ */

#define BSIZE 128
#define NONE -1
#define EOS '\0'

#define NUM 256
#define DIV 257
#define MOD 258
#define ID 259
#define DONE 260

#define STRMAX 999 /* Size of lexemes array*/
#define SYMAX 100 /* Size of symtable */

int tokenval; /* Value of token attribute */
int lineno;
int lookahead;

char lexbuf[BSIZE];
int lineno = 1;
int tokenval = NONE;

struct entry{  /* From of symbol table entry */
    char *lexptr;
    int token;
};    

struct entry symtable[]; /* Symbol table*/
struct entry keywords[] = {
    "div", DIV,
    "mod", MOD,
    0, 0
};

char lexemes[STRMAX];
int lastchar = -1; /* Last used position in lexemes*/

struct entry symtable[SYMAX];
int lastentry = 0; /* Last used position in symtable*/

// Functions prototypes
int lexan(void);
void parse(void);
void expr(void);
void term(void);
void factor(void);
void match(int t);
void emit(int t, int tval);
int lookup(char s[]);
int insert(char s[], int tok);
void init(void);
void error(char *m);

/* Main */
main(){
    init();
    parse();
    exit(0); /* Successful termination */
}

/* Lexer*/
int lexan(){ /* Lexical Analyzer*/
    int t;
    while (1){
        t = getchar();
        if (t == ' ' || t == '\t')
            ; /* Strip out white space*/
        else if (t == '\n')
            lineno = lineno + 1;
        else if (isdigit(t)){
            ungetc(t, stdin);
            scanf("%d", &tokenval);
            return NUM;
        }
        else if(isalpha(t)){ /* t is a letter */
            int p, b = 0;
            while(isalnum(t)){ /* t is alphanumeric*/
                lexbuf[b] = t;
                t = getchar();
                b = b + 1;
                if(b >= BSIZE)
                    error("Compiler Error");
            }

            lexbuf[b] = EOS;
            if(t != EOF)
                ungetc(t, stdin);
            p = lookup(lexbuf);

            if(p == 0)
                p = insert(lexbuf, ID);

            tokenval = p;
            return symtable[p].token;
            }

            else if (t == EOF)
                return DONE;

            else{
                tokenval = NONE;
                return t;
            }
    }
}

/* Parser */
parse(){ /* parser and translate expression list*/
    lookahead = lexan();
    while(lookahead != DONE){
        expr(); match(';');
    }
}

expr(){
    int t;
    term();
    while(1)
        switch(lookahead){
        case '+' : case '-':
            t = lookahead;
            match(lookahead); term(); emit(t, NONE);
            continue;
        default : 
            return;
        }
}

term(){
    int t;
    factor();
    while(1){
        switch(lookahead){
        case '*' : case '/' : case DIV : case MOD : case '%':
            t = lookahead;
            match(lookahead); factor(); emit(t, NONE);
            continue;

        default:
            return;
        }
    }
}

factor(){
    switch(lookahead){
        case '(':
            match('('); expr(); match(')'); break;
        case NUM:
            emit(NUM, tokenval); match(NUM); break;
        case ID:
            emit(ID, tokenval); match(ID); break;
        default:
            error("Syntax Error");
    }
}

match(int t) {
    if(lookahead == t)
        lookahead = lexan();
    else        
        error("Syntax Error");
}

/* Emitter */
emit(int t, int tval){
    switch(t){
        case '+' : case '-' : case '*' : case '/': case '%':
            printf("%c\n", t); break;
        case DIV:
            printf("DIV\n"); break;
        case MOD:
            printf("MOD\n"); break;
        case NUM:
            printf("%d\n", tval); break;
        case ID:
            printf("%s\n", symtable[tval].lexptr); break;
        default:
            printf("token %d, tokenval %d\n", t, tval);
    }
}

/* Symbol */
int lookup(char s[]){
    int p;
    for(p = lastentry; p > 0; p = p - 1)
        if(strcmp(symtable[p].lexptr, s) == 0)
            return p;
    return 0;
}

int insert(char s[], int tok){
    int len;
    len = strlen(s); 

    if(lastentry + 1 >= SYMAX)
        error("Symbol Table Full");
    if(lastchar + len + 1 >= STRMAX)
        error("Lexemes Array Full");

    lastentry = lastentry + 1;
    symtable[lastentry].token = tok;
    symtable[lastentry].lexptr = &lexemes[lastchar + 1];

    lastchar = lastchar + len + 1;
    strcpy(symtable[lastentry].lexptr, s);
    return lastentry;

}

/* Init */
init(){ /* Loads keywords into symtable*/
    struct entry *p;
    for(p = keywords; p->token; p++)
        insert(p->lexptr, p->token);
}

/* Error */
error(char *m){
    fprintf(stderr, "line %d: %s\n", lineno, m);
    exit(1); /* Unsuccessful termination */
}
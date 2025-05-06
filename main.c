#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> /* To provide a declaration of ‘exit’ */

#define BSIZE 128
#define NONE -1
#define EOS '\0'

#define NUM 256
#define VAR 257
#define INTEGER 258
#define ID 259
#define REAL 260
#define DONE 261
#define PROGRAM 262
#define EOEX 263
#define BEGIN 264
#define END 265
#define IF 266
#define THEN 267
#define REPEAT 268
#define UNTIL 269
#define ELSE 270
#define RELOP 271
#define ADDOP 272
#define MULOP 273
#define NOT 274
#define INPUT 275
#define OUTPUT 276
#define ASSIGNOP 277
#define WRITELN 278
#define EOD 279 // end of declarations

#define STRMAX 999 /* Size of lexemes array*/
#define SYMAX 100 /* Size of symtable */

int tokenval; /* Value of token attribute */
int lineno;
int lookahead;
int is_main_block = 0;

#define IDMAX 100
char* idlist[IDMAX];
int idcount = 0;

char lexbuf[BSIZE];
int lineno = 1;
int tokenval = NONE;

struct entry{  /* From of symbol table entry */
    char *lexptr;
    int token;
};    

// struct entry symtable[]; /* Symbol table*/
struct entry keywords[] = {
    "program", PROGRAM,
    "input", INPUT,
    "output", OUTPUT,
    "var", VAR,
    "integer", INTEGER,
    "real", REAL,
    "begin", BEGIN,
    "end", END,
    "if", IF,
    "then", THEN,
    "else", ELSE,
    "repeat", REPEAT,
    "until", UNTIL,
    "writeln", WRITELN,
    "OR", ADDOP,
    "DIV", MULOP,
    "MOD", MULOP,
    "AND", MULOP,
    "not", NOT,
    0, 0
};

char lexemes[STRMAX];
int lastchar = -1; /* Last used position in lexemes*/

struct entry symtable[SYMAX];
int lastentry = 0; /* Last used position in symtable*/

// Functions prototypes
int lexan(void);
void parse(void);
void program(void);
void header(void);
void declarations(void);
void variableDeclarations(void);
void r1 (void);
void variableDeclaration (void);
void identifierList (void);
void r2 (void);
void type (void);
void block (void);
void statements (void);
void r3 (void);
void statement (void);
void matchedStatement (void);
void unmatchedStatement (void);
void a1 (void);
void a2 (void);
void a3 (void);
void a4 (void);
void expressionList (void);
void r4 (void);
void expression (void);
void trivialCase (void);
void simpleExpression (void);
void r5 (void);
void term (void);
void r6 (void);
void factor();
void match(int t);
void emit(int t, int tval);
int lookup(char s[]);
int insert(char s[], int tok);
void init(void);
void error(char *m);

/* Main */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <inputfile> <outputfile>\n", argv[0]);
        return 1;
    }

    FILE *infile = fopen(argv[1], "r");
    if (!infile) {
        perror("Error while opening the input file");
        return 1;
    }

    FILE *outfile = fopen(argv[2], "w");
    if (!outfile) {
        perror("Error while opening the output file");
        fclose(infile);
        return 1;
    }

    // Redirect stdin and stdout to read and write to the files
    if (freopen(argv[1], "r", stdin) == NULL) {
        perror("Failed to redirect stdin");
        return 1;
    }
    if (freopen(argv[2], "w", stdout) == NULL) {
        perror("Failed to redirect stdout");
        return 1;
    }

    init();     // Load keywords into the symbol table
    parse();    // Start parsing

    fclose(infile);
    fclose(outfile);

    return 0; // Successful termination
}

/* Lexer*/
int lexan(void) { /* Lexical Analyzer */
    int t;
    while (1){
        t = getchar();
        if (t == ' ' || t == '\t')
            ; /* Strip out white space */
        else if (t == '\n')
            lineno = lineno + 1;
        // else if (t == '#') {
        //     while ((t = getchar()) != '\n')
        //         ; /* Strip out comments (whole line) */
        //     lineno = lineno + 1;
        // }
        else if (isdigit(t)) {
            int b = 0;
            lexbuf[b++] = t;
            t = getchar();

            // Integer part
            while (isdigit(t)) {
                if (b >= BSIZE) error("Compiler Error");
                lexbuf[b++] = t;
                t = getchar();
            }

            // Fractional part
            if (t == '.') {
                lexbuf[b++] = t;
                t = getchar();
                if (!isdigit(t)) {
                    error("Invalid number format");
                }
                while (isdigit(t)) {
                    if (b >= BSIZE) error("Compiler Error");
                    lexbuf[b++] = t;
                    t = getchar();
                }
            }

            // Exponent part
            if (t == 'e' || t == 'E') {
                lexbuf[b++] = t;
                t = getchar();
                if (t == '+' || t == '-') {
                    lexbuf[b++] = t;
                    t = getchar();
                }
                if (!isdigit(t)) {
                    error("Invalid exponent format");
                }
                while (isdigit(t)) {
                    if (b >= BSIZE) error("Compiler Error");
                    lexbuf[b++] = t;
                    t = getchar();
                }
            }

            lexbuf[b] = EOS;
            if (t != EOF)
                ungetc(t, stdin);
            tokenval = NONE;
            return NUM;
        } else if (isalpha(t)) { 
            int p, b = 0;
            while (isalnum(t)) { 
                if (b >= BSIZE)
                    error("Compiler Error");
                lexbuf[b++] = tolower(t); 
                t = getchar();
            }

            lexbuf[b] = EOS;
            if (t != EOF)
                ungetc(t, stdin);
            p = lookup(lexbuf);

            if (p == 0)
                p = insert(lexbuf, ID);

            tokenval = p;
            // printf("Token: %s, Type: %d\n", lexbuf, symtable[p].token); // Debug print
            return symtable[p].token;
        } else if (t == ':') {
            t = getchar();
            if (t == '=') {
                tokenval = NONE;
                return ASSIGNOP;
            } else {
                // TODO: if the : followed with = this it's assig. else it should br for declaration stat (VariableDeclaration -> IdentifierList : Type)
                // should i give it tokenval ??? the same for , ????
                if (t != EOF) ungetc(t, stdin);
                    tokenval = NONE;
                    return EOD; 
            }
        } else if (t == '<') {
            t = getchar();
            if (t == '=') {
                tokenval = NONE;
                return RELOP;
            } else if (t == '>') {
                tokenval = NONE;
                return RELOP;
            } else {
                if (t != EOF) ungetc(t, stdin);
                tokenval = NONE;
                return RELOP;
            }
        } else if (t == '>') {
            t = getchar();
            if (t == '=') {
                tokenval = NONE;
                return RELOP;
            } else {
                if (t != EOF) ungetc(t, stdin);
                tokenval = NONE;
                return RELOP;
            }
        } else if (t == '=') {
            tokenval = NONE;
            return RELOP;
        } else if (t == '+' || t == '-') {
            tokenval = NONE;
            return ADDOP;
        } else if (t == '*' || t == '/') {
            tokenval = NONE;
            return MULOP;
        } else if (t == EOF)
            return DONE;
        else {
            tokenval = NONE;
            return t;
        }
    }
}

/* Parser */
void parse(void){ /* parser and translate expression list*/
    lookahead = lexan();
    program();
}

void program(void) {
    header(); declarations(); 
    is_main_block = 1;
    // TODO : BLOCK. -> main, BLOCK -> normal block 
    block(); match('.');
}

void header(void) {
    // int t = lookahead;
    match(PROGRAM);
    emit(PROGRAM, NONE);
    match(ID); 
    match('('); 
    match(INPUT); 
    match(','); 
    match(OUTPUT);  
    match(')');  
    match(';');  
}

void declarations(void) {
    if (lookahead == VAR){
        match(VAR);  
        variableDeclarations();
    }else{
        // <epsilon> 
    }
}

void variableDeclarations(void) {
    variableDeclaration();
    r1();
}

void r1 (void){
    if(lookahead == ID){
        variableDeclaration();
        r1();        
    }else{
        // <epsilon> 
    }
}

void variableDeclaration (void){
    identifierList();
    match(EOD); emit(EOD, tokenval);

    type();

    match(';'); emit(';', tokenval);
}

void identifierList (void){
    match(ID); emit(ID, tokenval);
    r2();
}

void r2 (void){
    if(lookahead == ','){
        match(','); emit(',', tokenval);
        match(ID); emit(ID, tokenval);
        r2();
    }else{
        // <epsilon> 
    }
}

void type (void){
    switch (lookahead){
        case INTEGER:
            match(INTEGER); emit(INTEGER, tokenval);
            break;
        case REAL:
            match(REAL); emit(REAL, tokenval);
            break;
        default:
            return;
    }
}

void block (void){
    match(BEGIN);
    if (is_main_block) {
        printf("int main(void)\n");
    }
    emit(BEGIN, NONE); 

    statements();

    match(END);
    if (is_main_block) {
        printf("return 0;\n");
        is_main_block = 0; // reset after use
    }
    emit(END, NONE); 
}

void statements (void){
    statement();
    r3();
}

void r3 (void){
    if(lookahead == ';'){
        match(';');
        emit(';', tokenval);
        statement();
        r3();
    }else{
        // <epsilon> 
    }
}

void statement (void){
    if(lookahead == IF){
        unmatchedStatement();
    }else{
        matchedStatement();
    }
}

void matchedStatement (void){
    switch (lookahead){
        case ID:
            emit(ID, tokenval); match(ID);
            emit(ASSIGNOP, tokenval); match(ASSIGNOP);
            expression(); break;
        case BEGIN:
            block(); break;
        case IF:
            match(IF); emit(IF, tokenval); emit('(',tokenval); expression(); emit(')',tokenval);
            match(THEN); emit(THEN, tokenval); matchedStatement(); match(ELSE); emit(ELSE, tokenval); matchedStatement(); break;
        case REPEAT:
            match(REPEAT); emit(REPEAT, tokenval); statement(); match(UNTIL); emit(UNTIL, tokenval); emit('(', tokenval); expression(); emit(')', tokenval); break;
        case WRITELN:
            match(WRITELN); emit(WRITELN, tokenval); match('('); emit('(', tokenval); simpleExpression(); match(')'); emit(')', tokenval); 
        default:
            return;
    }
}

void unmatchedStatement (void){
    match(IF); emit(IF, tokenval); a1();
}

void a1 (void){
    expression(); a2();
}

void a2 (void){
    match(THEN); emit(THEN, tokenval); a3();
}

void a3 (void){
    statement();
    // TODO: or should ask DR. Nabil
    matchedStatement(); match(ELSE); emit(ELSE, tokenval); unmatchedStatement();
}

void expressionList (void){
    expression();
    r4();
}

void r4 (void){
    if(lookahead == ','){
        match(','); emit(',', tokenval); 
        expression(); 
        r4();
    }else{
        // <epsilon> 
    }
}

void expression (void){
    simpleExpression();
    a4();
}

void a4 (void){
    if(tokenval == RELOP){ // TODO: To be checked later (tokenval / lookahead)
        match(RELOP); emit(RELOP, tokenval); 
    }else{
        // <epsilon> 
    }
}

void trivialCase (void){
    if(tokenval == RELOP){ // TODO: To be checked later (tokenval / lookahead)
        match(RELOP); emit(RELOP, tokenval); 
        term();
    }else{
        term();
    } 
}

void simpleExpression (void){
    trivialCase();
    r5();
}

void r5 (void){
    if (tokenval == RELOP){  // TODO: To be checked later (tokenval / lookahead)
        match(RELOP); emit(RELOP, tokenval);  
        term(); r5();
    }else{
        // <epsilon> 
    }
}

void term (void){
    factor();
    r6();
}

void r6 (void){
    if (tokenval == MULOP){ // TODO: To be checked later (tokenval / lookahead)
        match(MULOP); emit(MULOP, tokenval);  factor(); r6();
    }else{
        // <epsilon> 
    }
}

void factor (void){
    switch (lookahead){
        case ID:
            emit(ID, tokenval); match(ID);break;
        case NUM:
            emit(NUM, tokenval); match(NUM);break;
        case '(':
            emit('(', tokenval); match('(');
            expression();
            emit(')', tokenval);match(')'); break;
        case NOT:
            emit(NOT, tokenval); match(NOT); factor(); break;
        default:
            return;
    }
}

void match(int t) {
    if(lookahead == t)
        lookahead = lexan();
    else {
        printf("\n\n\n");
        printf("%d ", t);
        printf("%d ", lookahead);
        printf("\n\n\n");
        error("Syntax Error");
    }       
        // printf(lookahead);
}

/* Emitter */
void emit(int t, int tval){
    switch(t){
        case '+' : case '-' : case '*' : case '/': case '%':
            printf("%c ", t); break;
        // case DIV:
        //     printf("DIV "); break;
        // case MOD:
        //     printf("MOD "); break;
        case PROGRAM:
            printf("#include<stdio.h>\n"); break;
        
        case 'm':
            printf("%s\n", "int main(void)"); break;

        case '(': case ')': case ';': case ',':
            printf("%c", t); break;

        case NUM:
            printf("%d ", tval); break;

        case BEGIN:
            printf("{\n"); break;

        case END:
            printf("}\n"); break;
        
        // case '.':
            // printf("return 0;\n"); break;
        
        case ID:
            printf("%s ", symtable[tval].lexptr); break;
        
        case 'i': // i for id
            printf("%c", tval); break;
        
        case ASSIGNOP:
            printf("="); break;
        
        case RELOP:
        
            if (strcmp(symtable[tval].lexptr, "<>") == 0){
                printf("%s ", "!="); break;
            }else{
                printf("%s ", symtable[tval].lexptr); break;
            }

        case MULOP: case ADDOP:
            printf("%s ", symtable[tval].lexptr); break;
        
        case INTEGER:
            printf("int "); break;
        
        case REAL:
            printf("float "); break;
        
        case IF:
            printf("if"); break;
        
        case THEN:
            printf("\n"); break;
        
        case ELSE:
            printf("else\n"); break;
        
        case NOT:
            printf("!"); break;
        
        case REPEAT:
            printf("do\n{\n"); break;
    
        case UNTIL:
            printf("}\nwhile !"); break;
            // printf("} while !");
        
        case WRITELN:
            if (symtable[tval].token == INTEGER) {
                printf("printf(\"%%d\\n\", %s);\n", symtable[tval].lexptr);
            } else if (symtable[tval].token == REAL) {
                printf("printf(\"%%f\\n\", %s);\n", symtable[tval].lexptr);
            } else {
                error("/* Unknown type for WRITELN */\n");
            }
            break;

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
void init(void){ /* Loads keywords into symtable*/
    struct entry *p;
    for(p = keywords; p->token; p++)
        insert(p->lexptr, p->token);
}

/* Error */
void error(char *m){
    fprintf(stderr, "line %d: %s\n", lineno, m);
    exit(1); /* Unsuccessful termination */
}
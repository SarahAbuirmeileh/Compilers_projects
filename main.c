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
#define GT 280 
#define GE 281 
#define LT 282
#define LE 283 
#define EQ 284 
#define NE 285 
#define DIV 286 
#define MOD 287 
#define AND 288 
#define OR 289
#define BOOL 290

#define STRMAX 999 /* Size of lexemes array*/
#define SYMAX 100 /* Size of symtable */

int tokenval; /* Value of token attribute */
int lineno;
int lookahead;
int is_main_block = 0;

char current_ids[SYMAX][BSIZE]; // to hold variable names temporarily
int current_id_count = 0;
int semicolon_read = 0;

char lexbuf[BSIZE];
char compareBuf[BSIZE];
int lineno = 1;
int tokenval = NONE;

struct entry{  /* From of symbol table entry */
    char *lexptr;
    int token;
    int type; 
}; 

struct emitEntry {
    int token;
    int tokenval;
};

struct emitEntry emitBuffer[SYMAX];
int emit_buffer_index = 0;
int store_emit_buffer = 0;

// struct entry symtable[]; /* Symbol table*/
struct entry keywords[] = {
    {"program", PROGRAM, NONE},
    {"input", INPUT, NONE},
    {"output", OUTPUT, NONE},
    {"var", VAR, NONE},
    {"integer", INTEGER, NONE},
    {"real", REAL, NONE},
    {"begin", BEGIN, NONE},
    {"end", END, NONE},
    {"if", IF, NONE},
    {"then", THEN, NONE},
    {"else", ELSE, NONE},
    {"repeat", REPEAT, NONE},
    {"until", UNTIL, NONE},
    {"writeln", WRITELN, NONE},
    {"OR", ADDOP, NONE},
    {"DIV", MULOP, NONE},
    {"MOD", MULOP, NONE},
    {"AND", MULOP, NONE},
    {"not", NOT, NONE},
    {0, 0, NONE} 
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
void moreVariableDeclarations (void);
void variableDeclaration (void);
void identifierList (void);
void moreIdList (void);
void type (void);
void block (void);
void statements (void);
void r3 (void);
void statement (void);
void ifStatement (void);
void optionalTail (void);
int a1 (int t);
void expressionList (void);
void r4 (void);
int expression (void);
int trivialCase (void);
int simpleExpression (void);
int r5 (int t);
int term (void);
int r6 (int t);
int factor();
void match(int t);
void emit(int t, int tval);
int lookup(char s[]);
int insert(char s[], int tok, int type);
void init(void);
void error(char *m);
void store_emit(int token, int tokenval);

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
    // fprintf(stderr, "Lexan\n");

    int t;
    int token;
    while (1){
        t = getchar();
        if (t == ' ' || t == '\t')//
            ; /* Strip out white space */
        else if (t == '\n')
            lineno = lineno + 1;
        else if (t == '#') {
            while ((t = getchar()) != '\n')
                ; /* Strip out comments (whole line) */
            lineno = lineno + 1;
        }else if (isdigit(t)) {
            int b = 0;
            int isFloat = 0;
            lexbuf[b++] = t;
            t = getchar();
        
            // Integer part
            while (isdigit(t)) {
                if (b < BSIZE) lexbuf[b++] = t;
                t = getchar();
            }
        
            // Optional fractional part
            if (t == '.') {
                isFloat = 1;
                if (b < BSIZE) lexbuf[b++] = t;
                t = getchar();
                if (!isdigit(t)) {
                    // Invalid float e.g., "123."
                    error("Invalid float literal");
                }
                while (isdigit(t)) {
                    if (b < BSIZE) lexbuf[b++] = t;
                    t = getchar();
                }
            }
        
            // Optional exponent part
            if (t == 'e' || t == 'E') {
                isFloat = 1;
                if (b < BSIZE) lexbuf[b++] = t;
                t = getchar();
                if (t == '+' || t == '-') {
                    if (b < BSIZE) lexbuf[b++] = t;
                    t = getchar();
                }
                if (!isdigit(t)) {
                    error("Invalid exponent in number");
                }
                while (isdigit(t)) {
                    if (b < BSIZE) lexbuf[b++] = t;
                    t = getchar();
                }
            }
        
            // Finalize
            if (t != EOF) ungetc(t, stdin);
            lexbuf[b] = EOS;
            int type = INTEGER;
            if(isFloat){
                type = REAL;
            }
            tokenval = insert(lexbuf, NUM, type);  // Keep the full num as string
            // fprintf(stderr, "Token: %s\n", lexbuf);
            return NUM;
        }else if (isalpha(t)) { 
            int p, b = 0;
            while (isalnum(t)) { 
                if (b >= BSIZE)
                    error("Compiler Error");
                compareBuf[b] = tolower(t);
                lexbuf[b++] = t; 
                t = getchar();
            }

            compareBuf[b] = EOS;
            lexbuf[b] = EOS;
            if (t != EOF)
                ungetc(t, stdin);

            if (strcmp(compareBuf, "div") == 0){
                tokenval = DIV;
                token = MULOP;
                p = lookup(compareBuf);
            }else if (strcmp(compareBuf, "and") == 0){
                tokenval = AND;
                token = MULOP;
            }else if (strcmp(compareBuf, "mod") == 0){
                tokenval = MOD;
                token = MULOP;
            }else if (strcmp(compareBuf, "or") == 0){
                tokenval = OR;
                token = ADDOP;
            }else{
                p = lookup(lexbuf);
                if (p == 0)
                    p = insert(lexbuf, ID, NONE);

                tokenval = p;
                token = symtable[p].token;
            }
            // fprintf(stderr, "Token: %s\n", lexbuf);
            
            // printf("Token: %s, Type: %d\n", lexbuf, symtable[p].token); // Debug print
            return token;
        } else if (t == ':') {
            t = getchar();
            if (t == '=') {
                tokenval = NONE;
            // fprintf(stderr, "Token: :=\n");

                return ASSIGNOP;
            } else {
                if (t != EOF) ungetc(t, stdin);
                    tokenval = NONE;
            // fprintf(stderr, "Token: :\n");

                    return EOD; 
            }
        } else if (t == '<') {
            t = getchar();
            if (t == '=') {
                tokenval = LE;
            // fprintf(stderr, "Token: <=\n");

                return RELOP;
            } else if (t == '>') {
                tokenval = NE;
            // fprintf(stderr, "Token: <>\n");

                return RELOP;
            } else {
                if (t != EOF) ungetc(t, stdin);
                tokenval = LT;
            // fprintf(stderr, "Token: <\n");

                return RELOP;
            }
        } else if (t == '>') {
            t = getchar();
            if (t == '=') {
                tokenval = GE;
            // fprintf(stderr, "Token: >=\n");

                return RELOP;
            } else {
                if (t != EOF) ungetc(t, stdin);
            // fprintf(stderr, "Token: >\n");

                tokenval = GT;
                return RELOP;
            }
        } else if (t == '=') {
            tokenval = EQ;
            // fprintf(stderr, "Token: =\n");

            return RELOP;
        } else if (t == '+' || t == '-') {
            tokenval = t;
            if (t == '+'){

                // fprintf(stderr, "Token: +\n" );
            }else{
                // fprintf(stderr, "Token: - \n" );

            }

            return ADDOP;
        } else if (t == '*' || t == '/') {
            tokenval = t;
            if (t == '*'){

                // fprintf(stderr, "Token: *\n" );
            }else{
                // fprintf(stderr, "Token: \\ \n" );

            }
            return MULOP;
        } else if (t == EOF)
            return DONE;
        else {
            tokenval = NONE;
            // fprintf(stderr, "Token: %d\n", t);
            return t;
        }
    }
}

/* Parser */
void parse(void){ /* parser and translate expression list*/
    // fprintf(stderr, "Parser\n");
    lookahead = lexan();
    program();
}

void program(void) {
    // fprintf(stderr, "Program\n");
    header(); declarations(); 
    is_main_block = 1;
    // TODO : BLOCK. -> main, BLOCK -> normal block 
    block(); match('.');
}

void header(void) {
    // fprintf(stderr, "Header\n");
    match(PROGRAM);
    match(ID); 
    match('('); 
    match(INPUT); 
    match(','); 
    match(OUTPUT);  
    match(')');  
    match(';');  
    emit(PROGRAM, tokenval);
}

void declarations(void) {
    // fprintf(stderr, "Declarations\n");
    if (lookahead == VAR){
        match(VAR);  
        variableDeclarations();
    }else{
        // <epsilon> 
    }
}

void variableDeclarations(void) {
    // fprintf(stderr, "VariableDeclarations\n");
    variableDeclaration();
    moreVariableDeclarations();
}

void moreVariableDeclarations (void){
    // fprintf(stderr, "MoreVariableDeclarations\n");
    if(lookahead == ID){
        variableDeclaration();
        moreVariableDeclarations();
    }else{
        // <epsilon> 
    }
}

void variableDeclaration (void){
    // fprintf(stderr, "VariableDeclaration\n");
    identifierList();
    match(EOD); 
    
    type();
    match(';'); 
}

void identifierList(void) {
    // fprintf(stderr, "IdentifierList\n");
    if (lookahead == ID) {
        strcpy(current_ids[current_id_count++], symtable[tokenval].lexptr);
        match(ID);
        moreIdList();
    } else {
        error("Expected identifier");
    }
}

void moreIdList(void) {
    // fprintf(stderr, "MoreIdList\n");
    if (lookahead == ',') {
        match(',');
        if (lookahead == ID) {
            strcpy(current_ids[current_id_count++], symtable[tokenval].lexptr);
            match(ID);
            moreIdList();
        } else {
            error("Expected identifier after ','");
        }
    }else{
        // <epsilon>
    }
}

void type(void){
    // fprintf(stderr, "Type\n");
    if (lookahead == INTEGER || lookahead == REAL) {
        int declared_type = lookahead;
        if (lookahead == INTEGER) {
            match(INTEGER); emit(INTEGER, tokenval);
        } else {
            match(REAL); emit(REAL, tokenval);
        }

        for (int i = 0; i < current_id_count; ++i) {
            int tval = lookup(current_ids[i]);
            if (tval == 0) tval = insert(current_ids[i], ID, declared_type);
            
            symtable[tval].type = declared_type;
            emit(ID, tval); 
            if (i < current_id_count - 1) {
                emit(',', tokenval);
            }
        }
        emit(';', tokenval);

        current_id_count = 0; // Reset for the next declaration
    } else {
        error("Expected type 'integer' or 'real' ");
    }
}

void block (void){
    // fprintf(stderr, "Block\n");
    match(BEGIN);
    if (is_main_block == 1) {
        printf("int main(void)\n");
    }
    is_main_block++; // reset after use
    int cur_is_main_block = is_main_block;
    emit(BEGIN, NONE); 

    statements();

    match(END);
    if (cur_is_main_block == 2) {
        printf("return 0;\n");
    }
    emit(END, NONE); 
}

void statements (void){
    // fprintf(stderr, "Statements\n");
    statement();
    r3();
}

void r3 (void){
    // fprintf(stderr, "R3\n");
    if(semicolon_read == 1){
        semicolon_read = 0;
        // fprintf(stderr, "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& %d\n", lookahead);

        statement();
        r3();
    }else if(lookahead == ';'){
        match(';'); 
        // fprintf(stderr, "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ %d\n", lookahead);
        statement();
        r3();
    }else{
        // <epsilon> 
    }
}

void statement (void){
    // fprintf(stderr, "statement\n");
    // fprintf(stderr, "\n **************************** Lookahead :%d ******************\n", lookahead);
    switch (lookahead){
        case ID:
            int tval = tokenval;
            int idToken = tval;
            match(ID); emit(ID, tval); 

            tval = tokenval;
            match(ASSIGNOP); emit(ASSIGNOP, tval); 

            int expr_type = expression(); 
            // Type checking
            if (symtable[idToken].type == INTEGER && expr_type == REAL) {
                error("Type Error: Can't assign float/real into integer");
            }
            emit(';', tokenval); break;
            break;
            
        case BEGIN:
            block(); break;
        case IF:
            ifStatement(); break;
        case REPEAT:
            // fprintf(stderr, "!!!!!!!!!!!!!!!! \n");
            match(REPEAT); emit(REPEAT, tokenval); 
            statements(); 
            match(UNTIL); emit(UNTIL, tokenval); 
            emit('(', tokenval); expression(); emit(')', tokenval); emit(';', tokenval); break;
        case WRITELN:
            match(WRITELN);  match('('); 
            store_emit_buffer = 1;
            int type = simpleExpression();
            emit(WRITELN, type);
            store_emit_buffer = 0;
            match(')'); emit(';', tokenval); break;
        default:
            return;
    }
}

void ifStatement(void){
    match(IF); emit(IF, tokenval);

    emit('(',tokenval);
    expression(); 
    emit(')',tokenval);

    match(THEN); emit(THEN, tokenval); 
   
    statement(); 
    if (lookahead == ';'){
        match(';');
        semicolon_read = 1;
    }
    optionalTail();
    // fprintf(stderr, "########### Lookahead: ######## %d\n", lookahead);
}

void optionalTail(void){
    // fprintf(stderr, "OptionalTail\n");
    if(lookahead == ELSE){
        match(ELSE); emit(ELSE, tokenval);
        statement();
    }else{
        // <epsilon>
    }
}

void expressionList (void){
    // fprintf(stderr, "ExpressionList\n");
    expression();
    r4();
}

void r4 (void){
    // fprintf(stderr, "R4\n");
    if(lookahead == ','){
        match(','); emit(',', tokenval); 
        expression(); 
        r4();
    }else{
        // <epsilon> 
    }
}

int simpleExpression(void){
    // fprintf(stderr, "SimpleExpression\n");
    int leftType = trivialCase();
    return r5(leftType);
}

int r5(int leftType){
    // fprintf(stderr, "R5\n");
    if (lookahead == ADDOP){
        int tval = tokenval;
        match(ADDOP); 
        if(store_emit_buffer == 0){
            emit(ADDOP, tval);  
        }else{
            store_emit(ADDOP, tval); 
        } 
        
        int rightType = term();
        // if (leftType != rightType) {
        //     error("Type Error: mismatch in addition/subtraction");
        // }
        int type = INTEGER;
        if (leftType == REAL || rightType == REAL){
            type = REAL;
        }
        return r5(type);
    }
    return leftType;
}

int trivialCase(void){
    // fprintf(stderr, "TrivialCase\n");
    if (lookahead == ADDOP) {
        int tval = tokenval;
        match(ADDOP);
        if(store_emit_buffer == 0){
            emit(ADDOP, tval); 
        }else{
            store_emit(ADDOP, tval);
        } 
        return term();
    }
    return term();
}

int term(void){
    // fprintf(stderr, "Term\n");
    int leftType = factor();
    return r6(leftType);
}

int r6(int leftType){
    // fprintf(stderr, "R6\n");
    if (lookahead == MULOP){ 
        int tval = tokenval;
        match(MULOP);
        
        if (store_emit_buffer == 0){
            emit(MULOP, tval);  
        }else{
            store_emit(MULOP, tval);  
        }
        int rightType = factor();
        int type = INTEGER;
        // if (leftType != rightType) {
        //     error("Type mismatch in multiplication/division");
        // }
        if(leftType == REAL || rightType == REAL){
            type = REAL;
        }
        return r6(type);
    }
    return leftType;
}

int expression(void){
    // fprintf(stderr, "Expression\n");
    int leftType = simpleExpression();
    return a1(leftType);
}

int a1(int leftType){
    // fprintf(stderr, "A1\n");
    if(lookahead == RELOP){ 
        int tval = tokenval;
        match(RELOP); emit(RELOP, tval); 

        int rightType = simpleExpression();
        if (leftType != rightType) {
            error("Type mismatch in relational expression");
        }
        return BOOL;
    }
    return leftType; 
}

int factor(void){
    // fprintf(stderr, "Factor\n");
    switch (lookahead){
        case ID: {
            int tval = tokenval;

            if (store_emit_buffer == 0){
                emit(ID, tokenval); 
            }else{
                store_emit(ID, tokenval); 
            }
            match(ID);
            return symtable[tval].type;
        }
        case NUM: {
            int t = symtable[tokenval].type; // implement based on your tokens
             
            if (store_emit_buffer == 0){
                emit(NUM, tokenval); 
            }else{
                store_emit(NUM, tokenval); 
            }
            match(NUM);
            return t;
        }
        case '(':
            if (store_emit_buffer == 0){
                emit('(', tokenval);
            }else{
                store_emit('(', tokenval);
            }

            match('(');
            int t = expression();

            if (store_emit_buffer == 0){
                emit(')', tokenval); 
            }else{
                store_emit(')', tokenval); 
            }
            
            match(')');
            return t;

        case NOT:

            if (store_emit_buffer == 0){
                emit(NOT, tokenval); 
            }else{
                store_emit(NOT, tokenval); 
            }
            match(NOT);
            int t2 = factor();
            if (t2 != BOOL) error("Expected boolean for NOT");
            return BOOL;
        default:
            error("Invalid factor");
            return INTEGER;
    }
}

void match(int t) {
    // fprintf(stderr, "Match\n");
    // fprintf(stderr, " t: %d, lookahead: %d tokenval: %d\n", t, lookahead, tokenval);
    if(lookahead == t)
        lookahead = lexan();
    else {
        error("Syntax Error");
    }       
}

/* Emitter */
void emit(int t, int tval){
    // fprintf(stderr, "Emit %d\n", t);
    switch(t){
        case '+' : case '-' : case '*' : case '/': case '%':
            printf("%c ", t); break;

        case PROGRAM:
            printf("#include<stdio.h>\n"); break;
        
        case 'm':
            printf("%s\n", "int main(void)"); break;

        case '(': case ')': case ',':
            printf("%c", t); break;

        case ';':
            printf("%c\n", t); break;

        case NUM:
            printf("%s", symtable[tval].lexptr); break;

        case BEGIN:
            printf("{\n"); break;

        case END:
            printf("}\n"); break;
        
        case ID:
            printf("%s", symtable[tval].lexptr); break;

        case ASSIGNOP:
            printf("="); break;
        
        case RELOP:
            switch (tval){
                case LT:
                    printf("<"); return;
                case LE:
                    printf("<="); return;
                case GT:
                    printf(">"); return;
                case GE:
                    printf(">="); return;
                case EQ:
                    printf("="); return;
                case NE:
                    printf("!="); return;
                
                default: return;
            }

        case MULOP: 
            switch (tval){
                case '*':
                    printf("*"); return;
                case '/':
                    printf("/"); return;
                case DIV:
                    printf("/"); return;
                case MOD:
                    printf("%%"); return;
                case AND:
                    printf("&&"); return;
                
                default: return;
            }
        
        case ADDOP:
            switch (tval){
                case '+':
                    printf("+"); return;
                case '-':
                    printf("-"); return;
                case OR:
                    printf("||"); return;
                
                default: return;
            }
        
        case INTEGER:
            printf("int "); break;
        
        case REAL:
            printf("float "); break;
        
        case IF:
            printf("if"); break;
        
        case THEN:
            printf("\n"); break;
        
        case ELSE:
            // fprintf(stderr, "ELSE ************\n");
            printf("else\n"); break;
        
        case NOT:
            printf("!"); break;
        
        case REPEAT:
            printf("do\n{\n"); break;
    
        case UNTIL:
            printf("}\nwhile !"); break;
            // printf("} while !");
        
        case WRITELN:
            printf("printf(\"%%");
            if(tval == INTEGER){
                printf("d");
            }else if (tval == REAL){
                printf("f");
            }
            printf("\\n\",");

            for (int i = 0; i < emit_buffer_index; i++){
                emit(emitBuffer[i].token, emitBuffer[i].tokenval);
            }
            emit_buffer_index = 0;
            printf(")");
            break;
        
        default:
            printf("token %d, tokenval %d\n", t, tval);
    }
}

/* Symbol */
int lookup(char s[]) {
    // fprintf(stderr, "Lookup\n");
    int p;
    char s_lower[BSIZE], p_lower[BSIZE];

    for (p = lastentry; p > 0; p = p - 1) {
        // Convert s to lowercase
        int i = 0;
        while (s[i] && i < BSIZE - 1) {
            s_lower[i] = tolower((unsigned char)s[i]);
            i++;
        }
        s_lower[i] = '\0';

        // Convert symtable[p].lexptr to lowercase
        i = 0;
        while (symtable[p].lexptr[i] && i < BSIZE - 1) {
            p_lower[i] = tolower((unsigned char)symtable[p].lexptr[i]);
            i++;
        }
        p_lower[i] = '\0';

        // Compare lowercased strings
        if (strcmp(s_lower, p_lower) == 0)
            return p;
    }
    return 0;
}


int insert(char s[], int tok, int type){
    // fprintf(stderr, "Insert\n");
    int len;
    len = strlen(s); 

    if(lastentry + 1 >= SYMAX)
        error("Symbol Table Full");
    if(lastchar + len + 1 >= STRMAX)
        error("Lexemes Array Full");

    lastentry = lastentry + 1;
    symtable[lastentry].token = tok;
    symtable[lastentry].lexptr = &lexemes[lastchar + 1];
    symtable[lastentry].type = type;

    lastchar = lastchar + len + 1;
    strcpy(symtable[lastentry].lexptr, s);
    return lastentry;

}

/* Init */
void init(void){ /* Loads keywords into symtable*/
    // fprintf(stderr, "Init\n");
    struct entry *p;
    for(p = keywords; p->token; p++)
        insert(p->lexptr, p->token, p->type);
}

/* Error */
void error(char *m){
    fprintf(stderr, "line %d: %s\n", lineno, m);
    exit(1); /* Unsuccessful termination */
}

void store_emit(int token, int tokenval) {
    if (emit_buffer_index < SYMAX) {
        emitBuffer[emit_buffer_index].token = token;
        emitBuffer[emit_buffer_index].tokenval = tokenval;
        emit_buffer_index++;
    } else {
        error("Emit buffer overflow");
    }
}
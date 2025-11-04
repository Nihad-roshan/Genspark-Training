%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void yyerror(const char *s);
int yylex(void);
 
%}

%union {
    int num;
    char *str;
}

%token <str> IDENTIFIER
%token <num> NUMBER
%token SELECT FROM WHERE INSERT INTO VALUES UPDATE SET EQ COMMA SEMICOLON

%type <str> column_list table_name condition

%start statement

%%

statement:
      SELECT column_list FROM table_name WHERE condition SEMICOLON
        {
            printf("\nParsed Successfully!\n");
            printf("Columns: %s\n", $2);
            printf("Table: %s\n", $4);
            printf("Condition: %s\n\n", $6);
        }
    ;

column_list:
      IDENTIFIER
        { $$ = $1; }
    | column_list COMMA IDENTIFIER
        {
            char *tmp = malloc(strlen($1) + strlen($3) + 2);
            sprintf(tmp, "%s,%s", $1, $3);
            $$ = tmp;
        }
    ;

table_name:
      IDENTIFIER
        { $$ = $1; }
    ;

condition:
      IDENTIFIER EQ NUMBER
        {
            char *tmp = malloc(strlen($1) + 20);
            sprintf(tmp, "%s=%d", $1, $3);
            $$ = tmp;
        }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    printf("Enter SQL statement:\n");
    yyparse();
    return 0;
}

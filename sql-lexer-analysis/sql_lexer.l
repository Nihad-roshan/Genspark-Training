%{
#include "sql_parser.tab.h"
#include <string.h>
%}

%%

"SELECT"        { return SELECT; }
"FROM"          { return FROM; }
"WHERE"         { return WHERE; }
"INSERT"        { return INSERT; }
"INTO"          { return INTO; }
"VALUES"        { return VALUES; }
"UPDATE"        { return UPDATE; }
"SET"           { return SET; }
"="             { return EQ; } 
";"             { return SEMICOLON; }
","             { return COMMA; }
[a-zA-Z_][a-zA-Z0-9_]*   { yylval.str = strdup(yytext); return IDENTIFIER; }
[0-9]+          { yylval.num = atoi(yytext); return NUMBER; }
[ \t\r\n]+      { /* ignore whitespace */ }
.               { printf("Unknown character: %s\n", yytext); }


%%

int yywrap(void) { return 1; }

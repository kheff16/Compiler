%option yylineno
%pointer

%{
    #include <stdlib.h>
    #include <errno.h>
    #include <limits.h>
    #include "ast.hpp"
    #include "parser.hpp"
    #include <string>
    void yyerror(const char *);
%}

/* WRITEME: Copy any definitions and start conditions from Project 3 here */
%x COMMENT
%%

  /* WRITEME: Copy your Flex rules from Project 3 here */
\/\*                        {BEGIN(COMMENT);}
<COMMENT>\n                 {;}
<COMMENT>.                  {;}
<COMMENT><<EOF>>            {yyerror("Reached EOF in comment.");}
<COMMENT>\*\/               {BEGIN(INITIAL);}




[[:space:]\n\t]+            {;}





equals                      {return T_equals;}
>=                          {return T_geq;}
->                          {return T_arrow;}
[\+\-\*\/>=\(\)\{\};\.,]    {return *yytext;}
and                         {return T_and;}
or                          {return T_or;}
not                         {return T_not;}
true                        {return T_true;}
false                       {return T_false;}
new                         {return T_new;}
if                          {return T_if;}
else                        {return T_else;}
while                       {return T_while;}
do                          {return T_do;}
print                       {return T_print;}
extends                     {return T_extends;}
return                      {return T_return;}
none                        {return T_none;}
boolean                     {return T_bool;}
integer                     {return T_integer;}
[a-zA-Z][a-zA-Z0-9]*        {std::string x(yytext); yylval.identifier_ptr = new IdentifierNode(x); return T_id;}
0|([1-9][0-9]*)             {long int value = strtol(yytext, NULL, 0); yylval.integer_ptr = new IntegerNode((int)value); return T_number;}
.                           { yyerror("invalid character"); }

%%

int yywrap(void) {
  return 1;
}

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <iostream>

    #include "ast.hpp"
    
    #define YYINITDEPTH 10000
    #define YYDEBUG 1
    int yylex(void);
    void yyerror(const char *);
    
    extern ASTNode* astRoot;
%}

%error-verbose

/* WRITEME: Copy your token and precedence specifiers from Project 3 here */
%token T_unary
%token T_equals
%token T_geq
%token T_arrow
%token T_and
%token T_or
%token T_not
%token T_true
%token T_false
%token T_new
%token T_if
%token T_else
%token T_while
%token T_do
%token T_print
%token T_extends
%token T_return
%token T_none
%token T_bool
%token T_integer
%token T_id
%token T_number

/* WRITEME: Specify precedence here */
%left T_or
%left T_and
%left '>' T_geq T_equals
%left '+' '-'
%left '*' '/'
%precedence T_not T_unary

/* WRITEME: Specify types for all nonterminals and necessary terminals here */
%type <identifier_ptr> T_id
%type <integer_ptr> T_number

%type <program_ptr> Start
%type <class_list_ptr> Classes
%type <class_ptr> Class
%type <declaration_list_ptr> Members Declarations
%type <method_list_ptr> Methods
%type <method_ptr> Method
%type <parameter_list_ptr> Parameters ParamList
%type <type_ptr> Type ReturnType
%type <methodbody_ptr> Body
%type <statement_list_ptr> Statements BlockState
%type <statement_ptr> Statement
%type <returnstatement_ptr> ReturnState
%type <parameter_ptr> Param
%type <declaration_ptr> Member Declaration
%type <identifier_list_ptr> DL
%type <expression_ptr> Expression
%type <assignment_ptr> Assignment
%type <methodcall_ptr> MethodCall
%type <ifelse_ptr> IfElse
%type <while_ptr> While
%type <dowhile_ptr> DoWhile
%type <print_ptr> Print
%type <expression_list_ptr> Arguments ArgumentsC
%%

/* WRITEME: This rule is a placeholder. Replace it with your grammar
            rules from Project 3 */
Start : Classes {astRoot = new ProgramNode($1);}
      ;

/* WRITME: Write your Bison grammar specification here */
Classes     :   Classes Class                                 {$$ = $1; $$->push_back($2);}
            |   Class                                         {$$ = new std::list<ClassNode*>(); $$->push_back($1);}
            ;
Class       :   T_id '{' Members Methods '}'                  {$$ = new ClassNode($1, NULL, $3, $4);}
            |   T_id T_extends T_id '{' Members Methods '}'   {$$ = new ClassNode($1, $3, $5, $6);}
            ;
Members     :   Members Member                                {$$ = $1; $$->push_back($2);}
            |   %empty                                        {$$ = new std::list<DeclarationNode*>();}
            ;            
Member      :   Type T_id ';'                                 {std::list<IdentifierNode*>* list = new std::list<IdentifierNode*>(); list->push_back($2); $$ = new DeclarationNode($1, list);}
            ;
Methods     :   Method Methods                                {$$ = $2; $$->push_front($1);}
            |   %empty                                        {$$ = new std::list<MethodNode*>();}
            ;            
Method      :   T_id '(' Parameters ')' T_arrow ReturnType '{' Body '}'  {$$ = new MethodNode($1, $3, $6, $8);}
            ;  
Body        :   Declarations Statements ReturnState           {$$ = new MethodBodyNode($1, $2, $3);}
            ;                                 
Parameters  :   ParamList                                     {$$ = $1;}
            |   %empty                                        {$$ = new std::list<ParameterNode*>();}
            ;
ParamList   :   ParamList ',' Param                           {$$ = $1; $$->push_back($3);}
            |   Param                                         {$$ = new std::list<ParameterNode*>(); $$->push_back($1);}
            ;
Param       :   Type T_id                                     {$$ = new ParameterNode($1, $2);}
            ;
ReturnType  :   T_integer                                     {$$ = new IntegerTypeNode();}
            |   T_bool                                        {$$ = new BooleanTypeNode();}
            |   T_id                                          {$$ = new ObjectTypeNode($1);}
            |   T_none                                        {$$ = new NoneNode();} 
            ;
Declarations:   Declarations Declaration                      {$$ = $1; $$->push_back($2);}
            |   %empty                                        {$$ = new std::list<DeclarationNode*>();}
            ;
Declaration :   Type DL                                       {$$ = new DeclarationNode($1, $2);}
            ;
DL          :   T_id ',' DL                                   {$$ = $3; $$->push_front($1);}
            |   T_id ';'                                      {$$ = new std::list<IdentifierNode*>(); $$->push_front($1);}
            ;
Statements  :	Statement Statements                            {$$ = $2; $$->push_front($1);}
	          |	%empty                                          {$$ = new std::list<StatementNode*>();}
	          ;
Statement   :   Assignment                                    {$$ = $1;}
            |   MethodCall ';'                                {$$ = new CallNode($1);}
            |   IfElse                                        {$$ = $1;}
            |   While                                         {$$ = $1;}
            |   DoWhile                                       {$$ = $1;}
            |   Print                                         {$$ = $1;}
            ;
Assignment  :   T_id '=' Expression ';'                       {$$ = new AssignmentNode($1, NULL, $3);}
            |   T_id '.' T_id '=' Expression ';'              {$$ = new AssignmentNode($1, $3, $5);}
            ;
IfElse      :   T_if Expression '{' BlockState '}'            {$$ = new IfElseNode($2, $4, NULL);}
            |   T_if Expression '{' BlockState '}' T_else '{' BlockState '}'  {$$ = new IfElseNode($2, $4, $8);}
            ;
While       :   T_while Expression '{' BlockState '}'         {$$ = new WhileNode($2, $4);}
            ;
DoWhile     :   T_do '{' BlockState '}' T_while '(' Expression ')' ';'  {$$ = new DoWhileNode($3, $7);}
            ;
Print       :   T_print Expression ';'        {$$ = new PrintNode($2);}
            ;
BlockState  :   BlockState Statement          {$$ = $1; $$->push_back($2);}
            |   Statement                     {$$ = new std::list<StatementNode*>(); $$->push_back($1);}
            ;            
ReturnState :   T_return Expression ';'       {$$ = new ReturnStatementNode($2);}
            |   %empty                        {$$ = NULL;}
            ;
Expression  :   Expression '+' Expression			{$$ = new PlusNode($1, $3);}
            |   Expression '-' Expression     {$$ = new MinusNode($1, $3);}
            |   Expression '*' Expression     {$$ = new TimesNode($1, $3);}
            |   Expression '/' Expression     {$$ = new DivideNode($1, $3);}
            |   Expression '>' Expression     {$$ = new GreaterNode($1, $3);}
            |   Expression T_geq Expression   {$$ = new GreaterEqualNode($1, $3);}
            |   Expression T_equals Expression{$$ = new EqualNode($1, $3);}
            |   Expression T_and Expression   {$$ = new AndNode($1, $3);}
            |   Expression T_or Expression    {$$ = new OrNode($1, $3);}
            |   T_not Expression              {$$ = new NotNode($2);}
            |   '-' Expression %prec T_unary  {$$ = new NegationNode($2);}
            |   T_id                          {$$ = new VariableNode($1);}
            |   T_id '.' T_id                 {$$ = new MemberAccessNode($1, $3);}
            |   MethodCall                    {$$ = $1;}
            |   '(' Expression ')'            {$$ = $2;}
            |   T_number                      {$$ = new IntegerLiteralNode($1);}
            |   T_true                        {$$ = new BooleanLiteralNode(new IntegerNode(1));} 
            |   T_false                       {$$ = new BooleanLiteralNode(new IntegerNode(0));}
            |   T_new T_id                    {$$ = new NewNode($2, NULL);}
            |   T_new T_id '(' Arguments ')'  {$$ = new NewNode($2, $4);}
            ;
MethodCall  :   T_id '(' Arguments ')'        {$$ = new MethodCallNode($1, NULL, $3);}
            |   T_id '.' T_id '(' Arguments ')' {$$ = new MethodCallNode($1, $3, $5);}
            ;
Type        :   T_integer                     {$$ = new IntegerTypeNode();}
            |   T_bool                        {$$ = new BooleanTypeNode();}
            |   T_id                          {$$ = new ObjectTypeNode($1);}
            ;            
Arguments   :   ArgumentsC                    {$$ = $1;}
            |   %empty                        {$$ = new std::list<ExpressionNode*>();}
            ;
ArgumentsC  :   ArgumentsC ',' Expression     {$$ = $1; $$->push_back($3);}
            |   Expression                    {$$ = new std::list<ExpressionNode*>(); $$->push_back($1);}
            ;
%%

extern int yylineno;

void yyerror(const char *s) {
  fprintf(stderr, "%s at line %d\n", s, yylineno);
  exit(0);
}

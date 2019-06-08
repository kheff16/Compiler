#include "codegeneration.hpp"

// CodeGenerator Visitor Functions: These are the functions
// you will complete to generate the x86 assembly code. Not
// all functions must have code, many may be left empty.


// Start of helper functions
int classSize(CodeGenerator *cg, ClassInfo classInfo) {
    if (classInfo.superClassName == "") {
        return classInfo.membersSize;
    }
    return classInfo.membersSize + classSize(cg, cg->classTable->at(classInfo.superClassName));
}

void p(std::string a){
    std::cout << a << std::endl;
}

// End of helper functions

void CodeGenerator::visitProgramNode(ProgramNode* node) {
    // WRITEME: Replace with code if necessary
    p(".data");
    p("printstr: .asciz \"%d\\n\"")
    p("");
    p(".text");
    p(".globl Main_main");
    node->visit_children(this);
    p("");
}

void CodeGenerator::visitClassNode(ClassNode* node) {
    // WRITEME: Replace with code if necessary
    currentClassName = node->identifier_1->name;
    currentClassInfo = classTable->at(currentClassName);
    node->visit_children(this);
}

void CodeGenerator::visitMethodNode(MethodNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitMethodBodyNode(MethodBodyNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitParameterNode(ParameterNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitDeclarationNode(DeclarationNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitReturnStatementNode(ReturnStatementNode* node) {
    // WRITEME: Replace with code if necessary

    //Execute/visit the expression
    node->expression->accept(this);
    p("#### RETURN STATEMENT");
    //Take result of last expression from top of stack and place into %eax
    p(" pop %eax");
    

    // %eax will be used to return values from functions.
    // Sooooo that's it!!!!
    p("#### END OF RETURN STATEMENT");
}

void CodeGenerator::visitAssignmentNode(AssignmentNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitCallNode(CallNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitIfElseNode(IfElseNode* node) {
    // WRITEME: Replace with code if necessary
    auto elseLabel = nextLabel();
    auto endLabel = nextLabel();
    std::cout << "#### IF ELSE" << std::endl;
    node->expression->accept(this);
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   mov  $0, %ebx" << std::endl;
    std::cout << "   cmp  %eax, %ebx" << std::endl;
    std::cout << "   je " << elseLabel << std::endl;
    // Go through the true statements
    if(node->statement_list_1) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list_1->begin();
            iter != node->statement_list_1->end(); iter++) {
            (*iter)->accept(this);
        }
    }
    std::cout << "   jmp " << endLabel << std::endl;
    std::cout << elseLabel << ":" << std::endl;
    // Go through the false statements
    if(node->statement_list_2) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list_2->begin();
            iter != node->statement_list_2->end(); iter++) {
            (*iter)->accept(this);
        }    
    }
    std::cout << endLabel << ":" << std::endl;
    p("#### END OF IF ELSE");
}

void CodeGenerator::visitWhileNode(WhileNode* node) {
    // WRITEME: Replace with code if necessary
    auto startLoopLabel = nextLabel();
    auto endLabel = nextLabel();
    std::cout << "#### WHILE" << std::endl;
    node->expression->accept(this);
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   mov  $0, %ebx" << std::endl;
    std::cout << "   cmp  %eax, %ebx" << std::endl;
    std::cout << "   je " << endLabel << std::endl;
    std::cout << startLoopLabel<< ":" << std::endl;
    for(std::list<StatementNode*>::iterator iter = node->statement_list->begin();
        iter != node->statement_list->end(); iter++) {
        (*iter)->accept(this);
    }
    node->expression->accept(this);
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   mov  $1, %ebx" << std::endl;
    std::cout << "   cmp  %eax, %ebx" << std::endl;
    std::cout << "   je " << startLoopLabel << std::endl;
    std::cout << endLabel << ":" << std::endl;

}

void CodeGenerator::visitPrintNode(PrintNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### PRINT" << std::endl;
    node->visit_children(this);
    std::cout << "   push $printstr" << std::endl;
    std::cout << "   call printf" << std::endl;
    std::cout << "   add  $8, %esp" << std::endl;

    std::cout << "#### END OF PRINT" << std::endl;
}

void CodeGenerator::visitDoWhileNode(DoWhileNode* node) {
    // WRITEME: Replace with code if necessary
    auto whileTrueLabel = nextLabel();
    auto endLabel = nextLabel();
    std::cout << "#### DO-WHILE" << std::endl;
    std::cout << whileTrueLabel<< ":" << std::endl;
    for(std::list<StatementNode*>::iterator iter = node->statement_list->begin();
        iter != node->statement_list->end(); iter++) {
        (*iter)->accept(this);
    }
    node->expression->accept(this);
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   mov  $0, %ebx" << std::endl;
    std::cout << "   cmp  %eax, %ebx" << std::endl;
    std::cout << "   jne " << whileTrueLabel << std::endl;
    std::cout << endLabel << ":" << std::endl;
}

void CodeGenerator::visitPlusNode(PlusNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### ADD" << std::endl;
    node->visit_children(this);
    std::cout << "   pop  %ebx" << std::endl;
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   add  %ebx, %eax" << std::endl;
    std::cout << "   push %eax" << std::endl;

    std::cout << "#### END OF ADD" << std::endl;
}

void CodeGenerator::visitMinusNode(MinusNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### MINUS" << std::endl;
    node->visit_children(this);
    std::cout << "   pop  %ebx" << std::endl;
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   sub  %ebx, %eax" << std::endl;
    std::cout << "   push %eax" << std::endl;

    std::cout << "#### END OF MINUS" << std::endl;
}

void CodeGenerator::visitTimesNode(TimesNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### MULTIPLY" << std::endl;
    node->visit_children(this);
    std::cout << "   pop  %ebx" << std::endl;
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   imul %ebx, %eax" << std::endl;
    std::cout << "   push %eax" << std::endl;

    std::cout << "#### END OF MULTIPLY" << std::endl;
}

void CodeGenerator::visitDivideNode(DivideNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### DIVIDE" << std::endl;
    node->visit_children(this);
    std::cout << "   pop  %ebx" << std::endl;
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   cdq" << std::endl;
    std::cout << "   idiv %ebx" << std::endl;
    std::cout << "   push %eax" << std::endl;

    std::cout << "#### END OF DIVIDE" << std::endl;
}

void CodeGenerator::visitGreaterNode(GreaterNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### GREATER THAN" << std::endl;
    node->visit_children(this);
    auto trueLabel = nextLabel();
    auto endLabel = nextLabel();
    std::cout << "   pop  %ebx" << std::endl;
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   cmp  %ebx, %eax" << std::endl;
    std::cout << "   jg   " << trueLabel << std::endl;
    std::cout << "   push $0" << std::endl;
    std::cout << "   jmp  " << endLabel << std::endl;
    std::cout << trueLabel << ":" << std::endl;
    std::cout << "   push $1" << std::endl;
    std::cout << endLabel << ":" << std::endl;

    std::cout << "#### END OF GREATER THAN" << std::endl;
}

void CodeGenerator::visitGreaterEqualNode(GreaterEqualNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### GREATER THAN OR EQUAL" << std::endl;
    node->visit_children(this);
    auto trueLabel = nextLabel();
    auto endLabel = nextLabel();
    std::cout << "   pop  %ebx" << std::endl;
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   cmp  %ebx, %eax" << std::endl;
    std::cout << "   jge   " << trueLabel << std::endl;
    std::cout << "   push $0" << std::endl;
    std::cout << "   jmp  " << endLabel << std::endl;
    std::cout << trueLabel << ":" << std::endl;
    std::cout << "   push $1" << std::endl;
    std::cout << endLabel << ":" << std::endl;

    std::cout << "#### END OF GREATER THAN OR EQUAL" << std::endl;
}

void CodeGenerator::visitEqualNode(EqualNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### EQUAL" << std::endl;
    node->visit_children(this);
    auto trueLabel = nextLabel();
    auto endLabel = nextLabel();
    std::cout << "   pop  %ebx" << std::endl;
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   cmp  %eax, %ebx" << std::endl;
    std::cout << "   je   " << trueLabel << std::endl;
    std::cout << "   push $0" << std::endl;
    std::cout << "   jmp  " << endLabel << std::endl;
    std::cout << trueLabel << ":" << std::endl;
    std::cout << "   push $1" << std::endl;
    std::cout << endLabel << ":" << std::endl;
    
    std::cout << "#### END OF EQUAL" << std::endl;
}

void CodeGenerator::visitAndNode(AndNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### AND OPERATOR" << std::endl;
    node->visit_children(this);
    std::cout << "   pop  %ebx" << std::endl;
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   andl %ebx, %eax" << std::endl;
    std::cout << "   push %eax" << std::endl;
    std::cout << "#### END OF AND" << std::endl;
}

void CodeGenerator::visitOrNode(OrNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### OR OPERATOR" << std::endl;
    node->visit_children(this);
    std::cout << "   pop  %ebx" << std::endl;
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   orl  %ebx, %eax" << std::endl;
    std::cout << "   push %eax" << std::endl;
    std::cout << "#### END OF OR" << std::endl;
}

void CodeGenerator::visitNotNode(NotNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### NOT OPERATOR" << std::endl;
    node->visit_children(this);
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   xor  $1, %eax" << std::endl;
    std::cout << "   push %eax" << std::endl;
    std::cout << "#### END OF NOT" << std::endl;
}

void CodeGenerator::visitNegationNode(NegationNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### NEGATION OPERATOR" << std::endl;
    node->visit_children(this);
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   neg  %eax" << std::endl;
    std::cout << "   push %eax" << std::endl;
    std::cout << "#### END OF NEGATION" << std::endl;
}

void CodeGenerator::visitMethodCallNode(MethodCallNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitMemberAccessNode(MemberAccessNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitVariableNode(VariableNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitIntegerLiteralNode(IntegerLiteralNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitBooleanLiteralNode(BooleanLiteralNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitNewNode(NewNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitIntegerTypeNode(IntegerTypeNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitBooleanTypeNode(BooleanTypeNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitObjectTypeNode(ObjectTypeNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitNoneNode(NoneNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitIdentifierNode(IdentifierNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitIntegerNode(IntegerNode* node) {
    // WRITEME: Replace with code if necessary
}

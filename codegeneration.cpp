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

bool localVar(CodeGenerator *cg, std::string id) {
    return cg->currentMethodInfo.variables->find(id) != cg->currentMethodInfo.variables->end();
}

VariableInfo getMemberVariableInfo(CodeGenerator *cg, std::string id, std::string className) {
    do {
        auto *members = cg->classTable->at(className).members;
        if (members->find(id) != members->end()) {
            return members->at(id);
        }
    } while ((className = cg->classTable->at(className).superClassName) != "");
    
    std::cout << "#### MEMBER NOT DEFINED: " << id << std::endl;
}

VariableInfo getLocalVariableInfo(CodeGenerator *cg, std::string id) {
    // Variable exists as parameter or local
    if (cg->currentMethodInfo.variables->find(id) != cg->currentMethodInfo.variables->end()) {
        return cg->currentMethodInfo.variables->at(id);
    }
     // Variable exists as member of class or superclass 
    else {
        return getMemberVariableInfo(cg, id, cg->currentClassName);
    }
}


int getMemberOffset(CodeGenerator *cg, std::string member, std::string className) {
    auto classInfo = cg->classTable->at(className);
    // Get the class this member was inherited from otherwise the offset will be wrong
    while (classInfo.members->find(member) == classInfo.members->end()) {
        className = classInfo.superClassName;
        classInfo = cg->classTable->at(classInfo.superClassName);
    }
    
    // Offset the offset by the combined size of all the superclasses (The symbol table doesn't do this automatically)
    int superClassOffset;
    if(classInfo.superClassName != "") {
      superClassOffset = classSize(cg, cg->classTable->at(classInfo.superClassName));  
    }
    else {
        superClassOffset = 0;
    } 
    VariableInfo memberInfo = getMemberVariableInfo(cg, member, className);
    return memberInfo.offset + superClassOffset;
}

std::string getLabel() {
    static int num = 0;
    std::string label = "Label" + std::to_string(num);
    num++;
    return label;
}

// End of helper functions

void CodeGenerator::visitProgramNode(ProgramNode* node) {
    // WRITEME: Replace with code if necessary
    p(".data");
    p("printstr: .asciz \"%d\\n\"");
    p("");
    p(".text");
    p(".globl Main_main");
    node->visit_children(this);
    p("");
    p("#### REACHED END OF PROGRAM");
}

void CodeGenerator::visitClassNode(ClassNode* node) {
    // WRITEME: Replace with code if necessary
    currentClassName = node->identifier_1->name;
    currentClassInfo = classTable->at(currentClassName);
    node->visit_children(this);
}

void CodeGenerator::visitMethodNode(MethodNode* node) {
    // WRITEME: Replace with code if necessary
    currentMethodName = node->identifier->name;
    currentMethodInfo = currentClassInfo.methods->at(currentMethodName);

    //  Create a label (class name + “_” + method name)
    std::string label = currentClassName + "_" + currentMethodName;
    p("#### METHOD START");

    p(label + ": ");
	// push old ebp

	p("   push %ebp");

    // Set new %ebp(to current %esp) 
    p("   mov %esp, %ebp"); // ebp gets value of esp

    // Allocate space for local variables //Subtract from stack pointer //Look into localsSize of MethodInfo
	p("   sub $" + std::to_string(currentMethodInfo.localsSize) + ", %esp");

    //Save callee-save registers (%ebx, %edi, %esi)
    	p("   push %ebx");
    	p("	  push %edi");
    	p("	  push %esi");

	node->methodbody->accept(this);

	//Restore callee-saved registers
	    p("	  pop %esi");
    	p("	  pop %edi");
    	p("	  pop %ebx");

	// Deallocate local varsspace by moving %esp to %ebp
	p("   mov %ebp, %esp"); // esp gets value of ebp

	//Restore old base pointer by popping old %ebp from the stack
	p("	  pop %ebp");

	//Return using return address (ret instruction)
	p("	  ret");

    p("#### METHOD END");
}

void CodeGenerator::visitMethodBodyNode(MethodBodyNode* node) {
    // WRITEME: Replace with code if necessary

    //Don’t need to do much; just executes the statements in your function!
    // if(node->declaration_list) {
    //     for(std::list<DeclarationNode*>::iterator iter = node->declaration_list->begin();
    //         iter != node->declaration_list->end(); iter++) {
    //         (*iter)->accept(this);
    //     }
    // }

    if(node->statement_list) {
        for(std::list<StatementNode*>::iterator iter = node->statement_list->begin();
            iter != node->statement_list->end(); iter++) {
            (*iter)->accept(this);
        }
    }

    if(node->returnstatement){
        node->returnstatement->accept(this);
    }
}

void CodeGenerator::visitParameterNode(ParameterNode* node) {
}

void CodeGenerator::visitDeclarationNode(DeclarationNode* node) {
}

void CodeGenerator::visitReturnStatementNode(ReturnStatementNode* node) {
    // WRITEME: Replace with code if necessary

    //Execute/visit the expression
    node->expression->accept(this);
    p("#### RETURN STATEMENT");
    //Take result of last expression from top of stack and place into %eax
    p("   pop %eax");
    

    // %eax will be used to return values from functions.
    // Sooooo that's it!!!!
    p("#### END OF RETURN STATEMENT");
}

void CodeGenerator::visitAssignmentNode(AssignmentNode* node) {
    // WRITEME: Replace with code if necessary
    node->visit_children(this);
    if(node->identifier_2) {
        std::string className = node->identifier_1->name;
        std::string memberName = node->identifier_2->name; 
        p("#### ASSIGNING TO " + memberName + " WHICH BELONGS TO " + className);
        const VariableInfo classInfo = getLocalVariableInfo(this, className);
        if(localVar(this, className)) {
            const int memberOffset = getMemberOffset(this, memberName, classInfo.type.objectClassName);
            std::cout << "   movl " << classInfo.offset << "(%ebp), %eax" << std::endl; // Load object address into accumulator
            std::cout << "   pop  %ebx" << std::endl;
            std::cout << "   movl %ebx, " << memberOffset << "(%eax)" << std::endl; // Store value in the member
        }
        else {
            int classOffset = getMemberOffset(this, className, currentClassName);
            int memberOffset = getMemberOffset(this, memberName, classInfo.type.objectClassName);
            std::cout << "   movl " << "8(%ebp), %eax" << std::endl; // Load self address into accumulator
            std::cout << "   movl " << classOffset << "(%eax), %eax" << std::endl; // Load object address into accumulator
            std::cout << "   pop  %ebx" << std::endl;;
            std::cout << "   movl %ebx, " << memberOffset << "(%eax)" << std::endl; // Store value in the member
        }
    }
    // Parameter, local method, or self
    else {
        std::string varName = node->identifier_1->name;
        p("#### ASSIGNING TO " + varName);
        if(localVar(this, varName)) {
            VariableInfo v = getLocalVariableInfo(this, varName);
            std::cout << "   pop  %eax" << std::endl;
            std::cout << "   movl %eax, " << v.offset << "(%ebp)" << std::endl;
        }
        else {
            const int memberOffset = getMemberOffset(this, varName, currentClassName);
            std::cout << "   movl " << "8(%ebp), %eax" << std::endl; // Load self address into accumulator
            std::cout << "   pop  %ebx" << std::endl;
            std::cout << "   movl %ebx, " << memberOffset << "(%eax)" << std::endl; // Store value in the member
        }
    }

    p("#### END OF ASSIGNMENT");
}

void CodeGenerator::visitCallNode(CallNode* node) {
    // WRITEME: Replace with code if necessary
    node->visit_children(this);
    p("#### IN CALL NODE");
    //std::cout << "   add $4, %esp" << std::endl; // return value not used (child node pushes return value on stack)
    p("#### END CALL NODE");
}

void CodeGenerator::visitIfElseNode(IfElseNode* node) {
    // WRITEME: Replace with code if necessary

    std::cout << "#### IF ELSE" << std::endl;
    node->expression->accept(this);
    std::cout << "   pop  %eax" << std::endl;
    std::cout << "   mov  $0, %ebx" << std::endl;
    std::cout << "   cmp  %eax, %ebx" << std::endl;
    auto elseLabel = getLabel();
    auto endLabel = getLabel();
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
    auto startLoopLabel = getLabel();
    auto endLabel = getLabel();
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

    p("#### END OF WHILE");

}

void CodeGenerator::visitPrintNode(PrintNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### PRINT" << std::endl;
    node->visit_children(this);
    std::cout << "   push $printstr" << std::endl;
    std::cout << "   call printf" << std::endl; 

    // PRETTY SURE printf will pop off call from stack

    std::cout << "   add $8, %esp" << std::endl;

    std::cout << "#### END OF PRINT" << std::endl;
}

void CodeGenerator::visitDoWhileNode(DoWhileNode* node) {
    // WRITEME: Replace with code if necessary
    auto whileTrueLabel = getLabel();
    auto endLabel = getLabel();
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

    std::cout << "#### END OF DO WHILE" << std::endl;
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
    auto trueLabel = getLabel();
    auto endLabel = getLabel();
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
    auto trueLabel = getLabel();
    auto endLabel = getLabel();
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
    auto trueLabel = getLabel();
    auto endLabel = getLabel();
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
    std::cout << "#### METHOD CALL NODE" << std::endl;
    // Save registers
    std::cout << "   push %eax" << std::endl;
    std::cout << "   push %ecx" << std::endl;
    std::cout << "   push %edx" << std::endl;
    
    // Load arguments __cdecl
    if (node->expression_list) {
        for (auto argument = node->expression_list->rbegin(); argument != node->expression_list->rend(); ++argument) {
            (*argument)->accept(this);
        }
    }

    //If class method then push class self reference to stack
    if(node->identifier_2) {
        std::string className = node->identifier_1->name;
        VariableInfo classInfo = getLocalVariableInfo(this, className);
        if(localVar(this, className)) {
            std::cout << "   movl " << classInfo.offset << "(%ebp), %eax" << std::endl;
            std::cout << "   push %eax" << std::endl;
        }
        //Object is in self
        else {
            int classOffset = getMemberOffset(this, className, currentClassName);
            std::cout << "   movl 8(%ebp), %eax" << std::endl; // Load self
            std::cout << "   movl " << classOffset << "(%eax), %eax" << std::endl; // Load object
            std::cout << "   push %eax" << std::endl; // Push object as self for next stackframe
        }
    }
    // Solo Case c0(args)
    else {
        if(currentClassName != "Main") {
            if (currentClassName != "Main") {
                std::cout << "   movl 8(%ebp), %eax" << std::endl;
                std::cout << "   push %eax" << std::endl;
            } 
            else {
                std::cout << "   add $-4, %esp" << std::endl; // Empty space for self to work with offsets
            }
        }
    }

    // This determines the method label (Which could be in a superClass)
    std::string className, methodName;
    ClassInfo classInfo;
    if(node->identifier_2) {
        className = getLocalVariableInfo(this, node->identifier_1->name).type.objectClassName ;
    } 
    else {
        className = currentClassName;
    }

    if(node->identifier_2) {
        classInfo = classTable->at(getLocalVariableInfo(this, node->identifier_1->name).type.objectClassName);
    } 
    else {
        classInfo = currentClassInfo;
    }

    if(node->identifier_2) {
        methodName = node->identifier_2->name;
    } 
    else {
        methodName = node->identifier_1->name;
    }
    while (classInfo.methods->find(methodName) == classInfo.methods->end()) { // While you haven't found a class containing the method
        className = classInfo.superClassName;
        classInfo = classTable->at(classInfo.superClassName);
    }

    // Call the function (Pushes return address to stack then jumps to label)
    std::cout << "   call " << className << "_" << methodName << std::endl;
    
    // Store return value in ebx because we're going to restore eax
    std::cout << "   movl %eax, %ebx" << std::endl;
    
    // Clear arguments and self from stack
    std::cout << "   add $" << 4 * (node->expression_list->size() + 1) << ", %esp" << std::endl;
    
    // Restore registers
    std::cout << "   pop  %edx" << std::endl;
    std::cout << "   pop  %ecx" << std::endl;
    std::cout << "   pop  %eax" << std::endl;
    
    // Put return value on the stack
    std::cout << "   push %ebx" << std::endl;

    std::cout << "#### END OF METHOD CALL" << std::endl;

}

void CodeGenerator::visitMemberAccessNode(MemberAccessNode* node) {
    // WRITEME: Replace with code if necessary
    std::string className = node->identifier_1->name;
    std::string varName = node->identifier_2->name;
    std::cout << "#### MEMBER ACCESS : " << className << "." << varName << std::endl;

    const VariableInfo classInfo = getLocalVariableInfo(this, className);
    // Class is a local var or parameter
    if(localVar(this, className)) {
        const int memberOffset = getMemberOffset(this, varName, classInfo.type.objectClassName);
        // Load object address into accumulator
        std::cout << "   movl " << classInfo.offset << "(%ebp), %eax" << std::endl; 
        // Load object member into accumulator
        std::cout << "   movl " << memberOffset << "(%eax), %eax" << std::endl; 
        std::cout << "   push %eax" << std::endl;
    }
    // Implicitly in self (Kind of confusing because you need to access the object from self then the member from that object)
    else {
        int classOffset = getMemberOffset(this, className, currentClassName);
        int memberOffset = getMemberOffset(this, varName, classInfo.type.objectClassName);
        // Load self address into accumulator
        std::cout << "   movl " << "8(%ebp), %eax" << std::endl; 
        // Load object address into accumulator
        std::cout << "   movl " << classOffset << "(%eax), %eax" << std::endl; 
        // Load member into accumulator
        std::cout << "   movl " << memberOffset << "(%eax), %eax" << std::endl; 
        std::cout << "   push %eax" << std::endl;
    }

    p("#### END OF MEMBER ACCESS");
}

void CodeGenerator::visitVariableNode(VariableNode* node) {
    // WRITEME: Replace with code if necessary
    std::string varName = node->identifier->name;
    std::string className = this->currentClassName;
    p("#### VARIABLE NODE: " + varName);
    VariableInfo v;
    // Should be a parameter or local var
    if(localVar(this, varName)) {
        VariableInfo v = getLocalVariableInfo(this, varName);
        std::cout << "   movl " << v.offset << "(%ebp), %eax" << std::endl;
        std::cout << "   push %eax" << std::endl;
    }
    // Class Member Variable
    else {
        int memberOffset = getMemberOffset(this, varName, currentClassName);
        // Load self address into accumulator
        std::cout << "   movl " << "8(%ebp), %eax" << std::endl; 
        // Load object member into accumulator
        std::cout << "   movl " << memberOffset << "(%eax), %eax" << std::endl; 
        std::cout << "   push %eax" << std::endl;
    }

    p("#### END OF VARIABLE NODE");
}

void CodeGenerator::visitIntegerLiteralNode(IntegerLiteralNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### INT LIT" << std::endl;
    std::cout << "   push " << '$' << node->integer->value << std::endl;
    std::cout << "#### END INT LIT" << std::endl;
}

void CodeGenerator::visitBooleanLiteralNode(BooleanLiteralNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### BOOL LIT" << std::endl;
    std::cout << "    push " << '$' << node->integer->value << std::endl;
    std::cout << "#### END OF BOOL LIT" << std::endl;
}

void CodeGenerator::visitNewNode(NewNode* node) {
    // WRITEME: Replace with code if necessary
    std::cout << "#### NEW OPERATOR" << std::endl;
    auto classInfo = classTable->at(node->identifier->name);
    std::cout << "   push $" << classSize(this, classInfo) << std::endl;
    std::cout << "   call malloc" << std::endl;
    std::cout << "   add  $4, %esp" << std::endl;
    std::cout << "   push %eax" << std::endl;
    
    if (classInfo.methods->find(node->identifier->name) != classInfo.methods->end()) {
        std::cout << "#### CALLING CONSTRUCTOR" << std::endl;
        // Save registers
        std::cout << "   push %eax" << std::endl;
        std::cout << "   push %ecx" << std::endl;
        std::cout << "   push %edx" << std::endl;
        
        // Load arguments in C_decl (Can't call visit children, arguments will be in reverse order)
        if (node->expression_list) {
            for (auto argument = node->expression_list->rbegin(); argument != node->expression_list->rend(); ++argument) {
                (*argument)->accept(this);
            }
        }
        
        // Push self
        std::cout << "   movl " << 4 * (node->expression_list->size()+3) << "(%esp), %eax" << std::endl;
        std::cout << "   push %eax" << std::endl;
        
        // Call constructor
        std::cout << "   call " << node->identifier->name << "_" << node->identifier->name << std::endl;
        
        // Clear arguments and self from stack
        std::cout << "   add $" << 4 * (node->expression_list->size() + 1) << ", %esp" << std::endl;
        
        // Restore registers
        std::cout << "   pop  %edx" << std::endl;
        std::cout << "   pop  %ecx" << std::endl;
        std::cout << "   pop  %eax" << std::endl;
        
    }

    std::cout << "#### END OF NEW" << std::endl;
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

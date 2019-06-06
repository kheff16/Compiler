#include "typecheck.hpp"

// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
  switch (code) {
    case undefined_variable:
      std::cerr << "Undefined variable." << std::endl;
      break;
    case undefined_method:
      std::cerr << "Method does not exist." << std::endl;
      break;
    case undefined_class:
      std::cerr << "Class does not exist." << std::endl;
      break;
    case undefined_member:
      std::cerr << "Class member does not exist." << std::endl;
      break;
    case not_object:
      std::cerr << "Variable is not an object." << std::endl;
      break;
    case expression_type_mismatch:
      std::cerr << "Expression types do not match." << std::endl;
      break;
    case argument_number_mismatch:
      std::cerr << "Method called with incorrect number of arguments." << std::endl;
      break;
    case argument_type_mismatch:
      std::cerr << "Method called with argument of incorrect type." << std::endl;
      break;
    case while_predicate_type_mismatch:
      std::cerr << "Predicate of while loop is not boolean." << std::endl;
      break;
    case do_while_predicate_type_mismatch:
      std::cerr << "Predicate of do while loop is not boolean." << std::endl;
      break;
    case if_predicate_type_mismatch:
      std::cerr << "Predicate of if statement is not boolean." << std::endl;
      break;
    case assignment_type_mismatch:
      std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
      break;
    case return_type_mismatch:
      std::cerr << "Return statement type does not match declared return type." << std::endl;
      break;
    case constructor_returns_type:
      std::cerr << "Class constructor returns a value." << std::endl;
      break;
    case no_main_class:
      std::cerr << "The \"Main\" class was not found." << std::endl;
      break;
    case main_class_members_present:
      std::cerr << "The \"Main\" class has members." << std::endl;
      break;
    case no_main_method:
      std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
      break;
    case main_method_incorrect_signature:
      std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
      break;
  }
  exit(1);
}

// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.

void TypeCheck::visitProgramNode(ProgramNode* node) {
  // WRITEME: Replace with code if necessary
  classTable = new ClassTable();
  node->visit_children(this);

  if(classTable->find("Main") == classTable->end()) {
    typeError(no_main_class);
  }
  //This means a Main class must exist, thus
  auto mainClass = classTable->at("Main");

  if(mainClass.members->size() != 0) {
    typeError(main_class_members_present);
  }
  if (mainClass.methods->find("main") == mainClass.methods->end()) {
    typeError(no_main_method);
  }
  auto mainMethod = mainClass.methods->at("main");
  if (mainMethod.parameters->size() != 0 || mainMethod.returnType.baseType != bt_none) {
    typeError(main_method_incorrect_signature);
  }
  
}

void TypeCheck::visitClassNode(ClassNode* node) {
  // WRITEME: Replace with code if necessary
  // Checking if super class is valid, then it must be in table
  if(node->identifier_2) {
    if(classTable->find(node->identifier_2->name) == classTable->end()) {
      typeError(undefined_class);
    }
  }
  currentClassName = node->identifier_1->name;
  ClassInfo c;
  c.methods = new MethodTable();
  c.members = new VariableTable();
  
  currentMethodTable = c.methods;
  currentVariableTable = c.members;

  currentMemberOffset = -4;
  currentLocalOffset = -1;

  if(node->identifier_2 == NULL) {
    c.superClassName = "";
  }
  else{
    c.superClassName = node->identifier_2->name;
  }


  c.membersSize = -(currentMemberOffset) * (c.members->size());
  classTable->insert(std::pair<std::string, ClassInfo> (currentClassName, c));

  node->visit_children(this); // Visits Declaration Node then Method Node
  
  /*
    Set all the tables
    iterate though decl list and call visitDeclNode
    set offset
     ''' '' '' ' ' ' ' 'memberlist
  */

}

void TypeCheck::visitMethodNode(MethodNode* node) {
  // WRITEME: Replace with code if necessary
  MethodInfo m;
  CompoundType c;
  VariableInfo *v = new VariableInfo();
  CompoundType a;

  m.variables = new VariableTable();
  currentVariableTable = m.variables;
  currentLocalOffset = 0;
  currentParameterOffset = 8;
  m.parameters = new std::list<CompoundType> ();

  // Here we insert the method parameters
  
  for(std::list<ParameterNode*>::iterator iter = node->parameter_list->begin(); 
  iter != node->parameter_list->end(); ++iter){
    
    (*iter)->visit_children(this);

    (*iter)->basetype = (*iter)->type->basetype;
    (*iter)->objectClassName = (*iter)->type->objectClassName;

    a.baseType = (*iter)-> basetype;
    a.objectClassName = (*iter)->objectClassName;

    v->type = a;
    v->size = 4;
    v->offset = currentParameterOffset + v->size;
    currentParameterOffset += v->size;
    m.parameters->push_back(a);

    currentVariableTable->insert(std::pair<std::string, VariableInfo> ((*iter)->identifier->name, *v));
  }

  node->visit_children(this);
  

  node->basetype = node->type->basetype;

  //TODO: Is setting objectClassName right when we don't know the exact basetype?
  c.baseType = node->basetype;
  c.objectClassName = node->objectClassName;
  
  m.returnType = c;

  m.localsSize = -(currentLocalOffset);

  

  ReturnStatementNode* returnState = node->methodbody->returnstatement;
  BaseType nodeType = node->type->basetype;

  if(!returnState && nodeType != bt_none) {
    typeError(return_type_mismatch);
  }
  if(nodeType == bt_none && returnState) {
    typeError(return_type_mismatch);
  }
  if(returnState && nodeType != returnState->basetype && nodeType != bt_none) {
    typeError(return_type_mismatch);
  }
  if(returnState && nodeType == bt_object && node->type->objectClassName != returnState->objectClassName) {
    typeError(return_type_mismatch);
  }
  if(node->identifier->name == this->currentClassName && nodeType != bt_none) {
    typeError(constructor_returns_type);
  }

  currentMethodTable->insert(std::pair<std::string, MethodInfo> (node->identifier->name, m));
  // LETS DO THIS IN PARAMETER NODE ... DONE!
  
}

void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
}

void TypeCheck::visitParameterNode(ParameterNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitDeclarationNode(DeclarationNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  CompoundType a;
  VariableInfo* v = new VariableInfo();
  bool classFlag = (currentLocalOffset == -1);



  //if-else to check offset (localOffset) if flag value then members
  //Local is for methods
  //members are for class
  if(classFlag) {
    //We are inserting the offset of members of class
    for(std::list<IdentifierNode*>::iterator iter = node->identifier_list->begin();
    iter != node->identifier_list->end(); iter++){

        if(node->type->basetype == bt_object && 
        !(*classTable).count(node->type->objectClassName)) {
          typeError(undefined_class);
        }

      a.baseType = node->type->basetype;
      a.objectClassName = node->type->objectClassName;
      v->size = 4;
      currentMemberOffset += v->size;
      v->offset = currentMemberOffset;
      v->type = a;


      currentVariableTable->insert(std::pair<std::string, VariableInfo> ((*iter)->name, *v));
      // delete v;
    } 
  }
  else{
    // HERE WE SHOULD INSERT METHOD VARIABLES OK SO NOW FIND THE TYPE
    for(std::list<IdentifierNode*>::iterator iter = node->identifier_list->begin();
    iter != node->identifier_list->end(); iter++){

      if(node->type->basetype == bt_object && 
      !(*classTable).count(node->type->objectClassName)) {
        typeError(undefined_class);
      }

      a.baseType = node->type->basetype;
      a.objectClassName = node->type->objectClassName;
      v->size = 4;
      currentLocalOffset -= v->size;
      v->offset = currentLocalOffset;
      v->type = a;

      currentVariableTable->insert(std::pair<std::string, VariableInfo> ((*iter)->name, *v));
      // delete v;
    } 
  }
}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  node->basetype = node->expression->basetype;
  //TODO: Must we check that basetype is bt_object
  node->objectClassName = node->expression->objectClassName;


}


bool isSubClass(TypeCheck *visit, std::string subClass, std::string superClass) {
    do {
        if (subClass == superClass) {
            return true;
        }
    } while ((subClass = visit->classTable->at(subClass).superClassName) != "");
    return false;
}

MethodInfo methodFromId(TypeCheck *visit, std::string id, std::string mClass){
  do{
    auto *methods = visit->classTable->at(mClass).methods;
    if(methods->find(id) != methods->end()){
      return (*methods)[id];
    }

  }while((mClass = visit->classTable->at(mClass).superClassName) != "");

  std::cout << "Error Method does not exist";
  exit(1);
}

bool definedClass(TypeCheck *visit,std::string mClass){
  if(visit->classTable->find(mClass) != visit->classTable->end()){
    return true;
  }
  else{
    return false;
  }
}

bool definedMember(TypeCheck *visit, std::string id, std::string mClass) {
    do {
        if (visit->classTable->at(mClass).members->find(id) != visit->classTable->at(mClass).members->end()) {
            return true;
        }
    } while ((mClass = visit->classTable->at(mClass).superClassName) != "");
    // If we don't find the member in the class or superclasses then, we return false
    return false;
}


bool definedMethod(TypeCheck *visit, std::string id, std::string mClass) {
    do {
        if (visit->classTable->at(mClass).methods->find(id) != visit->classTable->at(mClass).methods->end()) {
            return true;
        }
    } while ((mClass = visit->classTable->at(mClass).superClassName) != "");
    // No method in the current class or super classes
    return false;
}

bool definedVar(TypeCheck *visit, std::string id) {
    if (visit->currentVariableTable->find(id) != visit->currentVariableTable->end()) {
        return true;// Variable exists as local variable or parameter
    } 
    else { 
        return definedMember(visit, id, visit->currentClassName); // Check if variable exists as member 
    }
}

CompoundType memberType(TypeCheck *visit, std::string id, std::string mClass) {
    do {
        auto thisClassMembs = visit->classTable->at(mClass).members;
        if (thisClassMembs->find(id) != thisClassMembs->end()) {
            return thisClassMembs->at(id).type;
        }
    } while ((mClass = visit->classTable->at(mClass).superClassName) != "");
    CompoundType a;
    a.baseType = bt_integer;
    a.objectClassName = "lol";
    return a;
}

CompoundType varType(TypeCheck *visit, std::string id) {
    if (visit->currentVariableTable->find(id) == visit->currentVariableTable->end()) {
        return memberType(visit, id, visit->currentClassName); // Check if variable exists as member 
    } else { // Variable is a local variable or parameter
        return visit->currentVariableTable->at(id).type;
    }
}


void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  VariableInfo vi;
  ClassInfo c;
  BaseType  b, f = node->expression->basetype, test;
  CompoundType a;
  if(!currentVariableTable->count(node->identifier_1->name)) {
    typeError(undefined_variable);
  }

  if(node->identifier_2 == NULL){
    // Only need to check whether the id is of the same type
    int i = currentVariableTable->count(node->identifier_1->name);
    if(i){
      vi = (*currentVariableTable)[node->identifier_1->name];
      b = vi.type.baseType;

      if(b != f){ // b is before f...
        typeError(assignment_type_mismatch);
      }
    }
    else{
      typeError(undefined_variable);
    }
    
  }
  else{
    // Need to check if T_id.T_id is of the same type as the expression
    // Find what type.objectClassName of id1 is...
    a = varType(this, node->identifier_1->name);
    if(classTable->count(a.objectClassName)){
      //c = (*classTable)[a.objectClassName];
      // Can't just check one class... have to check superclasses too
      a = memberType(this, node->identifier_2->name, a.objectClassName);
      if( a.objectClassName == "lol") 
        typeError(undefined_member);

      test = node->expression->basetype;

      if(test != a.baseType){
        // CAUSING 22.good to break
        typeError(assignment_type_mismatch);
      }
    }
    else{
      typeError(not_object);
    }
  }
}
  
void TypeCheck::visitCallNode(CallNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  node->basetype = node->methodcall->basetype;
  node->objectClassName = node->methodcall->basetype;

}

void TypeCheck::visitIfElseNode(IfElseNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  auto e = node->expression->basetype;
  
  if(node->expression->basetype != bt_boolean){
    
    typeError(if_predicate_type_mismatch);
  }
}

void TypeCheck::visitWhileNode(WhileNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype != bt_boolean){
    typeError(while_predicate_type_mismatch);
  }
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype != bt_boolean){
    typeError(do_while_predicate_type_mismatch);
  }
  
}

void TypeCheck::visitPrintNode(PrintNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
}

void TypeCheck::visitPlusNode(PlusNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitMinusNode(MinusNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitTimesNode(TimesNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;
}

void TypeCheck::visitDivideNode(DivideNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_integer;

}

void TypeCheck::visitGreaterNode(GreaterNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;

}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;

}

void TypeCheck::visitEqualNode(EqualNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  BaseType LHS = node->expression_1->basetype;
  BaseType RHS = node->expression_2->basetype;
  if(!((LHS == bt_integer && RHS == bt_integer) || (LHS == bt_boolean && RHS == bt_boolean))) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;
  
}

void TypeCheck::visitAndNode(AndNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;
}

void TypeCheck::visitOrNode(OrNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }
  node->basetype = bt_boolean;

}

void TypeCheck::visitNotNode(NotNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype != bt_boolean) {
    typeError(expression_type_mismatch);
  }
  node->basetype = node->expression->basetype;

}

void TypeCheck::visitNegationNode(NegationNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype != bt_integer) {
    typeError(expression_type_mismatch);
  }
  node->basetype = node->expression->basetype;
  // TODO: or node->basetype = bt_integer

}

void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  std::string mClass, methodId, objId;
  MethodInfo methodInfo;
  //Type defined in MethodInfo
  if(node->identifier_2){
    // Then there is an object that this object was called on... find it in the current context...  
    objId = node->identifier_1->name;
    methodId = node->identifier_2->name;
    if(!definedVar(this, objId)){
      typeError(undefined_variable);
    }
    else if(varType(this, objId).baseType != bt_object){
      typeError(not_object);
    }
    mClass = varType(this, objId).objectClassName;
    if(!definedMethod(this, methodId, varType(this, objId).objectClassName)){
        typeError(undefined_method);
    }
  }
  else{
    // Explicit call on current class
    mClass = currentClassName;
    methodId = node->identifier_1->name;
    if (!definedMethod(this, methodId, mClass)) {
          typeError(undefined_method);
      }
  }

  methodInfo = methodFromId(this, methodId, mClass);
  auto *params = methodInfo.parameters;

  // Check if correct size
  if(params->size() != node->expression_list->size()){
    typeError(argument_number_mismatch);
  }
  
  auto paramIT = params->begin();
  auto methodCallIT = node->expression_list->begin();

  // now assume same size
  for(; paramIT != params->end(); ++paramIT, ++methodCallIT) {
        if (paramIT->baseType != (*methodCallIT)->basetype || (!isSubClass(this, (*methodCallIT)->objectClassName, paramIT->objectClassName) && (*methodCallIT)->basetype == bt_object)) {
            typeError(argument_type_mismatch);
        }
    }
  
  node->basetype = methodInfo.returnType.baseType;
  node->objectClassName = methodInfo.returnType.objectClassName;

}

void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
  // WRITEME: Replace with code if necessary
    std::string objId = node->identifier_1->name;
    std::string memId = node->identifier_2->name;
    CompoundType a;
    if (!definedVar(this, objId)) {
        typeError(undefined_variable);
    } else if (varType(this, objId).baseType != bt_object) {
        typeError(not_object);
    } else if (!definedMember(this, memId, varType(this, objId).objectClassName)) {
        typeError(undefined_member);
    }
    a = memberType(this, memId, varType(this, objId).objectClassName);
    node->basetype = a.baseType;
    node->objectClassName = a.objectClassName;
}

void TypeCheck::visitVariableNode(VariableNode* node) {
  // WRITEME: Replace with code if necessary
  std::string id = node->identifier->name;
  if(!definedVar(this, id)){
    typeError(undefined_variable);
  }
  node->basetype = varType(this, id).baseType;
  node->objectClassName = varType(this, id).objectClassName;
  //If variable is not defined as local var or param, then it must be a member of the class
  // but how do I know which currentVariable table I'm looking at?
  // if(this->currentVariableTable->find(variableName) == this->currentVariableTable->end()) {
  
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_boolean;
}

//Helper function to check polymorphism
bool classIsInherited(TypeCheck *tc, std::string inheritedClass, std::string superClass) {
  do {
    if(inheritedClass == superClass) {
      return true;
    }
  } while((inheritedClass = tc->classTable->at(inheritedClass).superClassName) != "");

  return false;
}

void TypeCheck::visitNewNode(NewNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(classTable->find(node->identifier->name) == classTable->end()) {
    typeError(undefined_class);
  }
  // node->identifier->name refers to the class name
  std::string mClass = node->identifier->name;
  bool constructorExists = false;
  //Checking if constructor is present in methodTable of class, or all super classes
  do {
    auto *methods = this->classTable->at(mClass).methods;
    if(methods->find(mClass) != methods->end()) {
      constructorExists = true;
      break;
    }
  } while((mClass = this->classTable->at(mClass).superClassName) != "");

  if(constructorExists) { // Sid said a class doesn't have to have a constructor so there's no error if it doesn't exist
    //Get constructor from methods of the class
    MethodInfo classMethods;
    auto *methods = this->classTable->at(mClass).methods;
    if(methods->find(mClass) != methods->end()) {
      classMethods = (*methods)[mClass];
    }
    else {
      std::cout << "The constructor should exist, but can't find it in methods table???\n";
    }
    auto *parameters = classMethods.parameters;
    if(node->expression_list->size() != parameters->size()) {
      typeError(argument_number_mismatch);
    }

    auto argsIter = node->expression_list->begin();
    auto paramIter = parameters->begin();
    //Cool for loop to increment and check two things at once
    for( ; paramIter != parameters->end(); ++paramIter, ++argsIter) {
      if(paramIter->baseType != (*argsIter)->basetype) {
        typeError(argument_type_mismatch);
      }
      //If they are both of type bt_object, I have to check they are referencing a valid class or super class
      else if((*argsIter)->basetype == bt_object && !classIsInherited(this, (*argsIter)->objectClassName, paramIter->objectClassName)) {
        typeError(argument_type_mismatch);
      }
    }


  }
  else { //No constructor means it must have 0 args, check for 0 args
    if(node->expression_list->size() != 0) {
      typeError(argument_number_mismatch);
    }
  }

  node->objectClassName = node->identifier->name;
  node->basetype = bt_object;
}


void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
  node->objectClassName = "";
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_boolean;
  node->objectClassName = "";
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
}

void TypeCheck::visitNoneNode(NoneNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_none;
  node->objectClassName = "";
}

void TypeCheck::visitIdentifierNode(IdentifierNode* node) {
  // c0 c1; THIS METHOD IS ABOUT c1

}

void TypeCheck::visitIntegerNode(IntegerNode* node) {
  // WRITEME: Replace with code if necessary

}


// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
  std::string string = std::string("");
  for (int i = 0; i < indent; i++)
    string += std::string(" ");
  return string;
}

std::string string(CompoundType type) {
  switch (type.baseType) {
    case bt_integer:
      return std::string("Integer");
    case bt_boolean:
      return std::string("Boolean");
    case bt_none:
      return std::string("None");
    case bt_object:
      return std::string("Object(") + type.objectClassName + std::string(")");
    default:
      return std::string("");
  }
}


void print(VariableTable variableTable, int indent) {
  std::cout << genIndent(indent) << "VariableTable {";
  if (variableTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
    std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
    if (it != --variableTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
  std::cout << genIndent(indent) << "MethodTable {";
  if (methodTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
    std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
    print(*it->second.variables, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --methodTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
  std::cout << genIndent(indent) << "ClassTable {" << std::endl;
  for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    if (it->second.superClassName != "")
      std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
    print(*it->second.members, indent + 4);
    std::cout << "," << std::endl;
    print(*it->second.methods, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --classTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
  print(classTable, 0);
}
